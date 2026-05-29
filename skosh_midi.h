/* skosh_midi.h is single header C11 MIDI 1.0 cross platform library. */

#include <stdint.h>

#include <alsa/asoundlib.h>

typedef struct {
    uint8_t size;
    uint8_t data[3];
} skm_msg;

typedef struct {
    snd_seq_t* seq;
    int port_id;
    snd_seq_port_subscribe_t* sub;
    snd_midi_event_t* midi_ev;
} skm_port;

int32_t skm_port_count(void);
int32_t skm_port_name(int32_t port, char* namebuf, size_t buflen);
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
            if (seq_out && (count == index)) {
                *seq_out = seq;
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

int32_t skm_port_name(int32_t port, char* namebuf, size_t buflen)
{
    int32_t result = -1;
    if (!namebuf || buflen == 0) return result;

    snd_seq_t* seq = NULL;
    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);
    if (skm_port_find(&seq, port, port_info) == port) {
        snprintf(namebuf, buflen, "%s", snd_seq_port_info_get_name(port_info));
        result = 0;
    }
    if (seq) snd_seq_close(seq);
    return result;
}

int32_t skm_port_open(int32_t port, skm_port* p)
{
    int32_t result = -1;
    if (!p) return result;

    snd_seq_t* seq = NULL;
    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);

    if (skm_port_find(&seq, port, port_info) == port) {
        int port_id = snd_seq_create_simple_port(
            seq, "skosh_midi", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION);
        if (port_id >= 0) {
            snd_seq_port_subscribe_t* sub;
            if (snd_seq_port_subscribe_malloc(&sub) >= 0) {
                snd_midi_event_t* midi_ev;
                if (snd_midi_event_new(0, &midi_ev) >= 0) {
                    snd_midi_event_no_status(midi_ev, 1);
                    snd_seq_addr_t sender, dest;
                    sender.client = (unsigned char)snd_seq_port_info_get_client(port_info);
                    sender.port = (unsigned char)snd_seq_port_info_get_port(port_info);
                    dest.client = (unsigned char)snd_seq_client_id(seq);
                    dest.port = (unsigned char)port_id;
                    snd_seq_port_subscribe_set_sender(sub, &sender);
                    snd_seq_port_subscribe_set_dest(sub, &dest);
                    if (snd_seq_subscribe_port(seq, sub) >= 0) {
                        p->seq = seq;
                        p->port_id = port_id;
                        p->sub = sub;
                        p->midi_ev = midi_ev;
                        result = 0;
                    }
                    if (result != 0) snd_midi_event_free(midi_ev);
                }
                if (result != 0) snd_seq_port_subscribe_free(sub);
            }
            if (result != 0) snd_seq_delete_simple_port(seq, port_id);
        }
        if (result != 0) snd_seq_close(seq);
    }
    return result;
}

int32_t skm_port_close(skm_port* p)
{
    if (!p) return -1;
    snd_seq_unsubscribe_port(p->seq, p->sub);
    snd_seq_port_subscribe_free(p->sub);
    snd_midi_event_free(p->midi_ev);
    snd_seq_delete_simple_port(p->seq, p->port_id);
    snd_seq_close(p->seq);
    p->seq = NULL;
    p->sub = NULL;
    p->port_id = -1;
    return 0;
}

int32_t skm_port_recv(skm_port* p, skm_msg* msg)
{
    if (!p || !p->seq || !msg) return -1;
    if (snd_seq_event_input_pending(p->seq, 1) == 0) return -1;

    snd_seq_event_t* ev;
    if (snd_seq_event_input(p->seq, &ev) < 0) return -1;

    uint8_t buf[sizeof(msg->data)];
    long size = snd_midi_event_decode(p->midi_ev, buf, sizeof(buf), ev);
    if (size <= 0 || size > 3) return -1;

    msg->size = (uint8_t)size;
    msg->data[0] = buf[0];
    msg->data[1] = (size > 1) ? buf[1] : 0;
    msg->data[2] = (size > 2) ? buf[2] : 0;
    return 0;
}
