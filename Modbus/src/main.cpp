#include "modbus_common.h"
#include "modbus_RTU.h"
#include "modbus_TCP.h"


int main()
{
	int choose = 0;
	printf("1��RTUģʽ��վ��\n2��TCP������\n���������ѡ��");
	scanf("%d", &choose);
	while (choose != 1 && choose != 2)
	{
		printf("����������������룺");
		scanf("%d", &choose);
	}
	if (choose == 1) RTU_mode_main_site_entry();
	else TCP_server_entry();
	return 0;
}