/** This is a Voice chat application used to test different network transport
 * protocols.  More specifically we are wanting to test DCCP and compare it
 * to UDP and TCP as far as congestion control goes.
 *
 * @author William McVicker
 */

#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>

#include <sstream>

#include "myChat.h"

using namespace std;

bool audioToFile = false;

int main(int argc, char *argv[]) {
   enum PROTO_TYPE protocolType;
   char *serverHostname;

   if ((argc == 3) || (argc == 4)) {
      protocolType = (enum PROTO_TYPE) atoi(argv[2]);
      serverHostname = strdup(argv[1]);

      if (argc == 4)
         audioToFile = true;
   } else {
      helpMenu();
      exit(EXIT_FAILURE);
   }

   // Initialize the client
   theClient = new Client(serverHostname, protocolType);

   loginScreen();

   startStateMachine();

   return EXIT_FAILURE; // never should exit
}

/** The main loop that handles the behavior of this application.  The structure
 * is built as a state machine.
 */
void startStateMachine() {
   enum CLIENT_STATE state = IDLE_S;
   uint32_t theCaller = 0;

   while (true) {
      switch (state) {
      case IDLE_S:
         printMenu();
         do {
            int theFD;
            if ((theFD = theClient->waitForRequestsOrInput(10))) {
               if ((theFD-1) == STDIN_FILENO)
                  state = getUserInput();
               else 
                  state = ANSWERING_S;
            }
         } while (state == IDLE_S);
         break;
      case CALLING_S:
         if ((theCaller = whoToCall())) {
            theClient->makeCall(theCaller);
            state = CHATTING_S;
         } else
            state = IDLE_S;

         break;
      case ANSWERING_S:
         theClient->acceptNewCall();

         if (theClient->chatting)
            state = CHATTING_S;
         else
            state = IDLE_S;
         break;
      case CHATTING_S:
         while (theClient->chatting) {
            if (theClient->waitForRequests(1))
               theClient->acceptNewCall();
         }

         state = IDLE_S;
         break;
      default:
         assert(false);
      }
   }

}

void loginScreen() {
   login_data loginData;
   string username((size_t) USERNAME_LEN, '\0');
   string password((size_t) PASSWORD_LEN, '\0');

   memset((void *) &loginData, 0, USERNAME_LEN + PASSWORD_LEN);

   for (int i = 0; i < 5; i++) {
      while (true) {
         cout << "username: ";
         try {
            getline(cin, username);
            break;
         } catch (ios_base::failure e) {
            cerr << "invalid username: maximum of 20 characters allowed.\n";
         }
      }

      while (true) {
         cout << "password: ";
         try {
            getline(cin, password);
            break;
         } catch (ios_base::failure e) {
            cerr << "invalid password: maximum of 20 characters allowed.\n";
         }
      }

      strncpy(loginData.username, username.c_str(), USERNAME_LEN);
      strncpy(loginData.password, password.c_str(), PASSWORD_LEN);

      if (theClient->loginToServer(loginData))
         return;
   }

   cout << "Login failed\n";
   exit(EXIT_FAILURE);
}

/** Prints the help menu */
void helpMenu() {

   cerr << "usage: myChat [server hostname] [transport protocol]\n";
   cerr << "\ttransport protocol:\n";
   cerr << "\t\tserver hostname: the hostname of the server\n";
   cerr << "\t\t0: DCCP\n";
   cerr << "\t\t1: UDP\n";
   cerr << "\t\t2: TCP\n";

}

/** Gives the user choices on what to do */
void printMenu() {

   cout << "\nWelcome " << theClient->whoAmI();
   cout << " to Audio Chatter! ))) --- (((\n";

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
         delete theClient;
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
   theClient->printFriendList();
   cout << "\tX: Return to main menu\n";

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
         if ((myStream >> uid) && theClient->verifyId(uid))
            break;
         else
            cerr << "Invalid option, try again: ";
      }
   }

   return uid;
}
