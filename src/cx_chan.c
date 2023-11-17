#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <poll.h>

typedef struct CxChanSelect {
    size_t  nreaders;
    int*    readers;
    size_t  nwriters;
    int*    writers;
} CxChanSelect;

 int cxChanSelect(CxChanSelect* sel, int timeout, int* selected) {

    // struct pollfd polls[8];
    // if (n > 8) {
    //     return -1;
    // }
    //
    // for (size_t i = 0; i < n; i++) {
    //     polls[i].fd = fds[i];
    //     polls[i].events = POLLIN;
    // }
    // int count = poll(polls, n, timeout);
    // if (count <= 0) {
    //     return count;
    // }
    // int seln = 0;
    // if (count > 1) {
    //     struct timespec now;
    //     timespec_get(&now, TIME_UTC);
    //     srand(now.tv_nsec);
    //     seln = rand() % count;
    // }
    // int selcount = 0;
    // for (size_t i = 0; i < n; i++) {
    //     if (polls[i].revents & POLLIN && selcount == seln) {
    //         *selected = i;
    //         return 0;
    //     }
    //     selcount++;
    // }
    return -1; 
}

