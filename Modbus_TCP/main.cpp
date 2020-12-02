#include "info\info.h"
#include "tcp\tcp.h"
#include "check\check.h"
#include "data\data.h"

CoilBuf_t coilBuf;
RegisterBuf_t regBuf;

int main()
{
	//传输载体
	tcp_request_t reqBuf;
	tcp_respond_t resBuf;
	memset(&reqBuf, 0, sizeof(tcp_request_t));
	memset(&resBuf, 0, sizeof(tcp_respond_t));

	int ret = 0;

	//创建一个套接字
	SOCKET socketfd = create_socket();
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

	//等待客户端接收
	SOCKADDR_IN clientAddr;
	int len = sizeof(SOCKADDR);
	SOCKET clientSocket = accept(socketfd, (SOCKADDR*)&clientAddr, &len);
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

	//while轮询接收发送信息
	while (1)
	{
		//接收请求
		ret = recvRequest(&reqBuf, clientSocket);
		if (ret == Error_ClientOutLine)//客户端退出连接
		{
			cout << inet_ntoa(clientAddr.sin_addr) << " is outLine";
			closesocket(clientSocket);
			closesocket(socketfd);//关闭套接字
			WSACleanup();//Release socket resource 
			break;
		}
		else if (ret < 0)//接收出错
		{
			perror("recv:");

			//清空
			memset(&reqBuf, 0, sizeof(tcp_request_t));
			memset(&resBuf, 0, sizeof(tcp_respond_t));

			continue;
		}
		else//接收成功
		{
			//判断解析请求
			if ((ret = check_request(&reqBuf)) != 0)
			{
				printErrorno(ret);//打印请求解析出错的结果
				continue;//这次循环直接结束
			}
			else//请求数据正确
			{
				//打印
				showhex((unsigned char *)&reqBuf, get_request_size(&reqBuf));
			}	
		}

		//填充响应
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
		showhex((unsigned char *)&resBuf, get_respond_size(&resBuf));


		//清空
		memset(&reqBuf, 0, sizeof(tcp_request_t));
		memset(&resBuf, 0, sizeof(tcp_respond_t));
	}


	cout << "Thanks for use TCP Slave" << endl;
	system("pause");
	return 0;
}