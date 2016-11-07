#define IPC_DATA_SIZE 56
//#define IPC_DATA_SIZE 80

#define  IPC_ALERT_DEBUG 1


#if IPC_ALERT_DEBUG
typedef struct {
  uint32_t meta;
  uint32_t data;
} alert_checksum_t;

#endif

typedef struct {
  char            data[IPC_DATA_SIZE];
  time_t          time_sent;
#if IPC_ALERT_DEBUG
  ngx_int_t       num;
  alert_checksum_t checksum;
  int16_t         dst_slot;
#endif
  int16_t         src_slot;
  uint16_t        worker_generation;
  uint8_t         code;
} ipc_alert_t;

#define IPC_WRITEBUF_SIZE 32

typedef struct ipc_writebuf_overflow_s ipc_writebuf_overflow_t;
struct ipc_writebuf_overflow_s {
  ipc_alert_t               alert;
  ipc_writebuf_overflow_t  *next;
};

typedef struct ipc_writebuf_s ipc_writebuf_t;
struct ipc_writebuf_s {
  //a ring buffer with a linked-list overflow for writing alerts, 
  uint16_t                  n;
  uint16_t                  first;
  int32_t                   overflow_n;
  ipc_writebuf_overflow_t  *overflow_first;
  ipc_writebuf_overflow_t  *overflow_last;
  ipc_alert_t               alerts[IPC_WRITEBUF_SIZE];
}; //ipc_writebuf_t

typedef struct ipc_s ipc_t;

typedef struct {
  ipc_t                 *ipc; //useful for write events
  ngx_socket_t           pipe[2];
  ngx_connection_t      *c;
  ipc_writebuf_t         wbuf;
  unsigned               active:1;
} ipc_process_t;

struct ipc_s {
  const char            *name;
  
  ipc_process_t         process[NGX_MAX_PROCESSES];
  
  void                  (*handler)(ngx_int_t, ngx_uint_t, void*);
}; //ipc_t

ngx_int_t ipc_init(ipc_t *ipc);
ngx_int_t ipc_open(ipc_t *ipc, ngx_cycle_t *cycle, ngx_int_t workers, void (*slot_callback)(int slot, int worker));
ngx_int_t ipc_set_handler(ipc_t *ipc, void (*alert_handler)(ngx_int_t, ngx_uint_t , void *data));
ngx_int_t ipc_register_worker(ipc_t *ipc, ngx_cycle_t *cycle);
ngx_int_t ipc_close(ipc_t *ipc, ngx_cycle_t *cycle);

ngx_int_t ipc_alert(ipc_t *ipc, ngx_int_t slot, ngx_uint_t code,  void *data, size_t data_size);
