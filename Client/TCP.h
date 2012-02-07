/** This class is used to send data across the TCP transport layer
 * protocol.  It is inherited from the TransProtocol class.
 *
 * @author William McVicker
 */

#ifndef TCP_H
#define TCP_H 

#include <sys/types.h>
#include <sys/socket.h>

#include "TransProtocol.h"

class TCP : public TransProtocol {
   public:
      TCP();
      ~TCP();

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
