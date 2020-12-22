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
			char ini_request[MAX_PATH] = {'\0'};
			char ini_respond[MAX_PATH] = {'\0'};
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
			rtu_request_t rtu_request;
			rtu_respond_t rtu_respond;
			memset(&rtu_request, 0, sizeof(rtu_request_t));
			memset(&rtu_respond, 0, sizeof(rtu_respond_t));

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
				memset(&rtu_request, 0, sizeof(rtu_request_t));
				memset(&rtu_respond, 0, sizeof(rtu_respond_t));

				//���ini�ļ�����
				memset(ini_request, 0, sizeof(ini_request));
				memset(ini_respond, 0, sizeof(ini_respond));


				//��ȡini�ļ�������
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

			//����null
			result = respond_check(nullptr, nullptr, 0);
			//Assert::AreEqual(result, -1);
		}

	};
}