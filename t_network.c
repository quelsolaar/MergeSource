/*
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include "forge.h"
#include "imagine.h"

#if defined _WIN32
//#include <winsock.h>
#include <winsock2.h>
//#include <Iptypes.h>
#include <iphlpapi.h>
#pragma comment(lib, "WSock32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
//#include <stdio.h>
typedef unsigned int uint;
typedef SOCKET VSocket;
#else
typedef int VSocket;
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif


#include "testify.h"

void testify_socket_destroy(uint32 socket)
{
#if defined _WIN32
	closesocket(socket);
#else
	close(socket);
#endif
}

#if defined _WIN32

boolean testify_socket_init_win32()
{
	static boolean initialized = FALSE;
	if(!initialized)
	{
		WSADATA wsaData;
		if(WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
		{
			fprintf(stderr, "Testify Error: WSAStartup failed.\n");
			return FALSE;
		}
		initialized = TRUE;
	}
	return TRUE;
}

#endif

uint32 testify_socket_create(boolean stream, uint16 port)
{
	struct sockaddr_in address;
    int buffer_size = 1 << 20;
	int option = TRUE;
	uint32 s;
#if defined _WIN32
	if(!testify_socket_init_win32())
		return -1;
#endif
	if(stream)
	{
		if((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
			return -1;	
	}else
	{
		if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
			return -1;	
	}

#if defined _WIN32
	if(!stream)
	{
		unsigned long	one = 1UL;
		if(ioctlsocket(s, FIONBIO, &one) != 0)
			return -1;
	}
#else
	if(fcntl(s, F_SETFL, O_NONBLOCK) != 0)
	{
		fprintf(stderr, "Testify: Couldn't make socket non-blocking\n");
		exit(0); /* FIX ME */
		return -1;
	}
#endif
//	return s;
	if(port != 0 || !stream)
	{

		memset(&address, 0, sizeof(struct sockaddr));   /* Zero out structure */
		address.sin_family = AF_INET;     /* host byte order */
		address.sin_port = htons(port); /* short, network byte order */
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		if(bind(s, (struct sockaddr *) &address, sizeof(struct sockaddr)) != 0)
		{
		//	fprintf(stderr, "Testify: Failed to bind(), code %d (%s)\n", errno, strerror(errno));
			testify_socket_destroy(s);
			return -1;
		}
	//	if(stream)
	//	   if(listen(s, 5) < 0)
	//			fprintf(stderr, "Testify: Failed to bind(), code %d (%s)\n", errno, strerror(errno));
	}

	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, &option, sizeof option) != 0)
		fprintf(stderr, "Testify: Couldn't set brodcast option of socket to %d\n", buffer_size);
	if(setsockopt(s, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof buffer_size) != 0)
		fprintf(stderr, "Testify: Couldn't set send buffer size of socket to %d\n", buffer_size);
	if(setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof buffer_size) != 0)
		fprintf(stderr, "Testify: Couldn't set receive buffer size of socket to %d\n", buffer_size);
	return s;
}

boolean testify_network_address_lookup(TestifyNetworkAddress *address, char *dns_name, uint16 default_port)
{
	struct hostent *he;
	char *colon = NULL, *buf = NULL;
	address->port = default_port;
	if((colon = strchr(dns_name, ':')) != NULL)
	{
		size_t	hl = strlen(dns_name);
		if((buf = malloc(hl + 1)) != NULL)
		{
			unsigned int	tp;
			strcpy(buf, dns_name);
			colon = buf + (colon - dns_name);
			*colon = '\0';
			dns_name = buf;
			if(sscanf(colon + 1, "%u", &tp) == 1)
			{
				address->port = (unsigned short) tp;
				if(address->port != tp)
					return FALSE;
			}else
				return FALSE;
		}else
			return FALSE;
	}

#if defined _WIN32
	if(!testify_socket_init_win32())
		return FALSE;
#endif
	if(dns_name != NULL && (he = gethostbyname(dns_name)) != NULL)
	{
		memcpy(&address->ip, he->h_addr_list[0], he->h_length);
		address->ip = ntohl(address->ip);
		return TRUE;
	}
	return FALSE;
}

boolean testify_network_address_compare(TestifyNetworkAddress *a, TestifyNetworkAddress *b)
{
	return a->ip == b->ip && a->port == b->port;
}


typedef struct{
	char adapter_name[64];
	uint8 hardware_address[6];
	uint32 ipv4_mask;
	uint32 ipv4_local_ip;
	uint32 ipv4_global_ip;
	uint32 ipv4_gateway;
}TestifyNetworkAdapter;

typedef struct{
	TestifyNetworkAdapter *adapter;
	uint adapter_count;
}TestifyNetworkArrangement;

TestifyNetworkArrangement testify_network_arrangement = {NULL, 0};

#ifdef _WIN32



uint32 testify_address_parse(char *path)
{
	uint32 ip = 0, i, pos = 0, value;
	for(i = 0; i < 4; i++)
	{
		value = 0;
		while(path[pos] != 0 && (path[pos] < '0' || path[pos] > '9'))
			pos++;
		if(path[pos] == 0)
			return 0;
		while(path[pos] != 0 && path[pos] >= '0' && path[pos] <= '9')
		{
			value *= 10;
			value += path[pos++] - '0';
		}

		ip |= value << ((3 - i) * 8);
	}
	return ip;
}

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

boolean testify_network_discover()
{
    IP_ADAPTER_INFO *adapter_info;
    IP_ADAPTER_INFO *adapter = NULL;
    DWORD dwRetVal = 0;
    uint i, j;

/* variables used to print DHCP time info */
    struct tm newtime;
    char buffer[32];
    errno_t error;
	uint32 size, output;
	size = sizeof *adapter_info;
    adapter_info = HeapAlloc(GetProcessHeap(), 0, sizeof (IP_ADAPTER_INFO));
    if(adapter_info == NULL)
        return FALSE;
	output = GetAdaptersInfo(adapter_info, &size);
    if(output == ERROR_BUFFER_OVERFLOW)
	{
        FREE(adapter_info);
        adapter_info = HeapAlloc(GetProcessHeap(), 0, size);
        if(adapter_info == NULL)
            return FALSE;
		output = GetAdaptersInfo(adapter_info, &size);
    }
    if(output == NO_ERROR)
	{
        adapter = adapter_info;
        for(i = 0; adapter != NULL; adapter = adapter->Next)
			i++;

		if(i == 0)
		{
			if(adapter_info != NULL)
				FREE(adapter_info);
			return FALSE;
		}
		if(testify_network_arrangement.adapter != NULL)
			free(testify_network_arrangement.adapter);
		testify_network_arrangement.adapter = malloc((sizeof *testify_network_arrangement.adapter) * i);
		testify_network_arrangement.adapter_count = i;
		adapter = adapter_info;
		for(i = 0; adapter != NULL; adapter = adapter->Next)
		{
			for(j = 0; j < 64 - 1 && adapter->Description[j] != 0; j++)
				testify_network_arrangement.adapter[i].adapter_name[j] = adapter->Description[j];
			testify_network_arrangement.adapter[i].adapter_name[j] = 0;
            for(j = 0; j < adapter->AddressLength && j < 6; j++)
				testify_network_arrangement.adapter[i].hardware_address[j] = adapter->Address[j];
            while(j < 6)
				testify_network_arrangement.adapter[i].hardware_address[j++] = 0;		
			testify_network_arrangement.adapter[i].ipv4_local_ip = testify_network_arrangement.adapter[i].ipv4_global_ip = testify_address_parse(adapter->IpAddressList.IpAddress.String); // ip
			testify_network_arrangement.adapter[i].ipv4_mask = testify_address_parse(adapter->IpAddressList.IpMask.String); // mask
			testify_network_arrangement.adapter[i].ipv4_gateway = testify_address_parse(adapter->GatewayList.IpAddress.String); // gateway
			i++;
        }
    }
    if(adapter_info != NULL)
        FREE(adapter_info);

    return TRUE;
}



#else

boolean testify_network_discover()
{
    struct ifaddrs *adapter = NULL, *a;
    union{uint32 address; uint8 parts[4]}convert;
    uint i, j;

	if(getifaddrs(&adapter) == 0)
	{
		i = 0;
		for(a = adapter; a != NULL; a = a->ifa_next)
			if(a->ifa_addr->sa_family == AF_INET &&
				a->ifa_flags & IFF_UP && // && /* is online */
		//		a->ifa_flags & IFF_BROADCAST && /* can broadcast */
				!(a->ifa_flags & IFF_LOOPBACK)) /* isnt a loopback */
				i++;
		if(i == 0)
			return FALSE;
		testify_network_arrangement.adapter = malloc((sizeof *testify_network_arrangement.adapter) * i);
		testify_network_arrangement.adapter_count = i;
        i = 0;
		for(a = adapter; a != NULL; a = a->ifa_next)
		{
			if(a->ifa_addr->sa_family == AF_INET &&
				(a->ifa_flags & IFF_UP) && /* is online */
		//		a->ifa_flags & IFF_BROADCAST && /* can broadcast */
				!(a->ifa_flags & IFF_LOOPBACK)) /* isnt a loopback */
			{
                if(a->ifa_name != NULL)
                {
                    for(j = 0; j < 64 - 1 && a->ifa_name[j] != 0; j++)
                        testify_network_arrangement.adapter[i].adapter_name[j] = a->ifa_name[j];
                    testify_network_arrangement.adapter[i].adapter_name[j] = 0;
                }else
                {
                    char *unnamed = "unnamed";
                    for(j = 0; j < 64 - 1 && unnamed[j] != 0; j++)
                        testify_network_arrangement.adapter[i].adapter_name[j] = unnamed[j];
                    testify_network_arrangement.adapter[i].adapter_name[j] = 0;
                }

                if(a->ifa_addr != NULL)
                {
                    convert.parts[0] = a->ifa_addr->sa_data[2];
                    convert.parts[1] = a->ifa_addr->sa_data[3];
                    convert.parts[2] = a->ifa_addr->sa_data[4];
                    convert.parts[3] = a->ifa_addr->sa_data[5];
                    testify_network_arrangement.adapter[i].ipv4_local_ip = testify_network_arrangement.adapter[i].ipv4_global_ip = htonl(convert.address); // ip
                }else
                    testify_network_arrangement.adapter[i].ipv4_local_ip = testify_network_arrangement.adapter[i].ipv4_global_ip = 0; // ip
                if(a->ifa_netmask != NULL)
                    
                {
                    convert.parts[0] = a->ifa_netmask->sa_data[2];
                    convert.parts[1] = a->ifa_netmask->sa_data[3];
                    convert.parts[2] = a->ifa_netmask->sa_data[4];
                    convert.parts[3] = a->ifa_netmask->sa_data[5];
                    testify_network_arrangement.adapter[i].ipv4_mask = htonl(convert.address); // mask
                }
                else
                    testify_network_arrangement.adapter[i].ipv4_mask = 0; // mask
#ifdef __APPLE__
                if(a->ifa_dstaddr != NULL)
                {
                    convert.parts[0] = a->ifa_dstaddr->sa_data[2];
                    convert.parts[1] = a->ifa_dstaddr->sa_data[3];
                    convert.parts[2] = a->ifa_dstaddr->sa_data[4];
                    convert.parts[3] = a->ifa_dstaddr->sa_data[5];
                    testify_network_arrangement.adapter[i].ipv4_gateway = htonl(convert.address); // gateway
                }
                else
                    testify_network_arrangement.adapter[i].ipv4_gateway = 0; // gateway
#else
                if(a->ifa_ifu.ifu_broadaddr != NULL)
                {
                    convert.parts[0] = a->ifa_ifu.ifu_broadaddr->sa_data[2];
                    convert.parts[1] = a->ifa_ifu.ifu_broadaddr->sa_data[3];
                    convert.parts[2] = a->ifa_ifu.ifu_broadaddr->sa_data[4];
                    convert.parts[3] = a->ifa_ifu.ifu_broadaddr->sa_data[5];
                    testify_network_arrangement.adapter[i].ipv4_gateway = htonl(convert.address); // gateway
                }
                else
                    testify_network_arrangement.adapter[i].ipv4_gateway = 0; // gateway
#endif
				i++;
			}
        }
    }
    return TRUE;
}

#endif


uint testify_network_adapter_count()
{
//	if(testify_network_arrangement.adapter == NULL)
	testify_network_discover();
	return testify_network_arrangement.adapter_count;
}

char *testify_network_adapter_name(uint id)
{
	return testify_network_arrangement.adapter[id].adapter_name;
}

uint32 testify_network_adapter_ipv4_local_address(uint id)
{
	return testify_network_arrangement.adapter[id].ipv4_local_ip;
}

uint32 testify_network_adapter_ipv4_global_address(uint id)
{
	return testify_network_arrangement.adapter[id].ipv4_global_ip;
}

uint32 testify_network_adapter_ipv4_mask(uint id)
{
	return testify_network_arrangement.adapter[id].ipv4_mask;
}

uint32 testify_network_adapter_ipv4_gateway(uint id)
{
	return testify_network_arrangement.adapter[id].ipv4_gateway;
}


uint8 *testify_network_adapter_mac_address(uint id)
{
	return testify_network_arrangement.adapter[id].hardware_address;
}

#define testify_ipv4_address(a, b, c, d) (a * (256 * 256 * 256) + b * (256 * 256) + c * 256 + d)

boolean testify_network_ipv4_local(uint32 ip_address)
{
	TestifyNetworkAddress address;
	char buffer[64], buffer2[64];
	address.ip = ip_address;
	address.port = 0;

	testify_network_debug_address_print(&address, buffer);
	if(ip_address == 0)
		return TRUE;
	if(ip_address == testify_ipv4_address(127, 0, 0, 1))
		return TRUE;
	if((ip_address & testify_ipv4_address(255, 0, 0, 0)) == testify_ipv4_address(10, 0, 0, 0))
		return TRUE;
	if(ip_address >= testify_ipv4_address(172, 16, 0, 0) && ip_address <= testify_ipv4_address(172, 31, 255, 255))
		return TRUE;
	address.ip = ip_address & testify_ipv4_address(255, 255, 0, 0);
	testify_network_debug_address_print(&address, buffer2);
	if((ip_address & testify_ipv4_address(255, 255, 0, 0)) == testify_ipv4_address(192, 168, 0, 0))
		return TRUE;
	return FALSE; 
}

void testify_network_adapter_print()
{
	uint i, count;
	union{uint32 ip; uint8 parts[4];}convert;
	count = testify_network_adapter_count();
	printf("Testify Network adapters:\n");
	if(count == 1)
		printf(" - one adapter available:\n \n");
	else
		printf(" - %u adapters available:\n\n", count);
	for(i = 0; i < count; i++)
	{
		printf("Adapters %u: %s\n", i, testify_network_adapter_name(i));
		convert.ip = ntohl(testify_network_adapter_ipv4_local_address(i));
		printf("\tLocal Address %u.%u.%u.%u\n", (uint)convert.parts[0], (uint)convert.parts[1], (uint)convert.parts[2], (uint)convert.parts[3]);
		convert.ip = ntohl(testify_network_adapter_ipv4_global_address(i));
		printf("\tGlobal Address %u.%u.%u.%u\n", (uint)convert.parts[0], (uint)convert.parts[1], (uint)convert.parts[2], (uint)convert.parts[3]);
		convert.ip = ntohl(testify_network_adapter_ipv4_mask(i));
		printf("\tMask %u.%u.%u.%u\n", (uint)convert.parts[0], (uint)convert.parts[1], (uint)convert.parts[2], (uint)convert.parts[3]);
		convert.ip = ntohl(testify_network_adapter_ipv4_gateway(i));
		printf("\tGateway %u.%u.%u.%u\n", (uint)convert.parts[0], (uint)convert.parts[1], (uint)convert.parts[2], (uint)convert.parts[3]);
	//	printf("\tAdapters %u: %s\n", i, testify_network_adapter_mac_address(i));

	}

}

uint testify_network_lan_count()
{
	uint i, j, add, count = 0, adapter_count;
	adapter_count = testify_network_adapter_count();
	for(i = 0; i < adapter_count; i++)
	{
		if(testify_network_arrangement.adapter[i].ipv4_local_ip != 0)
		{
			add = 1;
			for(j = 0; j < 16 && ((~testify_network_arrangement.adapter[i].ipv4_mask >> j) & 1); j++) /* Dont accept local networks bigger then */
				add *= 2;
			count += add;
		}
	}
	return count;
}

boolean testify_network_lan_address_get(uint id, char *buffer, uint buffer_length)
{
	uint i, j, add, count = 0, ip, adapter_count;
	buffer[0] = 0;
	if(buffer < 17)
		return FALSE;
	adapter_count = testify_network_adapter_count();
	for(i = 0; i < adapter_count; i++)
	{
		if(testify_network_arrangement.adapter[i].ipv4_local_ip != 0)
		{
			add = 1;
			for(j = 0; j < 16 && ((~testify_network_arrangement.adapter[i].ipv4_mask >> j) & 1); j++) /* Dont accept local networks bigger then */
				add *= 2;
			if(id >= count && id < count + add)
			{
				id -= count;
				ip = (testify_network_arrangement.adapter[i].ipv4_mask & testify_network_arrangement.adapter[i].ipv4_local_ip) + id;
				sprintf(buffer, "%u.%u.%u.%u", ((ip >> 24) & 255), ((ip >> 16) & 255), ((ip >> 8) & 255), (ip & 255));
				return TRUE;
			}
			count += add;
		}
	}
	return FALSE;
}

typedef struct{
	uint16 port;
	uint8 op;
	double timer;
}TestifyMappedPort;


typedef enum{
	T_NATPMPRT_SUCSESS = 0,
	T_NATPMPRT_UNSUPORTED_VERSION = 1,
	T_NATPMPRT_REFUSED = 2,
	T_NATPMPRT_NETWORK_FAILURE = 3,
	T_NATPMPRT_NO_REASORCES = 4,
	T_NATPMPRT_UNSUPPORTED_OP = 5,
}TestifyNATPMPReturnCodes;

typedef enum{
	T_PCP_SUCSESS = 0,
	T_PCP_UNSUPORTED_VERSION = 1,
	T_PCP_NOT_AUTHORIZED = 2,
	T_PCP_MALFORMED_REQUEST = 3,
	T_PCP_UNSUPP_OPCODE = 4,
	T_PCP_UNSUPP_OPTION = 5,
	T_PCP_MALFORMED_OPTION = 6,
	T_PCP_NETWORK_FAILURE = 7,
	T_PCP_NO_RESOURCES = 8,
	T_PCP_UNSUPP_PROTOCOL = 9,
	T_PCP_USER_EX_QUOTA = 10,
	T_PCP_CANNOT_PROVIDE_EXTERN = 11,
	T_PCP_ALADDRESS_MISMATCH = 12,
	T_PCP_EXCESSIVE_REMOTE_PEERS = 13
}TestifyPCPReturnCodes;

#define testify_ipv4_address(a, b, c, d) (a * (256 * 256 * 256) + b * (256 * 256) + c * 256 + d)


void testify_send_port_mapping(boolean udp, uint16 port)
{
	static TestifyMappedPort *ports = 0;
	static uint port_count = 0;
	static THandle *h = NULL;
	static seconds = 0, fractions = 0, protocol = 2;
	TestifyNetworkAddress to, from;
	uint i, j, k, size, time, adapter_count;
	uint32 s, f;
	double delta;
	if(h == NULL)
		h = testify_network_datagram_create(0);
	

	if(h != NULL)
	{
		if(port != 0)
		{
			for(i = 0; i < port_count && ports[i].port != port; i++);
			if(i == port_count)
			{
				if(i % 8 == 0)
					ports = realloc(ports, (sizeof *ports) * (i + 8));
				ports[port_count].port = port;
				if(udp)
					ports[port_count].op = 1;
				else
					ports[port_count].op = 2;
				ports[port_count].timer = -1;
				port_count++;
			}
			imagine_current_time_get(&s, &f);
			delta = imagine_delta_time_compute(s, f, seconds, fractions);
			seconds = s;
			fractions = f;
			for(i = 0; i < port_count; i++)
			{
				ports[i].timer -= delta;
				if(ports[i].timer < 0.0) /* NAT PMP */
				{
					to.port = 0;
					adapter_count = testify_network_adapter_count();
					for(j = 0; j < adapter_count; j++)
					{
						to.ip = testify_network_adapter_ipv4_gateway(j);
						if(to.ip != 0 && testify_network_ipv4_local(to.ip) && to.ip != testify_ipv4_address(127, 0, 0, 1))
						{
							to.port = 5351;
							switch(protocol)
							{
								case 0 :
									testify_pack_uint8(h, protocol, "version");
									testify_pack_uint8(h, 0, "Op");
									testify_network_datagram_send(h, &to);
									testify_pack_uint8(h, protocol, "version");
									testify_pack_uint8(h, ports[i].op, "TCP Protocol");
									testify_pack_uint16(h, 0, "Reserved");
									testify_pack_uint16(h, port, "Internal Port");
									testify_pack_uint16(h, port, "External Port");
									testify_pack_uint32(h, 60, "time");
									testify_network_datagram_send(h, &to);
								break;
								case 2 :
									testify_pack_uint8(h, protocol, "version");
									testify_pack_uint8(h, 1, "MAP");
									testify_pack_uint16(h, 0, "Reserved");
									testify_pack_uint32(h, 60, "time");
									if(TRUE)
									{
										for(k = 0; k < 10; k++)
											testify_pack_uint8(h, 0, "IP");
										for(k = 0; k < 2; k++)
											testify_pack_uint8(h, 255, "IP");
										testify_pack_uint32(h, testify_network_adapter_ipv4_local_address(j), "IP");	
									}else /* IP V6 */
										for(k = 0; k < 16; k++)
											testify_pack_uint8(h, 0, "IP");
									/* Header end */
									for(k = 0; k < 12; k++)
										testify_pack_uint8(h, k, "random");
									if(ports[i].op == 1) /* */
										testify_pack_uint8(h, 17, "protocol"); /*UDP*/
									else
										testify_pack_uint8(h, 6, "protocol"); /*TCP*/
									testify_pack_uint8(h, 0, "reserved");
									testify_pack_uint8(h, 0, "reserved");
									testify_pack_uint8(h, 0, "reserved");
									testify_pack_uint16(h, port, "Internal Port");
									testify_pack_uint16(h, port, "External Port");
									if(TRUE /*IP V4*/)
									{
										for(k = 0; k < 10; k++)
											testify_pack_uint8(h, 0, "IP");
										for(k = 0; k < 2; k++)
											testify_pack_uint8(h, 255, "IP");
										for(k = 0; k < 4; k++)
											testify_pack_uint8(h, 0, "IP");	
									}else /* IP V6 */
										for(k = 0; k < 16; k++)
											testify_pack_uint8(h, 0, "IP");
									testify_network_datagram_send(h, &to);
								break;
							}
						}
					}
					if(to.port != 0)
						ports[i].timer = 10.0; /* wait 10 seconds to try again */
				//	testify_network_datagram_send(h, &to);
				}
			}
		}
		if(size = testify_network_receive(h, &from))
		{
			if(from.port ==  5351 && size >= 4)
			{
				adapter_count = testify_network_adapter_count();
				for(i = 0; i < adapter_count && from.ip != testify_network_adapter_ipv4_gateway(i); i++);
				if(i < adapter_count)
				{
					uint version, opcode, result;
					version = testify_unpack_uint8(h, "version");
					opcode = testify_unpack_uint8(h, "opcode");
					result = testify_unpack_uint16(h, "result code");
					switch(version)
					{
						case 0 : /* NAT PMP */
							if(result == T_NATPMPRT_UNSUPORTED_VERSION ||
								result == T_NATPMPRT_UNSUPPORTED_OP)
								protocol = 2; /* Try a differenet protocol */
							else if(result == T_NATPMPRT_SUCSESS && opcode == 128 && size >= 16)
							{
								testify_unpack_uint32(h, "time");
								testify_network_arrangement.adapter[i].ipv4_global_ip = testify_unpack_uint32(h, "IP");	
							}
							else if(result == T_NATPMPRT_SUCSESS && size >= 16 && (opcode == 1 || opcode == 2))
							{
								testify_unpack_uint32(h, "time");
								port = testify_unpack_uint16(h, "private port");
								testify_unpack_uint16(h, "public port");
								time = testify_unpack_uint32(h, "lifetime");
								for(i = 0; i < port_count && ports[i].port != port; i++);
								if(i < port_count && opcode == 127 + ports[i].op)
									ports[i].timer = time - 5.0;
							}
						break;
						case 2 : /* PCP */
							if(result == T_PCP_UNSUPORTED_VERSION ||
								result == T_PCP_MALFORMED_REQUEST ||
								result == T_PCP_UNSUPP_OPCODE ||
								result == T_PCP_UNSUPP_OPTION ||
								result == T_PCP_MALFORMED_OPTION ||
								result == T_PCP_UNSUPP_PROTOCOL)
								protocol = 0; /* Try a differenet protocol */
							else if(result == T_PCP_SUCSESS && size >= 40)
							{
								uint16 public_port;
								time = testify_unpack_uint32(h, "lifetime");
								testify_unpack_uint32(h, "Epochtime");
								for(k = 0; k < 3; k++)
									testify_unpack_uint32(h, "reserved");
								for(k = 0; k < 3; k++)
									testify_unpack_uint32(h, "ranadom");
								testify_unpack_uint8(h, "protocol");
								testify_unpack_uint8(h, "reserved");
								testify_unpack_uint8(h, "reserved");
								testify_unpack_uint8(h, "reserved");
								port = testify_unpack_uint16(h, "private port");
								public_port = testify_unpack_uint16(h, "public port");

								for(k = 0; k < 10; k++)
									testify_unpack_uint8(h,"IP");
								for(k = 0; k < 2; k++)
									testify_unpack_uint8(h, "IP");
								testify_network_arrangement.adapter[i].ipv4_global_ip = testify_unpack_uint32(h, "IP");	
								/* get external IP here */
								for(i = 0; i < port_count && ports[i].port != port; i++);
								if(i < port_count && opcode == 127 + ports[i].op)
									ports[i].timer = time - 5.0;
							}
						break;
					}
				}
			}
		}
	}
}


THandle *testify_network_stream_address_create(const char *host_name, uint16 port)
{
	THandle *handle;
	struct hostent *he;
	char *colon = NULL, *buf = NULL;
	boolean ok = FALSE;
	uint socket;
	if(host_name == NULL)
	{
		socket = testify_socket_create(TRUE, port);
		if(socket == -1)
			return NULL;
		testify_send_port_mapping(FALSE, port);
	}else
	{
		socket = testify_socket_create(TRUE, 0);
		if(socket == -1)
			return NULL;
	}
/*	if(host_name == NULL)
		if(listen(address->socket, 16) < 0)
	        printf("listen() failed");*/
	if(host_name != NULL)
	{
		TestifyNetworkAddress ip;
		struct sockaddr_in address_in;
		int a;
		if(testify_network_address_lookup(&ip, host_name, port))
		{
			memset(&address_in, 0, sizeof(address_in));
			address_in.sin_family = AF_INET; 
			address_in.sin_port = htons(ip.port); 
			address_in.sin_addr.s_addr = ntohl(ip.ip);
			a = connect(socket, &address_in, sizeof(address_in));
			if(a < 0) 
			{
				return NULL;
			}
		//	printf("port %u\n", (uint)ip.port);
		}else
			return NULL;
	}
#if defined _WIN32
	{
		unsigned long	one = 1UL;
		if(ioctlsocket(socket, FIONBIO, &one) != 0)
			return NULL;
	}
#else
	if(fcntl(socket, F_SETFL, O_NONBLOCK) != 0)
	{
		fprintf(stderr, "Testify: Couldn't make socket non-blocking\n");
		exit(0); /* FIX ME */
		return NULL;
	}
#endif
	handle = malloc(sizeof *handle);
	if(host_name == NULL)
		testify_handle_clear(handle, T_HT_STREAMING_SERVER);
	else
		testify_handle_clear(handle, T_HT_STREAMING_CONNECTION);
	handle->port = port;
	handle->socket = socket;
	handle->connected = TRUE;
    return handle;
}

void testify_network_stream_address_destroy(THandle *handle)
{
	if(handle->read_buffer != NULL)
		free(handle->read_buffer);
	if(handle->write_buffer != NULL)
		free(handle->write_buffer);
	if(handle->file != NULL)
		fclose(handle->file);
	if(handle->text_copy != NULL)
		fclose(handle->text_copy);
}

THandle *testify_network_stream_wait_for_connection(THandle *listener, TestifyNetworkAddress *from)
{
	THandle *handle;
	struct sockaddr_in address;
	int socket;
	uint length;
	boolean read = TRUE;
	testify_send_port_mapping(FALSE, listener->port);
	if(listen(listener->socket, 16) >= 0)
	{
		testify_network_wait(&listener, &read, NULL, 1, 1);
		if(read)
		{
			length = sizeof(struct sockaddr_in);
			if((socket = accept(listener->socket, (struct sockaddr *) &address, &length)) >= 0)
			{

#if defined _WIN32
//		if(!stream)
		{
			unsigned long	one = 1UL;
			if(ioctlsocket(socket, FIONBIO, &one) != 0)
				return NULL;
		}
#else
		if(fcntl(socket, F_SETFL, O_NONBLOCK) != 0)
		{
			fprintf(stderr, "Testify: Couldn't make socket non-blocking\n");
			exit(0); /* FIX ME */
			return NULL;
		}
#endif

				handle = malloc(sizeof *handle);
				testify_handle_clear(handle, T_HT_STREAMING_CONNECTION);
				handle->debug_descriptor = listener->debug_descriptor;
				handle->socket = socket;
				handle->ip = ntohl(address.sin_addr.s_addr);
				handle->port = ntohs(address.sin_port);
				if(from != NULL)
				{
					from->ip = handle->ip;
					from->port = handle->port;
				}	
				return handle;
			}
		}
	}else
	{
		listener->connected = FALSE;
		printf("UNRAVEL; Listerner failed\n");
	}
	return NULL;
}
/*
typedef struct{
	unsigned int ip;
	unsigned short port;
	int			socket;
	THandleType type;
	uint8		*read_buffer;
	uint		read_buffer_size;
	uint		read_buffer_pos;	
	uint8		*write_buffer;
	uint		write_buffer_size;
	void		*file;
	void		*text_copy;
	boolean		debug_descriptor;
}THandle;*/


THandle *testify_network_datagram_create(uint16 port)
{
	THandle *handle;
	uint32 socket;
	socket = testify_socket_create(FALSE, port);
	if(socket == -1)
		return NULL;
	handle = malloc(sizeof *handle);
	handle->ip = 0;
	handle->port = port;
	handle->socket = socket;
	handle->type = T_HT_PACKET_PEER;
	handle->read_buffer = malloc((sizeof *handle->read_buffer) * 1500);
	handle->read_buffer_pos = 0;	
	handle->read_buffer_size = 1500;
	handle->read_marker = -1;
	handle->read_raw_progress = 0;
	handle->write_buffer = malloc((sizeof *handle->write_buffer) * 1500);
	handle->write_buffer_pos = 0;
	handle->write_buffer_size = 1500;
	handle->write_raw_progress = 0;
	handle->file = NULL;
	handle->text_copy = NULL;
	handle->debug_descriptor = FALSE;
	return handle;
}

int testify_network_datagram_send(THandle *handle, TestifyNetworkAddress *to)
{
	struct sockaddr_in	address_in;
	int out;
	address_in.sin_family = AF_INET;
	address_in.sin_port = htons(to->port);
	address_in.sin_addr.s_addr = htonl(to->ip);
	memset(&address_in.sin_zero, 0, sizeof address_in.sin_zero);
	out = sendto(handle->socket, handle->write_buffer, handle->write_buffer_pos, 0, (struct sockaddr *)&address_in, sizeof(struct sockaddr_in));
	if(out < 0)
	{
#ifdef _WIN32 
		printf("Testify: sendto failed with error %u\n", WSAGetLastError());
#endif // _WIN32 

		handle->connected = FALSE;
	}else
		out += 0;
	handle->write_buffer_pos = 0;
	return out;
}



void testify_network_debug_address_print(TestifyNetworkAddress *address, char *buffer)
{
	sprintf(buffer, "%u.%u.%u.%u:%u", (address->ip >> 24) & 0xFF, (address->ip >> 16) & 0xFF, (address->ip >> 8) & 0xFF, address->ip & 0xFF,  address->port);
}



void testify_network_debug_incomming(THandle *handle)
{
	printf("Testify: incomming data stream:\n");
	f_print_raw(&handle->read_buffer[handle->read_buffer_pos], handle->read_buffer_used - handle->read_buffer_pos);
}

void testify_network_debug_outbound(THandle *handle)
{
	printf("Testify: outbound data stream:\n");
	f_print_raw(handle->write_buffer, handle->write_buffer_pos);
}


size_t testify_network_stream_send(THandle *handle, uint8 *buffer, size_t length)
{	
	int size;
	if(length == 0)
		length += 0;
	size = send(handle->socket, buffer, length, 0);

	if(size < 0)
	{
		int error;
#if defined _WIN32
		error = WSAGetLastError();
		if(error == WSAEWOULDBLOCK)
			return 0;
#else
		error = errno;
		if(error ==  EAGAIN || error == EWOULDBLOCK || error == 0) /* Given that its non blocking recv may return an error if no data is available. EWOULDBLOCK */
			return 0;
#endif
		printf("send Error %u\n", error);
		handle->connected = FALSE;
		return 0;
	}
	return (size_t)size;
}

int testify_network_stream_send_force(THandle *handle)
{
	int i, size;
	if(handle->type == T_HT_STREAMING_CONNECTION)
	{
		if(handle->write_buffer_pos != 0 && handle->connected)
		{
			boolean write;
		/*	testify_network_wait(&handle, NULL, &write, 1, 0);
			if(!write)
				return FALSE;*/

			size = testify_network_stream_send(handle, handle->write_buffer, handle->write_buffer_pos);
			if(size == handle->write_buffer_pos)
				handle->write_buffer_pos = 0;
			else if(size != 0)
			{
				for(i = 0; i < handle->write_buffer_pos - size; i++)
					handle->write_buffer[i] = handle->write_buffer[size + i];
				handle->write_buffer_pos -= size;
			}
			return size;
		}
	}else
		printf("testify_network_stream_send_force FAILED %u %u %u", handle->type, T_HT_STREAMING_CONNECTION, handle->connected);

	if(handle->type == T_HT_FILE_WRITE)
		testify_pack_buffer_clear(handle);
	return 0;
}


uint testify_network_wait(THandle **handles, boolean *read, boolean *write, unsigned int handle_count, unsigned int microseconds)
{
	struct timeval	tv;
	fd_set fd_read, fd_write;
	unsigned int i, max = 0;

/*	if(microseconds == 0)
		return 0;*/
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	for(i = 0; i < handle_count; i++)
	{
		if(read != NULL && read[i] && handles[i]->connected)
		{
			FD_SET(handles[i]->socket, &fd_read);
			if(max < handles[i]->socket)
				max = handles[i]->socket;
		}
		if(write != NULL && write[i] && handles[i]->connected)
		{
			FD_SET(handles[i]->socket, &fd_write);
			if(max < handles[i]->socket)
				max = handles[i]->socket;
		}
	}
	tv.tv_sec  = microseconds / 1000000;
	tv.tv_usec = microseconds % 1000000;
	select(max + 1, &fd_read, &fd_write, NULL, &tv);
	for(i = 0; i < handle_count; i++)
	{
		if(read != NULL && read[i] && handles[i]->connected)
			read[i] = FD_ISSET(handles[i]->socket, &fd_read);
		if(write != NULL && write[i] && handles[i]->connected)
			write[i] = FD_ISSET(handles[i]->socket, &fd_write);
	}
	return microseconds;
}


size_t testify_network_stream_receve(THandle *handle, uint8 *buffer, size_t length)
{
	static uint seed = 0, sum = 0;
	boolean read = TRUE;
	int get;
/*	if(f_randf(seed++) < 0.8)
		return 0;*/
/*	testify_network_wait(&handle, &read, NULL, 0, 0);
	if(!read)
		return FALSE;*/
	get = recv(handle->socket, buffer, length, 0);
	if(get != 0)
		get += 0;
	if(get > 0)
		sum += get;
//	get = recv(handle->socket, &handle->read_buffer[handle->read_buffer_used], 1, 0);

/*	if(get > 0)
	{
		uint i;
		printf("Network GET\n");
		for(i = 0; i < get; i++)
			printf("data[i] = %u %c\n", i, (uint)handle->read_buffer[handle->read_buffer_used + i], handle->read_buffer[handle->read_buffer_used + i]);
	}*/
	
	if(get < 0)
	{
		int error;
#if defined _WIN32
		error = WSAGetLastError();
		if(error == WSAEWOULDBLOCK)
			return 0;
#else
		error = errno;
		if(error ==  EAGAIN || error == EWOULDBLOCK || error == 0) /* Given that its non blocking recv may return an error if no data is available. EWOULDBLOCK */
			return 0;
#endif
		handle->connected = FALSE;
		return 0;
	}
	return get;
}

/*
typedef struct{
	unsigned int ip;
	unsigned short port;
	int			socket;
	THandleType type;
	uint8		*read_buffer;
	uint		read_buffer_size;
	uint		read_buffer_pos;	
	uint8		*write_buffer;
	uint		write_buffer_size;
	void		*file;
	void		*text_copy;
	boolean		debug_descriptor;
}THandle;
*/
int testify_network_receive(THandle *handle, TestifyNetworkAddress *from)
{
	struct	sockaddr_in address_in;
	int	from_length = sizeof address_in, len;
	memset(&address_in, 0, sizeof address_in);
	address_in.sin_family = AF_INET;
	address_in.sin_port = htons(handle->port); 
	address_in.sin_addr.s_addr = INADDR_ANY;
	len = recvfrom(handle->socket,  handle->read_buffer, 1500, 0, (struct sockaddr *) &address_in, &from_length);
	if(len < 0)
	{
		len = WSAGetLastError();
		if(10035 != len)
			len = 0;
		return 0;
	}
	handle->read_buffer_pos = 0; 	
	handle->read_marker = -1;
	if(len > 0 && from != NULL)
	{
		handle->read_buffer_used = len;
		from->ip   = ntohl(address_in.sin_addr.s_addr);
		from->port = ntohs(address_in.sin_port);
		return len;
	}
	if(handle->port != 0)
		testify_send_port_mapping(TRUE, handle->port);
	return 0;
}


