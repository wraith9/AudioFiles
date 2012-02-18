/** This is a base class for our transport protocols
 *
 * @author William McVicker
 */

#ifndef TRANSPROTOCOL_H
#define TRANSPROTOCOL_H

#include "common.h"

#include <netdb.h>
extern int h_errno;

#include <string>

using namespace std;

class TransProtocol {
   public:
      TransProtocol();
      virtual ~TransProtocol() {};

      int waitForResponse(int seconds);
      int waitForRequests(int seconds);
      int waitForRequestsOrInput(int seconds);
      uint16_t getPortNum();

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
      virtual void initMaster(uint16_t portNum) =0;
      virtual void initSlave(string hostname, uint16_t portNum) =0;

};

#endif
