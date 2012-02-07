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


int select_call(int socket_num, int seconds, int useconds) {
   int retval;
   fd_set rfds;
   struct timeval tv;

   FD_ZERO(&rfds);
   FD_SET(socket_num, &rfds);

   tv.tv_sec = seconds;
   tv.tv_usec = useconds;

   retval = select(socket_num+1, &rfds, NULL, NULL, &tv);
   if (retval == -1) {
      perror("select()");
      close(socket_num);
      exit(EXIT_FAILURE);
   }

   return retval;
}
