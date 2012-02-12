/** This the main chat application's header
 *
 * @author William McVicker
 */

#ifndef MYCHAT_H
#define MYCHAT_H

#include "Server.h"

enum CLIENT_STATE {
   IDLE_S,
   CALLING_S,
   ANSWERING_S,
   CHATTING_S,
};
   
// Server stuff
void printMenu(); 
void helpMenu();
enum CLIENT_STATE getUserInput();

void printFriendList();
uint32_t whoToCall();
bool verifyId(uint32_t uid);

Server *theServer;

#endif
