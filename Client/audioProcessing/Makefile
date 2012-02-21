# CSC 564
# -----------------------
# Makefile 
#
# 1/22/2012
# @author William McVicker

CC=g++

# You can choose one of these to set the executable size (if there's a choice).
# Select SIZE64 only when the hardware supports 64-bit executables, and then
# select it in both CFLAGS and also in LDFLAGS.   Most of the time, SIZE32 will
# be the most useful, since these will execute just fine on 64-bit machines to,
# but it doesn't work the other way.
SIZE32=-m32
SIZE64=-m64

SIZE=$(SIZE32)
CFLAGS=-g -Wall -c -Werror -O2 -pedantic $(SIZE) -O2
LDFLAGS=$(SIZE) -lasound

ALL=testVS

all: $(ALL)

testVS: testVS.o VoiceStreamer.o
	$(CC) $(LDFLAGS) -o $@ $^

testVS.o: testVS.cpp VoiceStreamer.h
	$(CC) $(CFLAGS) -c $<

VoiceStreamer.o: VoiceStreamer.cpp VoiceStreamer.h
	$(CC) $(CFLAGS) -c $<

       
clean:
	rm -f core* *.o  *.gch $(ALL)
