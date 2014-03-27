#include "OnionInterface.h"
#include "OnionPacket.h"
#include <stdio.h>
#include <stdlib.h> /* malloc, free, rand */
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>

OnionInterface::OnionInterface() {
    recvPkt = 0;
    sock = 0; // Make sure we have something to show invalid socket
    struct timeval now;
    gettimeofday(&now, NULL);
    milliStart = now.tv_sec *1000 + (now.tv_usec/1000);
}

OnionInterface::OnionInterface(char* host, uint16_t port) {
    OnionInterface();
    open(host,port);
}

int8_t OnionInterface::open(char* hostname, uint16_t port) {
    if (!connected()) {
		int sd, rc;
        struct sockaddr_in localAddr, servAddr;
        struct hostent *host;
    
        host = gethostbyname(hostname);
        if(host==NULL) {
            printf("unknown host '%s'\n",hostname);
            return 0;
        }
    
        servAddr.sin_family = host->h_addrtype;
        memcpy((char *) &servAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
        servAddr.sin_port = htons(port);
    
        /* create socket */
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if(sd<0) {
            perror("cannot open socket ");
            return 0;
        }
    
        /* bind any port number */
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(0);
    
        rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
        if(rc<0) {
            printf("%s: cannot bind port TCP %u\n",hostname,port);
            perror("error ");
            return 0;
        }
    
        /* connect to server */
        rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
        if(rc<0) {
            perror("cannot connect ");
            return 0;
        }
        sock = sd;
    
        return 1;
	}
	return 1;
}

bool OnionInterface::connected(void) {
    // this should be more intelligent
	return (sock !=0);
}


int8_t OnionInterface::send(OnionPacket* pkt) {
    int length = pkt->getBufferLength();
    int rc = ::send(sock,(uint8_t*)pkt->getBuffer(), length,0);
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

char* OnionInterface::getIPAddr(void) {
    int len = sizeof(struct sockaddr);
    struct sockaddr_in addr;
    if (getsockname(sock, (struct sockaddr *) &addr,(socklen_t*) &len) != 0) {
        return 0;
    }
    return inet_ntoa(addr.sin_addr);
}

void OnionInterface::close(void) {
    ::close(sock);
}

int OnionInterface::getMillis(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    long nowMillis = now.tv_sec *1000 + (now.tv_usec/1000);
    return nowMillis-milliStart;
}
        
        
