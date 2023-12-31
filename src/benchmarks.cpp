#include <cstdlib>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <string>
#include <unistd.h>

#include "benchmarks.hpp"
#include "peer.hpp"
#include "constants.hpp"

using namespace std::chrono;

static void test_query(IPeer* peer, int num_files, int num_peers, const char* file_size);
static void test_transfer(IPeer* peer, int num_files, int num_peers, const char* file_size);

static void create_file_name(char* filename, int vm_id, int file_number, const char* file_size) {
    sprintf(filename, "vm%d_%s_%06d.data", vm_id, file_size, file_number);
}

void run_benchmark(IPeer* peer, int benchmark_id) {
	printf("Starting benchmark number %d in 10 seconds.\n", benchmark_id);
    sleep(10);

    switch(benchmark_id) {
        case 1:
            if(peer->get_id() == 1)
                return test_query(peer, 10000, 16, "10K");
            sleep(100000000);
            break;
        case 2:
            if(peer->get_id() <= 9)
                return test_query(peer, 10000, 16, "10K");
            sleep(100000000);
            break;
        case 3:
            if(peer->get_id() <= 9)
                return test_transfer(peer, 10000, 16, "10K");
            sleep(100000000);
            break;
        case 4:
            if(peer->get_id() <= 9)
                return test_transfer(peer, 10, 16, "100M");
            sleep(100000000);
            break;
        default:
            printf("Unknown benchmark id.\n");
    }
}

void test_query(IPeer* peer, const int num_files, const int num_peers, const char* file_size) {
    int vm_id, failures = 0;
    conn_t res;
    char filename[256] = {0};
    time_point<high_resolution_clock> t1, t2;

    printf("Timing runtime of %d searches...\n", num_files);
    t1 = high_resolution_clock::now();

    for(int i = 0; i < num_files; i++) {
        vm_id = rand() % num_peers + 1;
        create_file_name(filename, vm_id, i, file_size);
        res = peer->search_for_file(filename);

        if(res.addr == -1) {
            printf("Failed requesting file %s.\n", filename);
            failures++;
        }
    }

    t2 = high_resolution_clock::now();
    auto elapsed_time_us = duration_cast<microseconds>(t2 - t1);

    printf("Searched for %d files in %ld us \n", num_files, elapsed_time_us.count());
    printf("Failed %d/%d times.\n", failures, num_files);
}

void test_transfer(IPeer* peer, const int num_files, const int num_peers, const char* file_size) {
    int vm_id, res, failures = 0;
    char filename[256] = {0};
    time_point<high_resolution_clock> t1, t2;

    printf("Timing runtime of %d searches...\n", num_files);
    t1 = high_resolution_clock::now();

    for(int i = 0; i < num_files; i++) {
        vm_id = rand() % num_peers + 1;
        create_file_name(filename, vm_id, i, file_size);
        res = peer->request_file(filename);

        if(res == -1) {
            printf("Failed requesting file %s.\n", filename);
            failures++;
        }
    }

    t2 = high_resolution_clock::now();
    auto elapsed_time_us = duration_cast<microseconds>(t2 - t1);

    printf("Downloaded %d files in %ld us \n", num_files, elapsed_time_us.count());
    printf("Failed %d/%d times.\n", failures, num_files);
}
