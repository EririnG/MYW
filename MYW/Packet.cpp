#include "Packet.h"

PacketHandlerMap packetHandlers;

void HandleCLient(SOCKET clientSocket)
{
	printf("Ŭ���̾�Ʈ [%lld] �����. ��� ������ ����...\n", clientSocket);
	char headerBuffer[PACKET_HEADER_SIZE];
	std::vector<char> bodyBuffer;

	while (true)
	{
		// ��Ŷ ��� ����
		int bytesReceived = ReceiveExaactly(clientSocket, headerBuffer, PACKET_HEADER_SIZE);
		if (bytesReceived <= 0)
		{
			printf("Ŭ���̾�Ʈ [%lld] ���� ����\n", clientSocket);
			break; // ���� ����
		}

		PacketHeader* header = reinterpret_cast<PacketHeader*>(headerBuffer);
		uint32_t bodySize = ntohl(header->bodySize);
		uint16_t packetType = ntohs(header->packetType);

		if (bodySize > MAX_BODY_SIZE)
		{
			printf("��Ŷ ũ�� �ʰ�: %u\n", bodySize);
			break; // ��Ŷ ũ�� �ʰ�
		}


		if (bodySize > 0)
		{
			bodyBuffer.resize(bodySize);
			int bodyResult = ReceiveExaactly(clientSocket, bodyBuffer.data(), bodySize);
			if (bodyResult <= 0)
			{
				printf("Ŭ���̾�Ʈ [%lld] ���� ����\n", clientSocket);
				break; // ���� ����
			}
			printf("Ŭ���̾�Ʈ [%lld]�κ��� ��Ŷ ����: Ÿ��=%u, ũ��=%u\n", clientSocket, packetType, bodySize);
		}
		else
		{
			bodyBuffer.clear();
			printf("Ŭ���̾�Ʈ [%lld]�κ��� ��Ŷ ����: Ÿ��=%u, ũ��=%u\n", clientSocket, packetType, bodySize);
		}

		ProcessPacket(clientSocket, packetType, bodyBuffer);
	}

	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
	printf("Ŭ���̾�Ʈ [%lld] ���� ����\n", clientSocket);
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
			printf("��Ŷ ó�� �� ���� �߻�: %s\n", e.what());
			PacketHeader header;
			header.bodySize = htonl(static_cast<uint32_t>(body.size()));
			header.packetType = htons(static_cast<uint16_t>(PacketError::UNKNOWN_ERROR));
			SendExactly(clientsocket, reinterpret_cast<const char*>(&header), PACKET_HEADER_SIZE);
		}
	}
	else
	{
		printf("�� �� ���� ��Ŷ Ÿ��: %u\n", packetType);
	}
}

