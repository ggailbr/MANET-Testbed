// sudo LD_LIBRARY_PATH=/home/pi/Documents/MANET-Testbed:$LD_LIBRARY_PATH ./test.out
#include "manet_testbed.h"
int main(void)
{
    char *name = "wlan0";

	InitializeAPI();
	uint32_t a = GetInterfaceIP((uint8_t *)name, 0);
	if((int)a == -1)
		printf("error");
	else		
		printf("my local is: %x\n", a);
	return 0;
}