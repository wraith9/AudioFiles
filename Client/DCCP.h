/** This class is used to send data across the DCCP transport layer
 * protocol.  It is inherited from the TransProtocol class.
 *
 * @author William McVicker
 */

#ifndef DCCP_H
#define DCCP_H

#include <sys/types.h>
#include <sys/socket.h>

#include "TransProtocol.h"

#include <string>

using namespace std;

class DCCP : public TransProtocol {
   public:
      DCCP(enum PROTO_IO protoIO, char *hostname, uint16_t portNum);
      ~DCCP();

      int sendPacket(const void *buf, size_t len, int flags);
      int recvPacket(void *buf, size_t len, int flags);
      uint32_t getCallerID();
      void ignoreCaller();
      void answerCall();
      void endCall();

   protected:
      int openSocket();
      void initMaster(uint16_t portNum);
      void initSlave(char *hostname, uint16_t portNum);

};

#endif
