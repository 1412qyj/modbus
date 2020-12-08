#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Modbus_RTU/crc/crc.h"
#include "../Modbus_RTU/info/info.h"
#include "../Modbus_RTU/check/check.h"
#include "../Modbus_RTU/modbus/modbus.h"
#include <Windows.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#define INI_PATH "../Modbus_RTU/config.ini"
#define RESULT_PATH "result.txt"
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 1

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

#define MAX_PATH 1024

namespace Test_RTU
{		
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(RTU)
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
			
			char ini_request[MAX_PATH] = {'\0'};
			char ini_respond[MAX_PATH] = {'\0'};
			int ini_requestlen = 0;
			int ini_respondlen = 0;
			int ini_ret = 0;
			char sectionNames[MAX_PATH];
			GetPrivateProfileSectionNamesA(sectionNames, sizeof(sectionNames), INI_PATH);
			char *ptmp = sectionNames;
			char *sectionName[MAX_PATH] = {nullptr};

			for (int i = 0; i < MAX_PATH; i++)
			{
				if (sectionNames[i] == '\0' && sectionNames[i + 1] == '\0')
				{
					break;
				}

				if (sectionNames[i] == '\0' && sectionNames[i + 1] != '\0')
				{
					sectionNames[i] = ' ';
				}
			}

			rtu_request_t rtu_request;
			rtu_respond_t rtu_respond;
			memset(&rtu_request, 0, sizeof(rtu_request_t));
			memset(&rtu_respond, 0, sizeof(rtu_respond_t));
			int i = 0;
			uint8_t buf[3];
			int sectionCount = 0;
			int result = 0;
			char *pbuf = nullptr;

			while ((sectionName[sectionCount] = strtok(ptmp, " ")) != nullptr)//每有一个章节就会循环一次
			{
				//清空结构体
				memset(&rtu_request, 0, sizeof(rtu_request_t));
				memset(&rtu_respond, 0, sizeof(rtu_respond_t));

				//清空ini文件载体
				memset(ini_request, 0, sizeof(ini_request));
				memset(ini_respond, 0, sizeof(ini_respond));


				//获取ini文件的数据
				GetPrivateProfileStringA(sectionName[sectionCount], "request", "", ini_request, MAX_PATH, INI_PATH);
				ini_requestlen = GetPrivateProfileIntA(sectionName[sectionCount], "requestlen", 0, INI_PATH);
				GetPrivateProfileStringA(sectionName[sectionCount], "response", "", ini_respond, MAX_PATH, INI_PATH);
				ini_respondlen = GetPrivateProfileIntA(sectionName[sectionCount], "responselen", 0, INI_PATH);
				ini_ret = GetPrivateProfileIntA(sectionName[sectionCount], "return", 0, INI_PATH);


				ptmp = ini_respond;

				i = 0;
				while ((pbuf = strtok(ptmp, " ")) != nullptr)
				{
					rtu_respond.response.data[i] = char_to_hex((uint8_t *)pbuf);

					i++;
					ptmp = nullptr;
				}


				ptmp = ini_request;
				i = 0;
				//先填充request
				while ((pbuf = strtok(ptmp, " ")) != nullptr)
				{
					rtu_request.request.data[i] = char_to_hex((uint8_t *)pbuf);
					i++;
					ptmp = nullptr;
				}
				i = 0;
				

				result = respond_check(&rtu_respond, &rtu_request);
				Assert::AreEqual(result, ini_ret);

				ptmp = nullptr;
				sectionCount++;
			}

		}

	};
}