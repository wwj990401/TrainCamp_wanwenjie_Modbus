#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Modbus/inc/modbus_TCP.h"
#include "../Modbus/inc/modbus_common.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define config1  "../Modbus/config/modbus_TCP_Response.ini"
#define config2  "../Modbus/config/modbus_TCP_Exception_Response.ini"

uint8_t str_to_uint8_t2(char* str, uint8_t * nums, int length)
{
	int i = 0, j = 0;
	int num;
	for (i = 0; i < length; i++)
	{
		num = 0;
		if (str[i] >= '0'&&str[i] <= '9')
			num = num + str[i] - 48;
		else if (str[i] >= 'A'&&str[i] <= 'F')
			num = num + str[i] - 55;
		else
			num = num + str[i] - 87;
		i++;
		if (str[i] >= '0'&&str[i] <= '9')
			num = num * 16 + str[i] - 48;
		else if (str[i] >= 'A'&&str[i] <= 'F')
			num = num * 16 + str[i] - 55;
		else
			num = num * 16 + str[i] - 87;
		i++;
		nums[j++] = num;
	}
	return j;
}

namespace Test
{
	TEST_CLASS(Modbus_TCP)
	{
	public:

		TEST_METHOD(response)
		{
			// TODO:  在此输入测试代码
			char pAllName[1000];
			int i, j = 0;
			char *section;
			int size = GetPrivateProfileSectionNamesA(pAllName, sizeof(pAllName), config1);
			for (i = 0; i < size; i++)
			{
				section = pAllName + i;
				char str[1000];
				int str_length;
				uint8_t *Input_message;
				uint8_t *Expect_Output_response;
				uint8_t *Actual_Output_response;
				modbusMapping_t *mapping;
				mapping = (modbusMapping_t *)malloc(sizeof(modbusMapping_t));
				Init_register(mapping);
				uint16_t Expect_Output, Actual_Output;
				uint16_t Input_message_length;
				uint16_t Output_message_length;
				int station;

				str_length = GetPrivateProfileStringA(section, "Input_message", NULL, str, sizeof(str), config1);
				Input_message = (uint8_t *)malloc(sizeof(uint8_t)* 300);
				Input_message_length = str_to_uint8_t2(str, Input_message, str_length);

				station = GetPrivateProfileIntA(section, "Input_station", -1, config1);

				str_length = GetPrivateProfileStringA(section, "Output_message", NULL, str, sizeof(str), config1);
				Expect_Output_response = (uint8_t *)malloc(sizeof(uint8_t)* 300);
				Output_message_length = str_to_uint8_t2(str, Expect_Output_response, str_length);

				Actual_Output_response = (uint8_t *)malloc(sizeof(uint8_t)* 300);
				Actual_Output = Process_response_message(Input_message, Actual_Output_response, Input_message_length, mapping, station);

				Expect_Output = GetPrivateProfileIntA(section, "Output_length", -1, config1);

				Assert::IsTrue(Expect_Output == Actual_Output);

				for (int k = 0; k < Expect_Output; k++)
					Assert::IsTrue(Expect_Output_response[k] == Actual_Output_response[k]);

				j = strlen(section);
				i += j;
			}
		}

		TEST_METHOD(exceptional_response)
		{
			// TODO:  在此输入测试代码
			char pAllName[1000];
			int i, j = 0;
			char *section;
			int size = GetPrivateProfileSectionNamesA(pAllName, sizeof(pAllName), config2);
			for (i = 0; i < size; i++)
			{
				section = pAllName + i;
				char str[100];
				int str_length;
				uint8_t *Input_functioncode;
				uint8_t *Expect_Output_response;
				uint8_t *Actual_Output_response;
				uint16_t Expect_Output, Actual_Output;
				int Input_exceptioncode;

				str_length = GetPrivateProfileStringA(section, "Input_functioncode", NULL, str, sizeof(str), config2);
				Input_functioncode = (uint8_t *)malloc(sizeof(uint8_t)* 10);
				str_to_uint8_t2(str, Input_functioncode, str_length);

				Input_exceptioncode = GetPrivateProfileIntA(section, "Input_exceptioncode", -1, config2);

				str_length = GetPrivateProfileStringA(section, "Output_message", NULL, str, sizeof(str), config2);
				Expect_Output_response = (uint8_t *)malloc(sizeof(uint8_t)* 10);
				str_to_uint8_t2(str, Expect_Output_response, str_length);

				Actual_Output_response = (uint8_t *)malloc(sizeof(uint8_t)* 10);
				Actual_Output = Generate_exception_response_message(Actual_Output_response, Input_functioncode[0], Input_exceptioncode);

				Expect_Output = GetPrivateProfileIntA(section, "Output_length", -1, config2);

				Assert::IsTrue(Expect_Output == Actual_Output);

				Assert::IsTrue(Expect_Output_response[4] == Actual_Output_response[4]);
				Assert::IsTrue(Expect_Output_response[5] == Actual_Output_response[5]);
				Assert::IsTrue(Expect_Output_response[7] == Actual_Output_response[7]);
				Assert::IsTrue(Expect_Output_response[8] == Actual_Output_response[8]);

				j = strlen(section);
				i += j;
			}
		}
	};
}