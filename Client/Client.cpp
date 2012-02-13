/** The DCCP Client code
 *
 * @author William McVicker
 */

#include "Client.h"

#include <iostream>
#include <string>
#include <string.h>

/** This constructor specifies which transport protocol to use
 * @param type the type of transport protocol, i.e. DCCP, TCP, or UDP
 */
Client::Client(enum PROTO_TYPE type, uint32_t theUID) : chatting(false),
   callerID(0), myUID(theUID) {

      switch (type) {
      case DCCP_T:
         serverProtocol = new DCCP(SERVER_IO, "", 0);
         break;
      case UDP_T:
         assert(false); //tProtocol = new UDP();
         break;
      case TCP_T:
         serverProtocol = new TCP(SERVER_IO, "", 0);
         break;
      default:
         std::cout << "Invalid option: " << type << std::endl;
         exit(EXIT_FAILURE);
      }

   }

/** Client destructor */
Client::~Client() {

   delete clientProtocol;
   delete serverProtocol;
}

int Client::waitForRequests(int seconds) {

   return serverProtocol->waitForRequests(seconds);
}

/** Accepts the new call by starting a calling thread. */
void Client::acceptNewCall() {
   std::string toAccept = "";
   boost::posix_time::milliseconds noTime(0);

   callerID = serverProtocol->getCallerID();
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
      serverProtocol->ignoreCaller();
      chatting = false;
      callerID = 0;
   } else {
      serverProtocol->answerCall();
      m_thread = boost::shared_ptr<boost::thread>(
            new boost::thread(boost::bind(&Client::startChat, this, 
            serverProtocol)));
   }
}

/** Tries to call a friend
 * @param friendId the friend to call
 */
void Client::makeCall(uint32_t friendId) {

   friendPort = getFriendPort(friendId);
   theHost = getHostname(friendId);

   if (dynamic_cast<DCCP *>(serverProtocol))
      clientProtocol = new DCCP(CLIENT_IO, theHost, friendPort);
   else if (dynamic_cast<TCP *>(serverProtocol))
      clientProtocol = new TCP(CLIENT_IO, theHost, friendPort);
   //else if (dynamic_cast<UDP *>(&serverProtocol))
   //   assert(false);//clientProtocol = new UDP(CLIENT_IO, theHost, friendPort);
   //else
   //   assert(false);

   connectToFriend();
}

void Client::connectToFriend() {
   packet initPacket;
   int status;

   cout << "Dialing...";

   initPacket.type = request_chat;
   initPacket.uid = myUID;
   for (int i = 0; i < 5; i++) {
      status = clientProtocol->sendPacket((void *) &initPacket, sizeof(packet),
            0);
      if (status < 0) {
         perror("sendPacket()");
         return;
      }

      /* Check if the server responded */
      if (clientProtocol->waitForResponse(1))
         break;
      else if (i == 4) {
         cout << "No response\n";
         return;
      }
   }

   if (m_thread)
      m_thread->join();

   m_thread = boost::shared_ptr<boost::thread>(
         new boost::thread(boost::bind(&Client::startChat, this, 
         clientProtocol)));
}

/** Gets the port of the friend you want to chat with
 *
 * @param friendId the id of the friend you want to chat with
 * @return the port of your friend's computer
 */
uint16_t Client::getFriendPort(uint32_t friendId) {

   // get the port # from the server db

   // XXX Temporarily ask for it
   string input = "";
   uint16_t portNum = -1;

   cout << "Enter the port # of your friend's computer: ";

   getline(std::cin, input);
   stringstream myStream(input);
   if (myStream >> portNum)
      return portNum;

   assert(false);
}

/** Gets the hostname of the friend you want to chat with
 *
 * @param friendId the id of the friend you want to chat with
 * @return a string containing the hostname of your friend's computer
 */
string Client::getHostname(uint32_t friendId) {

   // get the hostname from the server db


   // XXX Temporarily ask for the hostname
   string theHostname = "";

   cout << "Enter the hostname of your friend's computer: ";

   getline(cin, theHostname);

   return theHostname;
}

/** Stops the current call by ending the calling thread. */
void Client::endCall() {

   assert(m_thread);
   m_thread->join();
}

/** Start of a new thread -- Accepts an incoming request */
void Client::startChat(TransProtocol *commProtocol) {
   packet ourPacket;
   packet theirPacket;
   int status;
   boost::posix_time::milliseconds sleep_time(50);

   chatting = true;

   ourPacket.uid = myUID;
   ourPacket.type = audio_data;
   strcpy((char *) ourPacket.data, "This data is from the server!!\n\0");
   ourPacket.dlength = strlen((char *) ourPacket.data) + 1;

   while (true) {
      if (commProtocol->waitForResponse(0)) {
         status = commProtocol->recvPacket((void *) &theirPacket, 
               sizeof(packet), 0);
         if (status < 0) {
            perror("recvPacket()");
            break;
         }

         if (theirPacket.type == audio_data) 
            cout << theirPacket.data;
      }

      status = commProtocol->sendPacket((void *) &ourPacket, sizeof(packet), 0);
      if (status < 0) {
         perror("sendPacket()");
         break;
      }

      boost::this_thread::sleep(sleep_time);
   }

   cout << "Call ended.\n";
   commProtocol->endCall();
   chatting = false;
}
