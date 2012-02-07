/** This is a test DCCP server
 *
 * @author William McVicker
 */

#include <stdlib.h>
#include <iostream>
#include <string>

#include "myChat.h"

using namespace std;

int main(int argc, char *argv[]) {
   enum PROTO_TYPE protocolType;
   enum REQUEST req;

   if (argc > 1) 
      protocolType = (enum PROTO_TYPE) atoi(argv[1]);
   else  {
      helpMenu();
      exit(EXIT_FAILURE);
   }

   // Initialize the server
   theServer = new Server(protocolType);

   printMenu();
   while (1) {
      for (;;) {
         if (select_call(STDIN_FILENO, 0, 0)) {
            if ((req = getUserInput()) != INVALID_REQ) 
               break;
         } else if (theServer->waitForClients(0)) { 
            req = INCOMING_REQ;
            break;
         }
      }

      switch (req) {
      case INCOMING_REQ:
         theServer->acceptNewCall();
         break;
      case OUTGOING_REQ:
         cout << "This feature is not yet enabled!\n";
         break;
      default:
         assert(false);
      }
   }

   return EXIT_FAILURE; // never should exit
}

/** Prints the help menu */
void helpMenu() {

   cerr << "usage: myChat [transport protocol]\n";
   cerr << "\ttransport protocol:\n";
   cerr << "\t\t0: DCCP\n";
   cerr << "\t\t1: UDP\n";
   cerr << "\t\t2: TCP\n";

}

/** Gives the user choices on what to do */
void printMenu() {

   cout << "Welcome to Audio Chatter! ))) --- (((\n";
   cout << "\nChoose one of the following options:\n";
   cout << "\t1: Call a friend\n";
   cout << "\t2: Quit\n";
   
}

enum REQUEST getUserInput() {
   int retval = 0;
   string input = "";
   enum REQUEST req = INVALID_REQ;

   getline(cin, input);
   stringstream myStream(input);
   if (myStream >> retval) {
      switch (retval) {
      case 1: // call a friend
         req = OUTGOING_REQ;
         break;
      case 2: // Quit
         delete theServer;
         exit(EXIT_SUCCESS);
      default:
         req = INVALID_REQ;
      }
   }

   if (req == INVALID_REQ)
      cerr << "Invalid option, please try again: ";

   return req;
}
