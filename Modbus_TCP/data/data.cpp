#include "data.h"

CoilBuf_t coilBuf;
RegisterBuf_t regBuf;
int handleRequest(tcp_request_t *m, tcp_respond_t *n)
{
	if (m && n)
	{
		//��������������
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
		//default://�����벻��ȷ������Ϊ�쳣��01:����Ĺ�����
			//set_response_funcode(n, get_request_funcode(m) + 0x80);
			//set_respond_errornum(n, exception_x01);//����Ĺ�����
		//	break;
		}
	}

	//���tcp��ͷ
	copy_req_to_res(m, n);

	//����Length
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

		//�ж���ʼ��ַ�Ƿ���ȷ
		if (address >= coil_end_addr || address < coil_start_addr)
		{
			set_response_funcode(n, x80_x01_read_coil);//���ò������
			
			set_respond_errornum(n, exception_x02);//�����쳣�룬��ַ�����쳣

			return 0;
		}

		//��ʼ��ַ+��ѯ��ַ̫��+����count==0
		if (count > MaxCoilCount || address + count > coil_end_addr || count == 0)
		{
			set_response_funcode(n, x80_x01_read_coil);//���ò������

			set_respond_errornum(n, exception_x03);//��ַ������Χ

			return 0;
		}

		//�������������ȷ�ģ�����ڴ����ݵ�����������
		//���ù�����
		set_response_funcode(n, x01_read_coil);
		
		//�����ֽ���
		set_response_byte(n, GetCoilCount(count));

		//CoilAddressToIndex�����ַ��Ӧ��ӳ��
		for (int i = address; i <= address + count; i++)
		{
			if (GetCoilState(coilBuf, i))//coil��true
			{
				n->response.x01.data[index] = setOne(n->response.x01.data[index], num++);//����Ӧ����Ӧ�е�λ��1
			}
			else//�෴
			{
				n->response.x01.data[index] = setZero(n->response.x01.data[index], num++);
			}
			
			if (num == 8)//һ���ֽ�Ū���ˣ�index++�� num��0��ʼ����
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

		//�ж���ʼ��ַ
		if (address < reg_start_addr || address >= reg_end_addr)
		{
			set_response_funcode(n, x80_x03_read_registers);
			set_respond_errornum(n, exception_x02);

			return 0;
		}

		//count > һ���ܲ�ѯ�Ĵ������������ֵ || ��ʼ��ַ+��ѯ��ַ̫��
		if ((count > MaxRegisterCount) || ((address + count) > reg_end_addr) || count == 0)
		{
			set_response_funcode(n, x80_x03_read_registers);
			set_respond_errornum(n, exception_x03);

			return 0;
		}

		//����������ȷ�����
		//���ù�����
		set_response_funcode(n, x03_read_registers);

		//�����ֽ���
		set_response_byte(n, 2*count);
	
		//��������
		for (int i = address, j = 0; i < count+address; i++, j++)
		{
			n->response.x03.data[2 * j] = (regBuf[i] & 0xff00) >> 8;//��λ
			n->response.x03.data[2 * j + 1] = regBuf[i] & 0xff;//��λ
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

		//�ж���ʼ��ַ�Ƿ���ȷ
		if (address >= coil_end_addr || address < coil_start_addr)
		{
			set_response_funcode(n, x80_x0f_write_coils);//���ò������

			set_respond_errornum(n, exception_x02);//�����쳣�룬��ַ�����쳣

			return 0;
		}
		
		//��ʼ��ַ+��ѯ��ַ̫��
		if (count > MaxCoilCount-32 || address + get_request_count(m) > coil_end_addr || count == 0)
		{
			set_response_funcode(n, x80_x0f_write_coils);//���ò������

			set_respond_errornum(n, exception_x03);//��ַ������Χ

			return 0;
		}

		//������ȷ
		//���ù�����
		set_response_funcode(n, x0f_write_coils);

		//������ʼ��ַ
		set_response_address(n, get_request_address(m));

		//����count
		set_response_count(n, get_request_count(m));

		//�����ڴ�
		for (int i = address; i <= address + count; i++)
		{
			if (GetCoilState(m->request.x0f.data, i - address))//��õ�ַ��Ӧ��coil״̬,true
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

		//�ж���ʼ��ַ
		if (address < reg_start_addr || address >= reg_end_addr)
		{
			set_response_funcode(n, x80_x10_write_registers);
			set_respond_errornum(n, exception_x02);

			return 0;
		}

		//count > һ����д�Ĵ������������ֵ || ��ʼ��ַ+��ѯ��ַ̫�� || ��ѯ�ĵ�ַ����һ���Ĵ����Ŀ�ʼ
		if ((count > MaxRegisterCount-2) || ((address + count) > reg_end_addr) || count == 0)
		{
			set_response_funcode(n, x80_x10_write_registers);
			set_respond_errornum(n, exception_x03);

			return 0;
		}

		//������ȷ
		//���ù�����
		set_response_funcode(n, x10_write_registers);
		
		//������ʼ��ַ
		set_response_address(n, get_request_address(m));

		//����count
		set_response_count(n, get_request_count(m));

		//�����ڴ�
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
		set_response_funcode(n, get_request_funcode(m)+0x80);//��0x80�Ĳ����
		n->response.data[ERRORNO_INDEX] = 0x01;//�����쳣��

		n->tcp_head.Length[1] = 0x03;//���ó���

		return 0;
	}

	return Error_Ok;
}