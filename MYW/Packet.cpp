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
