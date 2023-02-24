#include "forge.h"
#include "imagine.h"
#include "testify.h"

void testify_pack_sting_nulless(THandle *handle, char *string)
{
	printf("sending %s", string);
	while(*string != 0)
		testify_pack_int8(handle, *string++, "");
}

typedef struct{
	TestifyNetworkPeer *addresses;
	char *users;
	uint address_count;
	uint address_allocated;
	char *service;
//	char channel[64];
	char message[200];
	uint16 port;
	boolean joined;
	boolean global;
	uint32 udp_ping_timer;
}TestifyDiscoveryInternal;

TestifyDiscoveryInternal *testify_discovery = NULL;
uint testify_discovery_count = 0;
char *testify_discovery_character_transform_new = "zxvtsrqpnmkjhgdb";
char *testify_discovery_character_transform = "lfcmugypwbvkjxqz";
#define TESTIFY_DISCOVER_USER_NAME_LENGTH 12

void testify_send_port_mapping(boolean udp, uint16 port);

void testify_discover_encode(char *output, uint8 *input, uint input_count, boolean terminate)
{
	uint i;
	for(i = 0; i < input_count; i++)
	{
		output[i * 2] = testify_discovery_character_transform[input[i] / 16];
		output[i * 2 + 1] = testify_discovery_character_transform[input[i] % 16];
	}
	if(terminate)
		output[i * 2] = 0;
}
uint testify_discover_decode(char *output, uint8 *input, uint output_max)
{
	uint i = 0, j;
	output_max *= 2;
	while(i < output_max)
	{
		for(j = 0; j < 16 && testify_discovery_character_transform[j] != input[i]; j++);
		if(j == 16)
			return i / 2;
		output[i / 2] = j * 16;
		i++;
		for(j = 0; j < 16 && testify_discovery_character_transform[j] != input[i]; j++);
		if(j == 16)
			return i / 2;
		output[i / 2] += j;
		i++;
	}
	return i / 2;
}


void testify_discover_pack_message(THandle *handle, TestifyDiscoveryInternal *discovery, char *user, uint32 ip)
{
	uint8 address[7], buffer[512], channel[64 + 1];
	char *read, packed_address[15], packed_message[401] = {0};
	uint i;
	address[0] = 4; /* ip v4 address*/
	address[1] = (ip / (256 * 256 * 256)) % 256;
	address[2] = (ip / (256 * 256)) % 256;
	address[3] = (ip / (256)) % 256;
	address[4] = ip % 256;
	address[5] = (discovery->port / (256)) % 256;
	address[6] = discovery->port % 256;
	testify_discover_encode(packed_address, address, 7, TRUE);
	if(discovery->message != NULL)
	{
		for(i = 0; discovery->message[i] != 0; i++);
		if(i > 200)
			i = 200;
		testify_discover_encode(packed_message, discovery->message, i, TRUE);
	}
	for(i = 0; i < 32 && discovery->service[i] != 0; i++);
	testify_discover_encode(channel, discovery->service, i, TRUE);

	if(user == NULL)
		sprintf(buffer, "PRIVMSG #%s :%s%s%s\r\n", channel, channel, packed_address, packed_message);
	else
		sprintf(buffer, "PRIVMSG %s :%s%s%s\r\n", user, channel, packed_address, packed_message);
	testify_pack_sting_nulless(handle, buffer);
	testify_network_stream_send_force(handle);
}

uint testify_discover_channel(char *string)
{
	uint8 address[7], buffer[512], service[33];
	uint i, j;
	testify_discover_decode(service, string, 32);

	for(i = 0; i < testify_discovery_count; i++)
	{
		for(j = 0; testify_discovery[i].service[j] != 0 && testify_discovery[i].service[j] == service[j]; j++);
		if(testify_discovery[i].service[j] == 0)
			break;
	}
	if(i == testify_discovery_count)
		return -1;
	return i;
}



void testify_discover_user_remove(TestifyDiscoveryInternal *discover, char *user)
{
	uint i, j;
	for(i = 0; i < discover->address_count; i++)
	{
		for(j = 0; j < TESTIFY_DISCOVER_USER_NAME_LENGTH && discover->users[TESTIFY_DISCOVER_USER_NAME_LENGTH * i + j] == user[j]; j++);
		if(j == TESTIFY_DISCOVER_USER_NAME_LENGTH)
		{
			discover->address_count--;
			discover->addresses[i] = discover->addresses[discover->address_count];
			for(j = 0; j < TESTIFY_DISCOVER_USER_NAME_LENGTH; j++)
				discover->users[i * TESTIFY_DISCOVER_USER_NAME_LENGTH + j] =
				discover->users[discover->address_count * TESTIFY_DISCOVER_USER_NAME_LENGTH + j];
			return;
		}
	}
}


TestifyDiscoveryInternal *testify_discover_add(char *user, char *service, uint32 ip, uint16 port, char *message)
{
	uint8 buffer[513];
	uint i, j, k;
	for(i = 0; i < testify_discovery_count; i++)
	{
		for(j = 0; testify_discovery[i].service[j] != 0 && testify_discovery[i].service[j] == service[j]; j++);
		if(testify_discovery[i].service[j] == 0)
			break;
	}
	if(i == testify_discovery_count)
		return NULL;
	if(port == 0)
		return &testify_discovery[i];		
	if(user == NULL)
		for(j = 0; j < testify_discovery[i].address_count && (testify_discovery[i].addresses[j].address.port != port || testify_discovery[i].addresses[j].address.ip != ip); j++);
	else
		for(j = k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH && j < testify_discovery[i].address_count; j++)
			for(k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH && user[k] == testify_discovery[i].users[j * 32 + k]; k++);
	
	if(j == testify_discovery[i].address_count)
	{
		if(testify_discovery[i].address_count == testify_discovery[i].address_allocated)
		{
			testify_discovery[i].address_allocated += 16;
			testify_discovery[i].addresses = realloc(testify_discovery[i].addresses, (sizeof *testify_discovery[i].addresses) * testify_discovery[i].address_allocated);
			testify_discovery[i].users = realloc(testify_discovery[i].users, (sizeof *testify_discovery[i].users) * TESTIFY_DISCOVER_USER_NAME_LENGTH * testify_discovery[i].address_allocated);
		}
	
		if(user != NULL)
		{
			for(k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH; k++)
				testify_discovery[i].users[j * TESTIFY_DISCOVER_USER_NAME_LENGTH + k] = user[k];	
		}else
			for(k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH; k++)
				testify_discovery[i].users[j * TESTIFY_DISCOVER_USER_NAME_LENGTH + k] = 0;	
		testify_discovery[i].address_count++;
	}
	testify_discovery[i].addresses[j].address.ip = ip; 
	testify_discovery[i].addresses[j].address.port = port;
	if(message != NULL)
	{
		for(k = 0; k < TESTIFY_NETWORK_PEER_MESSAGE_LENGTH - 1 && message[k] != 0; k++)
			testify_discovery[i].addresses[j].message[k] = message[k];
		testify_discovery[i].addresses[j].message[k] = 0;
	}
	return &testify_discovery[i];
}

TestifyDiscoveryInternal *testify_discover_parse_message(char *user, char *string)
{
	uint8 buffer[513];
	uint16 port;
	uint32 ip;
	uint i, j, adapter_count;
	i = testify_discover_decode(buffer, string, 512);
	buffer[i] = 0;
	for(i = 0; i < testify_discovery_count; i++)
	{
		
		for(j = 0; testify_discovery[i].service[j] != 0 && testify_discovery[i].service[j] == buffer[j]; j++);
		if(testify_discovery[i].service[j] == 0)
			break;
	}
	if(i == testify_discovery_count)
		return NULL;

	if(buffer[j] == 4)
	{
		ip = buffer[j + 1] * (256 * 256 * 256) + buffer[j + 2] * (256 * 256) + buffer[j + 3] * (256) + buffer[j + 4];
		port = buffer[j + 5] * (256) + buffer[j + 6];
		adapter_count = testify_network_adapter_count();
		for(i = 0; i < adapter_count && testify_network_adapter_ipv4_global_address(i) != ip; i++);
		if(i == adapter_count)
		{
			buffer[j] = 0;
			return testify_discover_add(user, buffer, ip, port, &buffer[j + 7]);
		}else
			i = 0;
	}
	return NULL;
}

/*
TestifyDiscoveryInternal *testify_discover_parse_message_old(char *user, char *string)
{
	uint8 buffer[513];
	uint i, j, k;
	for(i = 0; i < testify_discovery_count; i++)
	{
		for(j = 0; testify_discovery[i].channel[j] != 0 && testify_discovery[i].channel[j] == string[j]; j++);
		if(testify_discovery[i].channel[j] == 0)
			break;
	}
	if(i == testify_discovery_count)
		return NULL;
	j = testify_discover_decode(buffer, &string[j], 512);
	buffer[j] = 0;
	if(buffer[0] == 4 && j > 6)
	{
		uint32 ip;
		uint16 port;
		ip = buffer[1] * (256 * 256 * 256) + buffer[2] * (256 * 256) + buffer[3] * (256) + buffer[4];
		port = buffer[5] * (256) + buffer[6];
		for(j = k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH && j < testify_discovery[i].address_count; j++)
			for(k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH && user[k] == testify_discovery[i].users[j * 32 + k]; k++);
		
		if(j == testify_discovery[i].address_count)
		{
			if(testify_discovery[i].address_count == testify_discovery[i].address_allocated)
			{
				testify_discovery[i].address_allocated += 16;
				testify_discovery[i].addresses = realloc(testify_discovery[i].addresses, (sizeof *testify_discovery[i].addresses) * testify_discovery[i].address_allocated);
				testify_discovery[i].users = realloc(testify_discovery[i].users, (sizeof *testify_discovery[i].users) * TESTIFY_DISCOVER_USER_NAME_LENGTH * testify_discovery[i].address_allocated);
			}		
			for(k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH; k++)
				testify_discovery[i].users[j * TESTIFY_DISCOVER_USER_NAME_LENGTH + k] = user[k];	
			testify_discovery[i].address_count++;
		}
		testify_discovery[i].addresses[j].address.ip = ip; 
		testify_discovery[i].addresses[j].address.port = port;
		for(k = 0; k < TESTIFY_NETWORK_PEER_MESSAGE_LENGTH - 1 && buffer[7 + k] != 0; k++)
			testify_discovery[i].addresses[j].message[k] = buffer[7 + k];
		testify_discovery[i].addresses[j].message[k] = 0;
	}
	return &testify_discovery[i];
}
*/
#define testify_ipv4_address(a, b, c, d) (a * (256 * 256 * 256) + b * (256 * 256) + c * 256 + d)

typedef enum{
	T_DUDPC_QUERY,
	T_DUDPC_ANNOUNCE,
	T_DUDPC_COUNT
}TestifyDiscoverUDPCommand;

void testify_discover_udp_pack(THandle *handle, TestifyNetworkAddress *address, boolean reply, char *service, uint16 port, char *message)
{
	testify_pack_uint8(handle, reply, "Command");
	testify_pack_uint16(handle, port, "Port");
	testify_pack_string(handle, service, "Service");
	testify_pack_string(handle, message, "Message");
	testify_network_datagram_send(handle, address);
}

TestifyNetworkPeer *testify_discover(char *service, uint16 port, uint *count, uint8 *seed, uint seed_length, char *message, boolean global)
{
	char *servers[11] = {"chat.freenode.net", "irc.dal.net", "irc.gamesurge.net", "open.ircnet.net", "irc.quakenet.org", "irc.rizon.net", "irc.swiftirc.net", "eu.undernet.org", "us.undernet.org", "irc.webchat.org", "irc.2600.net"};
	TestifyDiscoveryInternal *discovery = NULL;
	char /**sixteen = "LFCMUGYPWBVKJXQZ",*/ address[4];
	char *read, reply[512], code[8 + 2 + 1], user, *command, *name, *payload, *channel;
	static char buffer[512], user_name[13] = {0};
	static buffer_length = 0;
	static THandle *handle = NULL, *udp_handle = NULL;
	static boolean init = FALSE, connected = FALSE;
	static uint32 ip_address = 0;
	uint32 tmp;
	uint i, j, length, adapter_count;
	for(i = 0; i < testify_discovery_count; i++)
	{
		for(j = 0; service[j] != 0 && service[j] == testify_discovery[i].service[j]; j++);
		if(service[j] == testify_discovery[i].service[j])
		{
			discovery = &testify_discovery[i];
			break;
		}
	}
	if(i == testify_discovery_count)
	{
		if(testify_discovery_count % 4 == 0)
			testify_discovery = realloc(testify_discovery, (sizeof *testify_discovery) * (testify_discovery_count + 4));
		discovery = &testify_discovery[testify_discovery_count++];
		discovery->addresses = NULL;
		discovery->address_count = 0;
		discovery->address_allocated = 0;
		discovery->service = f_text_copy_allocate(service);
		discovery->users = NULL;
	//	discovery->channel[0] = 0;
		discovery->port = port;
		discovery->message[0] = 0;
		discovery->global = global;
		discovery->udp_ping_timer = 0;
		discovery->joined = FALSE;
	}
	for(i = 0; i < 199 && message[i] != 0 && discovery->message[i] == message[i]; i++);
	if(i == 199 || message[i] != 0)
	{
		ip_address = 0;
		for(i = 0; i < 199 && message[i] != 0; i++)
			discovery->message[i] = message[i];
		discovery->message[i] = 0;
	}	
	
	testify_send_port_mapping(0, port);

	if(udp_handle == NULL)
		for(i = 0; i < 4 &&	(udp_handle = testify_network_datagram_create(5352 + i)) == NULL; i++);

	/* Local discovery */	
	imagine_current_time_get(&tmp, NULL);
	

	if(udp_handle != NULL)
	{
		TestifyNetworkAddress a;
		if(discovery->udp_ping_timer + 10 < tmp || discovery->udp_ping_timer > tmp)
		{
			discovery->udp_ping_timer = tmp;
			adapter_count = testify_network_adapter_count();
			for(i = 0; i < adapter_count; i++)
			{
				uint32 local;
				local = testify_network_adapter_ipv4_local_address(i);
				if(local != 0 && local != testify_ipv4_address(127, 0, 0, 1) && testify_network_ipv4_local(local))
				{
					a.ip = testify_network_adapter_ipv4_local_address(i) | ~testify_network_adapter_ipv4_mask(i);
					for(j = 0; j < 4; j++)
					{
						a.port = j + 5352;
						testify_discover_udp_pack(udp_handle, &a, T_DUDPC_QUERY, service, port, message);
					}
				}
			}
		}
		while((length = testify_network_receive(udp_handle, &a)) >= 5)
		{	
			TestifyDiscoveryInternal *d = NULL;
			uint8 reply;
			uint16 reply_port;
			char s[32], m[200];
			reply = testify_unpack_uint8(udp_handle, "Command");
			reply_port = testify_unpack_uint16(udp_handle, "Port");
			testify_unpack_string(udp_handle, s, 32, "Service");
			testify_unpack_string(udp_handle, m, 200, "Message");
			d = testify_discover_add(NULL, s, a.ip, reply_port, m);
			if(d != NULL && d->port != 0 && reply == T_DUDPC_QUERY)
				testify_discover_udp_pack(udp_handle, &a, T_DUDPC_ANNOUNCE, d->service, d->port, d->message);
		}
	}
	if(!discovery->global)
	{
		*count = discovery->address_count;
		return discovery->addresses;  /* Remove me*/
	}
	/* Global discovery */
	if(!init)
	{
		if(testify_network_adapter_count() > 0)
		{
			for(i = 0; i < 11 && (handle = testify_network_stream_address_create(servers[i], 6667)) == NULL; i++);
			if(handle != NULL)
			{
				read = testify_network_adapter_mac_address(0);
				for(i = 0; i < 6; i++)
					reply[i] = read[i];
				for(i = 0; i < seed_length; i++)
					reply[i % 6] ^= seed[i]; /* create a semi random user name that probably wont collide*/
				testify_discover_encode(user_name, reply, 6, TRUE);
				sprintf(reply, "NICK %s\r\n", user_name);
				testify_pack_sting_nulless(handle, reply);
				sprintf(reply, "USER %s 8 * : %s\r\n", user_name, user_name);
				testify_pack_sting_nulless(handle, reply);
				testify_network_stream_send_force(handle);
			}
		}
		init = TRUE;
	}
	if(handle == NULL)
	{
		*count = discovery->address_count;
		return discovery->addresses; 
	}


	while(testify_retivable(handle, 1))
	{
		buffer[buffer_length++] = testify_unpack_int8(handle, "");
		if(buffer_length >= 2 && buffer[buffer_length - 1] == '\n')
		{
			buffer[buffer_length] = 0;
			read = buffer;
			if(read[0] == ':')
			{
				while(*read > ' ')
					read++;
				while(*read <= ' ' && *read != 0)
					read++;
			}
			command = read;
			while(*read != 0 && *read != ':')
				read++;
			if(*read != 0)
				payload = ++read;
			else
				payload = NULL;
			name = NULL; 
			if(buffer[0] == ':')
			{
				for(i = 1; buffer[i] != '!' && buffer[i] != 0; i++);
				if(buffer[i] == '!' && i == TESTIFY_DISCOVER_USER_NAME_LENGTH + 1)
				{
					buffer[i] = 0;
					name = &buffer[1];
				}
			}
			if(command[0] >= '0' && command[0] <= '9')
			{
				if(command[0] == '0' && command[1] == '0' && command[2] == '1') /* welcome command we are connected*/
					connected = TRUE;
			}else if(command[0] == 'P' && command[1] == 'R' && command[2] == 'I' && command[3] == 'V' && command[4] == 'M' && command[5] == 'S' && command[6] == 'G' && command[7] == ' ') /* We have sucsessfully joined our channel. */
			{
				if(name != NULL)
					testify_discover_parse_message(name, payload);
			}else if(command[0] == 'P' && command[1] == 'I' && command[2] == 'N' && command[3] == 'G') /* Respond to ping commands to keep the client alive. */
			{
				testify_pack_sting_nulless(handle, "PONG\r\n");
				testify_network_stream_send_force(handle);
			}else if(command[0] == 'J' && command[1] == 'O' && command[2] == 'I' && command[3] == 'N') /* Respond to ping commands to keep the client alive. */	
			{
				if(name != NULL)
				{
					printf("JOIN %s %s %s\n", buffer, name, payload);
					for(i = 0; user_name[i] != 0 && user_name[i] == name[i]; i++);
					if(user_name[i] != 0) /* remove any message from me*/
 					{
						for(i = 4; command[i] != 0 && command[i] != '#'; i++);
						if(command[i] == '#')
						{
							channel = &command[i + 1];
							i = testify_discover_channel(channel);
							if(i != -1 && ip_address != 0)
								testify_discover_pack_message(handle, &testify_discovery[i], name, ip_address);
						}
					}
				}
		//		testify_discover_pack_message(handle, TestifyDiscoveryInternal *discovery, name, uint32 ip, uint16 port, char *message)
			//	testify_pack_sting_nulless(handle, "PONG\r\n");
			//	testify_network_stream_send_force(handle);
			}else if((command[0] == 'Q' && command[1] == 'U' && command[2] == 'I' && command[3] == 'T') ||
					(command[0] == 'P' && command[1] == 'A' && command[2] == 'R' && command[3] == 'T')) /* Respond to ping commands to keep the client alive. */
			{
				printf("QUIT %s\n", buffer);
				if(name != NULL)
					for(j = 0; j < testify_discovery_count; j++)
						testify_discover_user_remove(&testify_discovery[j], name);
			//	testify_pack_sting_nulless(handle, "PONG\r\n");
			//	testify_network_stream_send_force(handle);
			}else
				printf("Ignored message %s\n", buffer);
			buffer_length = 0;
		}
	}

	if(connected && discovery->joined)
	{
		j = testify_network_adapter_count();
		for(i = 0; i < j; i++)
		{
			tmp = testify_network_adapter_ipv4_global_address(i);
			if(tmp / (256 * 256 * 256) % 256 != 192)
			{
				if(tmp != 0 && connected && ip_address != tmp)
				{
					testify_discover_pack_message(handle, discovery, NULL, tmp);
					printf("updating ip address %u.%u.%u.%u\n", tmp % 256, (tmp / (256)) % 256, (tmp / (256 * 256)) % 256, (tmp / (256 * 256 * 256)) % 256);
				}
				ip_address = tmp;
				break;
			}
		}
	}

	if(connected && !discovery->joined)
	{
		char channel[256];
		for(i = 0; i < 32 && service[i] != 0; i++);
		discovery->joined = TRUE;
		testify_discover_encode(channel, service, i, TRUE);
		sprintf(reply, "JOIN #%s\r\n", channel);
		printf(reply);
		testify_pack_sting_nulless(handle, reply);
	//	sprintf(reply, "PRIVMSG #%s :YfYfYfYf\r\n", discovery->channel);
	//	printf(reply);
	//	testify_pack_sting_nulless(handle, reply);
		testify_network_stream_send_force(handle);
	}

	*count = discovery->address_count;
	return discovery->addresses; 
}
/*
TestifyNetworkPeer *testify_discover_local(char *service, uint16 port, uint *count, uint8 *seed, uint seed_length, char *message)
{
	TestifyNetworkAddress *from;
	THandle *handle;
	char buffer[1500];
	uint8 command;
	uint i, size;
	while((size = testify_network_receive(handle, &from)) != 0)
	{
		command = testify_unpack_uint8(handle, "bytes");
		if(command == 0)
		{
		}
			buffer[i] = testify_unpack_uint8(handle, "bytes");
			for(i = 0; i < size; i++)
				buffer[i] = testify_unpack_uint8(handle, "bytes");
			for(
	}
}*/