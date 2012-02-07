/** The DCCP Server code
 *
 * @author William McVicker
 */

#include "Server.h"

#include <iostream>
#include <string>
#include <string.h>

/** Server constructor with a DCCP transport protocol */
Server::Server(): chatting(false), callerID(0) {

   tProtocol = new DCCP();
}

/** This constructor specifies which transport protocol to use
 * @param type the type of transport protocol, i.e. DCCP, TCP, or UDP
 */
Server::Server(enum PROTO_TYPE type) : chatting(false), callerID(0) {

   switch (type) {
   case DCCP_T:
      tProtocol = new DCCP();
      break;
   case UDP_T:
      assert(false); //tProtocol = new UDP();
      break;
   case TCP_T:
      tProtocol = new TCP();
      break;
   default:
      assert(false);
   }
   
}

/** Server destructor */
Server::~Server() {

   delete tProtocol;
}

int Server::waitForClients(int seconds) {

   return tProtocol->waitForClients(seconds);
}

/** Accepts the new call by starting a calling thread. */
void Server::acceptNewCall() {
   std::string toAccept = "";
   boost::posix_time::milliseconds noTime(0);

   callerID = tProtocol->getCallerID();
   if (callerID == 0)
      return;

   if (chatting) {
      std::cout << callerID << " tried to call you.\n";
      return;
   }

   // Handle previous threads 
   if (m_thread) 
      m_thread->join();

   std::cout << "\nIncoming call: ";
   std::cout << callerID;
   std::cout << " wants to chat with you. Accept? (Y/N) ";
   getline(std::cin, toAccept);
   if (toAccept.at(0) == 'N' || toAccept.at(0) == 'n') {
      tProtocol->ignoreCaller();
      chatting = false;
      callerID = 0;
   } else {
      tProtocol->answerCall();
      chatting = true;
      m_thread = boost::shared_ptr<boost::thread>(
            new boost::thread(boost::bind(&Server::startChat, this)));
   }
}

/** Stops the current call by ending the calling thread. */
void Server::endCall() {

   assert(m_thread);
   m_thread->join();
}

/** Start of a new thread -- Accepts an incoming request */
void Server::startChat() {
   packet thePacket;
   int status;
   boost::posix_time::milliseconds sleep_time(50);

   thePacket.uid = myUID;
   thePacket.type = audio_data;
   strcpy((char *) thePacket.data, "This data is from the server!!\n\0");
   thePacket.dlength = strlen((char *) thePacket.data) + 1;

   while (1) {
      status = tProtocol->sendPacket((void *) &thePacket, sizeof(packet), 0);
      if (status != sizeof(packet)) {
         if (status == -1)
            perror("send()");
         else if (status == 0) 
            std::cout << "Call ended.\n";
         break;
      }

      boost::this_thread::sleep(sleep_time);
   }

   tProtocol->endCall();
   std::cerr << "<chatting stopped>\n";
   chatting = false;
}
