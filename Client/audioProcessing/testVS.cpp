
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "VoiceStreamer.h"

int BUFSIZE = 1024000;

using namespace std;

bool testCapture();
bool testPlayback();
void printUsage();

int main(int argc, char *argv[]) {
   int mode;

   if (argc < 2 || argc > 3) {
      printUsage();
      exit(EXIT_FAILURE);
   } else {
      mode = atoi(argv[1]);
      if (argc == 3)
         BUFSIZE = atoi(argv[2]);

      if (mode < 1 || mode > 2) {
         cerr << "Invalid mode: " << mode << endl;
         printUsage();
         exit(EXIT_FAILURE);
      }
   }

   switch (mode) {
   case 1:
      if (!testCapture()) {
         cout << "Audio capture failed!\n";
         exit(EXIT_FAILURE);
      } else
         cout << "Audio capture passed.\n";
      break;
   case 2:
      if (!testPlayback()) {
         cout << "Audio playback failed!\n";
         exit(EXIT_FAILURE);
      } else
         cout << "Audio playback passed.\n";
      break;
   }

   return EXIT_SUCCESS;
}

bool testPlayback() {
   VoiceStreamer *pStreamer;
   char *someAudio = new char[BUFSIZE];
   int buflen = 0, r;

   pStreamer = new VoiceStreamer();
   if (!pStreamer->initDevice()) 
      return false;

   do {
      buflen = 0;
      r = 0;

      while (buflen < BUFSIZE) {
         if ((r = read(STDIN_FILENO, someAudio+buflen, BUFSIZE-buflen)) < 0) {
            perror("read()");
            exit(EXIT_FAILURE);
         }

         buflen += r;
         if (r == 0)
            break;
      }
      
      pStreamer->playBuffer(someAudio, buflen);
   } while (r > 0);

   delete someAudio;
   return true;
}

bool testCapture() {
   VoiceStreamer *cStreamer;
   char *someAudio = new char[BUFSIZE];
   memset(someAudio, 0, BUFSIZE);
   int retval;

   cStreamer = new VoiceStreamer();
   if (!cStreamer->initDevice())
      return false;

   while (true) {
      retval = cStreamer->fillBuffer(someAudio, BUFSIZE);

      if (write(STDOUT_FILENO, someAudio, retval) < 0) {
         perror("write()");
         return false;
      }
   }

   return true;
}

void printUsage() {

   cout << "usage: [mode]\n";
   cout << "\t1: capture audio\n";
   cout << "\t2: playback audio\n";
}
