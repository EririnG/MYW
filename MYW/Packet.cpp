#include "Packet.h"

PacketHandlerMap packetHandlers;

void HandleCLient(SOCKET clientSocket)
{
	printf("클라이언트 [%lld] 연결됨. 통신 스레드 시작...\n", clientSocket);
	char headerBuffer[PACKET_HEADER_SIZE];
	std::vector<char> bodyBuffer;

	while (true)
	{
		// 패킷 헤더 수신
		int bytesReceived = ReceiveExaactly(clientSocket, headerBuffer, PACKET_HEADER_SIZE);
		if (bytesReceived <= 0)
		{
			printf("클라이언트 [%lld] 연결 종료\n", clientSocket);
			break; // 연결 종료
		}

		PacketHeader* header = reinterpret_cast<PacketHeader*>(headerBuffer);
		uint32_t bodySize = ntohl(header->bodySize);
		uint16_t packetType = ntohs(header->packetType);

		if (bodySize > MAX_BODY_SIZE)
		{
			printf("패킷 크기 초과: %u\n", bodySize);
			break; // 패킷 크기 초과
		}


		if (bodySize > 0)
		{
			bodyBuffer.resize(bodySize);
			int bodyResult = ReceiveExaactly(clientSocket, bodyBuffer.data(), bodySize);
			if (bodyResult <= 0)
			{
				printf("클라이언트 [%lld] 연결 종료\n", clientSocket);
				break; // 연결 종료
			}
			printf("클라이언트 [%lld]로부터 패킷 수신: 타입=%u, 크기=%u\n", clientSocket, packetType, bodySize);
		}
		else
		{
			bodyBuffer.clear();
			printf("클라이언트 [%lld]로부터 패킷 수신: 타입=%u, 크기=%u\n", clientSocket, packetType, bodySize);
		}

		ProcessPacket(clientSocket, packetType, bodyBuffer);
	}

	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
	printf("클라이언트 [%lld] 소켓 종료\n", clientSocket);
}

int ReceiveExaactly(SOCKET socket, char* buffer, int length)
{
	int totalBytesReceived = 0;
	while (totalBytesReceived < length)
	{
		int bytesReceived = recv(socket, buffer + totalBytesReceived, length - totalBytesReceived, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			return SOCKET_ERROR; // 오류 발생
		}
		if (bytesReceived == 0)
		{
			return 0; // 연결 종료
		}
		// 수신된 바이트 수를 업데이트
		totalBytesReceived += bytesReceived;
	}
	return totalBytesReceived;
}

int SendExactly(SOCKET socket, const char* buffer, int length)
{
	int bytesSent = send(socket, buffer, length, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		printf("send() 오류: %d\n", WSAGetLastError());
		return SOCKET_ERROR; // 오류 발생
	}

	if (bytesSent < length)
	{
		// 모든 데이터가 전송되지 않은 경우
		int remainingBytes = length - bytesSent;
		const char* remainingBuffer = buffer + bytesSent;
		return SendExactly(socket, remainingBuffer, remainingBytes); // 재귀 호출
	}
	return bytesSent; // 모든 데이터가 성공적으로 전송됨
}

void HandleEchoRequest(SOCKET clientSocket, const std::vector<char>& body)
{
	// Echo 요청 처리
	PacketHeader header;
	header.bodySize = htonl(static_cast<uint32_t>(body.size()));
	header.packetType = htons(static_cast<uint16_t>(PacketType::ECHO_RESPONSE));

	if (SendExactly(clientSocket, reinterpret_cast<const char*>(&header), PACKET_HEADER_SIZE) >  0)
	{
		if (body.empty() || SendExactly(clientSocket, body.data(), body.size()) > 0)
		{
			printf("클라이언트 [%lld]에게 Echo 응답 전송 성공: %s\n", clientSocket, body.data());
			return;
		}
	}
	printf("클라이언트 [%lld]에게 Echo 응답 전송 실패: %s\n", clientSocket, body.data() + PACKET_HEADER_SIZE);
}

void HandleLoginRequest(SOCKET clientSocket, const std::vector<char>& body)
{
	// 로그인 요청 처리
	PacketHeader header;
	header.bodySize = htonl(static_cast<uint32_t>(body.size()));
	header.packetType = htons(static_cast<uint16_t>(PacketType::LOGIN_RESPONSE));

	if (SendExactly(clientSocket, reinterpret_cast<const char*>(&header), PACKET_HEADER_SIZE) > 0)
	{
		if (body.empty() || SendExactly(clientSocket, body.data(), body.size()) > 0)
		{
			printf("클라이언트 [%lld]에게 Echo 응답 전송 성공: %s\n", clientSocket, body.data());
			return;
		}
	}
	printf("클라이언트 [%lld]에게 Echo 응답 전송 실패: %s\n", clientSocket, body.data() + PACKET_HEADER_SIZE);
}

void RegisterPacketHandler()
{
	packetHandlers[static_cast<uint16_t>(PacketType::ECHO_REQUEST)] = HandleEchoRequest;
	packetHandlers[static_cast<uint16_t>(PacketType::LOGIN_REQUEST)] = HandleLoginRequest;
	// 다른 패킷 핸들러 등록
	printf("패킷 핸들러 등록 완료\n");
}

void ProcessPacket(SOCKET clientsocket, uint16_t packetType, const std::vector<char>& body)
{
	auto it = packetHandlers.find(packetType);
	if (it != packetHandlers.end())
	{
		PacketHandler handler = it->second;
		try
		{
			handler(clientsocket, body);
		}
		catch (const std::exception& e)
		{
			printf("패킷 처리 중 오류 발생: %s\n", e.what());
			PacketHeader header;
			header.bodySize = htonl(static_cast<uint32_t>(body.size()));
			header.packetType = htons(static_cast<uint16_t>(PacketError::UNKNOWN_ERROR));
			SendExactly(clientsocket, reinterpret_cast<const char*>(&header), PACKET_HEADER_SIZE);
		}
	}
	else
	{
		printf("알 수 없는 패킷 타입: %u\n", packetType);
	}
}

