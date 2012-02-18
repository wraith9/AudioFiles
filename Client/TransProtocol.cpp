

#include "TransProtocol.h"

TransProtocol::TransProtocol(): socket_num(-1), client_socket(-1),
   temp_socket(-1) {

   }

TransProtocol& TransProtocol::operator=(const TransProtocol &right) {

   socket_num = right.socket_num;
   client_socket = right.client_socket;
   temp_socket = right.temp_socket;

   return *this;
}

/** Waits for the socket to be ready for a read
 *
 * @param seconds the number of seconds to wait for the socket to be ready
 * @return 1 if someone is calling, else 0
 */
int TransProtocol::waitForRequests(int seconds) {

   return select_call(&socket_num, 1, seconds, 0);
}


int TransProtocol::waitForResponse(int seconds) {
   
   return select_call(&client_socket, 1, seconds, 0);
}

int TransProtocol::waitForRequestsOrInput(int seconds) {
   int myFds[2];

   myFds[0] = STDIN_FILENO;
   myFds[1] = socket_num;

   return select_call(myFds, 2, seconds, 0);
}

