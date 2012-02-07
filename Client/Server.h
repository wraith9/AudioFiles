/** The DCCP server class
 *
 * @author William McVicker
 */

#ifndef SERVER_H
#define SERVER_H


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

class Server {
   public:
      Server();
      Server(enum PROTO_TYPE type);
      ~Server();

      int waitForClients(int seconds);
      void acceptNewCall();
      void endCall();
      
      char friendsName[1024];
      volatile bool chatting;
      uint32_t callerID;

   private:
      void startChat();
      
      uint32_t myUID;
      TransProtocol *tProtocol;
      boost::shared_ptr<boost::thread> m_thread;

};

#endif
