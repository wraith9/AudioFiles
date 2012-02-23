/** The DCCP Client code
 *
 * @author William McVicker
 */

#include "Client.h"

#include <iostream>
#include <string>
#include <string.h>

#include <time.h>

#include <signal.h>

extern bool audioToFile;



/** This constructor specifies which transport protocol to use
 * @param type the type of transport protocol, i.e. DCCP, TCP, or UDP
 */
Client::Client(char *serverHostname, enum PROTO_TYPE type) {

   theServerName = serverHostname;

   chatting = false;
   callerID = 0;
   myType = type;

   endChatting = false;

}

/** Client destructor */
Client::~Client() {
   closeAllConnections();
   
   delete theServerName;
}

/** Closes all the connections */
void Client::closeAllConnections() {

   delete slaveProtocol;
   delete masterProtocol;
   delete mainServer;
}

/** Logs into the main server and opens the client-to-client
 * transport protocol that will be used for chatting.
 *
 * @param loginData the username and password to use for login
 * @return true if successfully logged into the server, else false
 */
bool Client::loginToServer(login_data loginData) {
   friend_list myInfo;

   switch (myType) {
   case DCCP_T:
      masterProtocol = new DCCP(SERVER_IO, NULL, 0);
      break;
   case UDP_T:
      masterProtocol = new UDP(SERVER_IO, NULL, 0);
      break;
   case TCP_T:
      masterProtocol = new TCP(SERVER_IO, NULL, 0);
      break;
   default:
      cout << "Invalid option: " << myType << endl;
      SAFE_EXIT(EXIT_FAILURE);
   }

   // inject the master protocol port number
   loginData.clientPort = htons(masterProtocol->getPortNum());
   
   if (!connectToServer(loginData))
      return false;

   // Add your name to the friends list so that you know who you are
   memcpy(myInfo.friendData.username, loginData.username, USERNAME_LEN);
   myInfo.friendData.uid = myUID;
   myFriends[myUID] = myInfo;

   // Start update request daemon
   m_updateThread = boost::shared_ptr<boost::thread>(
      new boost::thread(boost::bind(&Client::theUpdateDaemon, this)));
   
   return true;
}

bool Client::connectToServer(login_data loginData) {
   packet loginPacket;
   int responseVal;

   mainServer = new TCP(CLIENT_IO, theServerName, 9999);

   initPacketHeader(&loginPacket, 0, LOGIN_T, sizeof(login_data));
   memcpy(loginPacket.data, (void *) &loginData, sizeof(login_data));

   // Send login information
   if (mainServer->sendPacket((void *) &loginPacket, sizeof(packet), 0) < 0) {
      perror("connectToServer: sendPacket()");
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
      printErrMsg("connectToServer", loginPacket.type);
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
      endChatting = false;
      m_thread = boost::shared_ptr<boost::thread>(
            new boost::thread(boost::bind(&Client::startChat, this,
                  masterProtocol)));
   }
}

/** Tries to call a friend
 * @param friendId the friend to call
 */
void Client::makeCall(uint32_t friendId) {
   carOutFormat friendAddr;

   if (!getFriendAddr(&friendAddr, friendId))
      return;

   cerr << "Got the friend information: ";
   cerr << inet_ntoa(friendAddr.friendIP) << " ";
   cerr << ntohs(friendAddr.portNum) << endl;

   if (dynamic_cast<DCCP *>(masterProtocol))
      slaveProtocol = new DCCP(CLIENT_IO, inet_ntoa(friendAddr.friendIP), 
            ntohs(friendAddr.portNum));
   else if (dynamic_cast<TCP *>(masterProtocol))
      slaveProtocol = new TCP(CLIENT_IO, inet_ntoa(friendAddr.friendIP), 
            ntohs(friendAddr.portNum));
   else if (dynamic_cast<UDP *>(masterProtocol))
      slaveProtocol = new UDP(CLIENT_IO, inet_ntoa(friendAddr.friendIP), 
            ntohs(friendAddr.portNum));
   else
      assert(false);

   connectToFriend();
}

void Client::connectToFriend() {
   packet initPacket;
   int status;

   cout << "Dialing...";

   initPacketHeader(&initPacket, myUID, REQUEST_CHAT_T, 0);
   for (int i = 0; i < 5; i++) {
      status = slaveProtocol->sendPacket((void *) &initPacket, sizeof(packet),
            0);
      if (status < 0) {
         perror("connectToFriend: sendPacket()");
         return;
      } else if (status == 0) {
         cerr << "Failed to send request to chat packet\n";
         return;
      }

      /* Check if the client responded */
      if (slaveProtocol->waitForResponse(1))
         break;
      else if (i == 4) {
         cout << "No response\n";
         return;
      }
   }

   if (m_thread)
      m_thread->join();

   chatting = true;
   endChatting = false;
   m_thread = boost::shared_ptr<boost::thread>(
         new boost::thread(boost::bind(&Client::startChat, this,
               slaveProtocol)));
}

/** Start of a new thread -- Accepts an incoming request */
void Client::startChat(TransProtocol *commProtocol) {
   packet ourPacket;
   packet theirPacket;
   int status, tempFile = -1;
   boost::posix_time::milliseconds sleep_time(50);


   // XXX Debugging ///////////
   if (audioToFile) {
      stringstream ss;
      srand(time(NULL));
      ss << rand();
      string randFilename = "audioSession" + ss.str() + ".raw";
      tempFile = open(randFilename.c_str(), O_WRONLY | O_CREAT | O_NONBLOCK | 
            O_TRUNC, S_IRUSR | S_IWUSR);
      if (tempFile < 0) {
         perror("Failed to open a temporary file");
         commProtocol->endCall();
         chatting = false;
         return;
      }
   }
   ///////////////////////////

   voiceStream = new VoiceStreamer();
   if (!voiceStream->initDevice()) {
      cout << "Failed to initialize the microphone\n";
      commProtocol->endCall();
      chatting = false;
      return;
   }

   initPacketHeader(&ourPacket, myUID, AUDIO_DATA_T, BUF_LEN);

   cout << "\nConnected! Start talking...\n";

   numRecvPackets = 0;
   numSentPackets = 0;

   while (!endChatting) {
      if (commProtocol->waitForResponse(0)) {
         status = commProtocol->recvPacket((void *) &theirPacket,
               sizeof(packet), 0);
         if (status < 0) {
            perror("recvPacket()");
            break;
         } else if (status == 0) // socket has been closed
            break;

         if (theirPacket.type == AUDIO_DATA_T) {
            numRecvPackets++;
            if (!audioToFile) {
               voiceStream->playBuffer((char *) theirPacket.data, 
                     ntohs(theirPacket.dlength));
            } else {
               int numW;
               if ((numW = write(tempFile, theirPacket.data, 
                        ntohs(theirPacket.dlength))) < 0) {
                  perror("Failed to write to the temp file\n");
                  break;
               } else if (numW < ntohs(theirPacket.dlength)) 
                  cout << "short write!\n";
            }
         }
      }

      ourPacket.dlength = htons(
            voiceStream->fillBuffer((char *) ourPacket.data, BUF_LEN));
      status = commProtocol->sendPacket((void *) &ourPacket, sizeof(packet), 0);
      if (status < 0) {
         if (errno != EAGAIN) {
            perror("startChat: sendPacket()");
            break;
         }
      } else
         numSentPackets++;

   }

   cout << "Number of packets sent: " << numSentPackets << endl;
   cout << "Number of packets received: " << numRecvPackets << endl;

   delete voiceStream;
   cout << "Call ended.\n";
   commProtocol->endCall();
   chatting = false;
}


/** Gets the port of the friend you want to chat with
 *
 * @param friendId the id of the friend you want to chat with
 * @return the port of your friend's computer
 */
uint16_t Client::getFriendAddr(carOutFormat *theData, uint32_t friendId) {
   packet reqPacket;
   carFormat reqData;
   int retval;

   // Create the packet
   initPacketHeader(&reqPacket, myUID, REQUEST_ADDRESS_T, sizeof(carFormat));
   reqData.friendID = htonl(friendId);
   memcpy(reqPacket.data, &reqData, sizeof(carFormat));

   if (mainServer->sendPacket(&reqPacket, sizeof(packet), 0) < 0) {
      perror("getFriendAddr: sendPacket()");
      SAFE_EXIT(EXIT_FAILURE);
   }

   for (int i = 0; i < 10; i++) {
      if (!mainServer->waitForResponse(2))
         break;

      memset((void *) &reqPacket, 0, sizeof(packet));
      retval = mainServer->recvPacket((void *) &reqPacket, sizeof(packet), 0);
      if (retval < 0) {
         perror("getFriendAddr: recvPacket()");
         SAFE_EXIT(EXIT_FAILURE);
      } else if (retval == 0) { // connection has been shutdown
         cout << "Connection to server was lost.\n";
         return false;
      }

      switch (reqPacket.type) {
      case REQUEST_ADDRESS_T:
         memcpy(theData, reqPacket.data, sizeof(carOutFormat));
         return true;
      case NOT_LOGGED_IN_E:
         cout << myFriends[friendId].friendData.username;
         cout << " is not available.\n";
         return false;
      default:
         if (reqPacket.type >= LOWEST_ERRNO &&
               reqPacket.type <= HIGHEST_ERRNO) {
            printErrMsg("getFriendAddr", reqPacket.type);
            return false;
         } else
            cout << "getFriendAddr: received a non-request address packet.\n";
      }
   }

   cerr << "Failed to get the friend's IP and port # from the server!\n";
   return false;
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
   char temp[USERNAME_LEN+1];
   memset(temp, 0, USERNAME_LEN+1);
   memcpy(temp, myFriends[myUID].friendData.username, USERNAME_LEN);

   return string(temp);
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

      if (tempNode.friendData.uid == 0) // this isn't allowed
         continue;
      else
         myFriends[tempNode.friendData.uid] = tempNode;
   }

}

/** Prints a list of the user's friends with their uids and status. */
void Client::printFriendList() {

   map<uint32_t, friend_list>::iterator it;
   for (it = myFriends.begin(); it != myFriends.end(); it++) {
      if ((*it).first == myUID)
         continue;

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

   if (uid == 0)
      return false;
   else if (myFriends.find(uid) == myFriends.end())
      return false;

   return true;
}

/** Prints the server's error message.
 *
 * @param server_errno the error number from the server
 */
void Client::printErrMsg(string header, uint8_t server_errno) {

   cerr << header << ": ";

   switch (server_errno) {
   case LOGIN_DUP_E:
      cerr << "login duplicate\n";
      break;
   case NOT_A_FRIEND_E:
      cerr << "not a friend\n";
      break;
   case NOT_LOGGED_IN_E:
      cerr << "not logged in\n";
      break;
   case INVALID_LOGIN_FORMAT_E:
      cerr << "invalid login format\n";
      break;
   case INVALID_LOGIN_E:
      cerr << "invalid login\n";
      break;
   case INVALID_PW_E:
      cerr << "invalid password\n";
      break;
   case UNKNOWN_E:
      cerr << "unknown server error\n";
      break;
   default:
      cerr << "undefined error number: " << server_errno << endl;
   }
}

void Client::initPacketHeader(packet *thePacket, uint32_t uid, uint8_t type,
      uint16_t dlength) {

   thePacket->uid = htonl(uid);
   thePacket->type = type;
   thePacket->dlength = htons(dlength);
}
