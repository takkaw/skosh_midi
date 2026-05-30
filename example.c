#include "skosh_midi.h"
#include <stdbool.h>
#include <stdio.h>

#define SKOSH_MIDI_NAME_BUF (100)
#define SKOSH_MIDI_EXAMPLE_PORT (1)
#define SKOSH_MIDI_EXAMPLE_MODE (SKOSH_MIDI_OUT)
int main(void)
{
    int port_count = 0;
    char port_name[SKOSH_MIDI_NAME_BUF];

    /* In */
    char dir = SKOSH_MIDI_IN;
    port_count = skosh_midi_port_count(dir);
    printf("In port_count = %d\n", port_count);
    for (int i = 0; i < port_count; i++) {
        int result = skosh_midi_port_name(dir, i, port_name, SKOSH_MIDI_NAME_BUF);
        if (result >= 0) {
            printf("In port %d: %s\n", i, port_name);
        }
    }

    /* Out */
    dir = SKOSH_MIDI_OUT;
    port_count = skosh_midi_port_count(dir);
    printf("Out port_count = %d\n", port_count);
    for (int i = 0; i < port_count; i++) {
        int result = skosh_midi_port_name(dir, i, port_name, SKOSH_MIDI_NAME_BUF);
        if (result >= 0) {
            printf("In port %d: %s\n", i, port_name);
        }
    }

#if SKOSH_MIDI_EXAMPLE_MODE
    /* Recv */
    dir = SKOSH_MIDI_IN;
    skosh_midi_port p = {0};
    if (skosh_midi_port_open(dir, SKOSH_MIDI_EXAMPLE_PORT, &p) == 0) {
        skosh_midi_msg msg = {0};
        while (1) {
            if (skosh_midi_port_recv(&p, &msg) == 0) {
                for (int i = 0; i < msg.size; i++) {
                    printf("%02X ", msg.data[i]);
                }
                printf("\n");
            }
        }
        skosh_midi_port_close(&p);
    }
#else
    /* Send */
    dir = SKOSH_MIDI_OUT;
    skosh_midi_port p = {0};
    bool flag = 0;
    skosh_midi_msg msg[2] = {{.size = 3, .data = {0x90, 0x3C, 0x7F}},
                             {.size = 3, .data = {0x80, 0x3C, 0x00}}};
    if (skosh_midi_port_open(dir, SKOSH_MIDI_EXAMPLE_PORT, &p) == 0) {
        while (1) {
            if (getchar() != EOF) {
                for (int i = 0; i < msg[flag].size; i++) {
                    printf("%02X ", msg[flag].data[i]);
                }
                skosh_midi_port_send(&p, &msg[flag]);
                flag = !flag; /* toggle */
            }
        }
        skosh_midi_port_close(&p);
    }
#endif

    return 0;
}
