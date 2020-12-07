#include "../Modbus_RTU/crc/crc.h"
#include "../Modbus_RTU/info/info.h"
#include <sstream>
#include <Windows.h>
#include <iostream>
#include <cstdint>
using namespace std;

#define PATH "../Test_RTU/config.ini"

uint16_t ChangeNum(uint8_t* str)
{
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
}

int main()
{
	uint8_t p[] = "ae";
	cout << ChangeNum(p) << endl;
	printf("%02x\n", ChangeNum(p));
	
	string str = "01,01,00,01,00,11";
	
	int ret = 0;
	while (ret = str.find(',', ret))
	{
		if (ret == -1)
			break;

		str.replace(ret, 1, 1, ' ');
	}

	stringstream sstr;
	sstr << str;
	int i = 0;
	char nums[6];
	int j = 0;

	cout << str << endl;

	while (sstr >> str)
	{
		nums[j] = ChangeNum((uint8_t *)str.c_str());

		j++;
		i++;

	}

	for (int i = 0; i < 6; i++)
		printf("%02x\t", nums[i]);


	puts("");


	system("pause");

	return 0;
}