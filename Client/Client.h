/** The DCCP server class
 *
 * @author William McVicker
 */

#ifndef CLIENT_H
#define CLIENT_H


#include <stdio.h> // for perror
#include <stdlib.h>
#include <assert.h>
#include "common.h"

#include <sys/socket.h>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "DCCP.h"
#include "TCP.h"

#include <string>
#include <map>

using namespace std;


class Client {
   public:
      Client(enum PROTO_TYPE type);
      ~Client();

      int waitForRequests(int seconds);
      int waitForRequestsOrInput(int seconds);
      void acceptNewCall();
      void makeCall(uint32_t friendId);
      void endCall();
      bool loginToServer(login_data loginData);
      string whoAmI();

      // Friends list stuff
      void printFriendList();
      bool verifyId(uint32_t uid);

      volatile bool chatting;
      uint32_t callerID;
      map<uint32_t, friend_list> myFriends;

   private:
      void startChat(TransProtocol *commProtocol);
      uint16_t getFriendPort(uint32_t friendId);
      string getHostname(uint32_t friendId);
      void connectToFriend();
      bool connectToServer(login_data loginData);
      void extractFriends(friendList_data *data, uint16_t dlength);

      void theUpdateDaemon();

      enum PROTO_TYPE myType;
      uint32_t myUID;
      TransProtocol *masterProtocol;
      TransProtocol *slaveProtocol;
      TransProtocol *mainServer;
      boost::shared_ptr<boost::thread> m_thread;
      boost::shared_ptr<boost::thread> m_updateThread;

      // temporary variables till be get friend db
      uint16_t friendPort;
      string theHost;
};

#endif
