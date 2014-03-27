###### Onion Client Library ######
## 
##
##

C = gcc
CC = g++
CFLAGS = -c
C_EXE = cTest
CPP_EXE = cppTest

all: cppTest cTest

clean:
	rm -rf *.o $(EXE)
	rm -rf libOnionClient.a $(C_EXE) $(CPP_EXE)

cppTest: testCpp.o libOnionClient.a
	$(CC) -o $(CPP_EXE) testCpp.o libOnionClient.a

cTest: test.o libOnionClient.a
	$(CC) -o $(C_EXE) test.o libOnionClient.a

testCpp.o: test.cpp
	$(CC) $(CFLAGS) -o testCpp.o test.cpp

test.o: test.c
	$(C) $(CFLAGS) test.c

libOnionClient.a: OnionClient.o OnionParams.o OnionPacket.o OnionPayloadData.o OnionPayloadPacker.o OnionInterface.o
	ar rcs libOnionClient.a OnionClient.o OnionParams.o OnionPacket.o OnionPayloadData.o OnionPayloadPacker.o OnionInterface.o

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
