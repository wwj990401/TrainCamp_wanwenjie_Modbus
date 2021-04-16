#include "modbus_RTU.h"
#include "modbus_common.h"

HANDLE hComm;

//RTUģʽ��վ�����
bool RTU_mode_main_site_entry()
{
	The_divider();
	printf("                ����RTUģʽ��վ��\n");
	The_divider();
	TCHAR portId[5];
	TCHAR gszPort[20];
	printf("�����봮�ں�: ");
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
				printf("Tx��\t");
				RTU_print_data(adu, writeLength);
				printf("\n");
				flag = Send_byte(adu, writeLength);
				if (flag == false)
				{
					The_divider();
					printf("error:�����ѶϿ�\n");
					break;
				}
				uint8_t *readByte;
				uint8_t readLength = 0;
				uint8_t *pread_length = &readLength;
				if (adu[0] != 0)
					readByte = Read_byte(pread_length);
				if (readLength != 0)
				{
					printf("Rx��\t");
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

//�������������
uint8_t Receive_command(uint8_t *adu)
{
	int i = 0;
	printf("������վ���:");
	scanf("%X", adu + i++);
	while (adu[0] > MAX_SITE_NUMBLE)
	{
		printf("error:�����ڴ�վ��\n����������վ�㣺");
		scanf("%X", adu + i - 1);
	}

	printf("�����빦����:");
	scanf("%X", adu + i++);
	while (!(adu[i - 1] == MODBUS_FC_READ_COILS
		|| adu[i - 1] == MODBUS_FC_READ_HOLDING_REGISTERS
		|| adu[i - 1] == MODBUS_FC_WRITE_MULTIPLE_COILS
		|| adu[i - 1] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS))
	{
		printf("error:�����ڴ˹�����,���������빦���룺");
		scanf("%X", adu + i - 1);
	}

	printf("�������λ��ʼ��ַ��");
	scanf("%X", adu + i++);
	printf("�������λ��ʼ��ַ��");
	scanf("%X", adu + i++);

	while ((adu[2] * 256 + adu[3]) > (MODBUS_MAX_BITS - 1))
	{
		printf("\n");
		printf("error:�����ַ������Χ\n");
		printf("�����������λ��ʼ��ַ��");
		scanf("%X", adu + i - 2);
		printf("�����������λ��ʼ��ַ��");
		scanf("%X", adu + i - 1);
	}

	printf("�������λ������");
	scanf("%X", adu + i++);
	printf("�������λ������");
	scanf("%X", adu + i++);

	while (((adu[4] * 256 + adu[5]) > MODBUS_MAX_READ_BITS && adu[1] == MODBUS_FC_READ_COILS)
		|| ((adu[4] * 256 + adu[5]) > MODBUS_MAX_WR_READ_REGISTERS && adu[1] == MODBUS_FC_READ_HOLDING_REGISTERS)
		|| ((adu[4] * 256 + adu[5]) > MODBUS_MAX_WRITE_BITS && adu[1] == MODBUS_FC_WRITE_MULTIPLE_COILS)
		|| ((adu[4] * 256 + adu[5]) > MODBUS_MAX_WR_WRITE_REGISTERS && adu[1] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS)
		|| (adu[4] * 256 + adu[5]) == 0)
	{
		printf("error:�����������ڷ�Χ��\n");
		printf("�����������λ������");
		scanf("%X", adu + i - 2);
		printf("�����������λ������");
		scanf("%X", adu + i - 1);
	}

	switch (adu[1])
	{
	case 15:
	{
			   int number = adu[4] * 256 + adu[5];
			   int byteNumber = number / 8 + (number % 8 != 0);
			   adu[i++] = byteNumber;
			   printf("������д��ֵ��\n");
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
			   printf("������д��ֵ��\n");
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

//CRCУ��������
uint8_t Generate_crc16_check_code(uint8_t *msg, uint8_t msgLen)
{
	uint8_t length = msgLen;
	uint8_t uchCRCHi = 0xFF; //��CRC�ֽڳ�ʼ��
	uint8_t uchCRCLo = 0xFF; //��CRC �ֽڳ�ʼ��
	uint32_t uIndex;         //CRCѭ���е�����
	//������Ϣ������
	while (msgLen--)
	{
		//����CRC
		uIndex = uchCRCLo ^ *msg++;
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
		uchCRCHi = auchCRCLo[uIndex];
	}
	//���ؽ������λ��ǰ
	*msg++ = uchCRCLo;
	*msg++ = uchCRCHi;
	return length + 2;
}

//������Ӧ֡
bool Check_response_frame(uint8_t *adu, uint8_t *responseFrame, uint8_t frameLength)
{
	uint8_t CRC_L = responseFrame[frameLength - 2], CRC_H = responseFrame[frameLength - 1];
	Generate_crc16_check_code(responseFrame, frameLength - 2);
	if (CRC_L == responseFrame[frameLength - 2] && CRC_H == responseFrame[frameLength - 1])
		printf("CRCУ��ɹ�\n");
	else
	{
		printf("error:CRCУ��ʧ��\n");
		return false;
	}
	if (adu[0] == responseFrame[0])
		printf("վ��У��ɹ�\n");
	else
	{
		printf("error:վ��У��ʧ��\n");
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
		printf("error:�Ƿ���Ӧ������\n");
		return false;
	}
	return true;
}

//��������������
bool Process_normal_function_code(uint8_t *adu, uint8_t *responseFrame, uint8_t frameLength)
{
	if ((responseFrame[1] == MODBUS_FC_READ_COILS && responseFrame[2] == (frameLength - 5)) || (responseFrame[1] == MODBUS_FC_READ_HOLDING_REGISTERS && responseFrame[2] == (frameLength - 5)))
	{
		if ((responseFrame[1] == MODBUS_FC_READ_COILS && ((adu[4] * 256 + adu[5]) / 8 + !((adu[4] * 256 + adu[5]) % 8 == 0)) == responseFrame[2]) || (responseFrame[1] == MODBUS_FC_READ_HOLDING_REGISTERS && ((adu[4] * 256 + adu[5]) * 2) == responseFrame[2]))
		{
			printf("������Ӧ\n��ȡ�����ݣ�\n\t");
			RTU_print_data(responseFrame + 3, responseFrame[2]);
			return true;
		}
		else
		{
			printf("error��������Ӧ����\n");
			return false;
		}
	}
	else if ((responseFrame[1] == MODBUS_FC_WRITE_MULTIPLE_COILS && frameLength == 8) || (responseFrame[1] == MODBUS_FC_WRITE_MULTIPLE_REGISTERS && frameLength == 8))
	{
		if (adu[2] == responseFrame[2] && adu[3] == responseFrame[3] && adu[4] == responseFrame[4] && adu[5] == responseFrame[5])
		{
			printf("������Ӧ\nд�����ʼ��ַ��������\n\t");
			RTU_print_data(responseFrame + 2, 4);
			return true;
		}
		else
		{
			printf("error��������Ӧ��ַ������\n");
			return false;
		}
	}
	else
	{
		printf("error��������Ӧ֡����\n");
		return false;
	}
}

//�����쳣������
bool Handle_exception_function_code(uint8_t *responseFrame)
{
	printf("error:�쳣��Ӧԭ��");
	if (responseFrame[2] == 1) printf("�Ƿ�������\n");
	else if (responseFrame[2] == 2) printf("�Ƿ����ݵ�ַ\n");
	else if (responseFrame[2] == 3) printf("�Ƿ�����ֵ\n");
	else if (responseFrame[2] == 4) printf("��վ�豸����\n");
	else printf("δ֪�쳣\n");
	return true;
}

//�򿪴���
bool Open_port(TCHAR *gszPort)
{
	printf("���ڴ򿪴��� %s \n", gszPort);

	hComm = CreateFile(gszPort,                                     // ָ���ļ�����ָ��
		GENERIC_READ | GENERIC_WRITE,								// ����(��д)ģʽ
		0,                                                          // ����ģʽ
		0,                                                          // ��ȫ����ָ��
		OPEN_EXISTING,												// ��δ���
		0,                                                          // �ļ�����
		0);															// ����Ҫ�������Ե��ļ����

	if (hComm == INVALID_HANDLE_VALUE)
	{
		printf("error:�򿪴��� %s ʧ��\n", gszPort);
		return 0;
	}
	else
	{
		printf("���� %s �Ѵ� \n", gszPort);
		return 1;
	}
}

//���ô���
bool Setup_port()
{
	SetupComm(hComm, 1000, 1000); //���뻺����������������Ĵ�С����1000

	/*********************************��ʱ����**************************************/
	COMMTIMEOUTS TimeOuts;
	//�趨����ʱ
	TimeOuts.ReadIntervalTimeout = 40;         //�������ʱ
	TimeOuts.ReadTotalTimeoutMultiplier = 5;   //��ʱ��ϵ��
	TimeOuts.ReadTotalTimeoutConstant = 100;   //��ʱ�䳣��
	//�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 0;  //дʱ��ϵ��
	TimeOuts.WriteTotalTimeoutConstant = 0;    //дʱ�䳣��
	SetCommTimeouts(hComm, &TimeOuts); //���ó�ʱ

	printf("�������ô���\n");
	DCB dcb;
	if (!GetCommState(hComm, &dcb))
	{
		printf("error:�õ�DCBʧ��\n");
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
		printf("error:��������ʧ��(%d)\n", GetLastError());
		return 0;
	}
	else
	{
		printf("�����������\n");
		return 1;
	}
}

//�������������
bool Purge_port()
{
	if (PurgeComm(hComm, PURGE_RXCLEAR))
	{
		printf("�������������\n");
		return 1;
	}
	else
	{
		printf("error:�������������ʧ��\n");
		return 0;
	}
}

//�رմ���
bool Close_port()
{
	printf("���ڹرմ���\n");

	if (CloseHandle(hComm))
	{
		printf("�����ѹر�\n");
		return 1;
	}
	else
	{
		printf("error:���ڹر�ʧ��\n");
		return 0;
	}
}

//���ڷ�������
bool Send_byte(uint8_t *byteToWrite, uint8_t length)
{
	DWORD dwWritten;
	if (WriteFile(hComm, byteToWrite, length, &dwWritten, 0))
	{
		printf("���ڷ������ݳɹ�\n");
		return 1;
	}
	else
	{
		printf("error:���ڷ�������ʧ��\n");
		return 0;
	}
}

//���ڽ�������
uint8_t *Read_byte(uint8_t *read_length)
{
	DWORD start, end;
	DWORD dwRead;
	uint8_t *lpBuf;
	lpBuf = (uint8_t *)malloc(sizeof(uint8_t)* 1000);
	start = GetTickCount();
	while (true)
	{
		ReadFile(hComm,                     // Ҫ��ȡ���ļ����
			lpBuf,                          // �������ݵĻ�������ַ
			1000,					        // Ҫ��ȡ���ֽ���
			&dwRead,                        // ��ȡ���ֽ����ĵ�ַ
			0);                             // ���ݽṹ�ĵ�ַ
		end = GetTickCount();
		if (dwRead == 0 && (end - start) >= RTU_RECEIVES_TIMEOUT)
		{
			printf("error:��Ӧ��ʱ\n");
			break;
		}
		if (dwRead != 0) break;
	}
	printf("�ȴ���Ӧʱ��time=%d\n", end - start);
	*read_length = (uint8_t)dwRead;
	return lpBuf;
}