#include <cstdlib>
#include <stdio.h>
#include <bits/chrono.h>
#include <chrono>
#include "benchmarks.hpp"
#include "peer.hpp"
#include "constants.hpp"

using namespace std::chrono;

static void register_dir(int vm_id, int num_files, char file_size);
static void timed_search(int vm_id, int num_files, char file_size);
static void timed_search_request(int vm_id, int num_files, char file_size);

static void create_file_name(char* filename, int vm_id, int file_number, char file_size) {
    sprintf(filename, "vm%d_1%c_%06d.data", vm_id, file_size, file_number);
    printf("%s", filename);
}

void run_benchmark(int benchmark_id, int vm_id, int num_files, char file_size) {
    switch(benchmark_id) {
        case 1:
            return register_dir(vm_id, num_files, file_size);
        case 2:
            return timed_search(vm_id, num_files, file_size);
        case 3:
            return timed_search_request(vm_id, num_files, file_size);
        default:
            printf("Unknown benchmark id.\n");
    }
}

void register_rand_files(int vm_id, int num_files, char file_size) {
    printf("Registering %d files...\n", num_files);
    register_dir(SHARED_FILE_DIR);

    char* filename = (char*) malloc(256);
    
    for(int i = 0; i < num_files; i++) {
        create_file_name(filename, vm_id, i, file_size);
        register_file(filename);
    }

    free(filename);
}

void timed_search(int vm_id, int num_files, char file_size) {
    // time_point<high_resolution_clock> t1, t2;

    // t1 = high_resolution_clock::now();

    // printf("Timing runtime of %d searches...\n", num_searches);

    // for(int i = 0; i < num_searches; i++) {
    //     char filename[256] = {0};
    //     sprintf(filename, "file-%d.data", i);
    //     search_for_file(filename);
    // }

    // t2 = high_resolution_clock::now();
    // auto elapsed_time_us = duration_cast<microseconds>(t2 - t1);

    // printf("Searched for %d files in %ldus \n", num_searches, elapsed_time_us.count());

}