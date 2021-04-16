#include "modbus_RTU.h"
#include "modbus_common.h"

HANDLE hComm;

//RTU模式主站点入口
bool RTU_mode_main_site_entry()
{
	The_divider();
	printf("                进入RTU模式主站点\n");
	The_divider();
	TCHAR portId[5];
	TCHAR gszPort[20];
	printf("请输入串口号: ");
	scanf("%s", &portId);
	strcpy(gszPort, "COM");
	strcat(gszPort, portId);
	if (Open_port(gszPort))
	{
		if (Setup_port())
		{
			bool flag = 1;
			while (1)
			{
				The_divider();
				uint8_t *adu;
				adu = (uint8_t *)malloc(sizeof(uint8_t)* 256);
				uint8_t writeLength = Receive_command(adu);
				writeLength = Generate_crc16_check_code(adu, writeLength);
				Purge_port();
				printf("Tx：\t");
				RTU_print_data(adu, writeLength);
				printf("\n");
				flag = Send_byte(adu, writeLength);
				if (flag == false)
				{
					The_divider();
					printf("error:串口已断开\n");
					break;
				}
				uint8_t *readByte;
				uint8_t readLength = 0;
				uint8_t *pread_length = &readLength;
				if (adu[0] != 0)
					readByte = Read_byte(pread_length);
				if (readLength != 0)
				{
					printf("Rx：\t");
					RTU_print_data(readByte, readLength);
					printf("\n");
					Check_response_frame(adu, readByte, readLength);
				}
			}
		}
		The_divider();
		Close_port();
	}
	return 0;
}

//接收输入的命令
uint8_t Receive_command(uint8_t *adu)
{
	int i = 0;
	printf("请输入站点号:");
	scanf("%X", adu + i++);
	while (adu[0] > MAX_SITE_NUMBLE)
	{
		printf("error:不存在此站点\n请重新输入站点：");
		scanf("%X", adu + i - 1);
	}

	printf("请输入功能码:");
	scanf("%X", adu + i++);
	while (!(adu[i - 1] == MODBUS_FC_READ_COILS
		|| adu[i - 1] == MODBUS_FC_READ_HOLDING_REGISTERS
		|| adu[i - 1] == MODBUS_FC_WRITE_MULTIPLE_COILS
		|| adu[i - 1] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS))
	{
		printf("error:不存在此功能码,请重新输入功能码：");
		scanf("%X", adu + i - 1);
	}

	printf("请输入高位起始地址：");
	scanf("%X", adu + i++);
	printf("请输入低位起始地址：");
	scanf("%X", adu + i++);

	while ((adu[2] * 256 + adu[3]) > (MODBUS_MAX_BITS - 1))
	{
		printf("\n");
		printf("error:输入地址超出范围\n");
		printf("请重新输入高位起始地址：");
		scanf("%X", adu + i - 2);
		printf("请重新输入低位起始地址：");
		scanf("%X", adu + i - 1);
	}

	printf("请输入高位数量：");
	scanf("%X", adu + i++);
	printf("请输入低位数量：");
	scanf("%X", adu + i++);

	while (((adu[4] * 256 + adu[5]) > MODBUS_MAX_READ_BITS && adu[1] == MODBUS_FC_READ_COILS)
		|| ((adu[4] * 256 + adu[5]) > MODBUS_MAX_WR_READ_REGISTERS && adu[1] == MODBUS_FC_READ_HOLDING_REGISTERS)
		|| ((adu[4] * 256 + adu[5]) > MODBUS_MAX_WRITE_BITS && adu[1] == MODBUS_FC_WRITE_MULTIPLE_COILS)
		|| ((adu[4] * 256 + adu[5]) > MODBUS_MAX_WR_WRITE_REGISTERS && adu[1] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS)
		|| (adu[4] * 256 + adu[5]) == 0)
	{
		printf("error:输入数量不在范围内\n");
		printf("请重新输入高位数量：");
		scanf("%X", adu + i - 2);
		printf("请重新输入低位数量：");
		scanf("%X", adu + i - 1);
	}

	switch (adu[1])
	{
	case 15:
	{
			   int number = adu[4] * 256 + adu[5];
			   int byteNumber = number / 8 + (number % 8 != 0);
			   adu[i++] = byteNumber;
			   printf("请输入写入值：\n");
			   int j = 0;
			   while (byteNumber--)
			   {
				   scanf("%X", adu + i++);
				   j++;
			   }
			   break;
	}
	case 16:
	{
			   int number = adu[4] * 256 + adu[5];
			   int byteNumber = number * 2;
			   adu[i++] = byteNumber;
			   printf("请输入写入值：\n");
			   int j = 0;
			   while (byteNumber--)
			   {
				   scanf("%X", adu + i++);
				   j++;
			   }
			   break;
	}
	default:
		break;
	}
	return i;
}

//CRC校验码生成
uint8_t Generate_crc16_check_code(uint8_t *msg, uint8_t msgLen)
{
	uint8_t length = msgLen;
	uint8_t uchCRCHi = 0xFF; //高CRC字节初始化
	uint8_t uchCRCLo = 0xFF; //低CRC 字节初始化
	uint32_t uIndex;         //CRC循环中的索引
	//传输消息缓冲区
	while (msgLen--)
	{
		//计算CRC
		uIndex = uchCRCLo ^ *msg++;
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
		uchCRCHi = auchCRCLo[uIndex];
	}
	//返回结果，高位在前
	*msg++ = uchCRCLo;
	*msg++ = uchCRCHi;
	return length + 2;
}

//解析响应帧
bool Check_response_frame(uint8_t *adu, uint8_t *responseFrame, uint8_t frameLength)
{
	uint8_t CRC_L = responseFrame[frameLength - 2], CRC_H = responseFrame[frameLength - 1];
	Generate_crc16_check_code(responseFrame, frameLength - 2);
	if (CRC_L == responseFrame[frameLength - 2] && CRC_H == responseFrame[frameLength - 1])
		printf("CRC校验成功\n");
	else
	{
		printf("error:CRC校验失败\n");
		return false;
	}
	if (adu[0] == responseFrame[0])
		printf("站号校验成功\n");
	else
	{
		printf("error:站号校验失败\n");
		return false;
	}
	if (adu[1] == responseFrame[1])
	{
		return Process_normal_function_code(adu, responseFrame, frameLength);
	}
	else if (adu[1] == responseFrame[1] - 128)
	{
		return Handle_exception_function_code(responseFrame);
	}
	else
	{
		printf("error:非法响应功能码\n");
		return false;
	}
	return true;
}

//处理正常功能码
bool Process_normal_function_code(uint8_t *adu, uint8_t *responseFrame, uint8_t frameLength)
{
	if ((responseFrame[1] == MODBUS_FC_READ_COILS && responseFrame[2] == (frameLength - 5)) || (responseFrame[1] == MODBUS_FC_READ_HOLDING_REGISTERS && responseFrame[2] == (frameLength - 5)))
	{
		if ((responseFrame[1] == MODBUS_FC_READ_COILS && ((adu[4] * 256 + adu[5]) / 8 + !((adu[4] * 256 + adu[5]) % 8 == 0)) == responseFrame[2]) || (responseFrame[1] == MODBUS_FC_READ_HOLDING_REGISTERS && ((adu[4] * 256 + adu[5]) * 2) == responseFrame[2]))
		{
			printf("正常响应\n读取的数据：\n\t");
			RTU_print_data(responseFrame + 3, responseFrame[2]);
			return true;
		}
		else
		{
			printf("error：错误响应数量\n");
			return false;
		}
	}
	else if ((responseFrame[1] == MODBUS_FC_WRITE_MULTIPLE_COILS && frameLength == 8) || (responseFrame[1] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS && frameLength == 8))
	{
		if (adu[2] == responseFrame[2] && adu[3] == responseFrame[3] && adu[4] == responseFrame[4] && adu[5] == responseFrame[5])
		{
			printf("正常响应\n写入的起始地址和数量：\n\t");
			RTU_print_data(responseFrame + 2, 4);
			return true;
		}
		else
		{
			printf("error：错误响应地址或数量\n");
			return false;
		}
	}
	else
	{
		printf("error：错误响应帧长度\n");
		return false;
	}
}

//处理异常功能码
bool Handle_exception_function_code(uint8_t *responseFrame)
{
	printf("error:异常响应原因：");
	if (responseFrame[2] == 1) printf("非法功能码\n");
	else if (responseFrame[2] == 2) printf("非法数据地址\n");
	else if (responseFrame[2] == 3) printf("非法数据值\n");
	else if (responseFrame[2] == 4) printf("从站设备故障\n");
	else printf("未知异常\n");
	return true;
}

//打开串口
bool Open_port(TCHAR *gszPort)
{
	printf("正在打开串口 %s \n", gszPort);

	hComm = CreateFile(gszPort,                                     // 指向文件名的指针
		GENERIC_READ | GENERIC_WRITE,								// 访问(读写)模式
		0,                                                          // 共享模式
		0,                                                          // 安全属性指针
		OPEN_EXISTING,												// 如何创建
		0,                                                          // 文件属性
		0);															// 带有要复制属性的文件句柄

	if (hComm == INVALID_HANDLE_VALUE)
	{
		printf("error:打开串口 %s 失败\n", gszPort);
		return 0;
	}
	else
	{
		printf("串口 %s 已打开 \n", gszPort);
		return 1;
	}
}

//设置串口
bool Setup_port()
{
	SetupComm(hComm, 1000, 1000); //输入缓冲区和输出缓冲区的大小都是1000

	/*********************************超时设置**************************************/
	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout = 40;         //读间隔超时
	TimeOuts.ReadTotalTimeoutMultiplier = 5;   //读时间系数
	TimeOuts.ReadTotalTimeoutConstant = 100;   //读时间常量
	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 0;  //写时间系数
	TimeOuts.WriteTotalTimeoutConstant = 0;    //写时间常量
	SetCommTimeouts(hComm, &TimeOuts); //设置超时

	printf("正在设置串口\n");
	DCB dcb;
	if (!GetCommState(hComm, &dcb))
	{
		printf("error:得到DCB失败\n");
		return 0;
	}
	dcb.BaudRate = BAUD_RATE;
	dcb.fParity = TRUE;
	dcb.Parity = EVENPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.ByteSize = TRANSMITTED_DATA_BIT;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	if (!SetCommState(hComm, &dcb))
	{
		printf("error:串口设置失败(%d)\n", GetLastError());
		return 0;
	}
	else
	{
		printf("串口设置完成\n");
		return 1;
	}
}

//清除缓冲区数据
bool Purge_port()
{
	if (PurgeComm(hComm, PURGE_RXCLEAR))
	{
		printf("清除缓冲区数据\n");
		return 1;
	}
	else
	{
		printf("error:清除缓冲区数据失败\n");
		return 0;
	}
}

//关闭串口
bool Close_port()
{
	printf("正在关闭串口\n");

	if (CloseHandle(hComm))
	{
		printf("串口已关闭\n");
		return 1;
	}
	else
	{
		printf("error:串口关闭失败\n");
		return 0;
	}
}

//串口发送数据
bool Send_byte(uint8_t *byteToWrite, uint8_t length)
{
	DWORD dwWritten;
	if (WriteFile(hComm, byteToWrite, length, &dwWritten, 0))
	{
		printf("串口发送数据成功\n");
		return 1;
	}
	else
	{
		printf("error:串口发送数据失败\n");
		return 0;
	}
}

//串口接收数据
uint8_t *Read_byte(uint8_t *read_length)
{
	DWORD start, end;
	DWORD dwRead;
	uint8_t *lpBuf;
	lpBuf = (uint8_t *)malloc(sizeof(uint8_t)* 1000);
	start = GetTickCount();
	while (true)
	{
		ReadFile(hComm,                     // 要读取的文件句柄
			lpBuf,                          // 接收数据的缓冲区地址
			1000,					        // 要读取的字节数
			&dwRead,                        // 读取的字节数的地址
			0);                             // 数据结构的地址
		end = GetTickCount();
		if (dwRead == 0 && (end - start) >= RTU_RECEIVES_TIMEOUT)
		{
			printf("error:响应超时\n");
			break;
		}
		if (dwRead != 0) break;
	}
	printf("等待响应时间time=%d\n", end - start);
	*read_length = (uint8_t)dwRead;
	return lpBuf;
}