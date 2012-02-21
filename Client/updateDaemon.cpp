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
 *
 */
void Client::theUpdateDaemon() {
   packet thePacket;
   int retval;

#ifdef DEBUG
   cout << "Update Daemon started :)\n";
#endif

   if (!initDaemon())
      return;

   while (true) {
      // wait for input data
      while (!daemonProtocol->waitForResponse(30))
         ;

      // get the packet
      memset((void *) &thePacket, 0, sizeof(packet));
      retval = daemonProtocol->recvPacket((void *) &thePacket,
            sizeof(packet), 0);
      if (retval < 0) {
         perror("updateDaemon: recvPacket()");
         SAFE_EXIT(EXIT_FAILURE);
      } else if (retval == 0) { // connection has been shutdown
         cout << "Update Daemon exiting.\n";
         return;
      }

      if (thePacket.type != STATUS_UPDATES_T) {
#ifdef DEBUG
         cout << "Update Daemon: received a non-status update packet: ";
         fprintf(stderr, "0x%hhx\n", thePacket.type);
#endif
         continue;
      }

      // Update the friends' statuses
      updateFormat *theData = (updateFormat *) thePacket.data;
      map<uint32_t, friend_list>::iterator it;
      int numFriends = ntohs(thePacket.dlength)/sizeof(updateFormat);
      for (int i = 0; i < numFriends; i++) {
         uint32_t fID = ntohl(theData[i].friendID);

         if ((it = myFriends.find(fID)) == myFriends.end()) {
#ifdef DEBUG
            cerr << "Update Daemon: invalid friend id in status update!\n";
#endif
            continue;
         }
         if ((*it).second.status != theData[i].status) {
            if (theData[i].status)
               cout << myFriends[fID].friendData.username << " is online\n";
            else
               cout << myFriends[fID].friendData.username <<" is now offline\n";
         }
         (*it).second.status = theData[i].status; 
      }
   }
}

/** Opens a new connection with the server to receive status updates
 * of the user's friends.
 *
 * @return true if successfully, else false
 * @*/
bool Client::initDaemon() {
   packet thePacket;
   int retval;

   // Open a new socket to communicate with the server
   daemonProtocol = new TCP(CLIENT_IO, (char *) "127.0.0.1", 9999);

   // Initialize the packet to set up the connection
   memset((void *) &thePacket, 0, sizeof(packet));
   initPacketHeader(&thePacket, myUID, DAEMON_INIT, 0);

   // Send the init packet
   if (daemonProtocol->sendPacket((void *) &thePacket,
         sizeof(packet), 0) < 0) {
      perror("Update daemon: sendPacket()");
      cerr << "Failed to connect to the server\n";
      return false;
   }

   // Verify success
   do {
      while(!daemonProtocol->waitForResponse(10))
         ;

      retval = daemonProtocol->recvPacket(&thePacket, sizeof(thePacket), 0);
      if (retval < 0) {
         perror("Update Daemon: recvPacket");
         return false;
      } else if (retval == 0) {
         cerr << "Update Daemon: failed to verify connection\n";
         return false;
      }
   } while (thePacket.type != LOGIN_ACK_T);
   
   return true;
}
