/** Common to both server and client */

#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#define DEBUG // define when debugging

#define SOL_DCCP 269
#define MAX_DCCP_CONNECTION_BACK_LOG 5

#define BUF_LEN 1400
#define INIT_TIMEOUT 30 
#define USERNAME_LEN 20
#define PASSWORD_LEN 20

extern int select_call(int *socket_num, int numSockets, int seconds, int useconds);

enum PROTO_TYPE {DCCP_T, UDP_T, TCP_T};
enum PROTO_IO {SERVER_IO, CLIENT_IO};
enum REQUEST {INCOMING_REQ, OUTGOING_REQ, INVALID_REQ};

/*enum PTYPE {
   login,
   login_ack,
   login_failed,
   audio_data,
   status_updates,
   request_chat,
   answered_call,
   request_address
};*/
/////////// Packet Types ////////
#define LOGIN_T 0x1
#define LOGIN_ACK_T 0x2
#define LOGIN_FAILED_T 0x3
#define AUDIO_DATA_T 0x4
#define STATUS_UPDATES_T 0x5
#define REQUEST_CHAT_T 0x6
#define ANSWER_CALL_T 0x7
#define REQUEST_ADDRESS_T 0x8
////////////////////////////////

struct login_data {
   char username[USERNAME_LEN];
   char password[PASSWORD_LEN];
   uint16_t clientPort;
} __attribute__((__packed__));

struct friendList_data {
   char username[20];
   uint32_t uid;
} __attribute__((__packed__));

struct friend_list {
   friendList_data friendData;
   bool status;
} __attribute__((__packed__));

struct carFormat {
   uint32_t friendID;
} __attribute__((__packed__));

struct carOutFormat {
   uint32_t friendID;
   uint16_t portNum;
} __attribute__((__packed__)); // Client Address Request Outgoing

struct updateFormat {
   uint32_t friendID;
   uint8_t status;
} __attribute__((__packed__)); // Update of friend's online status

struct packet {
   uint32_t uid;
   //enum PTYPE type;
   uint8_t type;
   uint16_t dlength;
   uint8_t data[BUF_LEN];
} __attribute__((__packed__));

// XXX Debugging Macros
#define PRINT_PACKET(x) {\
   std::cerr << "***** Received a Packet *****\n"; \
   std::cerr << "\tuid: " << ntohl((x).uid) << std::endl; \
   std::cerr << "\ttype: "; \
   switch ((x).type) { \
      case LOGIN_T:           std::cerr << "login\n"; break; \
      case LOGIN_ACK_T:       std::cerr << "login ack\n"; break; \
      case LOGIN_FAILED_T:    std::cerr << "login failed\n"; break; \
      case AUDIO_DATA_T:      std::cerr << "audio data\n"; break; \
      case STATUS_UPDATES_T:  std::cerr << "status updates\n"; break; \
      case REQUEST_CHAT_T:    std::cerr << "request chat\n"; break; \
      case ANSWER_CALL_T:     std::cerr << "answer call\n"; break; \
      case REQUEST_ADDRESS_T: std::cerr << "request address\n"; break; \
      default: fprintf(stderr, "unrecognized type: 0x%hhx\n", (x).type); } \
   std::cerr << "\tdlength: " << ntohs((x).dlength) << std::endl; \
   std::cerr << "*****************************\n"; }

#endif
