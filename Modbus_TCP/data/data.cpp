#include "data.h"

CoilBuf_t coilBuf;
RegisterBuf_t regBuf;
int handleRequest(tcp_request_t *m, tcp_respond_t *n)
{
	if (m && n)
	{
		//填充载体的数据域
		switch (get_request_funcode(m))
		{
		case x01_read_coil:
			handle_x01(m, n);
			break;
		case x03_read_registers:
			handle_x03(m, n);
			break;
		case x0f_write_coils:
			handle_x0f(m, n);
			break;
		case x10_write_registers:
			handle_x10(m, n);
			break;
		//default://功能码不正确，设置为异常码01:错误的功能码
			//set_response_funcode(n, get_request_funcode(m) + 0x80);
			//set_respond_errornum(n, exception_x01);//错误的功能码
		//	break;
		}
	}

	//填充tcp包头
	copy_req_to_res(m, n);

	//更新Length
	set_respond_Length(n, get_response_size(n) - 6);

	return Error_Ok;
}

int handle_x01(tcp_request_t *m, tcp_respond_t *n)
{
	if (m && n)
	{	
		unsigned int address = get_request_address(m);
		unsigned short count = get_request_count(m);
		bool state = 0;
		int index = 0;
		int num = 0;

		//判断起始地址是否正确
		if (address >= coil_end_addr || address < coil_start_addr)
		{
			set_response_funcode(n, x80_x01_read_coil);//设置差错功能码
			
			set_respond_errornum(n, exception_x02);//设置异常码，地址索引异常

			return 0;
		}

		//起始地址+查询地址太大+或者count==0
		if (count > MaxCoilCount || address + count > coil_end_addr || count == 0)
		{
			set_response_funcode(n, x80_x01_read_coil);//设置差错功能码

			set_respond_errornum(n, exception_x03);//地址超出范围

			return 0;
		}

		//其他情况就是正确的，填充内存数据到传输载体中
		//配置功能码
		set_response_funcode(n, x01_read_coil);
		
		//配置字节数
		set_response_byte(n, GetCoilCount(count));

		//CoilAddressToIndex：求地址对应的映射
		for (int i = address; i <= address + count; i++)
		{
			if (GetCoilState(coilBuf, i))//coil是true
			{
				n->response.x01.data[index] = setOne(n->response.x01.data[index], num++);//将对应的响应中的位置1
			}
			else//相反
			{
				n->response.x01.data[index] = setZero(n->response.x01.data[index], num++);
			}
			
			if (num == 8)//一个字节弄完了，index++， num从0开始计数
			{
				num = 0;
				index++;
			}
		}

		return 0;
	}

	return Error_Ok;
}

int handle_x03(tcp_request_t *m, tcp_respond_t *n)
{
	if (m && n)
	{
		unsigned int address = get_request_address(m);
		unsigned int count = get_request_count(m);

		//判断起始地址
		if (address < reg_start_addr || address >= reg_end_addr)
		{
			set_response_funcode(n, x80_x03_read_registers);
			set_respond_errornum(n, exception_x02);

			return 0;
		}

		//count > 一次能查询寄存器数量的最大值 || 起始地址+查询地址太大
		if ((count > MaxRegisterCount) || ((address + count) > reg_end_addr) || count == 0)
		{
			set_response_funcode(n, x80_x03_read_registers);
			set_respond_errornum(n, exception_x03);

			return 0;
		}

		//其他都是正确的情况
		//配置功能码
		set_response_funcode(n, x03_read_registers);

		//配置字节数
		set_response_byte(n, 2*count);
	
		//配置数据
		for (int i = address, j = 0; i < count+address; i++, j++)
		{
			n->response.x03.data[2 * j] = (regBuf[i] & 0xff00) >> 8;//高位
			n->response.x03.data[2 * j + 1] = regBuf[i] & 0xff;//低位
		}
	}

	return Error_Ok;
}

int handle_x0f(tcp_request_t *m, tcp_respond_t *n)
{
	if (m && n)
	{
		unsigned short address = get_request_address(m);
		unsigned short count = get_request_count(m);
		bool state = 0;

		//判断起始地址是否正确
		if (address >= coil_end_addr || address < coil_start_addr)
		{
			set_response_funcode(n, x80_x0f_write_coils);//设置差错功能码

			set_respond_errornum(n, exception_x02);//设置异常码，地址索引异常

			return 0;
		}
		
		//起始地址+查询地址太大
		if (count > MaxCoilCount-32 || address + get_request_count(m) > coil_end_addr || count == 0)
		{
			set_response_funcode(n, x80_x0f_write_coils);//设置差错功能码

			set_respond_errornum(n, exception_x03);//地址超出范围

			return 0;
		}

		//内容正确
		//配置功能码
		set_response_funcode(n, x0f_write_coils);

		//配置起始地址
		set_response_address(n, get_request_address(m));

		//配置count
		set_response_count(n, get_request_count(m));

		//配置内存
		for (int i = address; i <= address + count; i++)
		{
			if (GetCoilState(m->request.x0f.data, i - address))//获得地址对应的coil状态,true
			{
				coilBuf[CoilAddressToIndex(i)] = setOne(coilBuf[CoilAddressToIndex(i)], CoilCharBit(i));
			}
			else//false
			{
				coilBuf[CoilAddressToIndex(i)] = setZero(coilBuf[CoilAddressToIndex(i)], CoilCharBit(i));
			}
		}

		return 0;
	}

	return Error_Ok;
}

int handle_x10(tcp_request_t *m, tcp_respond_t *n)
{
	if (m && n)
	{
		unsigned int address = get_request_address(m);
		unsigned short count = get_request_count(m);

		//判断起始地址
		if (address < reg_start_addr || address >= reg_end_addr)
		{
			set_response_funcode(n, x80_x10_write_registers);
			set_respond_errornum(n, exception_x02);

			return 0;
		}

		//count > 一次能写寄存器数量的最大值 || 起始地址+查询地址太大 || 查询的地址不是一个寄存器的开始
		if ((count > MaxRegisterCount-2) || ((address + count) > reg_end_addr) || count == 0)
		{
			set_response_funcode(n, x80_x10_write_registers);
			set_respond_errornum(n, exception_x03);

			return 0;
		}

		//内容正确
		//配置功能码
		set_response_funcode(n, x10_write_registers);
		
		//配置起始地址
		set_response_address(n, get_request_address(m));

		//配置count
		set_response_count(n, get_request_count(m));

		//配置内存
		for (int i = address; i < address + count; i++)
		{
			regBuf[i] = MakeShort(m->request.x10.data[2 * (i - address)], m->request.x10.data[2 * (i - address) + 1]);
		}

		return 0;
	}

	return Error_Ok;
}

int handleErrorFuncode(tcp_request_t *m, tcp_respond_t *n)
{
	if (m && n)
	{
		set_response_funcode(n, get_request_funcode(m)+0x80);//加0x80的差错码
		n->response.data[ERRORNO_INDEX] = 0x01;//设置异常码

		n->tcp_head.Length[1] = 0x03;//设置长度

		return 0;
	}

	return Error_Ok;
}