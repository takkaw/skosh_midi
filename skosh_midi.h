/* skosh_midi.h is single header C11 MIDI 1.0 cross platform library. */

#include <stdint.h>

#include <alsa/asoundlib.h>

#define SKOSH_MIDI_OUT 0 /* O looks like 0 */
#define SKOSH_MIDI_IN 1  /* I looks like 1 */
#define SKOSH_MIDI_MSG_SIZE 3

typedef struct {
    uint8_t size;
    uint8_t data[SKOSH_MIDI_MSG_SIZE];
} skosh_midi_msg;

typedef struct {
    snd_seq_t* seq;
    int port_id;
    snd_seq_port_subscribe_t* sub;
    snd_midi_event_t* midi_ev;
} skosh_midi_port;

int32_t skosh_midi_port_count(int8_t dir);
int32_t skosh_midi_port_name(int8_t dir, int32_t port, char* namebuf, size_t buflen);
int32_t skosh_midi_port_open(int8_t dir, int32_t port, skosh_midi_port* p);
int32_t skosh_midi_port_close(skosh_midi_port* p);
int32_t skosh_midi_port_recv(skosh_midi_port* p, skosh_midi_msg* msg);
int32_t skosh_midi_port_send(skosh_midi_port* p, const skosh_midi_msg* msg);

static int32_t skosh_midi_port_find(int8_t dir, snd_seq_t** seq_out, int32_t index,
                                    snd_seq_port_info_t* port_info_out)
{
    if ((dir < 0) || (dir > 1)) return -1;

    snd_seq_t* seq;
    if (snd_seq_open(&seq, "default", dir ? SND_SEQ_OPEN_INPUT : SND_SEQ_OPEN_OUTPUT, 0) < 0)
        return -1;

    snd_seq_client_info_t* client_info;
    snd_seq_client_info_alloca(&client_info);
    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);

    int32_t count = 0;
    unsigned int req = dir ? (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)
                           : (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE);

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

int32_t skosh_midi_port_count(int8_t dir) { return skosh_midi_port_find(dir, NULL, -1, NULL); }

int32_t skosh_midi_port_name(int8_t dir, int32_t port, char* namebuf, size_t buflen)
{
    int32_t result = -1;
    if (!namebuf || (buflen == 0) || (dir < 0) || (dir > 1)) return result;

    snd_seq_t* seq = NULL;
    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);
    if (skosh_midi_port_find(dir, &seq, port, port_info) == port) {
        (void)snprintf(namebuf, buflen, "%s", snd_seq_port_info_get_name(port_info));
        result = 0;
    }
    if (seq) snd_seq_close(seq);
    return result;
}

int32_t skosh_midi_port_open(int8_t dir, int32_t port, skosh_midi_port* p)
{
    int32_t result = -1;
    if (!p || (dir < 0) || (dir > 1)) return -1;

    snd_seq_t* seq = NULL;
    int port_id = -1;
    snd_seq_port_subscribe_t* sub = NULL;
    snd_midi_event_t* midi_ev = NULL;

    snd_seq_port_info_t* port_info;
    snd_seq_port_info_alloca(&port_info);

    do {
        if (skosh_midi_port_find(dir, &seq, port, port_info) != port) break;
        port_id =
            snd_seq_create_simple_port(seq, "skosh_midi",
                                       dir ? SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE
                                           : SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                                       SND_SEQ_PORT_TYPE_APPLICATION);
        if (port_id < 0) break;
        if (snd_seq_port_subscribe_malloc(&sub) < 0) break;
        if (snd_midi_event_new(dir ? 0 : SKOSH_MIDI_MSG_SIZE, &midi_ev) < 0) break;
        snd_midi_event_no_status(midi_ev, 1);

        snd_seq_addr_t addr[2] = {
            {.client = (unsigned char)snd_seq_client_id(seq), .port = (unsigned char)port_id},
            {.client = (unsigned char)snd_seq_port_info_get_client(port_info),
             .port = (unsigned char)snd_seq_port_info_get_port(port_info)}};
        snd_seq_port_subscribe_set_sender(sub, &addr[dir]);
        snd_seq_port_subscribe_set_dest(sub, &addr[1 - dir]);
        if (snd_seq_subscribe_port(seq, sub) < 0) break;
        *p = (skosh_midi_port){.seq = seq, .port_id = port_id, .sub = sub, .midi_ev = midi_ev};
        result = 0;
    } while (0);

    if (result) {
        if (midi_ev) snd_midi_event_free(midi_ev);
        if (sub) snd_seq_port_subscribe_free(sub);
        if (seq && port_id >= 0) snd_seq_delete_simple_port(seq, port_id);
        if (seq) snd_seq_close(seq);
    }
    return result;
}

int32_t skosh_midi_port_close(skosh_midi_port* p)
{
    if (!p) return -1;
    snd_seq_unsubscribe_port(p->seq, p->sub);
    snd_seq_port_subscribe_free(p->sub);
    snd_midi_event_free(p->midi_ev);
    snd_seq_delete_simple_port(p->seq, p->port_id);
    snd_seq_close(p->seq);
    *p = (skosh_midi_port){.seq = NULL, .port_id = -1, .sub = NULL, .midi_ev = NULL};
    return 0;
}

int32_t skosh_midi_port_recv(skosh_midi_port* p, skosh_midi_msg* msg)
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

int32_t skosh_midi_port_send(skosh_midi_port* p, const skosh_midi_msg* msg)
{
    if (!p || !p->seq || !msg || (msg->size == 0) || (msg->size > 3)) return -1;

    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_midi_event_init(p->midi_ev);
    if (snd_midi_event_encode(p->midi_ev, msg->data, (long)msg->size, &ev) <= 0) return -1;

    snd_seq_ev_set_source(&ev, (unsigned char)p->port_id);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    if (snd_seq_event_output(p->seq, &ev) < 0) return -1;
    if (snd_seq_drain_output(p->seq) < 0) return -1;

    return 0;
}
