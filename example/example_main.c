#include <signal.h>
#include <stdio.h>
#include <time.h>

#define SKOSH_MIDI_IMPLEMENTATION
#include "skosh_midi.h"

/* handle Ctrl-C and request termination */
static volatile sig_atomic_t exit_flg = 0;
static void sigint_handler(int sig)
{
    (void)sig;
    exit_flg = 1;
}

/* print usage with version */
static void usage(void)
{
    printf("skosh_midi version %d.%d.%d\n", (SKOSH_MIDI_VERSION >> 16) & 0xFF,
           (SKOSH_MIDI_VERSION >> 8) & 0xFF, SKOSH_MIDI_VERSION & 0xFF);
    printf("usage: skosh_midi_example in|out <port_num>\n");
    printf("Press Ctrl-C to exit\n");
    printf("Press Enter to toggle MIDI message in out mode.\n\n");
}

/* print MIDI in/out port list */
#define SKOSH_MIDI_NAME_BUF_SIZE (100)
static void midi_list(const int port_count[2])
{
    char port_name[SKOSH_MIDI_NAME_BUF_SIZE];
    printf("In port_count = %d\n", port_count[SKOSH_MIDI_IN]);
    for (int i = 0; i < port_count[SKOSH_MIDI_IN]; i++) {
        int result = skosh_midi_port_name(SKOSH_MIDI_IN, i, port_name, SKOSH_MIDI_NAME_BUF_SIZE);
        if (result >= 0) {
            printf("  port %d: %s\n", i, port_name);
        }
    }
    printf("Out port_count = %d\n", port_count[SKOSH_MIDI_OUT]);
    for (int i = 0; i < port_count[SKOSH_MIDI_OUT]; i++) {
        int result = skosh_midi_port_name(SKOSH_MIDI_OUT, i, port_name, SKOSH_MIDI_NAME_BUF_SIZE);
        if (result >= 0) {
            printf("  port %d: %s\n", i, port_name);
        }
    }
}

/* recv MIDI data and print it */
static void midi_recv(int port_no)
{
    static const struct timespec ts = {.tv_sec = 0, .tv_nsec = 1000000};
    skosh_midi_port port = {0};
    if (skosh_midi_port_open(SKOSH_MIDI_IN, port_no, &port) == 0) {
        skosh_midi_msg in_msg = {0};
        while (!exit_flg) {
            if (skosh_midi_port_recv(&port, &in_msg) == 0) {
                for (int i = 0; i < in_msg.size; i++) {
                    printf("%02X ", in_msg.data[i]);
                }
                printf("\n");
            }
            nanosleep(&ts, NULL);
        }
        skosh_midi_port_close(&port);
    }
}

/* send MIDI data toggle */
static void midi_send(int port_no)
{
    static const skosh_midi_msg out_msg[2] = {{.size = 3, .data = {0x90, 0x3C, 0x7F}},
                                              {.size = 3, .data = {0x80, 0x3C, 0x00}}};
    skosh_midi_port port = {0};
    int msg_no = 0;
    if (skosh_midi_port_open(SKOSH_MIDI_OUT, port_no, &port) == 0) {
        while (!exit_flg) {
            skosh_midi_port_send(&port, &out_msg[msg_no]);
            for (int i = 0; i < out_msg[msg_no].size; i++) {
                printf("%02X ", out_msg[msg_no].data[i]);
            }
            if (getchar() == EOF) break;
            msg_no = !msg_no; /* toggle */
        }
        skosh_midi_port_close(&port);
    }
}

int main(int argc, const char* argv[])
{
    int port_no = -1;
    int mode = -1;
    char* endptr;
    int port_count[2];
    port_count[SKOSH_MIDI_IN] = skosh_midi_port_count(SKOSH_MIDI_IN);
    port_count[SKOSH_MIDI_OUT] = skosh_midi_port_count(SKOSH_MIDI_OUT);

    if (argc == 3) {
        port_no = (int)strtol(argv[2], &endptr, 10);
        if ((strcmp(argv[1], "in") == 0) && (port_no < port_count[SKOSH_MIDI_IN]))
            mode = SKOSH_MIDI_IN;
        if ((strcmp(argv[1], "out") == 0) && (port_no < port_count[SKOSH_MIDI_OUT]))
            mode = SKOSH_MIDI_OUT;
    }

    (void)signal(SIGINT, sigint_handler);

    switch (mode) {
    case SKOSH_MIDI_IN:
        midi_recv(port_no);
        break;
    case SKOSH_MIDI_OUT:
        midi_send(port_no);
        break;
    default:
        usage();
        midi_list(port_count);
        break;
    }

    return 0;
}
