#include "./include/socketEventLoop.h"
#include <iostream>
#include <thread>

void run_echo(sockpp::tcp_socket sock)
{
    char buf[1024] = {0};
    ssize_t n;

    while ((n = sock.read(buf, sizeof(buf))) > 0)
    {
        std::cout << buf << std::endl;
        std::cout << "New Line" << std::endl;
        sock.write_n(buf, n);
    }

    std::cout << "Connection closed from " << sock.peer_address() << std::endl;
}

int main()
{
    kqueueEventLoop t;
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
            std::thread thr(run_echo, std::move(sock));
            thr.detach();
        }
    }
}