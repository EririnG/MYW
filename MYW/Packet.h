#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <functional>
#include <unordered_map>

#include "Server.h"

#pragma pack(push, 1)
struct PacketHeader
{
	uint32_t bodySize; // 패킷 크기
	uint16_t packetType; // 패킷 타입
};
#pragma pack(pop)

const size_t PACKET_HEADER_SIZE = sizeof(PacketHeader);
const uint32_t MAX_BODY_SIZE = 1024 * 1024; // 1MB

enum class PacketType : uint16_t
{
	ECHO_REQUEST = 0,
	ECHO_RESPONSE = 1,
	LOGIN_REQUEST = 2,
	LOGIN_RESPONSE = 3,
	LOGOUT_REQUEST = 4,
	LOGOUT_RESPONSE = 5,
	CHAT_REQUEST = 6,
	CHAT_RESPONSE = 7,

};
enum class PacketError : uint16_t
{
	SUCCESS = 0,
	INVALID_PACKET = 1,
	UNKNOWN_ERROR = 2,
};

using PacketHandler = std::function<void(SOCKET, const std::vector<char>&)>;
std::unordered_map<uint16_t, PacketHandler> packetHandlers;

void HandleCLient(SOCKET clientSocket);
int ReceiveExaactly(SOCKET socket, char* buffer, int length);
int SendExactly(SOCKET socket, const char* buffer, int length);
void HandleEchoRequest(SOCKET clientSocket, const std::vector<char>& body);
void HandleLoginRequest(SOCKET clientSocket, const std::vector<char>& body);
void RegisterPacketHandler();
void ProcessPacket(SOCKET clientsocket, uint16_t packetType, const std::vector<char>& body);