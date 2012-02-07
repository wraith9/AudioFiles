/** This class is used to send and receive data across the TCP 
 * transport layer.  It is inherited from the TransProtocol class.
 *
 * @author William McVicker
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "TCP.h"

#include <iostream>

TCP::TCP() {

   socket_num = -1;
   client_socket = -1;
   temp_socket = -1;

   openSocket();
}

TCP::~TCP() {

   close(socket_num);
   close(client_socket);
}

/** Opens a TCP socket
 * 
 * @return the socket number
 */
int TCP::openSocket() {
   int reuseport;
   struct sockaddr_in address;

   if ((socket_num = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("socket");
      return -1;
   }

   reuseport = 1;
   if (setsockopt(socket_num, SOL_SOCKET, SO_REUSEADDR,
            (const char *) &reuseport, sizeof(reuseport)) < 0) {
      perror("setsockopt");
      return -1;
   }

   /* name the socket */
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(7777);

   /* bind the socket to a port */
   if (bind(socket_num, (struct sockaddr *) &address, sizeof(address)) < 0) {
      perror("bind");
      close(socket_num);
      exit(EXIT_FAILURE);
   }

   if (listen(socket_num, SOMAXCONN) == -1) {
      perror("listen()");
      close(socket_num);
      exit(EXIT_FAILURE);
   }

   return socket_num;
}

/** Sends a packet over the TCP socket 
 *
 * @param buf the packet to send
 * @param len the size of the packet
 * @param flags flags to set
 * @return the number of characters sent. On error, -1 is returned
 */
int TCP::sendPacket(const void *buf, size_t len, int flags) {

   return send(client_socket, buf, len, flags);
}

/** Gets a packet from the TCP socket.
 *
 * @param buf the buffer to store the packet in
 * @param len the length of the packet received
 * @param flags flags that indicate how to receive the packet
 * @return the number of bytes received. On error, -1 is returned
 */
int TCP::recvPacket(void *buf, size_t len, int flags) {

   return recv(client_socket, buf, len, flags);
}

/** Identifies who is calling 
 *
 * @return the uid of the person calling when successful, else 0
 */
uint32_t TCP::getCallerID() {
   int status, rec_size;
   packet initPacket;

   temp_socket = accept(socket_num, NULL, NULL);
   if (temp_socket < 0) {
      perror("accept()");
      return 0;
   }

   status = select_call(temp_socket, INIT_TIMEOUT, 0);
   if (status) {
      rec_size = recv(temp_socket, (void *) &initPacket, sizeof(packet), 0);
      if (rec_size == 0) {
         // session was shutdown
         std::cout << "Call ended\n";
      } else if (rec_size != sizeof(packet)) {
         perror("recv()");
      } else {
         return initPacket.uid;
      }
   }

   close(temp_socket);
   return 0;
}

void TCP::ignoreCaller() {

   close(temp_socket);
}

void TCP::answerCall() {

   client_socket = temp_socket;
}

void TCP::endCall() {

   close(client_socket);
}
