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
#define INI_PATH "../Modbus_RTU/config/config.ini"
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
			
			//ini读取的buffer
			char ini_request[MAX_PATH] = {'\0'};
			char ini_respond[MAX_PATH] = {'\0'};
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
			rtu_request_t rtu_request;
			rtu_respond_t rtu_respond;
			memset(&rtu_request, 0, sizeof(rtu_request_t));
			memset(&rtu_respond, 0, sizeof(rtu_respond_t));

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
				memset(&rtu_request, 0, sizeof(rtu_request_t));
				memset(&rtu_respond, 0, sizeof(rtu_respond_t));

				//清空ini文件载体
				memset(ini_request, 0, sizeof(ini_request));
				memset(ini_respond, 0, sizeof(ini_respond));


				//获取ini文件的数据
				GetPrivateProfileStringA(sectionName, "request", "", ini_request, MAX_PATH, INI_PATH);
				ini_requestlen = GetPrivateProfileIntA(sectionName, "requestlen", 0, INI_PATH);
				GetPrivateProfileStringA(sectionName, "response", "", ini_respond, MAX_PATH, INI_PATH);
				ini_respondlen = GetPrivateProfileIntA(sectionName, "responselen", 0, INI_PATH);
				ini_ret = GetPrivateProfileIntA(sectionName, "return", 0, INI_PATH);


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

				while ((pbuf = strtok(ptmp, " ")) != nullptr)
				{
					rtu_request.request.data[i] = char_to_hex((uint8_t *)pbuf);
					i++;
					ptmp = nullptr;
				}
				i = 0;
				

				result = respond_check(&rtu_respond, &rtu_request, ini_respondlen);
				Assert::AreEqual(result, ini_ret);

				
				sectionCount++;
			}

			//测试null
			result = respond_check(nullptr, nullptr, 0);
			//Assert::AreEqual(result, -1);
		}

	};
}