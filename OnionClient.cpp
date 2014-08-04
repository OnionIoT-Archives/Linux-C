#include "OnionClient.h"
#include "OnionPacket.h"
#include "OnionPayloadData.h"
#include "OnionPayloadPacker.h"
#include "OnionInterface.h"
#include "msgpack_types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char OnionClient::domain[] = "ecovent.onion.io";
uint16_t OnionClient::port = 2721;

// A paired list of name value pairs to be published when device connects
//static char* publishMap[] = {"ipAddr","192.168.137.1","mac","deadbeef"};
static uint16_t publishLength = 2; // This is the number of pairs in the map
//static struct onionClientData = {0};


/*** Interface for C ***/

OnionClient *client;

void onion_init(char* deviceId, char* device_type, char* firmware_version, char* hardware_version) {
    client = new OnionClient(deviceId);
    client->setMetaData(device_type, firmware_version, hardware_version);
}

void onion_begin() {
    client->begin();
}

uint16_t onion_declare(char* id, remoteFunction function, char** params, uint8_t param_count) {
    return client->declare(id, function, params, param_count);
}

bool onion_publish(char* key, char* value) {
	return client->publish(key, value);
}

bool onion_publish_map(char** dataMap, uint8_t count) {
	return client->publish(dataMap, count);
}

void onion_loop(void) {
    client->loop();
}


/*** Interface for C++ ***/

OnionClient::OnionClient(char* deviceId) {
    FILE* deviceKeyFile;
    long length;

    if (deviceKeyFile = fopen(ONION_DEVICE_KEY_LOCATION, "r")) {
        fseek(deviceKeyFile, 0, SEEK_END);
        length = ftell(deviceKeyFile);
        fseek(deviceKeyFile, 0, SEEK_SET);
        this->deviceKey = (char*) malloc(length);
        fscanf(deviceKeyFile, "%s", this->deviceKey);
        fclose(deviceKeyFile);
    } else {
        this->deviceKey = NULL;
    }

	this->deviceId = new char[strlen(deviceId) + 1]();
	strcpy(this->deviceId, deviceId);
	
	this->isOnline = false;
	this->metaDataLength = 0;
	this->metaDataMap = 0;
	this->remoteFunctions = new remoteFunction[1];
	this->remoteFunctions[0] = NULL;
	this->totalFunctions = 1;
	this->lastSubscription = NULL;
	totalSubscriptions = 0;
	this->interface = new OnionInterface();
}

OnionClient::~OnionClient() {
    if (this->deviceId) delete[] this->deviceId;
    if (this->deviceKey) delete[] this->deviceKey;
}

void OnionClient::begin() {
    // Device has been registered
    if (deviceKey) {        
        printf("DEVICE_KEY found.\n");

        if (connect(deviceKey)) {
            subscribe();
        }
    } 

    // Register device for the first time
    else {
        printf("No DEVICE_KEY found.\n");

        char* mfrKey;
        FILE* mfrKeyFile;
        long length;

        if (mfrKeyFile = fopen(ONION_MFR_KEY_LOCATION, "r")) {
            fseek(mfrKeyFile, 0, SEEK_END);
            length = ftell(mfrKeyFile);
            fseek(mfrKeyFile, 0, SEEK_SET);
            mfrKey = new char[length];
            fscanf(mfrKeyFile, "%s", mfrKey);
            fclose(mfrKeyFile);

            if (connect(mfrKey, deviceId)) {
                subscribe();
            }

            delete[] mfrKey;

        } else {
            printf("Error: MFR_KEY not found.\n");
            logError(6);
            exit(EXIT_FAILURE);
        }
    }
}

void OnionClient::setMetaData(char* device_type, char* firmware_version, char* hardware_version) {
    if (metaDataMap != 0) {
        delete metaDataMap[1];
        delete metaDataMap[3];
        delete metaDataMap[5];
        metaDataMap[1] = new char[strlen(device_type)];
        strcpy(metaDataMap[1],device_type);
        metaDataMap[3] = new char[strlen(firmware_version)];
        strcpy(metaDataMap[1],firmware_version);
        metaDataMap[5] = new char[strlen(hardware_version)];
    } else {
        metaDataMap = new char*[6];
        metaDataMap[0] = "device_type";
        metaDataMap[1] = new char[strlen(device_type)];
        strcpy(metaDataMap[1],device_type);
        metaDataMap[2] = "firmware_version";
        metaDataMap[3] = new char[strlen(firmware_version)];
        strcpy(metaDataMap[3],firmware_version);
        metaDataMap[4] = "hardware_version";
        metaDataMap[5] = new char[strlen(hardware_version)];
        strcpy(metaDataMap[5],hardware_version);
        metaDataLength = 3;
    }        
}

// PRE-REGISTER
bool OnionClient::connect(char* mfrKey, char* deviceId) {
    if (interface == 0) {
        return false;
    }

	if (!interface->connected()) {
		if (interface->open(OnionClient::domain, OnionClient::port)) {

            OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
            pkt->setType(ONION_CONNECT);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
            pack->packArray(3);
            pack->packInt(ONION_PROTOCOL_VERSION);
            pack->packStr(mfrKey);
            pack->packStr(deviceId);
            interface->send(pkt);

            delete pack;
			lastInActivity = lastOutActivity = interface->getMillis();

            OnionPacket *recv_pkt = interface->getPacket();
			while (recv_pkt == 0) {
				unsigned long t = interface->getMillis();
				if (t - lastInActivity > ONION_KEEPALIVE * 1000UL) {
					interface->close();
					return false;
				}
			    recv_pkt = interface->getPacket();
			}

			uint8_t pkt_type = recv_pkt->getType();

			if (pkt_type == ONION_CONNACK) {
                uint16_t length = recv_pkt->getBufferLength();
                uint8_t *ptr = recv_pkt->getBuffer();
                OnionPayloadData* data = new OnionPayloadData(recv_pkt);
                data->unpack();

                OnionPayloadData* deviceKeyData = data->getItem(0);
                if (deviceKeyData == 0) {
                    // report an error and move on
                    //printf("Failed to get device key, got %d instead\n",data->getInt());
                    logError(data->getInt());
                    return false;
                }
                
                this->deviceKey = new char[deviceKeyData->getLength() + 1]();
                strcpy(this->deviceKey, (char*)deviceKeyData->getBuffer());

                delete data;

                FILE* deviceKeyFile;
                if (deviceKeyFile = fopen(ONION_DEVICE_KEY_LOCATION, "w")) {
                    fputs(this->deviceKey, deviceKeyFile);
                    fclose(deviceKeyFile);
                } else {
                    printf("Error: Cannot create DEVICE_KEY file.\n");
                    logError(6);
                    exit(EXIT_FAILURE);
                }

				lastInActivity = interface->getMillis();
				pingOutstanding = false;
                printf("Connected to server...\n");
				return true;
			}

			delete recv_pkt;
		} else {
            interface->close();
        }
	} else {
        printf("Already connected to server...\n");
        return true;
    }
}


// POST-REGISTER
bool OnionClient::connect(char* deviceKey) {
    if (interface == 0) {
        return false;
    }

    if (!interface->connected()) {

        if (interface->open(OnionClient::domain, OnionClient::port)) {
            OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
            pkt->setType(ONION_CONNECT);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
            pack->packArray(2);
            pack->packInt(ONION_PROTOCOL_VERSION);
            pack->packStr(deviceKey);
            interface->send(pkt);

            lastInActivity = lastOutActivity = interface->getMillis();
            delete pack;

            OnionPacket *recv_pkt = interface->getPacket();
            while (recv_pkt == 0) {
                unsigned long t = interface->getMillis();
                if (t - lastInActivity > ONION_KEEPALIVE * 1000UL) {
                    interface->close();
                    return false;
                }
                recv_pkt = interface->getPacket();
            }

            uint8_t pkt_type = recv_pkt->getType();
            uint16_t length = recv_pkt->getPayloadLength();
            uint8_t* payload = recv_pkt->getPayload();

            if ((pkt_type == ONION_CONNACK) && (length > 0)) {
                if (payload[0] == 0) {
                    lastInActivity = interface->getMillis();
                    pingOutstanding = false;
                    delete recv_pkt;

                    printf("Connected to server...\n");
                    return true;
                } else {
                    logError(payload[0]);
                }
            }
        } else {

            interface->close();

        }
    } else {
        printf("Already connected to server...\n");
        return true;
    }
}


uint16_t OnionClient::declare(char* id, remoteFunction function, char** params, uint8_t param_count) {
	remoteFunction* resized = new remoteFunction[totalFunctions + 1];
	
    if (lastSubscription == NULL) {
	    subscriptions.index = totalFunctions;
	    subscriptions.id = id;
	    subscriptions.params = params;
	    subscriptions.param_count = param_count;
	    lastSubscription = &subscriptions;
	} else {
	    subscription_t* new_sub = (subscription_t*)malloc(sizeof(subscription_t));
	    new_sub->index = totalFunctions;
	    new_sub->id = id;
	    new_sub->params = params;
	    new_sub->param_count = param_count;
	    new_sub->next = NULL;
	    lastSubscription->next = new_sub;
	    lastSubscription = new_sub; 
	}

	totalSubscriptions++;
	
	for (int i = 0; i < totalFunctions; i++) {
		resized[i] = remoteFunctions[i];
	}
	
	// Set the last element of resized as the new function
	resized[totalFunctions] = function;
	
	delete[] remoteFunctions;
	remoteFunctions = resized;
	
	return totalFunctions++;
}

void OnionClient::declare(char* id, char* value) {

}

bool OnionClient::publish(char* key, char* value) {
	int key_len = strlen(key);
	int value_len = strlen(value);
	if (interface->connected()) {
        OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
        pkt->setType(ONION_PUBLISH);
        OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
        pack->packMap(1);
        pack->packStr(key);
        pack->packStr(value);
        
	    interface->send(pkt);
        delete pack;
	}
	return false;
}

bool OnionClient::publish(char** dataMap, uint8_t count) {
    OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
    pkt->setType(ONION_PUBLISH);
    OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
    pack->packMap(count);
    for (uint8_t x=0; x<count; x++) {
        pack->packStr(*dataMap++);
        pack->packStr(*dataMap++);
    }
    
    interface->send(pkt);
    delete pack;
}

bool OnionClient::subscribe() {
	if (interface->connected()) {
	    // Generate 
	    if (totalSubscriptions > 0) {
            OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
            pkt->setType(ONION_SUBSCRIBE);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
	        subscription_t *sub_ptr = &subscriptions;
	        pack->packArray(totalSubscriptions);
        	//uint8_t string_len = 0;

        	uint8_t param_count = 0;
	        for (uint8_t i = 0; i < totalSubscriptions; i++) {
	            param_count = sub_ptr->param_count;
	            pack->packArray(param_count + 2);
	            pack->packStr(sub_ptr->id);
	            pack->packInt(sub_ptr->index);
	            for (uint8_t j = 0; j < param_count; j++) {
	                pack->packStr(sub_ptr->params[j]);
	            }
	            sub_ptr = sub_ptr->next;
	        }

	        // Send subscribe packet
	        interface->send(pkt);
	        // Since the packet is generated and sent we can delete the packer
	        delete pack;
	        return true;
	    }
	    
	}
	return false;
}

bool OnionClient::loop() {
	if (interface->connected()) {
		unsigned long t = interface->getMillis();
		if ((t - lastInActivity > ONION_KEEPALIVE * 1000UL) || (t - lastOutActivity > ONION_KEEPALIVE * 1000UL)) {
			if (pingOutstanding) {
				interface->close();
				return false;
			} else {
			    sendPingRequest();
				lastOutActivity = t;
				lastInActivity = t;
				pingOutstanding = true;
			}
		}

        OnionPacket* pkt = interface->getPacket();

		if (pkt != 0) {
			lastInActivity = t;
			uint8_t type = pkt->getType();
			if (type == ONION_CONNACK) {
				if(parseConnectionAck(pkt)){
				    subscribe();
				} else {
					interface->close();
					delete pkt;
					return false;
				}

		    } else if (type == ONION_SUBACK) {
        	    //Serial.print("Publishing Data\n");
        		//publish("/onion","isAwesome");
				isOnline = true;
				publish(metaDataMap,metaDataLength);
				lastOutActivity = t;
			} else if (type == ONION_PUBLISH) {
			    parsePublishData(pkt);
			} else if (type == ONION_PINGREQ) {
				sendPingResponse();
				lastOutActivity = t;
			} else if (type == ONION_PINGRESP) {
				pingOutstanding = false;
			} 
			delete pkt;
		}
		return true;

	} else {
	    unsigned long t = interface->getMillis();
		if (t - lastOutActivity > ONION_KEEPALIVE * 1000UL) {
		    this->begin();
		}
	}
}

bool OnionClient::parseConnectionAck(OnionPacket* pkt) {
    uint16_t length = pkt->getBufferLength();
    uint8_t *ptr = pkt->getBuffer();
    OnionPayloadData* data = new OnionPayloadData(pkt);
    data->unpack();
    if (data->getType() == MSGPACK_FIXINT_HEAD) {
        // This is a device connection response, or error.
        uint16_t response = data->getInt();
        if (response == 0) {
            delete data;
            return true;
        } else {
            logError(response);
            delete data;
            return false;
        }
    } else {
        // This is a response with the new device key
        
    }
    
}

void OnionClient::sendPingRequest(void) {
    OnionPacket* pkt = new OnionPacket(8);
    pkt->setType(ONION_PINGREQ);
    interface->send(pkt);
}

void OnionClient::sendPingResponse(void) {
    OnionPacket* pkt = new OnionPacket(8);
    pkt->setType(ONION_PINGRESP);
    interface->send(pkt);
}

void OnionClient::parsePublishData(OnionPacket* pkt) {
    uint16_t length = pkt->getBufferLength();
    uint8_t *ptr = pkt->getBuffer();
    OnionPayloadData* data = new OnionPayloadData(pkt);
    
    data->unpack();
    uint8_t count = data->getLength();
    uint8_t function_id = data->getItem(0)->getInt();

    printf("data length: %i, function ID: %i;\n", count, function_id);

    // OnionParams* params = new OnionParams(count-1);
    char** params = 0;
    if (count > 1) {
        params = new char*[count - 1];
	    // Get parameters
	    for (uint8_t i = 0; i < (count - 1); i++) {
	        OnionPayloadData* item = data->getItem(i + 1);
	        uint8_t strLen = item->getLength();
	        // Test
	        params[i] = (char*)(item->getBuffer());
	        //params->setStr(i,buf_ptr,strLen);
	    }
	}

	if (function_id < totalFunctions) {
	    if (remoteFunctions[function_id] != 0) {
	        char* returnedVal;
            returnedVal = remoteFunctions[function_id](params);
            int length = strlen(returnedVal);

            if (interface->connected()) {
                OnionPacket* pkt = new OnionPacket(ONION_MAX_PACKET_SIZE);
                pkt->setType(ONION_PUBACK);
                OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
                //pack->packArray(2);
                pack->packStr(returnedVal);
                interface->send(pkt);

                lastInActivity = lastOutActivity = interface->getMillis();
                delete pack;
            }
	    }
	} 
	if (params != 0) {
	    delete[] params;   
	}
	delete data;
}

void OnionClient::logError(uint8_t errorCode) {
    printf("Error Code %i: ", errorCode);

    if (errorCode == 1) {
        printf("Unacceptable protocol version.\n");
    } else if (errorCode == 2) {
        printf("Identifier rejected.\n");
    } else if (errorCode == 3) {
        printf("Server error.\n");
    } else if (errorCode == 4) {
        printf("Authentication failed.\n");
    } else if (errorCode == 5) {
        printf("Not authorized.\n");
    } else if (errorCode == 6) {
        printf("Device error.\n");
    }
}
