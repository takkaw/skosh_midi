/* skoshmidi.h is single header C11 MIDI 1.0 cross platform library. */

#include <stdint.h>

typedef struct {
    uint8_t size;
    uint8_t data[3];
} skm_msg;

typedef struct {
    void*   handle;
} skm_port;

int32_t skm_port_count(void);
int32_t skm_port_name(uint32_t port, uint32_t buflen, char* namebuf);
int32_t skm_port_open(uint32_t port, skm_port* p);
int32_t skm_port_close(skm_port* p);
int32_t skm_port_recv(skm_port* p, skm_msg* msg);
