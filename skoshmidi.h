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
int32_t skm_port_name(int32_t port, char* namebuf, uint32_t buflen);
int32_t skm_port_open(int32_t port, skm_port* p);
int32_t skm_port_close(skm_port* p);
int32_t skm_port_recv(skm_port* p, skm_msg* msg);

static int32_t skm_port_find(snd_seq_t** seq_out, int32_t index, snd_seq_port_info_t* port_info_out)
{
    snd_seq_t* seq;
    if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, 0) < 0) return -1;

    snd_seq_client_info_t* client_info;
    snd_seq_client_info_alloca(&client_info);

    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);

    int32_t count = 0;
    unsigned int req = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;

    snd_seq_client_info_set_client(client_info, -1);
    while (snd_seq_query_next_client(seq, client_info) == 0) {
        int client_id = snd_seq_client_info_get_client(client_info);
        if (client_id == snd_seq_client_id(seq)) continue;
        snd_seq_port_info_set_client(port_info, client_id);

        snd_seq_port_info_set_port(port_info, -1);
        while (snd_seq_query_next_port(seq, port_info) == 0) {
            unsigned int type = snd_seq_port_info_get_type(port_info);
            if (!(type & SND_SEQ_PORT_TYPE_MIDI_GENERIC)) continue;
            unsigned int cap = snd_seq_port_info_get_capability(port_info);
            if ((cap & req) != req) continue;
            if (count == index) {
                if (seq_out) *seq_out = seq;
                if (port_info_out) snd_seq_port_info_copy(port_info_out, port_info);
                return count;
            }
            count++;
        }
    }

    snd_seq_close(seq);

    return count;
}

int32_t skm_port_count(void) { return skm_port_find(NULL, -1, NULL); }

int32_t skm_port_name(int32_t port, char* namebuf, uint32_t buflen)
{
    snd_seq_t* seq = NULL;
    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);
    int32_t result = -1;
    if (skm_port_find(&seq, port, port_info) == port) {
        snprintf(namebuf, buflen, "%s", snd_seq_port_info_get_name(port_info));
        result = 0;
    }
    if (seq) snd_seq_close(seq);
    return result;
}
