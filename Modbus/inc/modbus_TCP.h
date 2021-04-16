#ifndef MODBUS_TCP_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

/* ����Ĵ��� */
typedef struct
{
	bool *tabCoils;              //ָ����Ȧ��ֵ
	uint16_t *tabRegisters;      //ָ�򱣳ּĴ�����ֵ
}modbusMapping_t;


bool Init_register(modbusMapping_t *mapping);	//��ʼ����Ȧ�ͼĴ���

int32_t Enter_port_number();		//����˿ں�
int32_t Enter_site_number();		//����վ���
bool Server_connection_configuration(SOCKET *slisten, SOCKET *sClient, int16_t port);		//��������������
bool Set_socket_timeout(SOCKET *sClient, int32_t t);										//���ö�д��ʱʱ��
uint8_t *Receive_message(SOCKET *sClient, int16_t *tcpAduLength, bool *flag);				//���ձ���
bool Send_message(SOCKET *sClient, uint8_t *responseMessage, uint16_t *tcpResponseLength);	//���ͱ���

//TCP���������
bool TCP_server_entry();
//��������
uint16_t Process_response_message(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping, int32_t station);
//�жϱ��ĳ���
bool Determine_message_length(uint16_t receiveMessageLength);
//�ж�MBAP����ͷ
bool Determine_MBAP_header(uint8_t *receiveMessage, uint16_t receiveMessageLength, int32_t station);
//����0x01������
uint16_t Process_0x01_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//����0x03������
uint16_t Process_0x03_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//����0x0F������
uint16_t Process_0x0F_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//����0x10������
uint16_t Process_0x10_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//�����쳣��Ӧ��Ϣ
uint16_t Generate_exception_response_message(uint8_t *responseMessage, uint8_t functionCode, uint8_t exceptionCode);

#define MODBUS_TCP_H
#endif