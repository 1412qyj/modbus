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
		//���
		memset(&reqBuf, 0, sizeof(tcp_request_t));
		memset(&resBuf, 0, sizeof(tcp_respond_t));

		//��������
		ret = recvRequest(&reqBuf, clientSocket);
		if (ret == 0)//�ͻ����˳�����
		{
			cout << inet_ntoa(clientAddr.sin_addr) << " is outLine" << endl;
			closesocket(clientSocket);
			closesocket(socketfd);//�ر��׽���
			WSACleanup();//Release socket resource 
			break;
		}
		else if (ret == -1)//���߶Ͽ�
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
		else if (ret < -1)//���ճ���
		{
			perror("recv:");

			//���
			memset(&reqBuf, 0, sizeof(tcp_request_t));
			memset(&resBuf, 0, sizeof(tcp_respond_t));

			continue;
		}
		else//���ճɹ�
		{
			//�ȱȽ����ݳ���
			if ((ret - 6) != get_request_length(&reqBuf))//��ʱ��ret��ʵ�ʽ��յĳ���
			{
				cout << "Error_InValidLength" << endl;
				continue;
			}

			ret = check_request(&reqBuf);
			//�жϽ�������
			if (ret != 0 && ret != Error_InValidfuncode)
			{
				printErrorno(ret);//��ӡ�����������Ľ��
				continue;//���ѭ��ֱ�ӽ���
			}
			else if (ret == Error_InValidfuncode)//����Ĺ�����
			{
				handleErrorFuncode(&reqBuf, &resBuf);
			}
			else//����������ȷ
			{
				//��ӡ
				cout << "recv >";
				showhex((unsigned char *)&reqBuf, get_request_size(&reqBuf));
			}	
		}

		//�����벻����Ļ��������Ӧ
		if (ret != Error_InValidfuncode)
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
		cout << "send >";
		showhex((unsigned char *)&resBuf, get_respond_size(&resBuf));



	}


	cout << "Thanks for use TCP Slave" << endl;
	system("pause");
	return 0;
}