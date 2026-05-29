#include "skosh_midi.h"
#include <stdio.h>

#define SKOSH_MIDI_NAME_BUF (100)
#define SKOSH_MIDI_EXAMPLE_PORT (1)
int main(void)
{
    int port_count = 0;
    char port_name[SKOSH_MIDI_NAME_BUF];
    port_count = skosh_midi_port_count();
    printf("port_count = %d\n", port_count);
    for (int i = 0; i < port_count; i++) {
        int result = skosh_midi_port_name(i, port_name, SKOSH_MIDI_NAME_BUF);
        if (result >= 0) {
            printf("port %d: %s\n", i, port_name);
        }
    }

    skosh_midi_port p = {0};
    if (skosh_midi_port_open(SKOSH_MIDI_EXAMPLE_PORT, &p) == 0) {
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
}
