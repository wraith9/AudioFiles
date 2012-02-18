/** This function calls select to handle detecting when
 * a socket is able to be read or written to.
 *
 * @author William McVicker
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/** 
 *
 * @return the file descriptor that triggered plus one
 */
int select_call(int *socket_num, int numSockets, int seconds, int useconds) {
   int retval, maxFD = -1;
   fd_set rfds;
   struct timeval tv;

   FD_ZERO(&rfds);

   for (int i = 0; i < numSockets; i++) {
      maxFD = (maxFD < socket_num[i]) ? socket_num[i] : maxFD;
      FD_SET(socket_num[i], &rfds);
   }

   tv.tv_sec = seconds;
   tv.tv_usec = useconds;

   retval = select(maxFD+1, &rfds, NULL, NULL, &tv);
   if (retval == -1) {
      perror("select()");
      for (int i = 0; i < numSockets; i++) {
         if (socket_num[i] > 2) // don't close stdin, stdout, or stderr
            close(socket_num[i]);
      }
      exit(EXIT_FAILURE);
   } else if (retval == 0)
      return 0;

   for (int i = 0; i < numSockets; i++) {
      if (FD_ISSET(socket_num[i], &rfds)) 
         return socket_num[i] + 1;
   }

   return 0;
}
