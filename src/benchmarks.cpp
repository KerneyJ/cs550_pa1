#include "stdio.h"
#include <bits/chrono.h>
#include <chrono>
#include "benchmarks.hpp"
#include "peer.hpp"

using namespace std::chrono;

void timed_search(int num_searches);
void timed_register(int num_searches);

void run_benchmark(int id, std::time_t start) {
	printf("Starting at %lu\n", start);
    switch(id) {
        case 1:
            return timed_search(10000);
        default:
            printf("Unknown benchmark id.\n");
    }
}

void timed_search(int num_searches) {
    time_point<high_resolution_clock> t1, t2;

    printf("Registering %d files...\n", num_searches);
    for(int i = 0; i < num_searches; i++) {
        char filename[256] = {0};
        sprintf(filename, "file-%d.data", i);
        register_file(filename);
    }

    t1 = high_resolution_clock::now();

    printf("Timing runtime of %d searches...\n", num_searches);

    for(int i = 0; i < num_searches; i++) {
        char filename[256] = {0};
        sprintf(filename, "file-%d.data", i);
        search_for_file(filename);
    }

    t2 = high_resolution_clock::now();
    auto elapsed_time_us = duration_cast<microseconds>(t2 - t1);

    printf("Searched for %d files in %ldus \n", num_searches, elapsed_time_us.count());

}
