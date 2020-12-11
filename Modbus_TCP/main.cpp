#include "info\info.h"
#include "tcp\tcp.h"
#include "check\check.h"
#include "data\data.h"

extern CoilBuf_t coilBuf;
extern RegisterBuf_t regBuf;

int main()
{
	//传输载体
	tcp_request_t reqBuf;
	tcp_respond_t resBuf;
	memset(&reqBuf, 0, sizeof(tcp_request_t));
	memset(&resBuf, 0, sizeof(tcp_respond_t));
	SOCKADDR_IN serversockaddr;

	int ret = 0;

	//创建一个套接字
	SOCKET socketfd = create_socket(&serversockaddr);
	if (socketfd < 0)
	{
		printSocketErr(socketfd);

		system("pause");
		return -1;
	}
	else
	{
		cout << "socket created successfully" << endl;
	}
	
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));

ACCEPT_CLIENT:
	//打印服务器ip和端口号
	cout << "Server Msg:" << endl;
	printSockMsg(&serversockaddr);

	printf("waiting for connection -----------------\n");

	//等待客户端接收
	
	int len = sizeof(SOCKADDR);
	clientSocket = accept(socketfd, (SOCKADDR*)&clientAddr, &len);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("accept %d\n", clientSocket);
		cout << "accept(serverSocket, (SOCKADDR*)&clientAddr, &len) execute failed!" << endl;
		closesocket(socketfd);
		WSACleanup();//Release socket resource 

		system("pause");
		return -1;
	}

	cout << "Accept: " << inet_ntoa(clientAddr.sin_addr) << endl;
	//打印客户端的信息
	cout << "Client Msg :" << endl;
	printSockMsg(&clientAddr);

	//while轮询接收发送信息
	while (1)
	{
		printf("=================================================================\n");
		//清空
		memset(&reqBuf, 0, sizeof(tcp_request_t));
		memset(&resBuf, 0, sizeof(tcp_respond_t));

		//接收请求
		ret = recvRequest(&reqBuf, clientSocket);
		if (ret == 0)//客户端退出连接
		{
			cout << "***************************************" << endl;
			cout << inet_ntoa(clientAddr.sin_addr) << " is outLine" << endl;
			cout << "See you next time" << endl;
			cout << "***************************************" << endl;
			closesocket(clientSocket);
			//closesocket(socketfd);//关闭套接字
			//WSACleanup();//Release socket resource 

			goto ACCEPT_CLIENT;
			break;
		}
		else if (ret == -1)//网线断开
		{
			cout << "ether is outline----" << endl;
			closesocket(clientSocket);

			printf("trying to reconnect----\r");

			while ((clientSocket = accept(socketfd, (SOCKADDR*)&clientAddr, &len)) == INVALID_SOCKET)
			{
				printf("trying to reconnect----\r");
				Sleep(1000);
			}

			printf("\nreconnect successfully----\n");

			continue;
		}
		else if (ret < -1)//接收出错
		{
			perror("recv:");

			//清空
			memset(&reqBuf, 0, sizeof(tcp_request_t));
			memset(&resBuf, 0, sizeof(tcp_respond_t));

			continue;
		}
		else//接收成功
		{
			//先比较数据长度
			if ((ret - 6) != get_request_length(&reqBuf))//此时的ret是实际接收的长度
			{
				cout << "Error_InValidLength" << endl;
				continue;
			}

			ret = check_request(&reqBuf);
			//判断解析请求
			if (ret != 0 && ret != Error_InValidfuncode)
			{
				printErrorno(ret);//打印请求解析出错的结果
				continue;//这次循环直接结束
			}
			else if (ret == Error_InValidfuncode)//错误的功能码
			{
				handleErrorFuncode(&reqBuf, &resBuf);
			}
			else//请求数据正确
			{
				//打印
				cout << "recv >";
				showhex((unsigned char *)&reqBuf, get_request_size(&reqBuf));
			}	
		}

		//功能码不错误的话，填充响应
		if (ret != Error_InValidfuncode)
			handleRequest(&reqBuf, &resBuf);

		//发送响应
		ret = sendRespond(&resBuf, clientSocket);
		if (ret < 0)
		{
			perror("send");

			//清空
			memset(&reqBuf, 0, sizeof(tcp_request_t));
			memset(&resBuf, 0, sizeof(tcp_respond_t));

			continue;
		}

		//打印响应
		cout << "send >";
		showhex((unsigned char *)&resBuf, get_respond_size(&resBuf));



	}


	cout << "Thanks for use TCP Slave" << endl;
	system("pause");
	return 0;
}