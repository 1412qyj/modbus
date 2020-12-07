#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Modbus_RTU/crc/crc.h"
#include "../Modbus_RTU/info/info.h"
#include "../Modbus_RTU/check/check.h"
#include "../Modbus_RTU/modbus/modbus.h"
#include "../Modbus_RTU/uart/uart.h"
#include <Windows.h>
#include <cstdint>
#include <iostream>
#include <sstream>
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
		
		TEST_METHOD(crc)
		{
			// TODO:  在此输入测试代码
			LPSTR pin = new char[MAX_PATH];
			int sout = 0;
			int len = 0;

			GetPrivateProfileStringA("CRC", "input", "", pin, MAX_PATH, INI_PATH);
			len = GetPrivateProfileIntA("CRC", "length", 0, INI_PATH);
			sout = GetPrivateProfileIntA("CRC", "output", 0, INI_PATH);

			stringstream sstr;

			sstr << pin;

			unsigned char *nums = (unsigned char *)malloc(len);
			int i = 0;
			while (sstr >> nums[i])
			{
				nums[i] -= '0';

				i++;
			}

			int ret = __crc_modbus((unsigned char *)nums, len) & 0xffff;

			Assert::AreEqual(sout, ret);
		}

		TEST_METHOD(check)
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



			LPSTR ini_request = new char[MAX_PATH];
			LPSTR ini_respond = new char[MAX_PATH];
			LPSTR ini_requestlen = new char[MAX_PATH];
			LPSTR ini_respondlen = new char[MAX_PATH];
			LPSTR ini_ret = new char[MAX_PATH];

			GetPrivateProfileStringA("CHECK", "request", "", ini_request, MAX_PATH, INI_PATH);
			GetPrivateProfileStringA("CHECK", "requestlen", "", ini_requestlen, MAX_PATH, INI_PATH);
			GetPrivateProfileStringA("CHECK", "response", "", ini_respond, MAX_PATH, INI_PATH);
			GetPrivateProfileStringA("CHECK", "responselen", "", ini_respondlen, MAX_PATH, INI_PATH);
			GetPrivateProfileStringA("CHECK", "return", "", ini_ret, MAX_PATH, INI_PATH);

			stringstream sstr_request;
			stringstream sstr_respond;
			stringstream sstr_requestlen;
			stringstream sstr_respondlen;
			stringstream sstr_ret;
			stringstream sstr_tmp;

			sstr_request << ini_request;
			sstr_respond << ini_respond;
			sstr_respondlen << ini_respondlen;
			sstr_requestlen << ini_requestlen;
			sstr_ret << ini_ret;

			rtu_request_t rtu_request;
			rtu_respond_t rtu_respond;
			memset(&rtu_request, 0, sizeof(rtu_request_t));
			memset(&rtu_respond, 0, sizeof(rtu_respond_t));

			string str_request;
			string str_respond;
			string str_tmp;
			str_request.clear();
			str_respond.clear();
			str_tmp.clear();

			int rtu_requestlen = 0;
			int rtu_respondlen = 0;
			int rtu_ret = 0;

			int i = 0;

			while (sstr_request >> str_request &&
				sstr_respond >> str_respond &&
				sstr_requestlen >> rtu_respondlen &&
				sstr_respondlen >> rtu_respondlen &&
				sstr_ret >> rtu_ret)
			{
				//将一串报文的'，'改成空格
				int ret = 0;
				while (ret = str_request.find(',', ret))
				{
					if (ret == -1)
						break;

					str_request.replace(ret, 1, 1, ' ');
				}//此时格式为 xx xx xx xx

				sstr_tmp << str_request;
				while (sstr_tmp >> str_tmp)
				{
					rtu_request.request.data[i] = char_to_hex((uint8_t *)str_tmp.c_str());

					i++;
				}

				ret = 0;
				//这里在转换respond的报文
				while (ret = str_respond.find(',', ret))
				{
					if (ret == -1)
						break;

					str_respond.replace(ret, 1, 1, ' ');
				}

				i = 0;
				stringstream sstr_tmp2;
				sstr_tmp2 << str_respond;

				while (sstr_tmp2 >> str_tmp)
				{
					rtu_respond.response.data[i] = char_to_hex((uint8_t *)str_tmp.c_str());
				}

				Assert::AreEqual(respond_check(&rtu_respond, &rtu_request), rtu_ret);
			}

		}

	};
}