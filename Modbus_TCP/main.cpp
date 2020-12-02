#include "info\info.h"
#include "tcp\tcp.h"
#include "check\check.h"
#include "data\data.h"

CoilBuf_t coilBuf;
RegisterBuf_t regBuf;

int main()
{
	//��������
	tcp_request_t reqBuf;
	tcp_respond_t resBuf;
	memset(&reqBuf, 0, sizeof(tcp_request_t));
	memset(&resBuf, 0, sizeof(tcp_respond_t));

	int ret = 0;

	//����һ���׽���
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

	//�ȴ��ͻ��˽���
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

	//while��ѯ���շ�����Ϣ
	while (1)
	{
		//��������
		ret = recvRequest(&reqBuf, clientSocket);
		if (ret == Error_ClientOutLine)//�ͻ����˳�����
		{
			cout << inet_ntoa(clientAddr.sin_addr) << " is outLine";
			closesocket(clientSocket);
			closesocket(socketfd);//�ر��׽���
			WSACleanup();//Release socket resource 
			break;
		}
		else if (ret < 0)//���ճ���
		{
			perror("recv:");

			//���
			memset(&reqBuf, 0, sizeof(tcp_request_t));
			memset(&resBuf, 0, sizeof(tcp_respond_t));

			continue;
		}
		else//���ճɹ�
		{
			//�жϽ�������
			if ((ret = check_request(&reqBuf)) != 0)
			{
				printErrorno(ret);//��ӡ�����������Ľ��
				continue;//���ѭ��ֱ�ӽ���
			}
			else//����������ȷ
			{
				//��ӡ
				showhex((unsigned char *)&reqBuf, get_request_size(&reqBuf));
			}	
		}

		//�����Ӧ
		handleRequest(&reqBuf, &resBuf);

		//������Ӧ
		ret = sendRespond(&resBuf, clientSocket);
		if (ret < 0)
		{
			perror("send");

			//���
			memset(&reqBuf, 0, sizeof(tcp_request_t));
			memset(&resBuf, 0, sizeof(tcp_respond_t));

			continue;
		}

		//��ӡ��Ӧ
		showhex((unsigned char *)&resBuf, get_respond_size(&resBuf));


		//���
		memset(&reqBuf, 0, sizeof(tcp_request_t));
		memset(&resBuf, 0, sizeof(tcp_respond_t));
	}


	cout << "Thanks for use TCP Slave" << endl;
	system("pause");
	return 0;
}