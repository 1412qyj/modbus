#ifndef __INFO_H
#define __INFO_H

#include <Windows.h>
#include <iostream>
#include<cstdlib>
#include <cmath>
using namespace std;



#define SLAVE_INDEX			0
#define FUNCTION_INDEX		1
#define ERRORNO_INDEX		2
#define ADDRESS_INDEX0		2
#define ADDRESS_INDEX1		3
#define COUNT_INDEX0		4
#define COUNT_INDEX1		5
#define BYTES_INDEX			6

#define MakeShort(h,l)	( (((short)(h)&0xff)<<8) | ((short)(l)&0xff) )
#define ArrayToShort(a)	( (unsigned short)MakeShort(a[0], a[1]) )
#define ArrayToCRC16(a)	( (unsigned short)MakeShort(a[1], a[0]) )
#define GetCoilCount(c) ( (c/8) + ((c%8) ? 1 : 0) )
#define ShortHig(v)		( (unsigned char)(((v)>> 8) & 0xff) )
#define ShortLow(v)		( (unsigned char)((v) & 0xff) )

typedef enum
{
	MinCoilCount = 1,
	MaxCoilCount = 2000,
}MinMaxCoilCount_t;

typedef enum
{
	MinRegisterCount = 1,
	MaxRegisterCount = 125,
}MinMaxRegisterCount_t;

typedef enum
{
	MinRegisterValue = 0x0000,
	MaxRegisterValue = 0xFFFF,
}RegisterValue_t;

typedef enum
{
	x01_read_coil = 0x01,
	x03_read_registers = 0x03,
	x0f_write_coils = 0x0F,
	x10_write_registers = 0x10,
}request_function_code_t;

typedef enum
{
	x80_x01_read_coil = (0x80 + 0x01),
	x80_x03_read_registers = (0x80 + 0x03),
	x80_x0f_write_coils = (0x80 + 0x0F),
	x80_x10_write_registers = (0x80 + 0x10),
}response_function_code_t;

typedef enum
{
	exception_x01 = 0x01,
	exception_x02 = 0x02,
	exception_x03 = 0x03,
	exception_x04 = 0x04,
}exception_code_t;

typedef enum
{
	Error_Ok = 0,
	Error_NotInit = -1,
	Error_SendFailed = -2,
	Error_InvalidParamter = -3,
	Error_InvalidCoilValue = -5,
	Error_InvalidCoilCount = -6,
	Error_InvalidRegisterCount = -7,
	Error_InvalidRegisterValue = -8,
	Error_Recieving = -40,
	Error_RecvTimeOut = -41,
	Error_InvalidResponseSlave = -42,
	Error_InvalidResponseFunc = -43,
	Error_InvalidResponseAddr = -44,
	Error_InvalidResponseCount = -45,
	Error_InvalidResponseValue = -46,
	Error_InvalidResponseByte = -47,
	Error_InvalidResponseCrc = -48,
	Error_Exception = -49,
	Error_InvalidExceptionCode = -50,
	Error_InvalidFormat = -51,
}ErrorCode_t;


//======================request========================
struct requestm
{
	unsigned char slave;
	unsigned char func;
	unsigned char addr[2];
	unsigned char count[2];
	unsigned char byte;
	unsigned char data[256];
};


struct requestx
{
	unsigned char slave;
	unsigned char func;
	unsigned char addr[2];
	unsigned char count[2];
	unsigned char crc[2];
};

//======================request========================

//======================exception========================

struct exceptions
{
	unsigned char slave;
	unsigned char func;
	unsigned char code;
	unsigned char crc[2];
};


//======================exception========================


//======================responsex========================

struct responsex
{
	unsigned char slave;
	unsigned char func;
	unsigned char byte;
	unsigned char data[256];
};

struct responsew
{
	unsigned char slave;
	unsigned char func;
	unsigned char addr[2];
	unsigned char count[2];
	unsigned char crc[2];
};

//======================responsex========================

// communication vector
typedef struct
{
	union{
		struct requestx x01;	 //read coil
		struct requestx x03;	 //read register
		struct requestm x0f;	 //write multiple coil
		struct requestm x10;	 //write multiple register
		unsigned char data[256]; //convenient for byte wise access
	}request;
}rtu_request_t;

typedef struct
{
	union{
		struct responsex x01;
		struct responsex x03;
		struct responsew x0f;
		struct responsew x10;
		struct exceptions exc;
		unsigned char data[256];
	}response;
}rtu_respond_t;


int input_slave(rtu_request_t *pRequest);
int input_func(rtu_request_t *pRequest);
int input_begin_addr(rtu_request_t *pRequest);
int input_count(rtu_request_t *pRequest);
int input_coils(rtu_request_t *pRequest);
int input_registers(rtu_request_t *pRequest);
int print_request(rtu_request_t *pRequest);
int print_respond(rtu_respond_t *pResponse);
int print_errno(int, rtu_respond_t *m);
char setOne(char p, int count);
char setZero(char p, int count);

#endif