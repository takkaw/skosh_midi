/* skoshmidi.h is single header C11 MIDI 1.0 cross platform library. */

#include <stdint.h>

#include <alsa/asoundlib.h>

typedef struct {
    uint8_t size;
    uint8_t data[3];
} skm_msg;

typedef struct {
    void* handle;
} skm_port;

int32_t skm_port_count(void);
int32_t skm_port_name(uint32_t port, uint32_t buflen, char* namebuf);
int32_t skm_port_open(uint32_t port, skm_port* p);
int32_t skm_port_close(skm_port* p);
int32_t skm_port_recv(skm_port* p, skm_msg* msg);

static int32_t skm_port_find(snd_seq_t** seq_out, int32_t index, snd_seq_port_info_t* port_info_out)
{
    snd_seq_t* seq;
    if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
        return -1;
    }
    *seq_out = seq;

    snd_seq_client_info_t* client_info;
    snd_seq_client_info_alloca(&client_info);

    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);

    int32_t count = 0;
    unsigned int req = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;

    snd_seq_client_info_set_client(client_info, -1);
    while (snd_seq_query_next_client(seq, client_info) == 0) {
        int client_id = snd_seq_client_info_get_client(client_info);
        snd_seq_port_info_set_client(port_info, client_id);

        snd_seq_port_info_set_port(port_info, -1);
        while (snd_seq_query_next_port(seq, port_info) == 0) {
            unsigned int cap = snd_seq_port_info_get_capability(port_info);
            if ((cap & req) == req) {
                if (count == index) {
                    if (port_info_out) {
                        snd_seq_port_info_copy(port_info_out, port_info);
                    }
                    return count;
                }
                count++;
            }
        }
    }

    return count;
}

int32_t skm_port_count(void)
{
    snd_seq_t* seq = NULL;
    int32_t count = skm_port_find(&seq, -1, NULL);
    if (seq) {
        snd_seq_close(seq);
    }
    return count;
}
