#include <cstdlib>
#include <stdio.h>
#include <chrono>
#include <thread>
#include "benchmarks.hpp"
#include "comms.h"
#include "peer.hpp"
#include "constants.hpp"

using namespace std::chrono;

static void register_files(int vm_id, int num_files, char file_size);
static void timed_search(int vm_id, int num_files, char file_size);
static void timed_search_request(int vm_id, int num_files, char file_size);

static void create_file_name(char* filename, int vm_id, int file_number, char file_size) {
    sprintf(filename, "vm%d_1%c_%06d.data", vm_id, file_size, file_number);
}

void run_benchmark(int benchmark_id, int vm_id, int vm_target, int num_files, char file_size, std::time_t start) {
    register_files(vm_id, num_files, file_size);

	printf("Starting in %lu seconds\n", start - time(NULL));
    std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(start));

    switch(benchmark_id) {
        case 1:
            return timed_search(vm_target, num_files, file_size);
        case 2:
            return timed_search_request(vm_target, num_files, file_size);
        default:
            printf("Unknown benchmark id.\n");
    }
}

void register_files(int vm_id, int num_files, char file_size) {
    printf("Registering %d files...\n", num_files);

    char filename[256] = {0};
    
    for(int i = 0; i < num_files; i++) {
        create_file_name(filename, vm_id, i, file_size);
        register_file(filename);
    }
}

void timed_search(int vm_id, int num_files, char file_size) {
    char filename[256] = {0};
    time_point<high_resolution_clock> t1, t2;

    printf("Timing runtime of %d searches...\n", num_files);
    t1 = high_resolution_clock::now();

    for(int i = 0; i < num_files; i++) {
        create_file_name(filename, vm_id, i, file_size);
        search_for_file(filename);
    }

    t2 = high_resolution_clock::now();
    auto elapsed_time_us = duration_cast<microseconds>(t2 - t1);

    printf("Searched for %d files in %ldus \n", num_files, elapsed_time_us.count());

}

void timed_search_request(int vm_id, int num_files, char file_size) {
    conn_t peer;
    char filename[256] = {0};
    time_point<high_resolution_clock> t1, t2;

    printf("Timing runtime of %d searches & downloads...\n", num_files);
    t1 = high_resolution_clock::now();

    for(int i = 0; i < num_files; i++) {
        create_file_name(filename, vm_id, i, file_size);
        peer = search_for_file(filename);
        request_file_from_peer(peer, filename);
    }

    t2 = high_resolution_clock::now();
    auto elapsed_time_us = duration_cast<microseconds>(t2 - t1);

    printf("Downloaded %d files in %ldus \n", num_files, elapsed_time_us.count());
}