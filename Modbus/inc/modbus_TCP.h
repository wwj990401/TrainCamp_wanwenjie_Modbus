#ifndef MODBUS_TCP_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

/* 定义寄存器 */
typedef struct
{
	bool *tabCoils;              //指向线圈的值
	uint16_t *tabRegisters;      //指向保持寄存器的值
}modbusMapping_t;


bool Init_register(modbusMapping_t *mapping);	//初始化线圈和寄存器

int32_t Enter_port_number();		//输入端口号
int32_t Enter_site_number();		//输入站点号
bool Server_connection_configuration(SOCKET *slisten, SOCKET *sClient, int16_t port);		//服务器连接配置
bool Set_socket_timeout(SOCKET *sClient, int32_t t);										//设置读写超时时间
uint8_t *Receive_message(SOCKET *sClient, int16_t *tcpAduLength, bool *flag);				//接收报文
bool Send_message(SOCKET *sClient, uint8_t *responseMessage, uint16_t *tcpResponseLength);	//发送报文

//TCP服务器入口
bool TCP_server_entry();
//解析报文
uint16_t Process_response_message(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping, int32_t station);
//判断报文长度
bool Determine_message_length(uint16_t receiveMessageLength);
//判断MBAP报文头
bool Determine_MBAP_header(uint8_t *receiveMessage, uint16_t receiveMessageLength, int32_t station);
//处理0x01功能码
uint16_t Process_0x01_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//处理0x03功能码
uint16_t Process_0x03_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//处理0x0F功能码
uint16_t Process_0x0F_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//处理0x10功能码
uint16_t Process_0x10_function_code(uint8_t *receiveMessage, uint8_t *responseMessage, uint16_t receiveMessageLength, modbusMapping_t *mapping);
//生成异常响应信息
uint16_t Generate_exception_response_message(uint8_t *responseMessage, uint8_t functionCode, uint8_t exceptionCode);

#define MODBUS_TCP_H
#endif