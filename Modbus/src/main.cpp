#include "modbus_common.h"
#include "modbus_RTU.h"
#include "modbus_TCP.h"


int main()
{
	int choose = 0;
	printf("1：RTU模式主站点\n2：TCP服务器\n请输入你的选择：");
	scanf("%d", &choose);
	while (choose != 1 && choose != 2)
	{
		printf("输入错误，请重新输入：");
		scanf("%d", &choose);
	}
	if (choose == 1) RTU_mode_main_site_entry();
	else TCP_server_entry();
	return 0;
}