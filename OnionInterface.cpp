#include "OnionInterface.h"
#include "OnionPacket.h"
#include <stdio.h>
#include <stdlib.h> /* malloc, free, rand */
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

OnionInterface::OnionInterface() {
    recvPkt = 0;
    sock = 0; // Make sure we have something to show invalid socket
}

OnionInterface::OnionInterface(char* host, uint16_t port) {
    OnionInterface();
    open(host,port);
}

int8_t OnionInterface::open(char* host, uint16_t port) {
    if (!connected()) {
		int sd, rc;
        struct sockaddr_in localAddr, servAddr;
        struct hostent *host;
    
        host = gethostbyname(host);
        if(host==NULL) {
            printf("unknown host '%s'\n",host);
            return 1;
        }
    
        servAddr.sin_family = host->h_addrtype;
        memcpy((char *) &servAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
        servAddr.sin_port = htons(port);
    
        /* create socket */
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if(sd<0) {
            perror("cannot open socket ");
            return 1;
        }
    
        /* bind any port number */
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(0);
    
        rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
        if(rc<0) {
            printf("%s: cannot bind port TCP %u\n",host,port);
            perror("error ");
            return 1;
        }
    
        /* connect to server */
        rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
        if(rc<0) {
            perror("cannot connect ");
            return 1;
        }
        sock = sd;
    
        return 0;
	}
	return 1;
}

bool OnionInterface::connected(void) {
    // this should be more intelligent
	return (sock !=0);
}


int8_t OnionInterface::send(OnionPacket* pkt) {
    int length = pkt->getBufferLength();
    int rc = send(sock,(uint8_t*)pkt->getBuffer(), length);
    if (length == rc) {
        delete pkt;
        return 1;
    }
    delete pkt;
    return 0;
}

OnionPacket* OnionInterface::getPacket(void) {
    recvPkt = new OnionPacket(128);
    int16_t count = 0;
    count = recv(sock,recvPkt->getBuffer(),128,MSG_DONTWAIT);
    recvPkt->incrementPtr(count);
    if (count > 0) {
        uint8_t type = recvPkt->getType();
        if (recvPkt->isComplete()) {
            OnionPacket* pkt = recvPkt;
            recvPkt = 0;
            return pkt;
        } else {
            return 0;
        }
    } else {
        delete recvPkt;
        recvPkt = 0;
        return 0;
    }
}

void OnionInterface::close(void) {
    _client->stop();
}

