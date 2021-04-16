#include "modbus_TCP.h"
#include "modbus_common.h"

//TCP服务器入口
bool TCP_server_entry()
{
	The_divider();
	printf("               进入TCP服务器\n");
	The_divider();
	modbusMapping_t *mapping;
	mapping = (modbusMapping_t *)malloc(sizeof(modbusMapping_t));
	Init_register(mapping);
	The_divider();
	int32_t port;
	port = Enter_port_number();
	int32_t station;
	station = Enter_site_number();
	The_divider();
	SOCKET slisten, sClient;
	if (!Server_connection_configuration(&slisten, &sClient, port))
	{
		printf("连接失败！\n");
		closesocket(sClient);
		closesocket(slisten);
		WSACleanup();
		return 0;
	}

	Set_socket_timeout(&sClient, TCP_WR_TIMEOUT);

	uint8_t *readMessage;
	int16_t readLength = 0;
	uint8_t *sendMessage;
	sendMessage = (uint8_t *)malloc(sizeof(uint8_t)* 300);
	uint16_t sendLength = 0;
	bool flag = 0;
	while (1)
	{
		The_divider();
		//接收报文
		readLength = 0;
		readMessage = Receive_message(&sClient, &readLength, &flag);
		if (flag == 1)
		{
			printf("客户端断开连接!程序结束\n");
			The_divider();
			break;
		}
		printf("Rx：\t");
		TCP_print_data(readMessage, readLength);
		printf("\n");

		//解析报文
		sendLength = Process_response_message(readMessage, sendMessage, readLength, mapping, station);

		//发送报文
		if (sendLength != 0)
		{
			Send_message(&sClient, sendMessage, &sendLength);
			printf("Tx：\t");
			TCP_print_data(sendMessage, sendLength);
			printf("\n");
		}
	}
	closesocket(sClient);
	closesocket(slisten);
	WSACleanup();
	return 0;
}

//初始化寄存器
bool Init_register(modbusMapping_t *mapping)
{
	mapping->tabCoils = (bool *)malloc(sizeof(bool)* MODBUS_MAX_BITS);
	mapping->tabRegisters = (uint16_t *)malloc(sizeof(uint16_t)* MODBUS_MAX_REGISTERS);
	int i;
	for (i = MODBUS_MAX_BITS - 1; i >= 0; i--)
	{
		mapping->tabCoils[i] = false;
	}
	for (i = MODBUS_MAX_REGISTERS - 1; i >= 0; i--)
	{
		mapping->tabRegisters[i] = 0xffff;
	}
	printf("初始化线圈和寄存器成功\n");
	return true;
}

//输入端口号
int32_t Enter_port_number()
{
	int32_t port;
	printf("请输入端口号：");
	scanf("%d", &port);
	while (port<MIN_PORT_NUMBLE || port>MAX_PORT_NUMBLE)
	{
		printf("error：输入端口号超出范围！\n请重新输入端口号：");
		scanf("%d", &port);
	}
	return port;
}

//输入站点号
int32_t Enter_site_number()
{
	int32_t station;
	printf("请输入站点号：");
	scanf("%d", &station);
	while (station<MIN_SITE_NUMBLE || station>MAX_SITE_NUMBLE)
	{
		printf("error：输入站点号超出范围！\n请重新输入站点号：");
		scanf("%d", &station);
	}
	return station;
}

//服务器连接配置
bool Server_connection_configuration(SOCKET *slisten, SOCKET *sClient, int16_t port)
{
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建套接字
	*slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*slisten == INVALID_SOCKET)
	{
		printf("创建套接字失败 !\n");
		return 0;
	}
	else printf("创建套接字成功 !\n");

	//绑定IP和端口
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(*slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("绑定端口失败 !\n");
		return 0;
	}
	else printf("绑定端口成功 !\n");

	//开始监听
	if (listen(*slisten, 5) == SOCKET_ERROR)
	{
		printf("监听失败 !\n");
		return 0;
	}
	else printf("监听成功 !\n");

	//建立连接
	struct sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	while (1)
	{
		printf("等待连接...\n");
		*sClient = accept(*slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
		if (*sClient == INVALID_SOCKET)
		{
			printf("连接失败 !\n");
			continue;
		}
		printf("接收到一个连接：%s \r\n", inet_ntoa(remoteAddr.sin_addr));
		break;
	}
	printf("连接成功 !\n");
	return 1;
}

//设置读写超时时间
bool Set_socket_timeout(SOCKET *sClient, int32_t t)
{
	DWORD Timeout = t;
	if (setsockopt(*sClient, SOL_SOCKET, SO_SNDTIMEO, (char *)&Timeout, sizeof(Timeout)) == SOCKET_ERROR)
	{
		return false;
	}


	Timeout = t;
	if (setsockopt(*sClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&Timeout, sizeof(Timeout)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

//接收报文
uint8_t *Receive_message(SOCKET *sClient, int16_t *tcpAduLength, bool *flag)
{
	char revData[1000];
	while (1)
	{
		*tcpAduLength = recv(*sClient, revData, 1000, 0);
		if (*tcpAduLength > 0) break;
		else if (*tcpAduLength == 0)
		{
			*flag = 1;
			return NULL;
		}
		else
		{
			if (send(*sClient, "FF", 2, 0) > 0)
			{
				continue;
			}
			else
			{
				*flag = 1;
				return NULL;
			}
		}
	}
	printf("接收到数据\n");
	uint8_t *lpBuf;
	lpBuf = (uint8_t *)malloc(sizeof(uint8_t)* *tcpAduLength);
	for (int i = 0; i < *tcpAduLength; i++)
	{
		lpBuf[i] = revData[i];
	}
	return lpBuf;
}

//发送报文
bool Send_message(SOCKET *sClient, uint8_t *responseMessage, uint16_t *tcpResponseLength)
{
	char *sendData;
	sendData = (char *)malloc(sizeof(char)* *tcpResponseLength);
	for (int i = 0; i < *tcpResponseLength; i++)
	{
		sendData[i] = responseMessage[i];
	}
	int length = send(*sClient, sendData, *tcpResponseLength, 0);
	if (length == *tcpResponseLength)
	{
		printf("发送成功\n");
		return 1;
	}
	else
	{
		printf("发送失败\n");
		return 0;
	}
}

//解析报文
uint16_t Process_response_message(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping, int32_t station)
{
	printf("判断报文长度\n");
	if (Determine_message_length(receiveMessageLength) == 0) return 0;

	printf("解析MBAP报文头\n");
	if (Determine_MBAP_header(receiveMessage, receiveMessageLength, station) == 0) return 0;

	responseMessage[0] = receiveMessage[0];
	responseMessage[1] = receiveMessage[1];
	responseMessage[2] = receiveMessage[2];
	responseMessage[3] = receiveMessage[3];
	responseMessage[6] = receiveMessage[6];

	switch (receiveMessage[7])
	{
	case 1: return Process_0x01_function_code(receiveMessage, responseMessage, receiveMessageLength, mapping);
	case 3: return Process_0x03_function_code(receiveMessage, responseMessage, receiveMessageLength, mapping);
	case 15: return Process_0x0F_function_code(receiveMessage, responseMessage, receiveMessageLength, mapping);
	case 16: return Process_0x10_function_code(receiveMessage, responseMessage, receiveMessageLength, mapping);
	default: return Generate_exception_response_message(responseMessage, receiveMessage[7], 1);
	}

	return 0;
}

//判断报文长度
bool Determine_message_length(uint16_t receiveMessageLength)
{
	if (receiveMessageLength >= MODBUS_MIN_TCP_ADU_LENGTH && receiveMessageLength <= MODBUS_MAX_TCP_ADU_LENGTH)
	{
		printf("报文长度符合要求\n");
		return 1;
	}
	else if (receiveMessageLength < MODBUS_MIN_TCP_ADU_LENGTH)
	{
		printf("error:报文长度过短，丢弃\n");
		return 0;
	}
	else
	{
		printf("error:报文长度过长，丢弃\n");
		return 0;
	}
}

//判断MBAP报文头
bool Determine_MBAP_header(uint8_t *receiveMessage, uint16_t receiveMessageLength, int32_t station)
{
	if ((receiveMessage[4] * 256 + receiveMessage[5]) == (receiveMessageLength - 6))
	{
		printf("附加长度信息检验正确\n");
	}
	else
	{
		printf("error：附加长度信息错误，丢弃\n");
		return 0;
	}

	if ((receiveMessage[2] * 256 + receiveMessage[3]) == 0)
	{
		printf("协议类型检验正确\n");
	}
	else
	{
		printf("error：协议类型错误，丢弃\n");
		return 0;
	}

	if (receiveMessage[6] == station)
	{
		printf("单元标识符检验正确\n");
	}
	else
	{
		printf("error：单元标识符错误，丢弃\n");
		return 0;
	}
	return 1;
}

//处理0x01功能码
uint16_t Process_0x01_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping)
{
	int i, j;
	if (receiveMessageLength != 12)
	{
		printf("error：PDU长度错误，丢弃\n");
		return 0;
	}
	int start_address = receiveMessage[8] * 256 + receiveMessage[9];
	int read_number = receiveMessage[10] * 256 + receiveMessage[11];
	if (read_number > MODBUS_MAX_READ_BITS || read_number < MODBUS_MIN_READ_BITS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 3);
	else if (start_address >(MODBUS_MAX_BITS - 1) || (start_address + read_number) > MODBUS_MAX_BITS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 2);
	else
	{
		i = 0;
		while ((read_number - 8 * i) > 8)
		{
			responseMessage[9 + i] =
				mapping->tabCoils[start_address + i * 8 + 0] +
				mapping->tabCoils[start_address + i * 8 + 1] * 2 +
				mapping->tabCoils[start_address + i * 8 + 2] * 4 +
				mapping->tabCoils[start_address + i * 8 + 3] * 8 +
				mapping->tabCoils[start_address + i * 8 + 4] * 16 +
				mapping->tabCoils[start_address + i * 8 + 5] * 32 +
				mapping->tabCoils[start_address + i * 8 + 6] * 64 +
				mapping->tabCoils[start_address + i * 8 + 7] * 128;
			i++;
		}
		responseMessage[9 + i] = 0;
		uint8_t count = read_number - 8 * i;
		for (j = 0; j < count; j++)
		{
			responseMessage[9 + i] = responseMessage[9 + i] + ((mapping->tabCoils[start_address + i * 8 + j])*(1 << j));
		}
		i++;
		responseMessage[4] = (i + 3) / 256;
		responseMessage[5] = (i + 3) % 256;
		responseMessage[7] = receiveMessage[7];
		responseMessage[8] = i;
		printf("读取线圈成功\n");
		return i + 9;
	}
}

//处理0x03功能码
uint16_t Process_0x03_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping)
{
	int i;
	if (receiveMessageLength != 12)
	{
		printf("error：PDU长度错误，丢弃");
		return 0;
	}
	int start_address = receiveMessage[8] * 256 + receiveMessage[9];
	int read_number = receiveMessage[10] * 256 + receiveMessage[11];
	if (read_number > MODBUS_MAX_WR_READ_REGISTERS || read_number < MODBUS_MIN_WR_WRITE_REGISTERS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 3);
	else if (start_address >(MODBUS_MAX_REGISTERS - 1) || (start_address + read_number) > MODBUS_MAX_REGISTERS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 2);
	else
	{
		i = 0;
		while ((read_number - i / 2) > 0)
		{
			responseMessage[9 + i] = mapping->tabRegisters[start_address + i / 2] >> 8;
			responseMessage[10 + i] = mapping->tabRegisters[start_address + i * 8 + 0] & 0x00FF;
			i = i + 2;
		}
		responseMessage[4] = (i + 3) / 256;
		responseMessage[5] = (i + 3) % 256;
		responseMessage[7] = receiveMessage[7];
		responseMessage[8] = i;
		printf("读取寄存器成功\n");
		return i + 9;
	}
}

//处理0x0F功能码
uint16_t Process_0x0F_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping)
{
	int i, j;
	if (((receiveMessage[10] * 256 + receiveMessage[11]) / 8 + !((receiveMessage[10] * 256 + receiveMessage[11]) % 8 == 0)) != receiveMessage[12])
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 3);
	int start_address = receiveMessage[8] * 256 + receiveMessage[9];
	int write_number = receiveMessage[10] * 256 + receiveMessage[11];
	if (write_number > MODBUS_MAX_WRITE_BITS || write_number < MODBUS_MIN_WRITE_BITS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 3);
	else if (start_address >(MODBUS_MAX_BITS - 1) || (start_address + write_number) > MODBUS_MAX_BITS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 2);
	else
	{
		if (receiveMessage[12] != (receiveMessageLength - 13))
		{
			printf("error：PDU长度错误，丢弃");
			return 0;
		}
		else
		{
			i = 0;
			while ((write_number - 8 * i) > 0)
			{
				if ((write_number - 8 * i) >= 8)
				{
					for (j = 0; j <= 7; j++)
					{
						mapping->tabCoils[start_address + i * 8 + j] = (receiveMessage[13 + i] / (1 << j)) & 1;
					}
				}
				else
				{
					uint8_t count = write_number - 8 * i;
					for (j = 0; j < count; j++)
					{
						mapping->tabCoils[start_address + i * 8 + j] = (receiveMessage[13 + i] / (1 << j)) & 1;
					}
				}
				i++;
			}
			responseMessage[4] = 0;
			responseMessage[5] = 6;
			responseMessage[7] = receiveMessage[7];
			responseMessage[8] = receiveMessage[8];
			responseMessage[9] = receiveMessage[9];
			responseMessage[10] = receiveMessage[10];
			responseMessage[11] = receiveMessage[11];
			printf("写线圈成功\n");
			return 12;
		}
	}
}

//处理0x10功能码
uint16_t Process_0x10_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping)
{
	int i;
	if (((receiveMessage[10] * 256 + receiveMessage[11]) * 2) != receiveMessage[12])
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 3);
	int start_address = receiveMessage[8] * 256 + receiveMessage[9];
	int write_number = receiveMessage[10] * 256 + receiveMessage[11];
	if (write_number > MODBUS_MAX_WR_WRITE_REGISTERS || write_number < MODBUS_MIN_WR_WRITE_REGISTERS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 3);
	else if (start_address >(MODBUS_MAX_REGISTERS - 1) || (start_address + write_number) > MODBUS_MAX_REGISTERS)
		return Generate_exception_response_message(responseMessage, receiveMessage[7], 2);
	else
	{
		if (receiveMessage[12] != (receiveMessageLength - 13))
		{
			printf("error：PDU长度错误，丢弃");
			return 0;
		}
		else
		{
			i = 0;
			while ((write_number - i / 2) > 0)
			{
				mapping->tabRegisters[start_address + i / 2] = receiveMessage[13 + i] * 256 + receiveMessage[14 + i];
				i = i + 2;
			}
			responseMessage[4] = 0;
			responseMessage[5] = 6;
			responseMessage[7] = receiveMessage[7];
			responseMessage[8] = receiveMessage[8];
			responseMessage[9] = receiveMessage[9];
			responseMessage[10] = receiveMessage[10];
			responseMessage[11] = receiveMessage[11];
			printf("写寄存器成功\n");
			return 12;
		}
	}
}

//生成异常响应信息
uint16_t Generate_exception_response_message(uint8_t *responseMessage, uint8_t functionCode, uint8_t exceptionCode)
{
	responseMessage[4] = 0;
	responseMessage[5] = 3;
	responseMessage[7] = functionCode + 128;
	responseMessage[8] = exceptionCode;
	if (exceptionCode == 1)
		printf("error：非法功能码\n");
	if (exceptionCode == 2)
		printf("error：非法数据地址\n");
	if (exceptionCode == 3)
		printf("error：非法数据值\n");
	if (exceptionCode == 4)
		printf("error：从站设备故障\n");
	else
		printf("error：未知异常\n");
	return 9;
}