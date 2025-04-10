#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

class TCPServer
{
private:
	int server_fd;
	struct sockaddr_in address;
	int port;
	std::atomic<bool> running;
	std::vector<std::thread> client_threads;
	std::mutex cout_mutex;						// cout을 thread-safe하게 사용하기 위한 뮤텍스

public:
	TCPServer(int port) : port(port), running(false)
	{
		server_fd = socket(af_inet)
	}
};

