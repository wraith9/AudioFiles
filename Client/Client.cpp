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
Client::Client(enum PROTO_TYPE type) { 

   chatting = false;
   callerID = 0; 
   myType = type;

}

/** Client destructor */
Client::~Client() {

   delete slaveProtocol;
   delete masterProtocol;
}

bool Client::loginToServer(login_data loginData) {

   switch (myType) {
   case DCCP_T:
      masterProtocol = new DCCP(SERVER_IO, "", 0);
      break;
   case UDP_T:
      assert(false); //tProtocol = new UDP();
      break;
   case TCP_T:
      masterProtocol = new TCP(SERVER_IO, "", 0);
      break;
   default:
      cout << "Invalid option: " << myType << endl;
      exit(EXIT_FAILURE);
   }

   // inject the master protocol port number
   loginData.clientPort = htons(masterProtocol->getPortNum());

   if (!connectToServer(loginData)) 
      return false;

   // Start update request daemon
   m_updateThread = boost::shared_ptr<boost::thread>(
         new boost::thread(boost::bind(&Client::theUpdateDaemon, this))); 

   return true;
}

bool Client::connectToServer(login_data loginData) {
   packet loginPacket;
   int responseVal;

   mainServer = new TCP(CLIENT_IO, "localhost", 9999);

   loginPacket.uid = htonl(0);
   loginPacket.type = LOGIN_T;
   loginPacket.dlength = htons(sizeof(login_data));
   memcpy(loginPacket.data, (void *) &loginData, sizeof(login_data));

   // Send login information
   if (mainServer->sendPacket((void *) &loginPacket, sizeof(packet), 0) < 0) {
      perror("sendPacket()");
      cerr << "Failed to connect to the server\n";
      return false;
   }

   // Get server verification
   if (!mainServer->waitForResponse(10)) {
      cout << "Server not responding\n";
      return false;
   }
   memset((void *) &loginPacket, 0, sizeof(packet));
   responseVal = mainServer->recvPacket((void *) &loginPacket, 
         sizeof(packet), 0);
   if (responseVal < 0) {
      perror("recvPacket()");
      cerr << "Failed to connect to the server\n";
      return false;
   } else if (responseVal == 0) // socket has been closed
      return false;
   
   if (loginPacket.type == LOGIN_ACK_T) {
      myUID = ntohl(loginPacket.uid);
      extractFriends((friendList_data *) loginPacket.data, 
            ntohs(loginPacket.dlength));
   } else if (loginPacket.type > REQUEST_ADDRESS_T) {
      cerr << "Got an error message from the server: ";
      fprintf(stderr, "x%hhx\n", loginPacket.type);
      return false;
   }

   return true;
}

int Client::waitForRequests(int seconds) {

   return masterProtocol->waitForRequests(seconds);
}

int Client::waitForRequestsOrInput(int seconds) {

   return masterProtocol->waitForRequestsOrInput(seconds);
}

/** Accepts the new call by starting a calling thread. */
void Client::acceptNewCall() {
   string toAccept = "";
   boost::posix_time::milliseconds noTime(0);

   callerID = masterProtocol->getCallerID();
   if (callerID == 0) 
      return;

   if (chatting) {
      cout << myFriends[callerID].friendData.username;
      cout << " tried to call you.\n";
      return;
   }

   // Handle previous threads
   if (m_thread)
      m_thread->join();

   cout << "\nIncoming call: ";
   cout << myFriends[callerID].friendData.username;
   cout << " wants to chat with you. Accept? (Y/N) ";
   getline(cin, toAccept);
   if (toAccept.at(0) == 'N' || toAccept.at(0) == 'n') {
      masterProtocol->ignoreCaller();
      chatting = false;
      callerID = 0;
   } else {
      masterProtocol->answerCall();
      chatting = true;
      m_thread = boost::shared_ptr<boost::thread>(
            new boost::thread(boost::bind(&Client::startChat, this, 
            masterProtocol)));
   }
}

/** Tries to call a friend
 * @param friendId the friend to call
 */
void Client::makeCall(uint32_t friendId) {

   friendPort = getFriendPort(friendId);
   theHost = getHostname(friendId);

   if (dynamic_cast<DCCP *>(masterProtocol))
      slaveProtocol = new DCCP(CLIENT_IO, theHost, friendPort);
   else if (dynamic_cast<TCP *>(masterProtocol))
      slaveProtocol = new TCP(CLIENT_IO, theHost, friendPort);
   //else if (dynamic_cast<UDP *>(&masterProtocol))
   //   assert(false);//slaveProtocol = new UDP(CLIENT_IO, theHost, friendPort);
   //else
   //   assert(false);

   connectToFriend();
}

void Client::connectToFriend() {
   packet initPacket;
   int status;

   cout << "Dialing...";

   initPacket.type = REQUEST_CHAT_T;
   initPacket.uid = htonl(myUID);
   for (int i = 0; i < 5; i++) {
      status = slaveProtocol->sendPacket((void *) &initPacket, sizeof(packet),
            0);
      if (status < 0) {
         perror("sendPacket()");
         return;
      }

      /* Check if the server responded */
      if (slaveProtocol->waitForResponse(1))
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
         slaveProtocol)));
}

/** Start of a new thread -- Accepts an incoming request */
void Client::startChat(TransProtocol *commProtocol) {
   packet ourPacket;
   packet theirPacket;
   int status;
   boost::posix_time::milliseconds sleep_time(50);

   ourPacket.uid = htonl(myUID);
   ourPacket.type = AUDIO_DATA_T;
   strcpy((char *) ourPacket.data, "This data is from the server!!\n\0");
   ourPacket.dlength = htons(strlen((char *) ourPacket.data) + 1);

   while (true) {
      if (commProtocol->waitForResponse(0)) {
         status = commProtocol->recvPacket((void *) &theirPacket, 
               sizeof(packet), 0);
         if (status < 0) {
            perror("recvPacket()");
            break;
         } else if (status == 0) // socket has been closed
            break;

         if (theirPacket.type == AUDIO_DATA_T) 
            cout << theirPacket.data;
         else if (theirPacket.type == STATUS_UPDATES_T)
            cout << "GOT A STATUS UPDATE IN startChat!!!!\n";
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

   getline(cin, input);
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

/** Returns a string containing user's name.
 *
 * @return a string containing the user's name
 */
string Client::whoAmI() {
   stringstream myStream;

   // XXX Need to get the users name from the db
   myStream << myUID;
   return myStream.str();
}

/** Extracts the friends information from the input data and adds them to 
 * the user's friends list.
 *
 * @param data the data containing the friend information
 * @param dlength the size of the data
 */
void Client::extractFriends(friendList_data *data, uint16_t dlength) {
   int numFriends = dlength/sizeof(friendList_data);
   friend_list tempNode;

   for (int i = 0; i < numFriends; i++) {
      tempNode.friendData = data[i];
      tempNode.friendData.uid = ntohl(tempNode.friendData.uid);
      tempNode.status = false;
      myFriends[tempNode.friendData.uid] = tempNode;
   }

}

/** Prints a list of the user's friends with their uids and status. */
void Client::printFriendList() {

   map<uint32_t, friend_list>::iterator it;
   for (it = myFriends.begin(); it != myFriends.end(); it++) {
      cout << "\t" << (*it).first << ": ";
      cout << (*it).second.friendData.username;
      cout << " is ";
      if ((*it).second.status)
         cout << "online\n";
      else
         cout << "offline\n";
   }

}

/** Verifies the input parameter is a valid id 
 * 
 * @param uid the id to verify
 * @return true if the id is valid, else false
 */
bool Client::verifyId(uint32_t uid) {
   map<uint32_t, friend_list>::iterator temp;

   if (myFriends.find(uid) == myFriends.end())
      return false;

   return true;
}
