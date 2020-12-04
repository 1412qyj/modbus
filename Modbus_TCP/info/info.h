#ifndef __INFO_H
#define __INFO_H

#include <iostream>
#include <cstdio>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")//Ìí¼Ósocket¿â
using namespace std;


#define FUNCTION_INDEX		0
#define ERRORNO_INDEX		1
#define ADDRESS_INDEX0		1
#define ADDRESS_INDEX1		2
#define COUNT_INDEX0		3
#define COUNT_INDEX1		4
#define BYTES_INDEX			5
#define Slave_Addr          0x01

#define showhex(data, len)						\
do{												\
	int i = 0;									\
	int l = (int)len;							\
	unsigned char *p = (unsigned char*)data;	\
												\
	if( NULL == p || l < 1 )					\
		{											\
		break;									\
		}											\
	for(i = 0; i < l; i++)						\
		{											\
		printf("%02X ", p[i]);					\
		}											\
	printf("\n");								\
}while(0)

typedef char CoilBuf_t[250];
typedef short RegisterBuf_t[125];


#define MakeShort(h,l)	( (((short)(h)&0xff)<<8) | ((short)(l)&0xff) )
#define ArrayToShort(a)	( (unsigned short)MakeShort(a[0], a[1]) )
#define GetCoilCount(c) ( (c/8) + ((c%8) ? 1 : 0) )
#define ShortHig(v)		( (unsigned char)(((v)>> 8) & 0xff) )
#define ShortLow(v)		( (unsigned char)((v) & 0xff) )
#define CoilAddressToIndex(a) ((a)/8)
#define CoilCharBit(a)        ((a)%8)
#define GetCoilState(buf, a)       ( (buf[CoilAddressToIndex(a)]) & (0x1 << (CoilCharBit(a) ) ))
#define GetRegIndex(a)  ((a-reg_start_addr)/4)


typedef enum{
	coil_start_addr = 0x0000,
	coil_end_addr   = 0x07D0,
}coil_addr;

typedef enum{
	reg_start_addr = 0x0000,
	reg_end_addr   = 0x007C
}reg_addr;

typedef enum
{
	MinAddress = 0x0000,
	MaxAddress = 0xFFFF,
}MinMaxAddressSize_t;

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
	exception_x05 = 0x05,
	exception_x06 = 0x06,
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
	Error_InvalidResponseFunc = -43,
	Error_InvalidResponseAddr = -44,
	Error_InvalidResponseCount = -45,
	Error_InvalidResponseValue = -46,
	Error_InvalidResponseByte = -47,
	Error_Exception = -49,
	Error_Socket = -60,
	Error_Bind = -61,
	Error_Listen = -62,
	Error_Accept = -63,
	Error_Version = -64,
	Error_Argc = -65,
	Error_ClientOutLine = -66,
	Error_InValidProtocol = -67, 
	Error_InValidUnitId = -68,
	Error_InValidLength = -69,
	Error_InValidfuncode = -70,
	Error_InValidCount = -71,
	Error_InValidByte = -72,
	Error_Datasize = -73,
}ErrorCode_t;


//======================request========================
struct requestm
{
	unsigned char func;
	unsigned char addr[2];
	unsigned char count[2];
	unsigned char byte;
	unsigned char data[256];
};


struct requestx
{
	unsigned char func;
	unsigned char addr[2];
	unsigned char count[2];
};

//======================request========================

//======================exception========================

struct exceptions
{
	unsigned func;
	unsigned code;
};


//======================exception========================


//======================responsex========================

struct responsex
{
	unsigned char func;
	unsigned char byte;
	unsigned char data[256];
};

struct responsew
{
	unsigned char func;
	unsigned char addr[2];
	unsigned char count[2];
};

//======================responsex========================


// =================communication vector====================
#pragma pack(1)
typedef struct{
	unsigned char tranId[2];
	unsigned char ProtoId[2];
	unsigned char Length[2];
	unsigned char UnitId;
}tcp_head_t;
#pragma pack(1)

typedef struct {
	tcp_head_t tcp_head;
	union{
		struct requestx x01;	 //read coil
		struct requestx x03;	 //read register
		struct requestm x0f;	 //write multiple coil
		struct requestm x10;	 //write multiple register
		unsigned char data[256]; //convenient for byte wise access
	}request;
}tcp_request_t;
#pragma pack(1)


typedef struct {
	tcp_head_t tcp_head;
	union{
		struct responsex x01;
		struct responsex x03;
		struct responsew x0f;
		struct responsew x10;
		struct exceptions exc;
		unsigned char data[256];
	}response;
}tcp_respond_t;
#pragma pack(1)


int printRequest(tcp_request_t *m);
int printRespond(tcp_respond_t *m);
int printErrorno(int errornum);


#endif