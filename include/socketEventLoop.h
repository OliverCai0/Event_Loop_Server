#ifndef SOCKET_EVENT_LOOP
#define SOCKET_EVENT_LOOP

#include <sys/event.h>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <iostream>


/*
 * enables the running of a thread to handle all 
*/

#define MAX_SOCKET_COUNT 500;

class kqueueEventLoop
{
private:
    struct kevent socket_arr[500];
    int socket_kqueue;
    std::unordered_map<int, int>  in_use;
    std::vector<int> offline;

public:
    kqueueEventLoop() {
        printf("Successfully initialized\n");
        socket_kqueue = kqueue();
        for(int i = 1; i < 501; i ++)
        {
            offline.push_back(i);
        }
    }

    ~kqueueEventLoop(){
        close(socket_kqueue);
        in_use.clear();
        offline.clear();
    }

    int addSocketEvent(int socket_fd){
        int not_used = offline.back();
        EV_SET(&socket_arr[not_used], socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if(kevent(socket_kqueue, &socket_arr[not_used], 1, NULL, 0, NULL) == -1){
            printf("Unfortunate");
            return -1;
        }
        return 0;
    }

};
#endif