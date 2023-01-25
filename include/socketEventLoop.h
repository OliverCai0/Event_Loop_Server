#ifndef SOCKET_EVENT_LOOP
#define SOCKET_EVENT_LOOP

#include "sockpp/tcp_acceptor.h"
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
    std::unordered_map<int, sockpp::tcp_socket*> in_use;
    std::unordered_map<int, int> index_tracker;
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

    int removeSocket(int socket_fd) {
        int not_used = index_tracker[socket_fd];
        offline.push_back(not_used);
        in_use.erase(socket_fd);
        return 0;
    }

    int addSocketEvent(sockpp::tcp_socket &sock){
        int handle = sock.handle();
        int not_used = offline.back();
        EV_SET(&socket_arr[not_used], sock.handle(), EVFILT_READ, EV_ADD, 0, 0, NULL);
        if(kevent(socket_kqueue, &socket_arr[not_used], 1, NULL, 0, NULL) == -1){
            printf("Unfortunate");
            return -1;
        }
        // Associate the file descriptor with an index
        in_use.insert({handle, &sock});
        index_tracker.insert({handle, not_used});
        offline.pop_back();
        return 0;
    }

    void runLoop(int (*job) (sockpp::tcp_socket &sock)){
        struct kevent evList[32];
        while(1)
        {
            new_events = kevent(socket_kqueue, NULL, 0, evList, 32, NULL);
            for(i = 0; i< new_events; i++)
            {
                if(evList[i].flags & EV_EOF)
                {
                    int fd = evList[i].ident;
                    int index = index_tracker[fd];
                    EV_SET(&socket_arr[index], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    if(kevent(socket_kqueue, &socket_arr[not_used], 1, NULL, 0, NULL) == -1)
                    {
                        print("Unfortunate");
                        return;
                    }
                    removeSocket(fd);
                }
            }
        }
    }

};
#endif