/** This file contains the update daemon code.  The update daemon is
 * used to update the friends list with updates received from the server.
 *
 * @author William McVicker
 */

#include "Client.h"
#include <iostream>
#include <string.h>

using namespace std;

/** This is the thread function for the update daemon.  The update daemon
 * updates the friends list with updates received form the main server.
 */
void Client::theUpdateDaemon() {
   packet updatePacket;
   int retval;

#ifdef DEBUG
   cout << "Update Daemon started :)\n";
#endif

   while (true) {

      // wait for input data
      while (!mainServer->waitForResponse(30)) 
         ;

      // get the packet
      memset((void *) &updatePacket, 0, sizeof(packet));
      retval = mainServer->recvPacket((void *) &updatePacket,
            sizeof(packet), 0);
      if (retval < 0) {
         perror("updateDaemon: recvPacket()");
         continue;
      } else if (retval == 0) { // connection has been shutdown
         cout << "Update Daemon exitting.\n";
         return;
      }

      if (updatePacket.type != STATUS_UPDATES_T) {
#ifdef DEBUG
         cout << "Update Daemon: received a non-status update packet :(\n";
#endif
         continue;
      }

#ifdef DEBUG
      cout << "UpdateDaemon: received a status update :)\n";
#endif

      updateFormat *theData = (updateFormat *) updatePacket.data;
      map<uint32_t, friend_list>::iterator it;
      for (int i = 0; 
            i < (int) (updatePacket.dlength/sizeof(updateFormat)); i++) {
         if ((it = myFriends.find(theData[i].friendID)) == myFriends.end()) {
#ifdef DEBUG
            cerr << "Update Daemon: invalid friend id in status update!\n";
#endif
            continue;
         }
         (*it).second.status = theData[i].status;
      }
   }
}
