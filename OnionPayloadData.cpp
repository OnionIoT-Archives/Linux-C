#include "OnionPayloadData.h"
#include "msgpack_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Public Functions
OnionPayloadData::OnionPayloadData() {
    // Blank object
}

OnionPayloadData::OnionPayloadData(OnionPacket* pkt) {
    this->init(pkt,0);
}

OnionPayloadData::OnionPayloadData(OnionPacket* pkt,uint16_t offset) {
    this->init(pkt,offset);
}

OnionPayloadData::~OnionPayloadData() {
    // Free any malloc'd or new'd data in this object
    if (dataObjectArray != 0) {
        for (uint16_t x = 0;x<length;x++) {
            delete dataObjectArray[x];
        }
    }
    
    if (data != 0) {
        free(data);
    }
    
}

uint16_t OnionPayloadData::getRawLength() {
    return this->rawLength;
}

void OnionPayloadData::init(OnionPacket* pkt,uint16_t offset) {
    // Initialize internal variables
    this->type = 0;
    this->length = 0;
    this->data = 0;
    this->dataObjectArray = 0;
    this->pkt = pkt;
    this->offset = offset;
    this->rawBuffer = (pkt->getPayload()) + offset;
    this->rawLength = pkt->getPayloadLength() - offset;
    this->dataIsObject = false;
    //printf("->init: pkt->length = %d , offset = %d, rawLength = %d\n",pkt->getLength(), offset,this->rawLength);
}

// Call unpack to try and unpack the data into link list of payload objects
// returns the count of bytes used to unpack
int8_t OnionPayloadData::unpack(void) {
    //printf("->unpack: rawLength = %d\n",this->rawLength);
    if (rawLength == 0) {
        printf("->unpack: kicked out b/c no rawLength\n");
        return 0;
    }
    int bytesParsed = 0;
    // If we have data then assume length is 1 until we parse an actual value
    length = 1;
    bytesParsed++;  // Add one to parsed bytes since all formats have at least 1 byte
    uint8_t rawType = rawBuffer[0];
    //printf("->unpack: rawType = %02X\n",rawType);
    if (((rawType & 0x80) == 0) || ((rawType & 0xE0) == 0xE0)) {
        type = MSGPACK_FIXINT_HEAD;
        data = calloc(1,sizeof(int));
        int *ptr = (int*) data;
        *ptr = rawType;
        //printf("->unpack: found fixint type, value = %d\n",*ptr);
    } else if ((rawType & 0xF0) == MSGPACK_FIXMAP_HEAD) {
        type = MSGPACK_FIXMAP_HEAD;
        length = rawType & 0x0F;
        
        // ************************** THIS NEEDS UPDATING *********************
        // Need to double length of data array for map pairs, and need to handle
        // it accordingly in the destructor (probably need another length or map bool
        dataObjectArray = new OnionPayloadData*[length];
        dataIsObject = true;
        for (int x=0;x<length;x++) {
            dataObjectArray[x] = new OnionPayloadData(pkt,offset+bytesParsed); // Begining of sub object is current offset + bytes parsed
            bytesParsed += dataObjectArray[x]->unpack();
        }
    } else if ((rawType & 0xF0) == MSGPACK_FIXARRAY_HEAD) {
        type = MSGPACK_FIXARRAY_HEAD;
        length = rawType & 0x0F;
        dataObjectArray = new OnionPayloadData*[length];
        dataIsObject = true;
        for (int x=0;x<length;x++) {
            //printf("->unpack: create a new object with offset=%d & bytesParsed=%d\n",offset,bytesParsed);
            dataObjectArray[x] = new OnionPayloadData(pkt,offset+bytesParsed); // Begining of sub object is current offset + bytes parsed
            bytesParsed += dataObjectArray[x]->unpack();
        }
    } else if ((rawType & 0xE0) == MSGPACK_FIXSTR_HEAD) {
        type = MSGPACK_FIXSTR_HEAD;
        length = rawType & 0x1F;
        data = new char[length+1];
        char* ptr = (char*) data;
        memcpy(ptr,rawBuffer+1,length);
        // Do I really need to add this null? probably, but may not be necessary
        ptr[length] = 0;
        bytesParsed += length;
    } else {
        type = rawType;
        switch (type) {
            case MSGPACK_NIL_HEAD: {
                data = 0;
                break;
            }   
            case MSGPACK_FALSE_HEAD: {
                bool* ptr = (bool*) data;
                *ptr = false;
                break;
            }   
            case MSGPACK_TRUE_HEAD: {
                bool* ptr = (bool*) data;
                *ptr = true;
                break;
            }   
            case MSGPACK_BIN8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_BIN16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_BIN32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_EXT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_EXT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_EXT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FLOAT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FLOAT64_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_UINT64_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_INT64_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT1_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT2_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT4_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_FIXEXT16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_STR8_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_STR16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_STR32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_ARRAY16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_ARRAY32_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_MAP16_HEAD: {
                // Not Implemented yet
                break;
            }   
            case MSGPACK_MAP32_HEAD: {
                // Not Implemented yet
                break;
            }   
        }
    }
    return bytesParsed;
}

// getItem(item) will return an item from an array or map so you can dig into the data structure.
// item is 0 based index of the array, i should be less than Length (from getLength() below)
OnionPayloadData* OnionPayloadData::getItem(uint16_t i) {
    if (length == 0) {
        return 0;
    } else if (i<length) {
        return dataObjectArray[i];
    }
}

// getType() will return the type of this object
uint8_t OnionPayloadData::getType(void) {
    return this->type;
}

// getLength() will return the number of items in an array or map, will return 1 for other data
// and 0 for any data that could not be parsed correctly.
uint16_t OnionPayloadData::getLength(void) {
    return this->length;
}

// getBuffer() will return a uint8_t* pointer to the data parse in the object. Useful for
// getting a pointer to a string/binary/raw data type
uint8_t* OnionPayloadData::getBuffer(void) {
    uint8_t *ptr = (uint8_t*) data;
    return ptr;
}

// ???  Should I add a getString to make sure the data has a null terminated string? Might be nice to be able to
// ???  pass an output directly to str functions
// getInt() will return the raw parse data as an int.  If the type is not int it will return 0
int16_t OnionPayloadData::getInt(void) {
    if (data != 0) {
        int *ptr = (int*) data;
        return *ptr;
    } else {
        return -1;
    }
}

// getBool() will return the parsed data of a bool, or false if it is another type
bool OnionPayloadData::getBool(void) {
    bool *ptr = (bool*) data;
    return *ptr;
}


// Protected Functions
void OnionPayloadData::unpackArray(void) {
    
}

void OnionPayloadData::unpackMap(void) {
    
}

void OnionPayloadData::unpackInt(void) {
    
}

void OnionPayloadData::unpackStr(void) {
    
}

void OnionPayloadData::unpackNil(void) {
    
}

void OnionPayloadData::unpackBool(void) {
    
}
