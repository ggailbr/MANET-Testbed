// sudo LD_LIBRARY_PATH=/home/pi/Documents/MANET-Testbed:$LD_LIBRARY_PATH ./test.out
#include "manet_testbed.h"

char *ntop(int domain, void *buf) // convert ip to string
{
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
}

int main(void)
{
    char *name = "wlan0";

	InitializeAPI();
	uint32_t a = GetInterfaceIP((uint8_t *)name, 0);
	if((int)a == -1)
		printf("error");
	else		
		//printf("my local is: %x\n", a);

	printf("a: %s", ntop(AF_INET, "192.168.1.8"));

	return 0;

}