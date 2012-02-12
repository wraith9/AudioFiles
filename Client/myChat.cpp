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
   enum CLIENT_STATE state = IDLE_S;
   enum PROTO_TYPE protocolType;
   uint32_t theCaller = 0;
   uint32_t myUID = 0;

   if (argc > 1 && argc < 4) {
      protocolType = (enum PROTO_TYPE) atoi(argv[1]);
      
      if (argc == 3)
         myUID = (uint32_t) atoi(argv[2]);
      else
         myUID = 1;
   } else {
      helpMenu();
      exit(EXIT_FAILURE);
   }

   // Initialize the server
   theServer = new Server(protocolType, myUID);

   while (true) {
      switch (state) {
      case IDLE_S:
         printMenu();
         do {
            if (select_call(STDIN_FILENO, 0, 0)) {
               state = getUserInput();
            } else if (theServer->waitForRequests(0))  
               state = ANSWERING_S;
         } while (state == IDLE_S);
         
         break;
      case CALLING_S:
         if ((theCaller = whoToCall())) {
            theServer->makeCall(theCaller);
            state = CHATTING_S;
         } else 
            state = IDLE_S;

         break;
      case ANSWERING_S:
         theServer->acceptNewCall();
         
         if (theServer->chatting)
            state = CHATTING_S;
         else 
            state = IDLE_S;
         break;
      case CHATTING_S:

         while (theServer->chatting) {
            if (theServer->waitForRequests(1)) 
               theServer->acceptNewCall();
         }

         cout << "Call ended.\n";

         state = IDLE_S;
         break;
      default:
         assert(false);
      }
   }

   return EXIT_FAILURE; // never should exit
}

/** Prints the help menu */
void helpMenu() {

   cerr << "usage: myChat [transport protocol] [uid]\n";
   cerr << "\ttransport protocol:\n";
   cerr << "\t\t0: DCCP\n";
   cerr << "\t\t1: UDP\n";
   cerr << "\t\t2: TCP\n";
   cerr << "\tuid: your user id\n";

}

/** Gives the user choices on what to do */
void printMenu() {

   cout << "\nWelcome to Audio Chatter! ))) --- (((\n";
   cout << "\nChoose one of the following options:\n";
   cout << "\t1: Call a friend\n";
   cout << "\t2: Quit\n";
   
}

/** Gets the response from the user on what he/she wants to do.
 *
 * @return the next state to process the request the user chooses
 */
enum CLIENT_STATE getUserInput() {
   int retval = 0;
   string input = "";
   enum CLIENT_STATE req = IDLE_S;

   getline(cin, input);
   stringstream myStream(input);
   if (myStream >> retval) {
      switch (retval) {
      case 1: // call a friend
         req = CALLING_S;
         break;
      case 2: // Quit
         delete theServer;
         exit(EXIT_SUCCESS);
      }
   }

   if (req == IDLE_S)
      cerr << "Invalid option, please try again: ";

   return req;
}

/** Asks the user which friend to call.  
 *
 * @return the uid of the friend to call, 0 when the user doesn't want to
 * call anyone.
 */
uint32_t whoToCall() {
   string input = "";
   uint32_t uid;
   char firstChar;

   cout << "Who would you like to call?\n";
   printFriendList();

   while (true) {   
      getline(cin, input);
      try {
         firstChar = input.at(0);
      } catch (out_of_range& e) {
         continue;
      }

      if (firstChar == 'X' || firstChar == 'x')
         return 0;
      else {
         stringstream myStream(input);
         if ((myStream >> uid) && verifyId(uid)) 
            break;
         else 
            cerr << "Invalid option, try again: ";
      }
   } 

   return uid;
}

/** Prints the user's friends list as a menu */
void printFriendList() {

   cout << "\t1: Friend1\n";
   cout << "\t2: Friend2\n";
   cout << "\t3: Friend3\n";

   cout << "\tX: Return to main menu\n";
}

/** Checks the friend database to verify the provided
 * user id is valid.
 *
 * @param uid the user id of interest
 * @return true if the user is friends with uid, else false
 */
bool verifyId(uint32_t uid) {

   // check friends database to verify this uid is valid

   if (uid > 0 && uid < 4)
      return true;
   else
      return false;

}
