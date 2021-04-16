#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Modbus/inc/modbus_RTU.h"
#include "../Modbus/inc/modbus_common.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define config1  "../Modbus/config/modbus_RTU_CRC.ini"
#define config2  "../Modbus/config/modbus_RTU_CHECK.ini"

uint8_t str_to_uint8_t(char* str, uint8_t * nums, int length)
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
	TEST_CLASS(Modbus_RTU)
	{
	public:

		TEST_METHOD(CRC)
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
				uint8_t *Input;
				uint8_t Output[2];
				uint8_t Input_length;
				str_length = GetPrivateProfileStringA(section, "Input", NULL, str, sizeof(str), config1);
				Input = (uint8_t *)malloc(sizeof(uint8_t)* 256);
				Input_length = str_to_uint8_t(str, Input, str_length);
				int Output_length = Generate_crc16_check_code(Input, Input_length);
				str_length = GetPrivateProfileStringA(section, "Output", NULL, str, sizeof(str), config1);
				str_to_uint8_t(str, Output, str_length);
				Assert::IsTrue(Input[Input_length] == Output[0]);
				Assert::IsTrue(Input[Input_length + 1] == Output[1]);
				j = strlen(section);
				i += j;
			}
		}

		TEST_METHOD(check)
		{
			// TODO:  在此输入测试代码
			char pAllName[1000];
			int i, j = 0;
			char *section;
			int size = GetPrivateProfileSectionNamesA(pAllName, sizeof(pAllName), config2);
			for (i = 0; i < size; i++)
			{
				section = pAllName + i;
				char str[1000];
				int str_length;
				uint8_t *Input1;
				uint8_t *Input2;
				bool Expect_Output;
				bool Actual_Output;
				uint8_t Input1_length;
				uint8_t Input2_length;
				str_length = GetPrivateProfileStringA(section, "Input1", NULL, str, sizeof(str), config2);
				Input1 = (uint8_t *)malloc(sizeof(uint8_t)* 256);
				Input1_length = str_to_uint8_t(str, Input1, str_length);
				str_length = GetPrivateProfileStringA(section, "Input2", NULL, str, sizeof(str), config2);
				Input2 = (uint8_t *)malloc(sizeof(uint8_t)* 256);
				Input2_length = str_to_uint8_t(str, Input2, str_length);

				Actual_Output = Check_response_frame(Input1, Input2, Input2_length);

				if (GetPrivateProfileIntA(section, "Output", -1, config2)) Expect_Output = 1;
				else Expect_Output = 0;

				Assert::IsTrue(Expect_Output == Actual_Output);
				j = strlen(section);
				i += j;
			}
		}

	};
}