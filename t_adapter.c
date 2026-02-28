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
#include <winsock2.h>
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


#define TESTIFY_INTERNAL
#include "testify.h"

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
	TestifyNetworkAdapter *next_adapter;
	uint next_adapter_count;
	void *mutex;
	uint version;
}TestifyNetworkAdapters;

TestifyNetworkAdapters testify_network_adapters = {NULL, 0, NULL, 0, NULL, 0};

extern boolean testify_sockadrr_to_address(TestifyAddress *output_address, struct sockaddr_in *input);
extern void testify_status_adapter_reset(); // remove all exising adapters

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


TestifyNetworkAdapter *testify_network_discover_legacy(uint *output_count)
{
    IP_ADAPTER_INFO *adapter_info;
    IP_ADAPTER_INFO *adapter = NULL;
	TestifyNetworkAdapter *output_adapters = NULL;
	uint adapter_count;
    DWORD dwRetVal = 0;
    uint i, j;
    struct tm newtime;
    char buffer[32];
    errno_t error;
	uint32 size, output;
	size = sizeof *adapter_info;
    adapter_info = malloc(sizeof (IP_ADAPTER_INFO));
	*output_count = 0;
    if(adapter_info == NULL)
        return NULL;
	output = GetAdaptersInfo(adapter_info, &size);
    if(output == ERROR_BUFFER_OVERFLOW)
	{
        free(adapter_info);
        adapter_info = malloc(size);
        if(adapter_info == NULL)
            return NULL;
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
				free(adapter_info);
			return NULL;
		}
		output_adapters = malloc((sizeof *output_adapters) * i);
		*output_count = i;
		adapter = adapter_info;
		for(i = 0; adapter != NULL; adapter = adapter->Next)
		{
			if(adapter->Type != MIB_IF_TYPE_LOOPBACK)
			{
				for(j = 0; j < 64 - 1 && adapter->Description[j] != 0; j++)
					output_adapters[i].adapter_name[j] = adapter->Description[j];
				output_adapters[i].adapter_name[j] = 0;
				for(j = 0; j < adapter->AddressLength && j < 6; j++)
					output_adapters[i].hardware_address[j] = adapter->Address[j];
		        while(j < 6)
					output_adapters[i].hardware_address[j++] = 0;
				output_adapters[i].ipv4_local_ip = output_adapters[i].ipv4_global_ip = testify_address_parse(adapter->IpAddressList.IpAddress.String); // ip
				output_adapters[i].ipv4_mask = testify_address_parse(adapter->IpAddressList.IpMask.String); // mask
				output_adapters[i].ipv4_gateway = testify_address_parse(adapter->GatewayList.IpAddress.String); // gateway
				i++;
			}
        }
    }



	if(adapter_info != NULL)
        free(adapter_info);
    return output_adapters;
}


void testify_adapter_update()
{
	PIP_ADAPTER_ADDRESSES adapter_addresses, address_current;
	DWORD size, return_value;
	TestifyAddress address;
	TestifyAddressType address_type;
	uint32 mask;
	char name[64];
	uint64 flags, flag_mask;
	flags = 0;
	flag_mask = TESTIFY_NS_CONNECTED | TESTIFY_NS_IPV4 | TESTIFY_NS_IPV6 | TESTIFY_NS_DNS | TESTIFY_NS_INTERNET_ACCESS_CONFIRMED | TESTIFY_NS_LAN_ACCESS | TESTIFY_NS_GLOBAL_IP_V4 | TESTIFY_NS_GLOBAL_IP_V6;

	testify_status_adapter_reset();
	GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &size);
	if(size == 0)
		return;

	adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);
	if(adapter_addresses == NULL)
		return;

	if(NO_ERROR == GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_GATEWAYS, NULL, adapter_addresses, &size))
	{
		for(address_current = adapter_addresses; address_current != NULL; address_current = address_current->Next)
		{
			if(address_current->OperStatus == 1 && address_current->IfType != IF_TYPE_SOFTWARE_LOOPBACK)
			{
				PIP_ADAPTER_MULTICAST_ADDRESS_XP multi_link;
				PIP_ADAPTER_UNICAST_ADDRESS_LH link;
			//	f_uint16_to_utf8_string(address_current->FriendlyName, name, 64); 
				name[0] = 0;

				for(link = address_current->FirstUnicastAddress; link != NULL; link = link->Next)
				{
					if((link->PrefixOrigin == IpPrefixOriginManual ||
						link->PrefixOrigin == IpPrefixOriginDhcp ||
						link->PrefixOrigin == IpPrefixOriginRouterAdvertisement) ||
						(link->SuffixOrigin == NlsoDhcp ||
						link->SuffixOrigin == NlsoLinkLayerAddress))
					{
						if(testify_sockadrr_to_address(&address, link->Address.lpSockaddr))
						{
							testify_status_service_set(TESTIFY_SST_ADAPTER, name, &address);
							address_type = testify_address_type(&address); 
							if(address_type == TESTIFY_AT_PRIVATE_NETWORK ||
								address_type == TESTIFY_AT_GLOBAL)
							{
								if(address.transport == TESTIFY_TRANSPORT_IPV4)
									flags |= TESTIFY_NS_CONNECTED | TESTIFY_NS_IPV4;
								if(address.transport == TESTIFY_TRANSPORT_IPV6)
									flags |= TESTIFY_NS_CONNECTED | TESTIFY_NS_IPV6;
								if(link->PrefixOrigin == IpPrefixOriginDhcp)
									flags |= TESTIFY_NS_CONNECTED | TESTIFY_NS_IPV4;
								if(address_type == TESTIFY_AT_PRIVATE_NETWORK)
									flags |= TESTIFY_NS_LAN_ACCESS;
								if(address_type == TESTIFY_AT_GLOBAL)
								{
									if(address.transport == TESTIFY_TRANSPORT_IPV4)
										flags |= TESTIFY_NS_GLOBAL_IP_V4;
									if(address.transport == TESTIFY_TRANSPORT_IPV6)
										flags |= TESTIFY_NS_GLOBAL_IP_V6;
								}
							}
							if(address.transport == TESTIFY_TRANSPORT_IPV4)
							{
								if(NO_ERROR == ConvertLengthToIpv4Mask(link->OnLinkPrefixLength, &mask))
								{
									mask = ntohl(mask);
									address.address.ipv4 = address.address.ipv4 | ~mask;
									testify_status_service_set(TESTIFY_SST_BROADCAST, name, &address);
								}
							}
						}
					}
				}
				for(link = address_current->FirstGatewayAddress; link != NULL; link = link->Next)
				{
					if(testify_sockadrr_to_address(&address, link->Address.lpSockaddr))
						testify_status_service_set(TESTIFY_SST_GATEWAY, name, &address);

						
				}
				for(link = address_current->FirstDnsServerAddress; link != NULL; link = link->Next)
				{
					if(testify_sockadrr_to_address(&address, link->Address.lpSockaddr))
					{
						flags |= TESTIFY_NS_DNS;
						testify_status_service_set(TESTIFY_SST_DNS_GIVEN, name, &address);
					}
				}

			}
		}
	}
	free(adapter_addresses);
	testify_status_flags_set(flags, flag_mask);
}


void testify_adapter_thread(void *user)
{
	while(TRUE)
	{
		NotifyAddrChange(NULL, NULL);
		testify_adapter_update();
	}
}

/*
	TestifyNetworkAdapter *next_adapter;
	uint next_adapter_count;
*/
void testify_adapter_thread_legacy(void *user)
{
	TestifyNetworkAdapter *output_adapters = NULL;
	uint count;
	while(TRUE)
	{
		testify_adapter_update();
		output_adapters = testify_network_discover_legacy(&count);
		imagine_mutex_lock(testify_network_adapters.mutex);
		if(testify_network_adapters.next_adapter != NULL && testify_network_adapters.next_adapter != testify_network_adapters.adapter)
			free(testify_network_adapters.next_adapter);
		testify_network_adapters.next_adapter = output_adapters;
		testify_network_adapters.next_adapter_count = count;
		imagine_mutex_unlock(testify_network_adapters.mutex);
		NotifyAddrChange(NULL, NULL);
	}
}

#else

boolean testify_service_available(TestifyServiceType type, TestifyAddress *address);

void testify_adapter_update()
{
	TestifyAddress address;
	TestifyAddressType address_type;
	uint32 ip;
    struct ifaddrs *adapter = NULL, *a;
	TestifyNetworkAdapter *output_adapters = NULL;
    union{uint32 address; uint8 parts[4]}convert;
	char *name;
    uint i, j;
	uint64 flags, flag_mask;
	flags = 0;
	flag_mask = TESTIFY_NS_CONNECTED | TESTIFY_NS_IPV4 | TESTIFY_NS_IPV6 | TESTIFY_NS_DNS | TESTIFY_NS_INTERNET_ACCESS_CONFIRMED | TESTIFY_NS_LAN_ACCESS | TESTIFY_NS_GLOBAL_IP_V4 | TESTIFY_NS_GLOBAL_IP_V6;

	i = 0;
	if(getifaddrs(&adapter) == 0)
	{
        for(i = 0; i < 2; i++)
		{
			for(a = adapter; a != NULL; a = a->ifa_next)
			{
				if((a->ifa_addr->sa_family == AF_INET || a->ifa_addr->sa_family == AF_INET6) &&
					(a->ifa_flags & IFF_UP) && /* is online */
			//		a->ifa_flags & IFF_BROADCAST && /* can broadcast */
					!(a->ifa_flags & IFF_LOOPBACK)) /* isnt a loopback */
				{
					if(i == 0)
					{
						if(testify_sockadrr_to_address(&address, a->ifa_addr))
						{
							if(!testify_service_available(TESTIFY_SST_ADAPTER, &address))
								break;

							if(address.transport == TESTIFY_TRANSPORT_IPV4)
							{
								ip = address.address.ipv4;
								if(testify_sockadrr_to_address(&address, a->ifa_netmask) && address.transport == TESTIFY_TRANSPORT_IPV4)
								{
									address.address.ipv4 = ip | ~address.address.ipv4;
									if(!testify_service_available(TESTIFY_SST_BROADCAST, &address))
										break;
								}
							}
						}
#ifdef __APPLE__
						if(testify_sockadrr_to_address(&address, a->ifa_dstaddr))
							if(!testify_service_available(TESTIFY_SST_GATEWAY, &address))
								break;
#else
						if(testify_sockadrr_to_address(&address, a->ifa_ifu.ifu_broadaddr))
							if(!testify_service_available(TESTIFY_SST_GATEWAY, &address))
								break;
#endif
					}else
					{
						name = "unnamed";
						if(a->ifa_name != NULL)
							name = a->ifa_name;
						if(testify_sockadrr_to_address(&address, a->ifa_addr))
						{
							testify_status_service_set(TESTIFY_SST_ADAPTER, name, &address);
							address_type = testify_address_type(&address); 
							if(address_type == TESTIFY_AT_PRIVATE_NETWORK ||
								address_type == TESTIFY_AT_GLOBAL)
							{
								if(address.transport == TESTIFY_TRANSPORT_IPV4)
									flags |= TESTIFY_NS_CONNECTED | TESTIFY_NS_IPV4;
								if(address.transport == TESTIFY_TRANSPORT_IPV6)
									flags |= TESTIFY_NS_CONNECTED | TESTIFY_NS_IPV6;
								if(address_type == TESTIFY_AT_PRIVATE_NETWORK)
									flags |= TESTIFY_NS_LAN_ACCESS;
								if(address_type == TESTIFY_AT_GLOBAL)
								{
									if(address.transport == TESTIFY_TRANSPORT_IPV4)
										flags |= TESTIFY_NS_GLOBAL_IP_V4;
									if(address.transport == TESTIFY_TRANSPORT_IPV6)
										flags |= TESTIFY_NS_GLOBAL_IP_V6;
								}
							}
							if(address.transport == TESTIFY_TRANSPORT_IPV4)
							{
								ip = address.address.ipv4;
								if(testify_sockadrr_to_address(&address, a->ifa_netmask) && address.transport == TESTIFY_TRANSPORT_IPV4)
								{
									address.address.ipv4 = ip | ~address.address.ipv4;
									testify_status_service_set(TESTIFY_SST_BROADCAST, name, &address);
								}
							}
						}
#ifdef __APPLE__
						if(testify_sockadrr_to_address(&address, &a->ifa_dstaddr))
							testify_status_service_set(TESTIFY_SST_GATEWAY, name, &address);
#else
						if(testify_sockadrr_to_address(&address, &a->ifa_ifu.ifu_broadaddr))
							testify_status_service_set(TESTIFY_SST_GATEWAY, name, &address);
#endif

					}
				}
			}
			if(a == NULL)
				break;
			testify_status_adapter_reset();
        }
    }
	testify_status_flags_set(flags, flag_mask);
}


TestifyNetworkAdapter *testify_network_discover_legacy(uint *output_count)
{
    struct ifaddrs *adapter = NULL, *a;
	TestifyNetworkAdapter *output_adapters = NULL;
    union{uint32 address; uint8 parts[4]}convert;
    uint i, j;
	i = 0;
	if(getifaddrs(&adapter) == 0)
	{
		for(a = adapter; a != NULL; a = a->ifa_next)
			if(a->ifa_addr->sa_family == AF_INET &&
				a->ifa_flags & IFF_UP && // && /* is online */
		//		a->ifa_flags & IFF_BROADCAST && /* can broadcast */
				!(a->ifa_flags & IFF_LOOPBACK)) /* isnt a loopback */
				i++;
		if(i == 0)
			return NULL;
		output_adapters = malloc((sizeof *output_adapters) * i);
		testify_network_adapters.adapter_count = i;
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
                        output_adapters[i].adapter_name[j] = a->ifa_name[j];
					output_adapters[i].adapter_name[j] = 0;
                }else
                {
                    char *unnamed = "unnamed";
                    for(j = 0; j < 64 - 1 && unnamed[j] != 0; j++)
                        output_adapters[i].adapter_name[j] = unnamed[j];
                    output_adapters[i].adapter_name[j] = 0;
                }

                if(a->ifa_addr != NULL)
                {
                    convert.parts[0] = a->ifa_addr->sa_data[2];
                    convert.parts[1] = a->ifa_addr->sa_data[3];
                    convert.parts[2] = a->ifa_addr->sa_data[4];
                    convert.parts[3] = a->ifa_addr->sa_data[5];
                    output_adapters[i].ipv4_local_ip = output_adapters[i].ipv4_global_ip = htonl(convert.address); // ip
                }else
                    output_adapters[i].ipv4_local_ip = output_adapters[i].ipv4_global_ip = 0; // ip
                if(a->ifa_netmask != NULL)

                {
                    convert.parts[0] = a->ifa_netmask->sa_data[2];
                    convert.parts[1] = a->ifa_netmask->sa_data[3];
                    convert.parts[2] = a->ifa_netmask->sa_data[4];
                    convert.parts[3] = a->ifa_netmask->sa_data[5];
                    output_adapters[i].ipv4_mask = htonl(convert.address); // mask
                }
                else
                    output_adapters[i].ipv4_mask = 0; // mask
#ifdef __APPLE__
                if(a->ifa_dstaddr != NULL)
                {
                    convert.parts[0] = a->ifa_dstaddr->sa_data[2];
                    convert.parts[1] = a->ifa_dstaddr->sa_data[3];
                    convert.parts[2] = a->ifa_dstaddr->sa_data[4];
                    convert.parts[3] = a->ifa_dstaddr->sa_data[5];
                    output_adapters[i].ipv4_gateway = htonl(convert.address); // gateway
                }
                else
                    output_adapters[i].ipv4_gateway = 0; // gateway
#else
                if(a->ifa_ifu.ifu_broadaddr != NULL)
                {
                    convert.parts[0] = a->ifa_ifu.ifu_broadaddr->sa_data[2];
                    convert.parts[1] = a->ifa_ifu.ifu_broadaddr->sa_data[3];
                    convert.parts[2] = a->ifa_ifu.ifu_broadaddr->sa_data[4];
                    convert.parts[3] = a->ifa_ifu.ifu_broadaddr->sa_data[5];
                    output_adapters[i].ipv4_gateway = htonl(convert.address); // gateway
                }
                else
                    output_adapters[i].ipv4_gateway = 0; // gateway
#endif
				i++;
			}
        }
    }
	*output_count = 0;
    return output_adapters;
}

#endif


uint testify_network_adapter_count()
{
	static boolean init = FALSE;
	if(!init)
	{
		testify_network_adapters.mutex = imagine_mutex_create();
		testify_network_adapters.adapter = testify_network_discover_legacy(&testify_network_adapters.adapter_count);
#ifdef _WIN32
		imagine_thread(testify_adapter_thread_legacy, NULL, "Testify adapter update legacy");
#endif
		init = TRUE;
	}
#ifdef _WIN32
	if(testify_network_adapters.adapter != testify_network_adapters.next_adapter)
	{
		imagine_mutex_lock(testify_network_adapters.mutex);
		if(testify_network_adapters.adapter != NULL)
			free(testify_network_adapters.adapter);
		testify_network_adapters.adapter = testify_network_adapters.next_adapter;
		testify_network_adapters.adapter_count = testify_network_adapters.next_adapter_count;
		imagine_mutex_unlock(testify_network_adapters.mutex);
	}
#endif
	return testify_network_adapters.adapter_count;
}

char *testify_network_adapter_name(uint id)
{
	return testify_network_adapters.adapter[id].adapter_name;
}

uint32 testify_network_adapter_ipv4_local_address(uint id)
{
	return testify_network_adapters.adapter[id].ipv4_local_ip;
}

uint32 testify_network_adapter_ipv4_global_address(uint id)
{
	return testify_network_adapters.adapter[id].ipv4_global_ip;
}

uint32 testify_network_adapter_ipv4_mask(uint id)
{
	return testify_network_adapters.adapter[id].ipv4_mask;
}

uint32 testify_network_adapter_ipv4_gateway(uint id)
{
	return testify_network_adapters.adapter[id].ipv4_gateway;
}


uint8 *testify_network_adapter_mac_address(uint id)
{
	return testify_network_adapters.adapter[id].hardware_address;
}

#define testify_ipv4_address(a, b, c, d) ((uint32)a * (256 * 256 * 256) + (uint32)b * (256 * 256) + (uint32)c * 256 + (uint32)d)
/*
typedef enum{
	TESTIFY_AT_LOOPBACK,
	TESTIFY_AT_PRIVATE_NETWORK,
	TESTIFY_AT_DOCUMENTATION,
	TESTIFY_AT_BROADCAST,
	TESTIFY_AT_MULTICAST,
	TESTIFY_AT_GLOBAL
}TestifyAddressType;
*/

extern boolean testify_service_is_localhost(TestifyAddress *address);

TestifyAddressType testify_address_type(TestifyAddress *address)
{
	if(address->transport == TESTIFY_TRANSPORT_IPV4)
	{
		uint32 subnet;

		subnet = address->address.ipv4 & testify_ipv4_address(255, 0, 0, 0);
		if(subnet <= testify_ipv4_address(127, 0, 0, 0))
		{
			if(subnet == testify_ipv4_address(127, 0, 0, 0))
				return TESTIFY_AT_LOOPBACK;
			if(subnet == testify_ipv4_address(0, 0, 0, 0))
				return TESTIFY_AT_PRIVATE_NETWORK;
			if(subnet == testify_ipv4_address(10, 0, 0, 0))
				return TESTIFY_AT_PRIVATE_NETWORK;

			subnet = address->address.ipv4 & testify_ipv4_address(255, 192, 0, 0);
			if((address->address.ipv4 & testify_ipv4_address(240, 0, 0, 0)) == testify_ipv4_address(100, 64, 0, 0)) // Software
				return TESTIFY_AT_PRIVATE_NETWORK;
		}else
		{
			if(subnet < testify_ipv4_address(198, 0, 0, 0))
			{
				if(subnet == testify_ipv4_address(192, 0, 0, 0))
				{
					subnet = address->address.ipv4 & testify_ipv4_address(255, 255, 0, 0);
					if(subnet == testify_ipv4_address(192, 168, 0, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;

					subnet = address->address.ipv4 & testify_ipv4_address(255, 255, 255, 0);
					if(subnet == testify_ipv4_address(192, 0, 0, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;
					if(subnet == testify_ipv4_address(192, 0, 2, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;
					if(subnet == testify_ipv4_address(192, 88, 99, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;
				}else
				{
					subnet = address->address.ipv4 & testify_ipv4_address(255, 255, 0, 0);
					if(subnet == testify_ipv4_address(169, 254, 0, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;

					subnet = address->address.ipv4 & testify_ipv4_address(255, 240, 0, 0);
					if((address->address.ipv4 & testify_ipv4_address(240, 0, 0, 0)) == testify_ipv4_address(172, 16, 0, 0)) // Software
						return TESTIFY_AT_PRIVATE_NETWORK;
				}
			}else
			{
				if(subnet == testify_ipv4_address(198, 0, 0, 0))
				{
					subnet = address->address.ipv4 & testify_ipv4_address(255, 255, 0, 0);

					if(subnet == testify_ipv4_address(198, 18, 0, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;
					if(subnet == testify_ipv4_address(198, 19, 0, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;

					subnet = address->address.ipv4 & testify_ipv4_address(255, 255, 255, 0);
					if(subnet == testify_ipv4_address(198, 51, 100, 0))
						return TESTIFY_AT_DOCUMENTATION;

					subnet = address->address.ipv4 & testify_ipv4_address(255, 240, 0, 0);
					if((address->address.ipv4 & testify_ipv4_address(240, 0, 0, 0)) == testify_ipv4_address(198, 18, 0, 0))
						return TESTIFY_AT_PRIVATE_NETWORK;
				}else
				{
					subnet = address->address.ipv4 & testify_ipv4_address(255, 255, 255, 0);
					if(subnet == testify_ipv4_address(203, 0, 113, 0))
						return TESTIFY_AT_DOCUMENTATION;
					if(subnet == testify_ipv4_address(233, 252, 0, 0))
						return TESTIFY_AT_DOCUMENTATION;


					subnet = address->address.ipv4 & testify_ipv4_address(240, 0, 0, 0);
					if((address->address.ipv4 & testify_ipv4_address(240, 0, 0, 0)) == testify_ipv4_address(224, 0, 0, 0))
						return TESTIFY_AT_MULTICAST;
				}
			}
		}
		return TESTIFY_AT_GLOBAL;
	}else if(address->transport == TESTIFY_TRANSPORT_IPV6)
	{
		if(address->address.ipv6[0] == 0 &&
			address->address.ipv6[1] == 0 &&
			address->address.ipv6[2] == 0 &&
			address->address.ipv6[3] == 0)
			return TESTIFY_AT_LOOPBACK;
		{
			if(address->address.ipv6[4] == 0 &&
				address->address.ipv6[5] == 0 &&
				address->address.ipv6[6] == 0 &&
				address->address.ipv6[7] == 1)
				return TESTIFY_AT_LOOPBACK;

			if((address->address.ipv6[4] == 0 &&
				address->address.ipv6[5] == 0xFFFF) ||
				(address->address.ipv6[4] == 0xFFFF &&
				address->address.ipv6[5] == 0))
			{
				TestifyAddress a;
				a.transport = TESTIFY_TRANSPORT_IPV4;
				a.address.ipv4 = ((uint32)address->address.ipv6[6] << 16) | (uint32)address->address.ipv6[7];
				return testify_address_type(&a);
			}
			return TESTIFY_AT_GLOBAL;
		}

		if(address->address.ipv6[0] == 0x64 &&
			address->address.ipv6[1] == 0xff9b &&
			(address->address.ipv6[2] == 1 || address->address.ipv6[2] == 0) &&
			address->address.ipv6[3] == 0)
		{
			TestifyAddress a;
			a.transport = TESTIFY_TRANSPORT_IPV4;
			a.address.ipv4 = ((uint32)address->address.ipv6[6] << 16) | (uint32)address->address.ipv6[7];
			return testify_address_type(&a);
		}
		if(address->address.ipv6[0] == 0xFE80 &&
			address->address.ipv6[1] == 0 &&
			address->address.ipv6[2] == 0 &&
			address->address.ipv6[3] == 0)
			return TESTIFY_AT_PRIVATE_NETWORK;

		if(address->address.ipv6[0]  >= 0xFC00 && address->address.ipv6[0]  <= 0xFDFF)
			return TESTIFY_AT_PRIVATE_NETWORK;
		if(address->address.ipv6[0] >> 8 == 0xFC)
			return TESTIFY_AT_PRIVATE_NETWORK;

		if(address->address.ipv6[0] == 0x2001 && address->address.ipv6[1] == 0x0db8)
			return TESTIFY_AT_DOCUMENTATION;

		return TESTIFY_AT_GLOBAL;
	}
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
		if(testify_network_adapters.adapter[i].ipv4_local_ip != 0)
		{
			add = 1;
			for(j = 0; j < 16 && ((~testify_network_adapters.adapter[i].ipv4_mask >> j) & 1); j++) /* Dont accept local networks bigger then */
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
	if(buffer_length < 17)
		return FALSE;
	adapter_count = testify_network_adapter_count();
	for(i = 0; i < adapter_count; i++)
	{
		if(testify_network_adapters.adapter[i].ipv4_local_ip != 0)
		{
			add = 1;
			for(j = 0; j < 16 && ((~testify_network_adapters.adapter[i].ipv4_mask >> j) & 1); j++) /* Dont accept local networks bigger then */
				add *= 2;
			if(id >= count && id < count + add)
			{
				id -= count;
				ip = (testify_network_adapters.adapter[i].ipv4_mask & testify_network_adapters.adapter[i].ipv4_local_ip) + id;
				sprintf(buffer, "%u.%u.%u.%u", ((ip >> 24) & 255), ((ip >> 16) & 255), ((ip >> 8) & 255), (ip & 255));
				return TRUE;
			}
			count += add;
		}
	}
	return FALSE;
}
