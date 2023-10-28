/*
Andre Koka - Created 10/8/2023
             Last Updated: 10/27/2023

The basic API file for the MANET Testbed - to implement:
- all common functions between other API files
*/

#include "manet_testbed.h"
#include "api.h"
#include "api_if.h"
#include "api_send.h"
#include "api_route.h"

pthread_mutex_t lock;
int fd = 0;
int f_err = 0;
uint32_t local_ip = 0; 
uint32_t broadcast_ip = 0; 

int InitializeAPI() // required to be called first
{
	check(InitializeIF());
	check(InitializeRoute());
	check(InitializeSend());
	if(f_err != 0)
		return -1;
	return 0;
}

void check(int val) // check for error returned
{
	if (val < 0) {
		f_err = 1;
	}
} 

char *ntop(int domain, void *buf) // convert ip to string
{
	static char ip[INET6_ADDRSTRLEN];
	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);
	return ip;
}


