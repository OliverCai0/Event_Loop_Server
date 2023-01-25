#ifndef SOCKET_EVENT_LOOP
#define SOCKET_EVENT_LOOP

#include "sockpp/tcp_acceptor.h"
#include "threadPool.h"
#include <sys/event.h>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <functional>


/*
 * enables the running of a thread to handle all 
*/

#define MAX_SOCKET_COUNT 500;

class kqueueEventLoop
{
private:
    struct kevent socket_arr[500];
    int socket_kqueue;
    int (*conditional)(char* buffer);
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
        conditional = [](char* buffer){
            return 0;
        };
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
        printf("Added socket event");
        // Associate the file descriptor with an index
        in_use.insert({handle, &sock});
        index_tracker.insert({handle, not_used});
        offline.pop_back();
        return 0;
    }

    void runLoop(int (*light_job) (sockpp::tcp_socket &sock, std::vector<char*> request), 
                 int (*heavy_job) (sockpp::tcp_socket &sock, std::vector<char*> request))
    {
        struct kevent evList[32];
        while(1)
        {
            int new_events = kevent(socket_kqueue, NULL, 0, evList, 32, NULL);
            // printf("kqueue triggered");
            for(int i = 0; i< new_events; i++)
            {
                if(evList[i].flags & EV_EOF)
                {
                    int fd = evList[i].ident;
                    int index = index_tracker[fd];
                    EV_SET(&socket_arr[index], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    if(kevent(socket_kqueue, &socket_arr[index], 1, NULL, 0, NULL) == -1)
                    {
                        printf("Unfortunate");
                        return;
                    }
                    printf("Disconnected");
                    removeSocket(fd);
                }
                else if(evList[i].flags & EV_ERROR)
                {
                    printf("Unforseen error");
                    return;
                }
                else if(evList[i].filter &  EVFILT_READ)
                {
                    // sockpp::tcp_socket sock = *(in_use[evList[i].ident]);
                    std::vector<char*> args;
                    char buf[1024] = {0};
                    (*in_use[evList[i].ident]).read(buf, sizeof(buf));
                    args.push_back(buf);
                    if(conditional(buf))
                    {
                        heavy_job(*(in_use[evList[i].ident]), args);
                    }
                    else
                    {
                        light_job(*(in_use[evList[i].ident]), args);
                    }
                }
            }
        }
    }

};
#endif