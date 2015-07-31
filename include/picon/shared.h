#ifndef __SHARED_H__
#define __SHARED_H__




#ifdef __cplusplus
extern "C" {
#endif





#include <unistd.h>
#include <stdint.h>

#include <picon/log.h>




#define likely(x)              __builtin_expect(!!(x), 1)
#define unlikely(x)            __builtin_expect(!!(x), 0)





#define ENV_FD_MONITOR_TO_LOADING "CFI_MONITOR_TO_LOADING"
#define ENV_FD_LOADING_TO_MONITOR "CFI_LOADING_TO_MONITOR"
#define ENV_FD_CLIENT_TO_MONITOR "CFI_CLIENT_TO_MONITOR"
#define ENV_FD_MONITOR_TO_CLIENT "CFI_MONITOR_TO_CLIENT"










typedef enum {
  CFI_BEFORE_JUMP,
  CFI_AFTER_JUMP,
  CFI_CALL,
  CFI_ENTER,
  CFI_EXIT,
  CFI_RETURNED,
} client_event;



typedef uint16_t module_id;
typedef uint32_t function_id;
typedef uint32_t block_id;




typedef uint64_t module_function_ids;

#define MODULE_FUNCTION_GET_MODULE(r) ((module_id)((r)>>32))
#define MODULE_FUNCTION_GET_FUNCTION(r) ((function_id)((r) & 0xffffffff))

#define MODULE_FUNCTION(m,f) ((((uint64_t)(m)) << 32) | (((uint64_t)(f)) & 0xffffffff))


typedef struct {
  client_event event : 8;
  module_id module;
  function_id function;
  block_id block;
} __attribute__((packed)) client_signal;


#define READ_CLIENT_SIGNAL(fd,dst,err)                          \
  do {                                                          \
    LOG_DEBUG_MONITOR("waiting for client signal\n");           \
    if(unlikely(read((fd),&(dst),sizeof(client_signal)) !=      \
                sizeof(client_signal))) {                       \
      (err) = 1;                                                \
    }                                                           \
  } while(0)


#define WRITE_CLIENT_SIGNAL(fd,dst,err)                         \
  do {                                                          \
    LOG_DEBUG_CLIENT("sending signal\n");                       \
    if(unlikely(write((fd),&(dst),sizeof(client_signal)) !=     \
                sizeof(client_signal))) {                       \
      (err) = 1;                                                \
    }                                                           \
  } while(0)










typedef enum {
  CFI_OK = 0x42,
} monitor_event;


typedef struct {
  monitor_event event : 8;
} __attribute__((packed)) monitor_answer;


#define READ_MONITOR_ANSWER(fd,dst,err)                         \
  do {                                                          \
    LOG_DEBUG_CLIENT("waiting for answer\n");                   \
    if(unlikely(read((fd),&(dst),sizeof(monitor_answer)) !=     \
                sizeof(monitor_answer))) {                      \
      (err) = 1;                                                \
    }                                                           \
  } while(0)


#define WRITE_MONITOR_ANSWER(fd,dst,err)                        \
  do {                                                          \
    LOG_DEBUG_MONITOR("sending answer\n");                      \
    if(unlikely(write((fd),&(dst),sizeof(monitor_answer)) !=    \
                sizeof(monitor_answer))) {                      \
      (err) = 1;                                                \
    }                                                           \
  } while(0)









typedef enum {
        FCT_INTERNAL,
        FCT_EXTERNAL,
        FCT_INIT,
        FCT_FINI,
        FCT_MAIN,
} fct_flag_t;

typedef struct {
        function_id        f_id;
        fct_flag_t         f_flags;
        const char * const f_name;
} __attribute((packed)) fctid_t;

typedef struct {
        function_id        id;
        uint32_t           num_callees;
        const char * const callees;
} __attribute((packed)) callgraph_entry_t;



/* PROTOCOL DEFINITION
 * for each message, this is the format of the 'value' field of the loading packet
 */

/* CFI_LOADING_MODULE_BEGIN */
typedef struct {
        uint32_t   num_fcts;
} __attribute__((packed)) msg_begin_t;

/* CFI_LOADING_SECTION_FUNCTION_ID */
typedef struct {
  function_id        f_id;
  fct_flag_t         f_flags;
  char               f_name[0]; /* NULL-terminated string */
} __attribute__((packed)) msg_fct_t;

typedef struct {
  uint32_t  num_fcts;
  msg_fct_t fcts[0]; /* list of (non-fixed size) msg_fct_t */
} __attribute__((packed)) msg_fct_l;

/* CFI_LOADING_SECTION_FUNCTION_TRANSITION */
typedef struct {
  function_id        f_id;
  uint32_t           f_num_calls;
  function_id        f_calls[0]; /* array of size f_num_calls */
} __attribute__((packed)) msg_fct_trans_t;

typedef struct {
  uint32_t        num_fct_trans;
  msg_fct_trans_t fct_trans[0]; /* list of (non-fixed size) msg_fct_trans_t */
} __attribute__((packed)) msg_fct_trans_l;

/* CFI_LOADING_SECTION_BLOCK_TRANSITION */
typedef struct {
  block_id           b_id;
  uint32_t           b_num_succ;
  block_id           b_succ[0]; /* array of size b_num_succ */
} __attribute__((packed)) msg_block_trans_t;

typedef struct {
  function_id        f_id;
  uint32_t           num_block;       /* total number of blocks in function */
  uint32_t           num_block_trans; /* number of msg_block_trans_t elements */
  msg_block_trans_t  block_trans[0];  /* list of (non-fixed size) msg_block_trans_t */
} __attribute__((packed)) msg_block_trans_l;

/* CFI_LOADING_SECTION_BLOCK_IPD */
typedef struct {
  function_id        f_id;
  block_id           b_id;
  block_id           b_ipd_id;
  block_id           b_last_id;
} __attribute__((packed)) msg_block_ipd_t;

typedef struct {
  uint32_t           num_ipd;       /* total number of ipd in function */
  msg_block_ipd_t    block_ipd[0];  /* list of (non-fixed size) msg_block_ipd_t */
} __attribute__((packed)) msg_block_ipd_l;







typedef enum {
  CFI_LOADING_MODULE_BEGIN,
  CFI_LOADING_SECTION_FUNCTION_ID,
  CFI_LOADING_SECTION_FUNCTION_TRANSITION,
  CFI_LOADING_SECTION_BLOCK_TRANSITION,
  CFI_LOADING_SECTION_BLOCK_IPD,
  CFI_LOADING_MODULE_END,
  CFI_LOADING_TERMINATED,
} loading_event;

typedef union {
  msg_begin_t begin;
  msg_fct_l fct;
  msg_fct_trans_l fct_trans;
  msg_block_trans_l block_trans;
  msg_block_ipd_l block_ipd;
} loading_packet_value;

typedef struct {
  loading_event event;
  uint32_t size;
  loading_packet_value *value;
} loading_packet;



#define READ_LOADING_PACKET(fd,dst,err)                         \
  do {                                                          \
    LOG_DEBUG_MONITOR("waiting for loading packet\n");          \
    (dst).size = 0;                                             \
    (dst).value = NULL;                                         \
    if(read((fd),&((dst).event),sizeof(loading_event)) !=       \
       sizeof(loading_event)) {                                 \
      (err) = 1;                                                \
    } else if(read((fd),&((dst).size),sizeof(uint32_t)) !=      \
              sizeof(uint32_t)) {                               \
      (err) = 2;                                                \
    } else if((dst).size == 0) {                                \
    } else if(((dst).value = calloc((dst).size, sizeof(char))) == NULL) { \
      (err) = 3;                                                \
    } else if(read((fd),(dst).value,(dst).size) !=              \
              (dst).size) {                                     \
      (dst).size = 0;                                           \
      free((dst).value);                                        \
      (err) = 4;                                                \
    }                                                           \
  } while(0)


#define FREE_LOADING_PACKET(pkt)                \
  do {                                          \
    if((pkt).value) {                           \
      free((pkt).value);                        \
      (pkt).value = NULL;                       \
    }                                           \
  } while(0)


#define WRITE_LOADING_PACKET(fd,src,err)                        \
  do {                                                          \
    LOG_DEBUG_CLIENT("sending loading packet (size = %u)\n", (src).size); \
    if(write((fd),&((src).event),sizeof(loading_event)) !=      \
       sizeof(loading_event)) {                                 \
      (err) = 1;                                                \
    } else if(write((fd),&((src).size),sizeof(uint32_t)) !=     \
              sizeof(uint32_t)) {                               \
      (err) = 2;                                                \
    } else if((src).size) {                                     \
      if(write((fd),(src).value,(src).size) != (src).size) {    \
        (err) = 3;                                              \
      }                                                         \
    }                                                           \
  } while(0)






typedef struct {
  module_id id;
} monitor_to_loading_packet;


#define READ_MONITOR_TO_LOADING_PACKET(fd,dst,err)                      \
  do {                                                                  \
    LOG_DEBUG_CLIENT("waiting for monitor to loading packet\n");        \
    if(read((fd),&((dst).id),sizeof(module_id)) != sizeof(module_id)) { \
      (err) = 1;                                                        \
    }                                                                   \
  } while(0)

#define WRITE_MONITOR_TO_LOADING_PACKET(fd,src,err)                     \
  do {                                                                  \
    LOG_DEBUG_MONITOR("sending monitor to loading packet\n");           \
    if(write((fd),&((src).id),sizeof(module_id)) != sizeof(module_id)) { \
      (err) = 1;                                                        \
    }                                                                   \
  } while(0)






#ifdef __cplusplus
}
#endif




#endif /* __SHARED_H__ */
