/** this is a really fast and sloppy client used for testing my server
 *
 * @author William McVicker
 */

#include <stdlib.h>
#include <stdio.h>

#include <iostream>

#include <netdb.h>
extern int h_errno;

#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>

#include "common.h"

using namespace std;


void connectToServer(int socket_num); 

int main(int argc, char *argv[]) {
   int socket_num;
   struct hostent *theHost;
   struct sockaddr_in address;
   uint16_t portNum;

   if (argc == 2)
      portNum = (uint16_t) atoi(argv[1]);
   else
      portNum = 7777;

   fprintf(stderr, "Using port %hu\n", portNum);

   if ((socket_num = socket(AF_INET, SOCK_DCCP, IPPROTO_DCCP)) < 0) {
      perror("socket()");
      exit (EXIT_FAILURE);
   }

   int on = 0;
   setsockopt(socket_num, SOL_DCCP, SO_REUSEADDR, 
         (const char *) &on, sizeof(on));

   theHost = gethostbyname("localhost");
   memcpy(&address.sin_addr, theHost->h_addr, theHost->h_length);
   address.sin_family = AF_INET;
   address.sin_port = htons(portNum);

   if (connect(socket_num, (struct sockaddr *) &address, sizeof(address)) < 0) {
      perror("connect()");
      exit(EXIT_FAILURE);
   }

   connectToServer(socket_num);
   return EXIT_SUCCESS;
}

void connectToServer(int socket_num) {
   packet initPacket;
   int status, rec_size;
   packet thePacket;

   cerr << "Connecting...";

   initPacket.type = request_chat;
   initPacket.uid = 7;
   for (int i = 0; i < 5; i++) {
      if (sizeof(packet) != (status = 
            send(socket_num, (void *) &initPacket, sizeof(packet), 0))) {
         if (i == 4) {
            perror("send()");
            cout << "Call failed.\n";
            return;
         }
         cout << ".";
      } else
         break;
   }

   while (1) {
      if (!select_call(socket_num, 10, 0))
         break;

      rec_size = recv(socket_num, (void *) &thePacket, sizeof(packet), 0);
      if (rec_size == -1) {
         perror("recv()");
         break;
      } else if (rec_size == 0)
         break;

      if (thePacket.type == audio_data) {
         if (write(STDOUT_FILENO, thePacket.data, thePacket.dlength) < 0) 
            perror("write()");
      }
   }

   cout << "Call ended\n";
}
