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
		printf("WSAStartup 실패: %d\n", result);
		return 1;
	}
	printf("WSAStartup 성공\n");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_protocol = IPPROTO_TCP; // TCP
	hints.ai_flags = AI_PASSIVE; // 로컬 주소 사용
	
	std::string portStr = std::to_string(DEFAULT_PORT);

	result = getaddrinfo(NULL, portStr.c_str()
		, &hints, &resultAddrInfo);
	if (result != 0)
	{
		printf("getaddrinfo 실패: %d\n", result);
		WSACleanup();
		return 1;
	}
	printf("getaddrinfo 성공\n");

	ListenSocket = socket(resultAddrInfo->ai_family, resultAddrInfo->ai_socktype, resultAddrInfo->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("소켓 생성 실패: %d\n", WSAGetLastError());
		freeaddrinfo(resultAddrInfo);
		WSACleanup();
		return 1;
	}
	printf("리스닝 소켓 생성 성공\n");

	result = bind(ListenSocket, resultAddrInfo->ai_addr, (int)resultAddrInfo->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind 실패: %d\n", WSAGetLastError());
		freeaddrinfo(resultAddrInfo);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("bind 성공\n");

	freeaddrinfo(resultAddrInfo);

	result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("listen 실패: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("리스닝 시작\n");

	while (true)
	{
		printf("클라이언트 연결 대기 중...\n");
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			printf("accept 실패: %d\n", WSAGetLastError());
			continue; // 다음 클라이언트 연결 대기
		}
		printf("클라이언트 [%lld] 연결됨\n", ClientSocket);

		std::thread clientThread(HandleCLient, ClientSocket);
		clientThread.detach();
	}


	return 0;
}

