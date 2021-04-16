#include "modbus_common.h"

//�ָ���
bool The_divider()
{
	printf("\n----------------------------------------------------------\n\n");
	return 0;
}

//RTU��ӡ����
bool RTU_print_data(uint8_t *data, uint8_t length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		printf("0x%.2X ", data[i]);
		if ((i + 1) % 10 == 0)
			printf("\n\t");
	}
	return 1;
}

//TCP��ӡ����
bool TCP_print_data(uint8_t *data, uint16_t length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		printf("0x%.2X ", data[i]);
		if ((i + 1) % 10 == 0)
			printf("\n\t");
	}
	return 1;
}