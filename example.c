#include <stdio.h>
#include "skoshmidi.h"

int main(void)
{
    int32_t port_count = 0;
    port_count = skm_port_count();
    printf("port_count = %d\n", port_count);
}
