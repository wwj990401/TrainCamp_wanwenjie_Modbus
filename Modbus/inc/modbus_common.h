#ifndef MODBUS_COMMON_H

#include <stdint.h>
#include <stdio.h>

/* ������ */
#define MODBUS_FC_READ_COILS                0x01
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10


/*�˿ں������Сֵ*/
#define MAX_PORT_NUMBLE      65535
#define MIN_PORT_NUMBLE      0

/*վ��������Сֵ*/
#define MAX_SITE_NUMBLE      247
#define MIN_SITE_NUMBLE      1

/* �㲥��ַ */
#define MODBUS_BROADCAST_ADDRESS    0

/* ����д��Ȧ���� */
#define MODBUS_MAX_READ_BITS       2000
#define MODBUS_MAX_WRITE_BITS      1968

/* ��С��д��Ȧ���� */
#define MODBUS_MIN_READ_BITS       1
#define MODBUS_MIN_WRITE_BITS      1

/* ����д�Ĵ������� */
#define MODBUS_MAX_WR_WRITE_REGISTERS      123
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* ��С��д�Ĵ������� */
#define MODBUS_MIN_WR_WRITE_REGISTERS      1
#define MODBUS_MIN_WR_READ_REGISTERS       1

/* �����Ȧ���� */
#define MODBUS_MAX_BITS       10000

/* ���Ĵ������� */
#define MODBUS_MAX_REGISTERS  10000

/* ���PDU���� */
#define MODBUS_MAX_PDU_LENGTH         253

/* RTU���ADU���� */
#define MODBUS_MAX_RTU_ADU_LENGTH         256

/* TCP�����СADU���� */
#define MODBUS_MAX_TCP_ADU_LENGTH         260
#define MODBUS_MIN_TCP_ADU_LENGTH         9

/* RTU���ճ�ʱ */
#define RTU_RECEIVES_TIMEOUT    3000

/* TCP��д��ʱ */
#define TCP_WR_TIMEOUT    5000

/* ���ڲ����� */
#define   BAUD_RATE    9600

/* ��������λ */
#define TRANSMITTED_DATA_BIT    8

bool The_divider();									    //�ָ���
bool RTU_print_data(uint8_t *data, uint8_t length);		//RTU��ӡ����
bool TCP_print_data(uint8_t *data, uint16_t length);	//TCP��ӡ����

#define MODBUS_COMMON_H
#endif