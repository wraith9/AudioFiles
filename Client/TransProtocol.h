/** This is a base class for our transport protocols
 *
 * @author William McVicker
 */

#ifndef TRANSPROTOCOL_H
#define TRANSPROTOCOL_H

#include "common.h"

class TransProtocol {
   public:
      TransProtocol();
      virtual ~TransProtocol() {};

      int waitForClients(int seconds);

      virtual int sendPacket(const void *buf, size_t len,
            int flags) =0;
      virtual int recvPacket(void *buf, size_t len, int flags) =0;
      virtual uint32_t getCallerID() =0;
      virtual void ignoreCaller() =0;
      virtual void answerCall() =0;
      virtual void endCall() =0;

      TransProtocol &operator=(const TransProtocol &right);

   protected:
      int socket_num;
      int client_socket;
      int temp_socket;
      virtual int openSocket() =0;

};

#endif
