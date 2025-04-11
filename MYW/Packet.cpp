#include "Packet.h"

void HandleCLient(SOCKET clientSocket)
{
	char recvbuf[DEFAULT_BUFFER_SIZE];
	int recvbuflen = DEFAULT_BUFFER_SIZE;
	int iResult;

	printf("Ŭ���̾�Ʈ [%lld]�� ����Ǿ����ϴ�.\n", clientSocket);

	// Ŭ���̾�Ʈ�κ��� ������ ����
	do
	{
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Ŭ���̾�Ʈ [%lld]�κ��� %d ����Ʈ ����: %s\n", clientSocket, iResult, recvbuf);
			re
				// Ŭ���̾�Ʈ���� ������ ����
				send(clientSocket, recvbuf, iResult, 0);
		}
		else if (iResult == 0)
		{
			printf("Ŭ���̾�Ʈ [%lld]�� ������ �����߽��ϴ�.\n", clientSocket);
		}
		else
		{
			printf("recv() ����: %d\n", WSAGetLastError());
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
			return SOCKET_ERROR; // ���� �߻�
		}
		if (bytesReceived == 0)
		{
			return 0; // ���� ����
		}
		// ���ŵ� ����Ʈ ���� ������Ʈ
		totalBytesReceived += bytesReceived;
	}
	return totalBytesReceived;
}

int SendExactly(SOCKET socket, const char* buffer, int length)
{
	int bytesSent = send(socket, buffer, length, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		printf("send() ����: %d\n", WSAGetLastError());
		return SOCKET_ERROR; // ���� �߻�
	}

	if (bytesSent < length)
	{
		// ��� �����Ͱ� ���۵��� ���� ���
		int remainingBytes = length - bytesSent;
		const char* remainingBuffer = buffer + bytesSent;
		return SendExactly(socket, remainingBuffer, remainingBytes); // ��� ȣ��
	}
	return bytesSent; // ��� �����Ͱ� ���������� ���۵�
}

void HandleEchoRequest(SOCKET clientSocket, const std::vector<char>& body)
{
	// Echo ��û ó��
	PacketHeader header;
	header.bodySize = htonl(static_cast<uint32_t>(body.size()));
	header.packetType = htons(static_cast<uint16_t>(PacketType::ECHO_RESPONSE));

	if (SendExactly(clientSocket, reinterpret_cast<const char*>(&header), PACKET_HEADER_SIZE) >  0)
	{
		if (body.empty() || SendExactly(clientSocket, body.data(), body.size()) > 0)
		{
			printf("Ŭ���̾�Ʈ [%lld]���� Echo ���� ���� ����: %s\n", clientSocket, body.data());
			return;
		}
	}
	printf("Ŭ���̾�Ʈ [%lld]���� Echo ���� ���� ����: %s\n", clientSocket, body.data() + PACKET_HEADER_SIZE);
}

void HandleLoginRequest(SOCKET clientSocket, const std::vector<char>& body)
{
	// �α��� ��û ó��
	PacketHeader header;
	header.bodySize = htonl(static_cast<uint32_t>(body.size()));
	header.packetType = htons(static_cast<uint16_t>(PacketType::LOGIN_RESPONSE));

	if (SendExactly(clientSocket, reinterpret_cast<const char*>(&header), PACKET_HEADER_SIZE) > 0)
	{
		if (body.empty() || SendExactly(clientSocket, body.data(), body.size()) > 0)
		{
			printf("Ŭ���̾�Ʈ [%lld]���� Echo ���� ���� ����: %s\n", clientSocket, body.data());
			return;
		}
	}
	printf("Ŭ���̾�Ʈ [%lld]���� Echo ���� ���� ����: %s\n", clientSocket, body.data() + PACKET_HEADER_SIZE);
}

void RegisterPacketHandler()
{
	packetHandlers[static_cast<uint16_t>(PacketType::ECHO_REQUEST)] = HandleEchoRequest;
	packetHandlers[static_cast<uint16_t>(PacketType::LOGIN_REQUEST)] = HandleLoginRequest;
	// �ٸ� ��Ŷ �ڵ鷯 ���
	printf("��Ŷ �ڵ鷯 ��� �Ϸ�\n");
}

