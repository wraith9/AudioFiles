
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "VoiceStreamer.h"


VoiceStreamer::VoiceStreamer() {

   format = SND_PCM_FORMAT_S16_LE;
   rate = 8000;
   channels = 2;
   buffer_size = 0;    // auto
   period_size = 0;    // auto
   latency_min = 32;   // in frames/2
   latency_max = 2048; // in frames/2
   loop_sec = 30;      // seconds
   resample = 1;
   output = NULL;
   
   loop_limit = loop_sec * rate;

   phandle = NULL;
   chandle = NULL;

}

VoiceStreamer::~VoiceStreamer() {

   delete buffer;

   if (phandle) 
      snd_pcm_close(phandle);
   if (chandle)
      snd_pcm_close(chandle);
}

bool VoiceStreamer::initDevice() {
   int err;

   if ((err = snd_output_stdio_attach(&output, stdout, 0)) < 0) {
      printf("Output failed: %s\n", snd_strerror(err));
      return false;
   }

   if ((err = snd_pcm_open(&phandle, "default", SND_PCM_STREAM_PLAYBACK, 
               SND_PCM_NONBLOCK)) < 0) {
      printf("Playback open error: %s\n", snd_strerror(err));
      return false;
   }
   if ((err = snd_pcm_open(&chandle, "default" , SND_PCM_STREAM_CAPTURE, 
               SND_PCM_NONBLOCK)) < 0) {
      printf("Record open error: %s\n", snd_strerror(err));
      return false;
   }

   latency = latency_min - 4;
   buffer = new char[(latency_max*snd_pcm_format_width(format)/8)*2];

   if (setparams_p(phandle, &latency) < 0) 
      return false;
   if (setparams_c(chandle, &latency) < 0) 
      return false;
   if ((err = snd_pcm_start(chandle)) < 0) {
      printf("Go error: %s\n", snd_strerror(err));
      exit(EXIT_FAILURE);
   }

   frames_in = frames_out = 0;
   in_max = 0;

   return true;
}

/** Plays the audio contained in the provided buffer.
 *
 * @param data the audio to play
 * @param len the length of the data parameter
 */
void VoiceStreamer::playBuffer(char *data, int len) {
   int data_left = len; 
   ssize_t r;
   char *data_ptr = data;
   bool done = false;

   while (!done) {
      while (frames_in < loop_limit) {
         if (data_left > latency<<2) {
            memcpy(buffer, data_ptr, latency<<2);
            data_ptr += (latency<<2);
            data_left -= (latency<<2);
            r = (latency<<2);
         } else {
            memcpy(buffer, data_ptr, data_left);
            data_ptr += data_left;
            r = data_left;
            data_left = 0;
         }

         if (r > 0) {
            frames_in += r>>2;
            if ((ssize_t) in_max < (r>>2))
               in_max = (r>>2);

            if (writebuf(phandle, buffer, r>>2, &frames_out) < 0) 
               break;

         } else if (r == 0) { // done reading input file
            done = true;
            break;
         } else
            break;
      }

      if (!done) {
         /*snd_pcm_nonblock(phandle, 0);
         snd_pcm_drain(phandle);
         snd_pcm_nonblock(phandle, 1);
         snd_pcm_hw_free(phandle);*/

         frames_in = frames_out = in_max = 0;
         if (setparams_p(phandle, &latency) < 0) 
            break;
      }
   }

}

/** Fills the input buffer with audio data from the microphone.
 *
 * @param data the buffer to fill with audio data
 * @param len the size of the buffer
 * @return a pointer to the filled buffer
 */
uint16_t VoiceStreamer::fillBuffer(char *data, int len) {
   int latency, bufsize, room_left = len; 
   size_t frames_in, frames_out, in_max;
   ssize_t r;
   char *buffer, *data_ptr = data;
   bool done = false;

   latency = latency_min - 4;
   bufsize = (latency_max*snd_pcm_format_width(format)/8)*2;
   buffer = new char[bufsize];

   while (!done) {
      while (frames_in < loop_limit) {
         if ((r = readbuf(chandle, buffer, latency, &frames_in, 
               &in_max)) < 0) {
            break;
         } else {
            if (room_left > r<<2) {
               memcpy(data_ptr, buffer, r<<2);
               data_ptr += (r<<2);
               room_left -= (r<<2);
            } else {
               memcpy(data_ptr, buffer, room_left);
               data_ptr += room_left;
               room_left = 0;
               done = true;
               break;
            }
         } 
         
      }
      
      if (!done) {
         /*snd_pcm_drop(chandle);
         snd_pcm_unlink(chandle);
         snd_pcm_hw_free(chandle);*/
         
         frames_in = frames_out = in_max = 0;
         if (setparams_c(chandle, &latency) < 0) 
            break;
      }
   }

   return uint16_t (len - room_left);
}

long VoiceStreamer::readbuf(snd_pcm_t *handle, char *buf, long len, size_t *frames, 
      size_t *max) {
   long r;

   do {
      r = snd_pcm_readi(handle, buf, len);
   } while ( r == -EAGAIN);

   if (r > 0) {
      *frames += r;
      if ((long) *max < r)
         *max = r;
   }

   return r;
}

long VoiceStreamer::writebuf(snd_pcm_t *handle, char *buf, long len, size_t *frames) {
   long r;

   while (len > 0) {
      r = snd_pcm_writei(handle, buf, len);
      if (r == -EAGAIN)
         continue;
      else if (r < 0)
         return r;

      buf += r*4;
      len -= r;
      *frames += r;
   }

   return 0;
}
         
int VoiceStreamer::setparams_p(snd_pcm_t *phandle, int *bufsize) {
   int err, last_bufsize = *bufsize;
   snd_pcm_hw_params_t *pt_params;
   snd_pcm_hw_params_t *p_params;
   snd_pcm_sw_params_t *p_swparams;
   snd_pcm_uframes_t p_size, p_psize;
   unsigned int val;

   snd_pcm_hw_params_alloca(&p_params);
   snd_pcm_hw_params_alloca(&pt_params);
   snd_pcm_sw_params_alloca(&p_swparams);
   if ((err = setparams_stream(phandle, pt_params, "playback")) < 0) {
      printf("Unable to set parameters for playback stream: %s\n",
            snd_strerror(err));
      exit(EXIT_FAILURE);
   }

   bool again = true;
   if (buffer_size > 0) {
      *bufsize = buffer_size;
      again = false;
   }

   for (;; again = true) {
      if (again) {
         if (buffer_size > 0)
            return -1;
         if (last_bufsize == *bufsize)
            *bufsize +=4;
         last_bufsize = *bufsize;
         if (*bufsize > latency_max)
            return -1;
      }
      if ((err = setparams_bufsize(phandle, p_params, pt_params, *bufsize,
                  "playback")) < 0) {
         printf("Unable to set sw parameters for playback stream: %s\n",
               snd_strerror(err));
         exit(EXIT_FAILURE);
      }

      snd_pcm_hw_params_get_period_size(p_params, &p_psize, NULL);
      if (p_psize > (unsigned int) *bufsize)
         *bufsize = p_psize;

      snd_pcm_hw_params_get_buffer_size(p_params, &p_size);
      if (p_psize*4 < p_size) {
         snd_pcm_hw_params_get_periods_min(p_params, &val, NULL);
         if (val > 4) {
            printf("Playback device does not support 4 periods per buffer\n");
            exit(EXIT_FAILURE);
         }
         continue;
      }
      
      break;
   }

   if ((err = setparams_set(phandle, p_params, p_swparams, "playback")) < 0) {
      printf("Unable to set sw parameters for playback stream: %s\n", 
            snd_strerror(err));
      exit(EXIT_FAILURE);
   }

   fflush(stdout);
   return 0;

}

int VoiceStreamer::setparams_c(snd_pcm_t *chandle, int *bufsize) {
   int err, last_bufsize = *bufsize;
   snd_pcm_hw_params_t *ct_params; // templates w/ rate, format and channels
   snd_pcm_hw_params_t *c_params;
   snd_pcm_sw_params_t *c_swparams;
   snd_pcm_uframes_t c_size, c_psize;
   unsigned int val;

   snd_pcm_hw_params_alloca(&c_params);
   snd_pcm_hw_params_alloca(&ct_params);
   snd_pcm_sw_params_alloca(&c_swparams);
   if((err = setparams_stream(chandle, ct_params, "capture")) < 0) {
      printf("Unable to set parameters for capture stream: %s\n", 
            snd_strerror(err));
      exit(EXIT_FAILURE);
   }

   bool again = true;
   if (buffer_size > 0) {
      *bufsize = buffer_size;
      again = false;
   }

   for (;; again = true) {
      if (again) {
         if (buffer_size > 0)
            return -1;
         if (last_bufsize == *bufsize)
            *bufsize +=4;
         last_bufsize = *bufsize;
         if (*bufsize > latency_max)
            return -1;
      }
      if ((err = setparams_bufsize(chandle, c_params, ct_params, *bufsize,
                  "capture")) < 0) {
         printf("Unable to set sw parameters for capture stream: %s\n",
               snd_strerror(err));
         exit(EXIT_FAILURE);
      }

      snd_pcm_hw_params_get_period_size(c_params, &c_psize, NULL);
      if (c_psize > (unsigned int) *bufsize)
         *bufsize = c_psize;

      snd_pcm_hw_params_get_buffer_size(c_params, &c_size);
      if (c_psize*4 < c_size) {
         snd_pcm_hw_params_get_periods_min(c_params, &val, NULL);
         if (val > 4) {
            printf("Capture device does not support 4 periods per buffer\n");
            exit(EXIT_FAILURE);
         }
         continue;
      }
      
      break;
   }

   if ((err = setparams_set(chandle, c_params, c_swparams, "capture")) < 0) {
      printf("Unable to set sw parameters for capture stream: %s\n", 
            snd_strerror(err));
      exit(EXIT_FAILURE);
   }

   fflush(stdout);
   return 0;
}

int VoiceStreamer::setparams_stream(snd_pcm_t *handle, snd_pcm_hw_params_t *params, 
      const char *id) {
   int err;
   unsigned int rrate;

   if ((err = snd_pcm_hw_params_any(handle, params)) < 0) {
      printf("Broken configuration for %s PCM: no configs available: %s\n",
            snd_strerror(err), id);
      return err;
   }
   if ((err = snd_pcm_hw_params_set_rate_resample(handle, params, resample)) < 0) {
      printf("Resample setup failed for %s (val %i): %s\n", id, resample,
            snd_strerror(err));
      return err;
   }
   if ((err = snd_pcm_hw_params_set_access(handle, params,
               SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
      printf("Access type not available for %s: %s\n", id, snd_strerror(err));
      return err;
   }
   if ((err = snd_pcm_hw_params_set_format(handle, params, format)) < 0) {
      printf("Sample format not available for %s: %s\n", id, snd_strerror(err));
      return err;
   }
   if ((err = snd_pcm_hw_params_set_channels(handle, params, channels)) < 0) {
      printf("Channels count (%i) not available for %s: %s\n", channels, id,
            snd_strerror(err));
      return err;
   }
   rrate = rate;
   if ((err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0)) < 0) {
      printf("Rate %iHz not available for %s: %s\n", rate, id,
            snd_strerror(err));
      return err;
   }
   if ((int) rrate != rate) {
      printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
      return -EINVAL;
   }
   
   return 0;
}
  
int VoiceStreamer::setparams_bufsize(snd_pcm_t *handle, snd_pcm_hw_params_t *params,
      snd_pcm_hw_params_t *tparams, snd_pcm_uframes_t bufsize, const char *id) {
   int err;
   snd_pcm_uframes_t periodsize;

   snd_pcm_hw_params_copy(params, tparams);
   periodsize = bufsize*2;
   if ((err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
               &periodsize)) < 0) {
      printf("Unable to set buffer size %li for %s: %s\n", bufsize*2, id,
            snd_strerror(err));
      return err;
   }
   if (period_size > 0)
      periodsize = period_size;
   else
      periodsize /= 2;
   if ((err = snd_pcm_hw_params_set_period_size_near(handle, params,
               &periodsize, 0)) < 0) {
      printf("Unable to set period size %li for %s: %s\n", periodsize, id,
            snd_strerror(err));
      return err;
   }
   
   return 0;
}

int VoiceStreamer::setparams_set(snd_pcm_t *handle, snd_pcm_hw_params_t *params,
      snd_pcm_sw_params_t *swparams, const char *id) {
   int err;
   snd_pcm_uframes_t val;

   if ((err = snd_pcm_hw_params(handle, params)) < 0) {
      printf("Unable to set hw params for %s: %s\n", id, snd_strerror(err));
      return err;
   }
   if ((err = snd_pcm_sw_params_current(handle, swparams)) < 0) {
      printf("Unable to determine current swparams for %s: %s\n", id,
            snd_strerror(err));
      return err;
   }
   if ((err = snd_pcm_sw_params_set_start_threshold(handle, swparams, 
               0x7fffffff)) < 0) {
      printf("Unable to set start threshold mode for %s: %s\n", id,
            snd_strerror(err));
      return err;
   }
   val = 4;
   if ((err = snd_pcm_sw_params_set_avail_min(handle, swparams, val)) < 0) {
      printf("Unable to set sw params for %s: %s\n", id, snd_strerror(err));
      return err;
   }

   return 0;
}
