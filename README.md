# MANET-Testbed
An API that provides functions to implement a testbed for Mobile Ad-Hoc Network (MANET) routing protocols. This testbed is intended to be used with ad-hoc network nodes that are Raspberry PI devices running Linux Kernel 2.16.X or later. It uses built-in Linux Kernel modules Netlink and Netfilter to provide the user with ways to interact with the Linux Networking Stack without ever needing to know the specifics of routing tables, sockets, and Linux.

Netlink is used to communicate between user and kernel-space on the Linux nodes, thus allowing for API development to take place without the need to develop a Kernel module. Then, libnetfilter-queue is used to work with Netfilter to queue packets of interest into user-space so that the testbed user can access packets, make a verdict on them (using a function unique to the routing protocol they want to test), and then allow the Kernel to take the appropriate action for the packet. 

Further development on this API requires knowledge of Linux sockets, Netfilter, and Netlink. Documentation and examples for these tools can be limited. The `Examples\` directiory contains the best examples I could find, none of which are my own code. Otherwise, the best resource for Netfilter can be found at: https://www.netfilter.org/projects/libnetfilter_queue/

This API is to be used as a dynamic library that is linked to a specific executable during compilation. In addition, the tools used within the library include Netlink, Netfilter, Broadcast UDP Sockets, and iptables, which all required sudo-permissons, leading to a specific required build process:

1) Implement a routing protocol (such as AODV) into a source file. As an example, let's assume it's called `prot.c`.

2) Use the testbed's `Makefile` and run `make` to build the testbed. The result should be a file called `libtestbed.so`.

3) Compile and link the protocol by running `gcc -Wall prot.c -o prot.out -ltestbed -L<LIBPATH> -pthread -lnetfilter_queue`, where LIBPATH is replaced with the path to the folder which contains `libtestbed.so`. For example, `-home/pi/Documents/MANET-Testbed`.

4) Run the executable as sudo-user by exporting the environment variable, once again replaceing LIBPATH with the appropriate path to the library: `sudo LD_LIBRARY_PATH=<LIBPATH>$LD_LIBRARY_PATH ./prot.out`

The specific functionality available with this API and its structure are summarized below.

## File Structure
``` bash
.
├── debug.h
├── Examples
│   ├── aodvv2_shell.sh
│   ├── blog_code.c
│   ├── blog_code_rt.c
│   ├── filterQueue.c
│   ├── ip_show.c
│   ├── libnetfilter_queue_example.c
│   ├── random_git_code.c
│   ├── rtnetlink_test.c
│   ├── simple_udp_client.c
│   ├── simple_udp_server.c
│   └── testbed_api.c
├── head
│   ├── api.h
│   ├── api_if.h
│   ├── api_queue.h
│   ├── api_route.h
│   └── api_send.h
├── Makefile
├── manet_testbed.h
├── obj
│   ├── api_if.o
│   ├── api.o
│   ├── api_queue.o
│   ├── api_route.o
│   └── api_send.o
├── README.md
├── src
│   ├── api.c
│   ├── api_if.c
│   ├── api_queue.c
│   ├── api_route.c
│   └── api_send.c
├── test.c
```

## Files
`debug.h` : (UNUSED) Defines a custom debug print function for testing purposes. Found to be incompatible when used in compilation of a dynamic library.

`Example/` : Contains several example files pulled from various other GitHub repositories that were used/adapted during creation of the testbed.

`head/` : Contains header files for each of the source files of the API.

`src/` : Contains all source (.c) files that implement all functionality of the API.

`api.c/h` : Used to declare variables and implement functions that are shared between API source files. 
  Implements: InitializeAPI()

`api_if.c/h` : Implements all functions related to the wireless interfaces. Currently, testbed only supports ipv4 communication on interface "wlan0". 
  Implements: GetInterfaceIP()

`api_route.c/h` : Implements all functions related to modifying the routing table to create routes between nodes of the MANET. 
  Implements: AddUnicastRoutingEntry(), DeleteEntry()

`api_send.c/h` : Implements all functions related to sending messages. The API sends packets using UDP sockets. 
  Implements: SendUnicast(), SendBroadcast()

`api_queue.c/h` : Implements all functions related to Netfilter queueing of incoming/outgoing/forwarded packets. 
  Implements: RegisterIncomingCallback(), RegisterOutgoingCallback(), RegisterForwardCallback()

`manet_testbed.h` : Declares API functions and defintions that are available to the user. It is the only file that should be interacted with by the user in any way.

`Makefile` : Holds make targets for the testbed, which is compiled into a dynamic library called `libtestbed.so`, and for `test.c`, which can be built using `make test`.

`obj/` : Stores all object files that are used as intermediates during the build process. These object files are not used after compilation of the library has finished. 

`README.md` : Standard GitHub README file for documentation (you're reading it now).

`test.c` : Arbitrary test file for development purposes. Can be compiled and linked with the appropriate libraries (including the api itself) using `make test`.

## Functions
All functions that are intended for the user are defined in `manet_testbed.h` and implemented across different source files:
1) InitializeAPI() - In `api.c` - Must be called by the user before execution of the routing protocol begins. It performs initial setup of the testbed, including establishment of local ip addresses and iptables rules.

2) AddUnicastRoutingEntry() - In `api_route.c` - Adds a given destination as a unicast route to the main routing table of the current node. Uses Netlink and RTNetlink.

3) DeleteEntry() - In `api_route.c` - Deletes a given routing entry from the main routing table of the current node. Should only be used to delete routes that have been added by AddUnicastRoutingEntry(). Uses Netlink and RTNetlink.

4) SwitchRoutingTable() - (UNUSED) - Intended to allow switching away from the main routing table and adding routes into a custom routing table.

5) SendUnicast() - In `api_send.c` - Sends a message from one single node to another using UDP sockets. Should be used for Control Plane Messages only (messages that are unique to the routing protocol being tested).

6) SendBroadcast() - In `api_send.c` - Broadcasts a message to the given network. Uses the broadcast address associated with the given node's "wlan0" interface.

7) GetInterfaceIP() - In `api_if.c` - Gets the local and broadcast ipv4 addresses of the current node at the given interface. Note that the testbed only supports the use of "wlan0", even though this function can get the ip of any interface. Uses Netlink.

8) SetInterface() - (UNUSED) - Intended to set the interface that should be used by the testbed (and therefore the routing protocol). Currently, testbed only supports "wlan0".

9) SearchTable() - (UNUSED) - Intended to search the main routing table to see if a certain entry is present.

10) RegisterIncomingCallback() - In `api_queue.c` - Registers a function as the function used to decide the verdict of queued incoming packets. Uses the `libnetfilter-queue` library.

11) RegisterOutgoingCallback() - In `api_queue.c` - Registers a function as the function used to decide the verdict of queued outgoing packets. Uses the `libnetfilter-queue` library.

12) RegisterForwardCallback() - In `api_queue.c` - Registers a function as the function used to decide the verdict of queued forwarded packets. Uses the `libnetfilter-queue` library.


## Operation and Limitations

## To-do
Can also be found in the header comment of `api.h` and are work-in-progress items:
1) Implement queue status capabilities - Currently there is no way to tell when a specific Netfilter Queue is full, and whether or not that is affecting the testbed performance.
2) Implement queueing into different queues based on destination of the given packet
3) Create destructor or CloseAPI() functions that closes all sockets, closes all queues, and clears all iptables rules upon closure of the testbed.
