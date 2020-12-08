#include "modbus.h"
#include "../crc/crc.h"


unsigned char coilInfos[125];
unsigned char registerInfos[256];
int set_request_slave(rtu_request_t* m, int slave)
{
	if (m)
	{
		m->request.data[SLAVE_INDEX] = (slave & 0xff);
	}
	return Error_Ok;
}

int get_request_slave(rtu_request_t* m)
{
	if (m)
	{
		return 0xff & m->request.data[SLAVE_INDEX];
	}
	return 0;
}


int get_request_funcode(rtu_request_t* m)
{
	if (m)
	{
		return 0xff & m->request.data[FUNCTION_INDEX];
	}
	return -1;
}
int set_request_funcode(rtu_request_t* m, int funtion)
{
	if (m)
	{
		m->request.data[FUNCTION_INDEX] = (funtion & 0xff);
	}
	return Error_Ok;
}

int get_request_address(rtu_request_t* m)
{
	int address = 0;

	if (m)
	{
		address += 0xff00 & (m->request.data[ADDRESS_INDEX0] << 8);
		address |= 0xff & (m->request.data[ADDRESS_INDEX1] << 0);
	}
	return address;
}
int set_request_address(rtu_request_t* m, int address)
{
	if (m)
	{
		m->request.data[ADDRESS_INDEX0] = (address >> 8) & 0xff;
		m->request.data[ADDRESS_INDEX1] = (address >> 0) & 0xff;
	}
	return Error_Ok;
}

int get_request_count(rtu_request_t* m)
{
	int count = 0;

	switch (get_request_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		count += 0xff00 & m->request.x01.count[0] << 8;
		count += 0xff & m->request.x01.count[1] << 0;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		count += 0xff & m->request.x10.count[0] << 8;
		count += 0xff & m->request.x10.count[1] << 0;
		break;
	}
	return count;
}
int set_request_count(rtu_request_t* m, int count)
{
	switch (get_request_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		m->request.x01.count[0] = (count >> 8) & 0xff;
		m->request.x01.count[1] = (count >> 0) & 0xff;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		m->request.x10.count[0] = (count >> 8) & 0xff;
		m->request.x10.count[1] = (count >> 0) & 0xff;
		break;
	}
	return Error_Ok;
}

int get_request_byte(rtu_request_t* m)
{
	switch (get_request_funcode(m))
	{
	case x0f_write_coils:
	case x10_write_registers:
		return (0xff & m->request.x10.byte);
	}
	return Error_Ok;
}

int set_request_byte(rtu_request_t* m, int byte)
{
	switch (get_request_funcode(m))
	{
	case x0f_write_coils:
	case x10_write_registers:
		m->request.data[BYTES_INDEX] = (byte & 0xff);
		break;
	}
	return Error_Ok;
}


int get_request_crc(rtu_request_t* m)
{
	int crc = 0;
	int index = 0;

	switch (m->request.data[1])
	{
	case x01_read_coil:
	case x03_read_registers:
		crc += m->request.x01.crc[1] << 8;
		crc += m->request.x01.crc[0] << 0;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		index = 0xff & m->request.x10.byte;
		crc += m->request.x10.data[index + 1] << 8;
		crc += m->request.x10.data[index + 0] << 0;
		break;
	}
	return crc;
}
int set_request_crc(rtu_request_t* m, int crc)
{
	int index = 0;

	switch (m->request.data[1])
	{
	case x01_read_coil:
	case x03_read_registers:
		m->request.x01.crc[0] = (crc >> 0) & 0xff;
		m->request.x01.crc[1] = (crc >> 8) & 0xff;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		index = 0xff & m->request.x10.byte;
		m->request.x10.data[index + 0] = (crc >> 0) & 0xff;
		m->request.x10.data[index + 1] = (crc >> 8) & 0xff;
		break;
	}
	return Error_Ok;
}

int calc_request_crc(rtu_request_t* m)
{
	int len = get_request_length(m) - 2;
	if (m)
	{
		if (len >= sizeof(m->request))
		{
			return 0x0000;
		}
		return 0xffff & crc_modbus(m->request.data, len);
	}
	return 0x0000;
}


int get_response_length(rtu_respond_t* m)
{
	int len = 0;

	switch (get_response_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		/* ************
		* slave(1)
		* func(1)
		* byte(1)
		* data(byte)
		* crc(2)
		* ************/
		len = 1 + 1 + 1 + m->response.x01.byte + 2;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		/* ***********
		* slave(1)
		* func(1)
		* addr(2)
		* count(2)
		* crc(2)
		* **********/
		len = 1 + 1 + 2 + 2 + 2;
		break;
	case x80_x01_read_coil:
	case x80_x03_read_registers:
	case x80_x0f_write_coils:
	case x80_x10_write_registers:
		/* ***********
		* slave(1)
		* exfunc(1)
		* excode(1)
		* crc(2)
		* **********/
		len = 1 + 1 + 1 + 2;
		break;
	}
	return len;
}

int get_response_slave(rtu_respond_t* m)
{
	return (0xff & m->response.data[SLAVE_INDEX]);
}

//int set_response_slave(rtu_respond_t* m, int slave)
//{
//	if (m)
//	{
//		m->response.data[SLAVE_INDEX] = (0xff & slave);
//	}
//	return Error_Ok;
//}

int get_response_funcode(rtu_respond_t* m)
{
	if (m)
	{
		return (0xff & m->response.data[FUNCTION_INDEX]);
	}
	return 0xff;
}

//int set_response_funcode(rtu_respond_t* m, int funtion)
//{
//	if (m)
//	{
//		m->response.data[FUNCTION_INDEX] = (0xff & funtion);
//	}
//	return Error_Ok;
//}

int get_response_address(rtu_respond_t* m)
{
	int address = 0;

	switch (get_response_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		break;
	case x0f_write_coils:
	case x10_write_registers:
		address += (0xff00 & m->response.x10.addr[0] << 8);
		address += (0xff & m->response.x10.addr[1] << 0);
		break;
	}
	return address;
}

//int set_response_address(rtu_respond_t* m, int address)
//{
//	switch (get_response_funcode(m))
//	{
//	case x01_read_coil:
//	case x03_read_registers:
//		break;
//	case x0f_write_coils:
//	case x10_write_registers:
//		m->response.x10.addr[0] = (0xff & (address >> 8));
//		m->response.x10.addr[1] = (0xff & (address >> 0));
//		break;
//	}
//	return Error_Ok;
//}

int get_response_count(rtu_respond_t* m)
{
	int count = 0;

	switch (get_response_funcode(m))
	{
	case x01_read_coil:
		count = (m->response.x01.byte * 8);
		break;
	case x03_read_registers:
		count = (0xff & (m->response.x01.byte / 2));
		break;
	case x0f_write_coils:
	case x10_write_registers:
		count += (m->response.x10.count[0] << 8);
		count += (m->response.x10.count[1] << 0);
		break;
	}
	return count;

}


//int set_response_count(rtu_respond_t* m, int count)
//{
//	switch (get_response_funcode(m))
//	{
//	case x01_read_coil:
//		m->response.x01.byte = 0xff & (count / 8);
//		break;
//	case x03_read_registers:
//		m->response.x01.byte = 0xff & (count * 2);
//		break;
//	case x0f_write_coils:
//	case x10_write_registers:
//		m->response.x10.count[0] = 0xff & (count >> 8);
//		m->response.x10.count[1] = 0xff & (count >> 0);
//		break;
//	}
//	return Error_Ok;
//}

int get_response_byte(rtu_respond_t* m)
{
	int byte = 0;

	switch (get_response_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		byte = (0xff & m->response.x01.byte);
		break;
	}
	return byte;
}
//int set_response_byte(rtu_respond_t* m, int byte)
//{
//	switch (get_response_funcode(m))
//	{
//	case x01_read_coil:
//	case x03_read_registers:
//		m->response.x01.byte = (0xff & byte);
//		break;
//	}
//	return Error_Ok;
//}

int get_request_length(rtu_request_t* m)
{
	int len = 0;

	switch (get_request_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		/* ***************
		* slave(1)
		* func(1)
		* addr(2)
		* count(2)
		* crc(2)
		* ***************/
		len = 1 + 1 + 2 + 2 + 2;
		break;
	case x0f_write_coils:
	case x10_write_registers:
		/* *************
		* slave(1)
		* func(1)
		* addr(2)
		* count(2)
		* byte(1)
		* data()
		* crc(2)
		* *************/
		len = 1 + 1 + 2 + 2 + 1 + m->request.x10.byte + 2;
		break;
	}
	return len;
}

int get_response_crc(rtu_respond_t* m)
{
	int crc = 0;
	int index = 0;

	switch (get_response_funcode(m))
	{
	case x01_read_coil:
	case x03_read_registers:
		index = m->response.x01.byte;
		crc += (m->response.x01.data[index + 1] << 8);
		crc += (m->response.x01.data[index + 0] << 0);
		break;
	case x0f_write_coils:
	case x10_write_registers:
		crc += (m->response.x10.crc[1] << 8);
		crc += (m->response.x10.crc[0] << 0);
		break;
	case x80_x01_read_coil:
	case x80_x03_read_registers:
	case x80_x0f_write_coils:
	case x80_x10_write_registers:
		crc += (m->response.exc.crc[1] << 8);
		crc += (m->response.exc.crc[0] << 0);
		break;
	}
	return crc;
}



