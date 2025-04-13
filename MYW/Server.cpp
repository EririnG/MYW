#include "Packet.h"
#include "Server.h"

int main()
{
	WSADATA wsaData;
	struct addrinfo hints;
	struct addrinfo* resultAddrInfo = NULL;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		printf("WSAStartup ����: %d\n", result);
		return 1;
	}
	printf("WSAStartup ����\n");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_protocol = IPPROTO_TCP; // TCP
	hints.ai_flags = AI_PASSIVE; // ���� �ּ� ���
	
	std::string portStr = std::to_string(DEFAULT_PORT);

	result = getaddrinfo(NULL, portStr.c_str()
		, &hints, &resultAddrInfo);
	if (result != 0)
	{
		printf("getaddrinfo ����: %d\n", result);
		WSACleanup();
		return 1;
	}
	printf("getaddrinfo ����\n");

	ListenSocket = socket(resultAddrInfo->ai_family, resultAddrInfo->ai_socktype, resultAddrInfo->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("���� ���� ����: %d\n", WSAGetLastError());
		freeaddrinfo(resultAddrInfo);
		WSACleanup();
		return 1;
	}
	printf("������ ���� ���� ����\n");

	result = bind(ListenSocket, resultAddrInfo->ai_addr, (int)resultAddrInfo->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind ����: %d\n", WSAGetLastError());
		freeaddrinfo(resultAddrInfo);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("bind ����\n");

	freeaddrinfo(resultAddrInfo);

	result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("listen ����: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("������ ����\n");

	while (true)
	{
		printf("Ŭ���̾�Ʈ ���� ��� ��...\n");
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			printf("accept ����: %d\n", WSAGetLastError());
			continue; // ���� Ŭ���̾�Ʈ ���� ���
		}
		printf("Ŭ���̾�Ʈ [%lld] �����\n", ClientSocket);

		std::thread clientThread(HandleCLient, ClientSocket);
		clientThread.detach();
	}


	return 0;
}

