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
						num[i] = str[i] - 48;//字符0的ASCII值为48
					else if ((str[i] >= 'a') && (str[i] <= 'f'))
						num[i] = str[i] - 'a' + 10;
					else if ((str[i] >= 'A') && (str[i] <= 'F'))
						num[i] = str[i] - 'A' + 10;
					else
						num[i] = 0;
					result = result + num[i] * count;
					count = count * 16;//十六进制(如果是八进制就在这里乘以8)    
				}
				return result;
			};


			//ini读取的buffer
			char ini_request[MAX_PATH] = { '\0' };
			char ini_respond[MAX_PATH] = { '\0' };
			int ini_requestlen = 0;
			int ini_respondlen = 0;
			int ini_ret = 0;

			//所有章节的名称
			char sectionNames[MAX_PATH];
			GetPrivateProfileSectionNamesA(sectionNames, sizeof(sectionNames), INI_PATH);

			//章节数量
			int sectionCount = 0;

			//一个章节的长度
			char sectionName[20] = { '\0' };

			//两个报文的结构体
			tcp_request_t tcp_request;
			tcp_respond_t tcp_respond;
			tcp_respond_t tcp_respond_compare;
			memset(&tcp_request, 0, sizeof(tcp_request_t));
			memset(&tcp_respond, 0, sizeof(tcp_respond_t));
			memset(&tcp_respond_compare, 0, sizeof(tcp_respond_t));


			//循环变量
			int i = 0;

			//执行结果
			int result = 0;

			//报文提取辅助变量
			char *pbuf = nullptr;
			char *ptmp = nullptr;

			//流
			stringstream sstr_sectionNames;


			for (int i = 0; i < MAX_PATH; i++)
			{
				if (sectionNames[i] == '\0' && sectionNames[i + 1] == '\0')
					break;

				if (sectionNames[i] == '\0' && sectionNames[i + 1] != '\0')
					sectionNames[i] = ' ';
			}

			sstr_sectionNames << sectionNames;


			while (sstr_sectionNames >> sectionName)//每有一个章节就会循环一次
			{

				//清空结构体
				memset(&tcp_request, 0, sizeof(tcp_request_t));
				memset(&tcp_respond, 0, sizeof(tcp_respond_t));
				memset(&tcp_respond_compare, 0, sizeof(tcp_respond_t));

				//清空ini文件载体
				memset(ini_request, 0, sizeof(ini_request));
				memset(ini_respond, 0, sizeof(ini_respond));


				//获取ini文件的数据
				GetPrivateProfileStringA(sectionName, "request", "", ini_request, MAX_PATH, INI_PATH);
				ini_requestlen = GetPrivateProfileIntA(sectionName, "requestlen", 0, INI_PATH);
				GetPrivateProfileStringA(sectionName, "response", "", ini_respond, MAX_PATH, INI_PATH);
				ini_respondlen = GetPrivateProfileIntA(sectionName, "responselen", 0, INI_PATH);
				ini_ret = GetPrivateProfileIntA(sectionName, "return", 0, INI_PATH);

				ptmp = ini_request;
				i = 0;

				//先填充request
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
				//再填充respond
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


				//测试函数在此
				//第一步验证主机发送的报文是否正确
				result = check_request(&tcp_request, ini_requestlen);

				Assert::AreEqual(result, ini_ret);//无论如何都有一个check返回值

				if (result != 0 && result != Error_InValidfuncode)//这种情况下不需要回馈响应
				{
					//啥也不干
					continue;
				}
				else if (result == Error_InValidfuncode)//功能码出错进行异常处理
				{
					handleErrorFuncode(&tcp_request, &tcp_respond_compare);

					Assert::IsTrue(!strncmp((char *)&tcp_respond, (char *)&tcp_respond_compare, ini_respondlen));
				}
				else//正常回馈响应
				{
					//填充compare结构体
					handleRequest(&tcp_request, &tcp_respond_compare);

					Assert::IsTrue(!strncmp((char *)&tcp_respond, (char *)&tcp_respond_compare, ini_respondlen));
				}

				sectionCount++;//大循环结尾
			}
		}
		

	};
}