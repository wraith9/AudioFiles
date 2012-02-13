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

using namespace std;

class Client {
   public:
      Client(enum PROTO_TYPE type, uint32_t theUID);
      ~Client();

      int waitForRequests(int seconds);
      void acceptNewCall();
      void makeCall(uint32_t friendId);
      void endCall();
      
      char friendsName[1024];
      volatile bool chatting;
      uint32_t callerID;

   private:
      void startChat(TransProtocol *commProtocol);
      uint16_t getFriendPort(uint32_t friendId);
      string getHostname(uint32_t friendId);
      void connectToFriend();
     
      uint32_t myUID;
      TransProtocol *serverProtocol;
      TransProtocol *clientProtocol;
      boost::shared_ptr<boost::thread> m_thread;

      // temporary variables till be get friend db
      uint16_t friendPort;
      string theHost;
};

#endif
