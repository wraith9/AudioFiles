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

class DCCP : public TransProtocol {
   public:
      DCCP();
      ~DCCP();

      int sendPacket(const void *buf, size_t len, int flags);
      int recvPacket(void *buf, size_t len, int flags);
      uint32_t getCallerID();
      void ignoreCaller();
      void answerCall();
      void endCall();

   protected:
      int openSocket();

};

#endif