#include "./include/socketEventLoop.h"
#include <iostream>
#include <thread>


int light_job(sockpp::tcp_socket &sock, std::vector<char*> request)
{
    sock.write_n(request[0], sizeof(request[0]));
    return 0;
}

int heavy_job(sockpp::tcp_socket &sock, std::vector<char*> request)
{
    /*
    Placeholder for heavy io load
    */

   for(int i; i < 100; i++)
   {
        sock.write_n(request[0], sizeof(request[0]));
   }
   return 0;
}
// void run_echo(sockpp::tcp_socket sock)
// {
//     char buf[1024] = {0};
//     ssize_t n;

//     while ((n = sock.read(buf, sizeof(buf))) > 0)
//     {
//         std::cout << buf << std::endl;
//         std::cout << "New Line" << std::endl;
//         sock.write_n(buf, n);
//     }

//     std::cout << "Connection closed from " << sock.peer_address() << std::endl;
// }

int main()
{
    kqueueEventLoop t;
    std::thread thr(&kqueueEventLoop::runLoop, t, std::ref(light_job), std::ref(heavy_job));
    int16_t port = 12345;
    sockpp::tcp_acceptor acc(port);

    if (!acc)
    {
        std::cerr << acc.last_error_str() << std::endl;
    }
    std::cout << "Listening on port " << port << std::endl;
    // Accept a new client connection
    sockpp::tcp_socket sock = acc.accept();

    while (true)
    {
        // Accept a new client connection
        sockpp::tcp_socket sock = acc.accept();

        if (!sock)
        {
            std::cerr << "Error accepting incoming connection: " << acc.last_error_str() << std::endl;
        }
        else
        {
            // Create a thread and transfer the new stream to it.
            t.addSocketEvent(sock);
            // thr.detach();
        }
    }
}