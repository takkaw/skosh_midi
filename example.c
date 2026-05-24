#include "skoshmidi.h"
#include <stdio.h>

#define SKM_NAME_BUF (100)
int main(void)
{
    int port_count = 0;
    char port_name[SKM_NAME_BUF];
    port_count = skm_port_count();
    printf("port_count = %d\n", port_count);
    for (int i = 0; i < port_count; i++) {
        int result = skm_port_name(i, port_name, SKM_NAME_BUF);
        if (result >= 0) {
            printf("port %d: %s\n", i, port_name);
        }
    }
}
