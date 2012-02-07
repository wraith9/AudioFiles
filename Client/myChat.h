/** This the main chat application's header
 *
 * @author William McVicker
 */

#ifndef MYCHAT_H
#define MYCHAT_H

#include "Server.h"


// Server stuff
void printMenu(); 
void helpMenu();
enum REQUEST getUserInput();

Server *theServer;

#endif
