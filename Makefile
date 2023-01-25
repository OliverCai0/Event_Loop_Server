all: threadPool.exe socketEventLoop.exe server.exe

server.exe: server.cpp
	g++ -std=c++14 -o server -I/usr/local/include -L/usr/local/lib server.cpp -lsockpp

socketEventLoop.exe: include/socketEventLoop.cpp
	g++ -std=c++14 -o socketEventLoop ./include/socketEventLoop.cpp

threadPool.exe: include/threadPool.cpp
	g++ -std=c++14 -o threadPool ./include/threadPool.cpp

clean:
	rm -f server 