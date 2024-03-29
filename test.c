// sudo LD_LIBRARY_PATH=/home/pi/Documents/MANET-Testbed:$LD_LIBRARY_PATH ./test.out
#include "manet_testbed.h"
#include "debug.h"

char *ntop(int domain, void *buf) // convert ip to string
{
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
}

int in(uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length)
{
	printf("incoming!!");
}
int in2(uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length)
{
	printf("incoming!! 2");
}
int out(uint8_t *raw_pack, uint32_t src, uint32_t dest, uint8_t *payload, uint32_t payload_length)
{
	printf("outgoing happening\n");
	//printf("src:%d\tdest:%d\tpay %d\n")
	return 1;
}


int main(void)
{
    char *name = "wlan0";
	printf("test\n");
	InitializeAPI();
	uint32_t a = GetInterfaceIP((uint8_t *)name, 0);
	a = a + 0x02000000;
	printf("%s\n", ntop(AF_INET, &a));

	RegisterIncomingCallback(&in, &in2);
	RegisterOutgoingCallback(&out);

	while(1)
	{
		sleep(1);
		SendBroadcast("test message", 13, NULL);
	}

	return 0;

}