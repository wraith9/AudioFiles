/** This class is used to send and receive data across the UDP 
 * transport layer.  It is inherited from the TransProtocol class.
 *
 * @author William McVicker
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "UDP.h"

#include <sstream>
#include <iostream>
#include <string.h>

UDP::UDP(enum PROTO_IO protoIO, char *hostname, uint16_t portNum) {

   socket_num = -1;
   client_socket = -1;
   temp_socket = -1;

   if (protoIO == SERVER_IO) {
      initMaster(portNum);
   } else if (protoIO == CLIENT_IO)
      initSlave(hostname, portNum);
   else {
      cerr << "Invalid protocol I/O: " << protoIO << endl;
      exit(EXIT_FAILURE);
   }
}

UDP::~UDP() {

   close(socket_num);
   close(client_socket);
}

/** Opens a UDP socket
 * 
 * @return the socket number
 */
int UDP::initMaster_custom(uint16_t portNum) {
   int reuseport;
   struct sockaddr_in address;
   int socketNum;

   if ((socketNum = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket");
      exit(EXIT_FAILURE);
   }

   reuseport = 0;
   if (setsockopt(socketNum, SOL_SOCKET, SO_REUSEADDR,
            (const char *) &reuseport, sizeof(reuseport)) < 0) {
      perror("setsockopt");
      close(socketNum);
      exit(EXIT_FAILURE);
   }

   /* name the socket */
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(portNum);

   /* bind the socket to a port */
   if (bind(socketNum, (struct sockaddr *) &address, sizeof(address)) < 0) {
      perror("bind");
      close(socket_num);
      exit(EXIT_FAILURE);
   }

   return socketNum;
}

void UDP::initMaster(uint16_t portNum) {

   socket_num = initMaster_custom(portNum);
}

void UDP::initSlave(char *hostname, uint16_t portNum) {
/*   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int retval, reuseport;
   stringstream myStream;
   myStream << portNum;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_DGRAM;

   if ((retval = getaddrinfo(hostname, (myStream.str()).c_str(), &hints, &result)) != 0) {
      cerr << "getaddrinfo: " << gai_strerror(retval) << endl;
      exit(EXIT_FAILURE);
   }

   for (rp = result; rp != NULL; rp = rp->ai_next) {
      socket_num = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (socket_num < 0)
         continue;

      if (connect(socket_num, rp->ai_addr, rp->ai_addrlen) != -1 ) {
         memcpy(&client_addr, (struct sockaddr_in *) rp->ai_addr, 
               sizeof(struct sockaddr_in));
         client_addr_len = sizeof(struct sockaddr_in);
         break;
      }

      close(socket_num);
   }

   if (rp == NULL) {
      cerr << "Could not connect to server\n";
      exit(EXIT_FAILURE);
   }

   reuseport = 0;
   if (setsockopt(socket_num, SOL_SOCKET, SO_REUSEADDR,
            (const char *) &reuseport, sizeof(reuseport)) < 0) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
   }

   freeaddrinfo(result);
   client_socket = socket_num;*/

   struct hostent *theHost;

   if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket");
      exit(EXIT_FAILURE);
   }

   if (NULL == (theHost = gethostbyname(hostname))) {
      cerr << "gethostbyname() failed!\n";
      exit(EXIT_FAILURE);
   }
   memcpy(&client_addr.sin_addr, theHost->h_addr, theHost->h_length);

   client_addr.sin_family = AF_INET;
   client_addr.sin_port = htons(portNum);
   client_addr_len = sizeof(client_addr);

   //client_socket = socket_num;
}

/** Sends a packet over the UDP socket 
 *
 * @param buf the packet to send
 * @param len the size of the packet
 * @param flags flags to set
 * @return the number of characters sent. On error, -1 is returned
 */
int UDP::sendPacket(const void *buf, size_t len, int flags) {

   return sendto(client_socket, buf, len, flags, (struct sockaddr *) &client_addr, 
         client_addr_len);
}

/** Gets a packet from the UDP socket.
 *
 * @param buf the buffer to store the packet in
 * @param len the length of the packet received
 * @param flags flags that indicate how to receive the packet
 * @return the number of bytes received. On error, -1 is returned
 */
int UDP::recvPacket(void *buf, size_t len, int flags) {

   return recvfrom(client_socket, buf, len, 0, (struct sockaddr *) &client_addr, 
         &client_addr_len);
}

/** Identifies who is calling 
 *
 * @return the uid of the person calling when successful, else 0
 */
uint32_t UDP::getCallerID() {
   int status, rec_size;
   packet initPacket;

   status = select_call(&socket_num, 1, INIT_TIMEOUT, 0);
   if (status) {
      rec_size = recvfrom(socket_num, (void *) &initPacket, sizeof(packet), 0,
            (struct sockaddr *) &client_addr, &client_addr_len);
      if (rec_size == 0) {
#ifdef DEBUG
         cerr << "getCallerID: socket_num shutdown\n";
#endif
      } else if (rec_size != sizeof(packet)) {
         perror("recv()");
      } else {
         return ntohl(initPacket.uid);
      }
   }

   return 0;
}

void UDP::ignoreCaller() {

}

void UDP::answerCall() {

   client_socket = initMaster_custom(0);
}

void UDP::endCall() {

   close(client_socket);
}
