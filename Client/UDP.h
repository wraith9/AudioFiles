/** This class is used to send data across the UDP transport layer
 * protocol.  It is inherited from the TransProtocol class.
 *
 * @author William McVicker
 */

#ifndef UDP_H
#define UDP_H 

#include <sys/types.h>
#include <sys/socket.h>

#include "TransProtocol.h"

#include <string>

using namespace std;

class UDP : public TransProtocol {
   public:
      UDP(enum PROTO_IO protoIO, char *hostname, uint16_t portNum);
      ~UDP();

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

      struct sockaddr_in client_addr;
      socklen_t client_addr_len;

};

#endif
