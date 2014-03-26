###### Onion Client Library ######
## 
##
##


CC = g++
CFLAGS = -c -Wall
EXE = testExe

all: testApp

clean:
	rm -rf *.o $(EXE)

testApp: test.o OnionClient.a
	$(CC) -o $(EXE) test.o OnionClient.a

test.o: test.cpp
	$(CC) $(CFLAGS) test.cpp

OnionClient.a: OnionClient.o OnionParams.o OnionPacket.o OnionPayloadData.o OnionPayloadPacker.o OnionInterface.o
	ar rcs OnionClient.a OnionClient.o OnionParams.o OnionPacket.o OnionPayloadData.o OnionPayloadPacker.o OnionInterface.o

OnionClient.o: OnionClient.cpp
	$(CC) $(CFLAGS) OnionClient.cpp

OnionParams.o: OnionParams.cpp
	$(CC) $(CFLAGS) OnionParams.cpp
    
OnionPacket.o: OnionPacket.cpp
	$(CC) $(CFLAGS) OnionPacket.cpp
    
OnionPayloadData.o: OnionPayloadData.cpp
	$(CC) $(CFLAGS) OnionPayloadData.cpp
    
OnionPayloadPacker.o: OnionPayloadPacker.cpp
	$(CC) $(CFLAGS) OnionPayloadPacker.cpp
    
OnionInterface.o: OnionInterface.cpp
	$(CC) $(CFLAGS) OnionInterface.cpp
