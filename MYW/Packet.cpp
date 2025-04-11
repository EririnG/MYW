#include "Packet.h"

void HandleCLient(SOCKET clientSocket)
{
	char recvbuf[DEFAULT_BUFFER_SIZE];
	int recvbuflen = DEFAULT_BUFFER_SIZE;
	int iResult;

	printf("클라이언트 [%lld]이 연결되었습니다.\n", clientSocket);

	// 클라이언트로부터 데이터 수신
	do
	{
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("클라이언트 [%lld]로부터 %d 바이트 수신: %s\n", clientSocket, iResult, recvbuf);
			re
				// 클라이언트에게 데이터 전송
				send(clientSocket, recvbuf, iResult, 0);
		}
		else if (iResult == 0)
		{
			printf("클라이언트 [%lld]이 연결을 종료했습니다.\n", clientSocket);
		}
		else
		{
			printf("recv() 오류: %d\n", WSAGetLastError());
		}
	} while (iResult > 0);
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

