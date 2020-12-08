#include "../Modbus_RTU/crc/crc.h"
#include "../Modbus_RTU/info/info.h"
#include <sstream>
#include <Windows.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include <iterator>
using namespace std;

#define PATH "../Modbus_RTU/config.ini"

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
	int i = 0;
	int j = 3;

	while (j--)
		i++;

	j = 0;

	system("pause");

	return 0;
}