#include "tcp.h"

SOCKET create_socket(void)
{
	/*1.加载套接字库*/
	WSADATA wsaData;
	int iRet = 0;
	iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != iRet)
	{
		cout << "WSAStartup(MAKEWORD(2, 2), &wsaData) execute failed!" << endl;
		return -1;
	}

	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
	{
		WSACleanup();
		cout << "WSADATA version is not correct!" << endl;
		return -1;
	}

	/*创建套接字*/
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == serverSocket)
	{
		cout << "socket failed!" << endl;
		WSACleanup();//Release socket resource 
		return  -1;
	}
	//初始化服务器地址族变量
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(PROT_NUM);

	//绑定
	iRet = bind(serverSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (iRet == SOCKET_ERROR)
	{
		cout << "bind(serverSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) execute failed!" << endl;
		closesocket(serverSocket);
		WSACleanup();//Release socket resource 
		return -1;
	}

	//监听
	iRet = listen(serverSocket, 1);
	if (iRet == SOCKET_ERROR)
	{
		cout << "listen(serverSocket, 10) execute failed!" << endl;
		closesocket(serverSocket);
		WSACleanup();//Release socket resource 
		return -1;
	}
	cout << "listen success!" << endl;

	return serverSocket;
}

int printSocketErr(int errorno)
{
	switch (errorno)
	{
	case Error_Socket:
		cout << "socket func failed" << endl;
		break;
	case Error_Bind:
		cout << "bind func failed" << endl;
		break;
	case Error_Accept:
		cout << "accept func failed" << endl;
		break;
	case Error_Listen:
		cout << "listen func failed" << endl;
		break;
	case Error_Version:
		cout << "Version func failed" << endl;
		break;
	case Error_Argc:
		cout << "Invalid Argc" << endl;
		break;
	default:
		cout << "不知名的错误" << endl;
	}

	return 0;
}

int recvRequest(tcp_request_t *preq, int clientfd)
{
	if (nullptr == preq || clientfd <= 0)
		return Error_Argc;
#if 0
	int ret = recv(clientfd, (char *)preq, sizeof(tcp_request_t), 0);
	if (ret == 0)//客户端下线
		return Error_ClientOutLine;
	else if (ret < 0)//socket_error
	{
		return ret;
	}
	else//发送成功
		return 0;
#else

	return recv(clientfd, (char *)preq, sizeof(tcp_request_t), 0);
#endif
}

int sendRespond(tcp_respond_t *pres, int clientfd)
{
	if (nullptr == pres || clientfd <= 0)
		return Error_Argc;

#if 0
	int ret = send(clientfd, (char *)pres, get_respond_size(pres), 0);

	if (ret <= 0)
		return ret;
	else
		return 0;
#else
	return send(clientfd, (char *)pres, get_respond_size(pres), 0);

#endif
}