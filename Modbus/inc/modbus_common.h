#ifndef MODBUS_COMMON_H

#include <stdint.h>
#include <stdio.h>

/* 功能码 */
#define MODBUS_FC_READ_COILS                0x01
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10


/*端口号最大最小值*/
#define MAX_PORT_NUMBLE      65535
#define MIN_PORT_NUMBLE      0

/*站点号最大最小值*/
#define MAX_SITE_NUMBLE      247
#define MIN_SITE_NUMBLE      1

/* 广播地址 */
#define MODBUS_BROADCAST_ADDRESS    0

/* 最大读写线圈数量 */
#define MODBUS_MAX_READ_BITS       2000
#define MODBUS_MAX_WRITE_BITS      1968

/* 最小读写线圈数量 */
#define MODBUS_MIN_READ_BITS       1
#define MODBUS_MIN_WRITE_BITS      1

/* 最大读写寄存器数量 */
#define MODBUS_MAX_WR_WRITE_REGISTERS      123
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* 最小读写寄存器数量 */
#define MODBUS_MIN_WR_WRITE_REGISTERS      1
#define MODBUS_MIN_WR_READ_REGISTERS       1

/* 最大线圈数量 */
#define MODBUS_MAX_BITS       10000

/* 最大寄存器数量 */
#define MODBUS_MAX_REGISTERS  10000

/* 最大PDU长度 */
#define MODBUS_MAX_PDU_LENGTH         253

/* RTU最大ADU长度 */
#define MODBUS_MAX_RTU_ADU_LENGTH         256

/* TCP最大最小ADU长度 */
#define MODBUS_MAX_TCP_ADU_LENGTH         260
#define MODBUS_MIN_TCP_ADU_LENGTH         9

/* RTU接收超时 */
#define RTU_RECEIVES_TIMEOUT    3000

/* TCP读写超时 */
#define TCP_WR_TIMEOUT    5000

/* 串口波特率 */
#define   BAUD_RATE    9600

/* 传输数据位 */
#define TRANSMITTED_DATA_BIT    8

bool The_divider();									    //分隔符
bool RTU_print_data(uint8_t *data, uint8_t length);		//RTU打印数据
bool TCP_print_data(uint8_t *data, uint16_t length);	//TCP打印数据

#define MODBUS_COMMON_H
#endif