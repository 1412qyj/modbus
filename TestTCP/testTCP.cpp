#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Modbus_TCP/check/check.h"
#include "../Modbus_TCP/data/data.h"
#include "../Modbus_TCP/info/info.h"
#include "../Modbus_TCP/modbus/modbus.h"
#define INI_PATH "../Modbus_TCP/tcp_test.ini"
#define MAX_PATH 1024
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestTCP
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TCP)
		{
			auto char_to_hex = [](uint8_t *str){
				int   num[16] = { 0 };
				int   count = 1;
				uint16_t   result = 0;
				for (int i = 1; i >= 0; i--)
				{
					if ((str[i] >= '0') && (str[i] <= '9'))
						num[i] = str[i] - 48;//�ַ�0��ASCIIֵΪ48
					else if ((str[i] >= 'a') && (str[i] <= 'f'))
						num[i] = str[i] - 'a' + 10;
					else if ((str[i] >= 'A') && (str[i] <= 'F'))
						num[i] = str[i] - 'A' + 10;
					else
						num[i] = 0;
					result = result + num[i] * count;
					count = count * 16;//ʮ������(����ǰ˽��ƾ����������8)    
				}
				return result;
			};


			//ini��ȡ��buffer
			char ini_request[MAX_PATH] = { '\0' };
			char ini_respond[MAX_PATH] = { '\0' };
			int ini_requestlen = 0;
			int ini_respondlen = 0;
			int ini_ret = 0;

			//�����½ڵ�����
			char sectionNames[MAX_PATH];
			GetPrivateProfileSectionNamesA(sectionNames, sizeof(sectionNames), INI_PATH);

			//�½�����
			int sectionCount = 0;

			//һ���½ڵĳ���
			char sectionName[20] = { '\0' };

			//�������ĵĽṹ��
			tcp_request_t tcp_request;
			tcp_respond_t tcp_respond;
			tcp_respond_t tcp_respond_compare;
			memset(&tcp_request, 0, sizeof(tcp_request_t));
			memset(&tcp_respond, 0, sizeof(tcp_respond_t));
			memset(&tcp_respond_compare, 0, sizeof(tcp_respond_t));


			//ѭ������
			int i = 0;

			//ִ�н��
			int result = 0;

			//������ȡ��������
			char *pbuf = nullptr;
			char *ptmp = nullptr;

			//��
			stringstream sstr_sectionNames;


			for (int i = 0; i < MAX_PATH; i++)
			{
				if (sectionNames[i] == '\0' && sectionNames[i + 1] == '\0')
					break;

				if (sectionNames[i] == '\0' && sectionNames[i + 1] != '\0')
					sectionNames[i] = ' ';
			}

			sstr_sectionNames << sectionNames;


			while (sstr_sectionNames >> sectionName)//ÿ��һ���½ھͻ�ѭ��һ��
			{

				//��սṹ��
				memset(&tcp_request, 0, sizeof(tcp_request_t));
				memset(&tcp_respond, 0, sizeof(tcp_respond_t));
				memset(&tcp_respond_compare, 0, sizeof(tcp_respond_t));

				//���ini�ļ�����
				memset(ini_request, 0, sizeof(ini_request));
				memset(ini_respond, 0, sizeof(ini_respond));


				//��ȡini�ļ�������
				GetPrivateProfileStringA(sectionName, "request", "", ini_request, MAX_PATH, INI_PATH);
				ini_requestlen = GetPrivateProfileIntA(sectionName, "requestlen", 0, INI_PATH);
				GetPrivateProfileStringA(sectionName, "response", "", ini_respond, MAX_PATH, INI_PATH);
				ini_respondlen = GetPrivateProfileIntA(sectionName, "responselen", 0, INI_PATH);
				ini_ret = GetPrivateProfileIntA(sectionName, "return", 0, INI_PATH);

				ptmp = ini_request;
				i = 0;

				//�����request
				while ((pbuf = strtok(ptmp, " ")) != nullptr)
				{
					switch (i)
					{
					case 0:
						tcp_request.tcp_head.tranId[0] = char_to_hex((uint8_t *)pbuf);
						break;
					case 1:
						tcp_request.tcp_head.tranId[1] = char_to_hex((uint8_t *)pbuf);
						break;
					case 2:
						tcp_request.tcp_head.ProtoId[0] = char_to_hex((uint8_t *)pbuf);
						break;
					case 3:
						tcp_request.tcp_head.ProtoId[1] = char_to_hex((uint8_t *)pbuf);
						break;
					case 4:
						tcp_request.tcp_head.Length[0] = char_to_hex((uint8_t *)pbuf);
						break;
					case 5:
						tcp_request.tcp_head.Length[1] = char_to_hex((uint8_t *)pbuf);
						break;
					case 6:
						tcp_request.tcp_head.UnitId = char_to_hex((uint8_t *)pbuf);
						break;
					default:
						tcp_request.request.data[i - 7] = char_to_hex((uint8_t *)pbuf);
						break;
					}


					i++;
					ptmp = nullptr;
				}

				pbuf = nullptr;
				ptmp = ini_respond;
				i = 0;
				//�����respond
				while ((pbuf = strtok(ptmp, " ")) != nullptr)
				{
					switch (i)
					{
					case 0:
						tcp_respond.tcp_head.tranId[0] = char_to_hex((uint8_t *)pbuf);
						break;
					case 1:
						tcp_respond.tcp_head.tranId[1] = char_to_hex((uint8_t *)pbuf);
						break;
					case 2:
						tcp_respond.tcp_head.ProtoId[0] = char_to_hex((uint8_t *)pbuf);
						break;
					case 3:
						tcp_respond.tcp_head.ProtoId[1] = char_to_hex((uint8_t *)pbuf);
						break;
					case 4:
						tcp_respond.tcp_head.Length[0] = char_to_hex((uint8_t *)pbuf);
						break;
					case 5:
						tcp_respond.tcp_head.Length[1] = char_to_hex((uint8_t *)pbuf);
						break;
					case 6:
						tcp_respond.tcp_head.UnitId = char_to_hex((uint8_t *)pbuf);
						break;
					default:
						tcp_respond.response.data[i - 7] = char_to_hex((uint8_t *)pbuf);
						break;
					}


					i++;
					ptmp = nullptr;
				}
				i = 0;


				//���Ժ����ڴ�
				//��һ����֤�������͵ı����Ƿ���ȷ
				result = check_request(&tcp_request, ini_requestlen);

				Assert::AreEqual(result, ini_ret);//������ζ���һ��check����ֵ

				if (result != 0 && result != Error_InValidfuncode)//��������²���Ҫ������Ӧ
				{
					//ɶҲ����
					continue;
				}
				else if (result == Error_InValidfuncode)//�������������쳣����
				{
					handleErrorFuncode(&tcp_request, &tcp_respond_compare);

					Assert::IsTrue(!strncmp((char *)&tcp_respond, (char *)&tcp_respond_compare, ini_respondlen));
				}
				else//����������Ӧ
				{
					//���compare�ṹ��
					handleRequest(&tcp_request, &tcp_respond_compare);

					Assert::IsTrue(!strncmp((char *)&tcp_respond, (char *)&tcp_respond_compare, ini_respondlen));
				}

				sectionCount++;//��ѭ����β
			}
		}
		

	};
}