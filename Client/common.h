/** Common to both server and client */

#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#define SOL_DCCP 269
#define MAX_DCCP_CONNECTION_BACK_LOG 5

#define BUF_LEN 1400
#define INIT_TIMEOUT 30 

extern int select_call(int socket_num, int seconds, int useconds);

enum PROTO_TYPE {DCCP_T, UDP_T, TCP_T};
enum REQUEST {INCOMING_REQ, OUTGOING_REQ, INVALID_REQ};

enum PTYPE {
   login,
   login_ack,
   audio_data,
   status_updates,
   request_chat,
   answered_call
};

#pragma pack(push, 1) // set current alignment to 1 byte 

typedef struct {
   uint32_t uid;
   enum PTYPE type;
   uint16_t dlength;
   uint8_t data[BUF_LEN];
} packet;

#pragma pack(pop) // restore original alignment

#endif