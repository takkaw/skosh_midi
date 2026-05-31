#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define SKOSH_MIDI_IMPLEMENTATION
#include "skosh_midi.h"

#define SKOSH_MIDI_NAME_BUF (100)

bool exit_flg = 0;
static void sigint_handler(int sig)
{
    (void)sig;
    exit_flg = 1;
}

int main(int argc, const char* argv[])
{
    int port_no = -1;
    int mode = -1;
    char* endptr;
    int out_port_count = skosh_midi_port_count(SKOSH_MIDI_OUT);
    int in_port_count = skosh_midi_port_count(SKOSH_MIDI_IN);
    skosh_midi_port p = {0};

    if (argc == 3) {
        port_no = (int)strtol(argv[2], &endptr, 10);
        if ((strcmp(argv[1], "out") == 0) && (port_no < out_port_count)) mode = SKOSH_MIDI_OUT;
        if ((strcmp(argv[1], "in") == 0) && (port_no < in_port_count)) mode = SKOSH_MIDI_IN;
    }

    (void)signal(SIGINT, sigint_handler);

    switch (mode) {
    case SKOSH_MIDI_OUT: /* Send */
        bool flag = 0;
        skosh_midi_msg out_msg[2] = {{.size = 3, .data = {0x90, 0x3C, 0x7F}},
                                     {.size = 3, .data = {0x80, 0x3C, 0x00}}};
        if (skosh_midi_port_open(SKOSH_MIDI_OUT, port_no, &p) == 0) {
            while (1) {
                skosh_midi_port_send(&p, &out_msg[flag]);
                for (int i = 0; i < out_msg[flag].size; i++) {
                    printf("%02X ", out_msg[flag].data[i]);
                }
                flag = !flag; /* toggle */
                if (getchar() == EOF) break;
                if (exit_flg) break;
            }
            skosh_midi_port_close(&p);
        }
        break;
    case SKOSH_MIDI_IN: /* Recv */
        if (skosh_midi_port_open(SKOSH_MIDI_IN, port_no, &p) == 0) {
            skosh_midi_msg in_msg = {0};
            while (1) {
                if (exit_flg) break;
                if (skosh_midi_port_recv(&p, &in_msg) == 0) {
                    for (int i = 0; i < in_msg.size; i++) {
                        printf("%02X ", in_msg.data[i]);
                    }
                    printf("\n");
                }
            }
            skosh_midi_port_close(&p);
        }
        break;
    default:
        printf("skosh_midi version %d.%d.%d\n", (SKOSH_MIDI_VERSION >> 16) & 0xFF,
               (SKOSH_MIDI_VERSION >> 8) & 0xFF, SKOSH_MIDI_VERSION & 0xFF);
        printf("usage: skoshi_midi_example in|out <port_num>\n\n");

        char port_name[SKOSH_MIDI_NAME_BUF];
        printf("In port_count = %d\n", in_port_count);
        for (int i = 0; i < in_port_count; i++) {
            int result = skosh_midi_port_name(SKOSH_MIDI_IN, i, port_name, SKOSH_MIDI_NAME_BUF);
            if (result >= 0) {
                printf("  port %d: %s\n", i, port_name);
            }
        }
        printf("Out port_count = %d\n", out_port_count);
        for (int i = 0; i < out_port_count; i++) {
            int result = skosh_midi_port_name(SKOSH_MIDI_OUT, i, port_name, SKOSH_MIDI_NAME_BUF);
            if (result >= 0) {
                printf("  port %d: %s\n", i, port_name);
            }
        }
        break;
    }

    return 0;
}
