#include "util.h"
#include "cache_utils.c"

struct state {
    char *buffer;
    int step;
    int iterations;
    int wait_cycles_between_measurements;
    bool debug;
};

bool detect_bit(struct state *state) {
    unsigned long long misses = 0;
    unsigned long long hits = 0;

    for (int k = 0; k < state->iterations; k++) {
        for (int i = 0; i < CACHE_WAYS; i++) {
            CYCLES time = measure_one_block_access_time((ADDR_PTR) (state->buffer + state->step * i));

            // printf("%" PRIx64 "\n", (uint64_t) (buffer + two_o_s * i));
            // printf("Time %d: %d\n", i, time);

            if (time > 80) {
                misses++;
            } else {
                hits++;
            }
        }

        // Busy loop to give time to the sender
        // The number of cycles of this loop are currently
        // decided by the program first argument.
        for (int junk = 0; junk < state->wait_cycles_between_measurements; junk++) {}
    }

    if (state->debug) {
        printf("Hits: %lld\n", hits);
        printf("Misses: %lld\n", misses);
    }

    // Consider a 1 when more than 1/20 of the accesses were cache misses
    return (misses > (state->iterations * CACHE_WAYS) / 20);
}

void init_state(struct state *state, int argc, char **argv) {
    int n = CACHE_WAYS;
    int o = 6;              // log_2(64), where 64 is the line size
    int s = 6;              // log_2(64), where 64 is the number of cache sets

    int two_o_s = ipow(2, o + s);       // 4096
    int b = n * two_o_s;                // 32,768

    state->step = two_o_s;
    state->buffer = malloc((size_t) b);

    // Set some default values; need to be tuned up
    state->iterations = 100;
    state->wait_cycles_between_measurements = 45000;

    int option;
    while ((option = getopt(argc, argv, "di:w:")) != -1) {
        switch (option) {
            case 'd':
                state->debug = true;
                break;
            case 'i':
                state->iterations = atoi(optarg);
                break;
            case 'w':
                state->wait_cycles_between_measurements = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(1);
            default:
                exit(1);
        }
    }
}

int main(int argc, char **argv) {

    // Setup code
    struct state state;
    init_state(&state, argc, argv);

    printf("Press enter to measure the time to read an address.\n");

    int receiving = 1;
    while (receiving) {
        char text_buf[128];
        fgets(text_buf, sizeof(text_buf), stdin);

        detect_bit(&state);

        // Put your covert channel code here
    }

    printf("Receiver finished.\n");
    return 0;
}