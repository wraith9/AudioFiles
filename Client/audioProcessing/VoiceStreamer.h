/** This class handles streaming audio from your microphone.  It has the
 * ability to capture from the mic and also playback pre-recorded audio
 * to the speakers.  
 *
 * @author William McVicker
 */

#ifndef VOICESTREAMER_H
#define VOICESTREAMER_H

#include <stdint.h>

#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <list>
#include "../common.h"
#include "Semaphore.h"

#define MIN_PACKET_BUF 5 

class VoiceStreamer {
   public:
      VoiceStreamer();
      ~VoiceStreamer();

      bool initDevice();
      void addToBuffer(packet &dataPacket);
      uint16_t fillBuffer(char *data, int len);

   private:
      snd_pcm_format_t format;
      int rate, channels, buffer_size, period_size, latency;
      int latency_min, latency_max, loop_sec, resample;
      unsigned long loop_limit;
      char *buffer;
      size_t frames_in, frames_out, in_max;

      snd_output_t *output;
      snd_pcm_t *phandle, *chandle;  // the playback and capture handles
      std::list<packet> packetBuffer;
      
      bool startPlayback;
      boost::shared_ptr<boost::thread> m_playbackThread;
      boost::mutex mutex;
      Semaphore *nstored;

      int setparams_set(snd_pcm_t *handle, snd_pcm_hw_params_t *params,
            snd_pcm_sw_params_t *swparams, const char *id);
      int setparams_bufsize(snd_pcm_t *handle, snd_pcm_hw_params_t *params,
            snd_pcm_hw_params_t *tparams, snd_pcm_uframes_t bufsize, const char *id);
      int setparams_stream(snd_pcm_t *handle, snd_pcm_hw_params_t *params, 
            const char *id);
      int setparams_p(snd_pcm_t *phandle, int *bufsize);
      int setparams_c(snd_pcm_t *chandle, int *bufsize);
      long readbuf(snd_pcm_t *handle, char *buf, long len, size_t *frames, 
            size_t *max);
      long writebuf(snd_pcm_t *handle, char *buf, long len, size_t *frames);

      bool popAudioPacket(packet &audioPacket);
      void playbackAudio(); // the playbackThread function
};

#endif
