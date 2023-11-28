#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "input.h"

int get_input_params(int arc, char *argv[], input_params *params) {
    if (arc != 5) {
        printf("Usage: %s <buffer_size> <num_producers> <num_consumers> <upper_limit>\n", argv[0]);
        return 1;
    }
    for (int i = 1; i < 5; i++) {
        unsigned long len = strlen(argv[i]);
        if (len != 0 && argv[i][0] == '-') {
            printf("Error: argument \"%s\" must be positive\n", argv[i]);
            return 1;
        }
    }

    params->buffer_size = strtoul(argv[1], NULL, 10);
    params->num_producers = strtoul(argv[2], NULL, 10);
    params->num_consumers = strtoul(argv[3], NULL, 10);
    params->upper_limit = (int)strtoul(argv[4], NULL, 10);

    return 0;
}
