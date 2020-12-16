#include "info.h"
#include "../modbus/modbus.h"

int printErrorno(int errornum)
{
	switch (errornum)
	{
		case Error_Socket:
			cout << "Error_Socket" << endl;
			break;
		case Error_Bind:
			cout << "Error_Bind" << endl;
			break;
		case Error_Listen:
			cout << "Error_Listen" << endl;
			break;
		case Error_Accept:
			cout << "Error_Accept" << endl;
			break;
		case Error_Version:
			cout << "Error_Version" << endl;
			break;
		case Error_Argc:
			cout << "Error_Argc" << endl;
			break;
		case Error_ClientOutLine:
			cout << "Error_ClientOutLine" << endl;
			break;
		case Error_InValidProtocol:
			cout << "Error_InValidProtocol" << endl;
			break;
		case Error_InValidUnitId:
			cout << "Error_InValidUnitId" << endl;
			break;
		case Error_InValidLength:
			cout << "Error_InValidLength" << endl;
			break;
		case Error_InValidfuncode:
			cout << "Error_InValidfuncode" << endl;
			break;
		case Error_InValidCount:
			cout << "Error_InValidCount" << endl;
			break;
		case Error_InValidByte:
			cout << "Error_InValidByte" << endl;
			break;
		default:
			cout << "errornum: " << errornum << "£º ²»¿ÉÃèÊöµÄ´íÎó" << endl;
			break;
	}

	return 0;
}

