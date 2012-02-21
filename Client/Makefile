# Makefile for DCCP Chat Application

CC = g++
CFLAGS = -g -Wall -c -Werror -lsocket -lnsl -O2
LIBS = -lboost_thread-mt -lpthread -lasound

ALL = audioProcessing/VoiceStreamer.o myChat 

all: $(ALL)

myChat: myChat.o Client.o updateDaemon.o TransProtocol.o DCCP.o TCP.o UDP.o util.o VoiceStreamer.o
	$(CC) -o $@ $^ $(LIBS) 

myChat.o: myChat.cpp myChat.h Client.h
	$(CC) $(CFLAGS) -c $< $(LIBS)

Client.o: Client.cpp Client.h common.h TransProtocol.h DCCP.h TCP.h UDP.h audioProcessing/VoiceStreamer.h
	$(CC) $(CFLAGS) -c $< $(LIBS)

updateDaemon.o: updateDaemon.cpp Client.h
	$(CC) $(CFLAGS) -c $< $(LIBS)

TransProtocol.o: TransProtocol.cpp TransProtocol.h common.h
	$(CC) $(CFLAGS) -c $< $(LIBS)

DCCP.o: DCCP.cpp DCCP.h TransProtocol.h common.h
	$(CC) $(CFLAGS) -c $< $(LIBS)

TCP.o: TCP.cpp TCP.h TransProtocol.h common.h
	$(CC) $(CFLAGS) -c $< $(LIBS)

UDP.o: UDP.cpp UDP.h TransProtocol.h common.h
	$(CC) $(CFLAGS) -c $< $(LIBS)

client: client.o util.o
	$(CC) -o $@ $^

client.o: client.cpp
	$(CC) $(CFLAGS) -c $<

util.o: util.c
	$(CC) $(CFLAGS) -c $<

audioProcessing/VoiceStreamer.o: audioProcessing/VoiceStreamer.cpp audioProcessing/VoiceStreamer.h
	$(CC) $(CFLAGS) -c $< 

clean: 
	rm -f *.o $(ALL)
