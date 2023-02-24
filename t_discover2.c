
#include <string.h>
#include "forge.h"
#include "imagine.h"
#include "testify.h"


uint8 testify_discover_id[6];

typedef enum{
	T_DUDPC_QUERY,
	T_DUDPC_ANNOUNCE,
	T_DUDPC_RETIRE,
	T_DUDPC_RELAY_BIT = 0x8,
	T_DUDPC_COUNT
}TestifyDiscoverUDPCommand;


void testify_discover_debug(TestifyDiscovery *discover)
{
	uint i = 0;
	if(discover->peer_count > 0 && discover->peers_private[0].user_id[0] == 0 && discover->peers_private[0].user_id[1] == 0 && discover->peers_private[0].user_id[2] == 0 && discover->peers_private[0].user_id[3] == 0)
		i += 0;
}

void testify_discover_id_create()
{
	union {uint32 integer[2]; uint8 array[8];} merge;
	uint8 *read;
	uint i, j;
	imagine_current_time_get(&merge.integer[0], &merge.integer[1]);
	for(i = 0; i < 6; i++)
		testify_discover_id[(merge.integer[1] + i) % 6] = merge.array[i] ^ merge.array[i + 2];
	for(i = 0; i < testify_network_adapter_count(); i++)
	{
		read = testify_network_adapter_mac_address(i);
		for(j = 0; j < 6; j++)
			testify_discover_id[(merge.integer[0] + j) % 6] ^= read[j];
	}
}

void testify_pack_sting_nulless(THandle *handle, char *string)
{
	printf("sending %s", string);
	while(*string != 0)
		testify_pack_int8(handle, *string++, "");
}

int testify_discover_user_compare(TeestifyDiscoverUser *user_a, TeestifyDiscoverUser *user_b)
{
	return user_a->user_id[0] == user_a->user_id[0] && 
			user_a->user_id[1] == user_a->user_id[1] && 
			user_a->user_id[2] == user_a->user_id[2] && 
			user_a->user_id[3] == user_a->user_id[3] && 
			user_a->user_id[4] == user_a->user_id[4] && 
			user_a->user_id[5] == user_a->user_id[5];
}



TestifyDiscovery *testify_discover_list = NULL;
void *testify_discover_list_mutex = NULL;
THandle *testify_discovery_udp_handle_port = NULL;
THandle *testify_discovery_udp_handle_alt = NULL;

char *testify_discovery_character_transform_new = "zxvtsrqpnmkjhgdb";
char *testify_discovery_character_transform = "lfcmugypwbvkjxqz";


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


void testify_discover_pack_message(THandle *handle, TestifyDiscovery *discovery, char *user, uint32 ip)
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

TestifyDiscovery *testify_discover_get(char *service)
{
	TestifyDiscovery *d;
	uint i;
	imagine_mutex_lock(testify_discover_list_mutex);
	d = testify_discover_list;
	imagine_mutex_unlock(testify_discover_list_mutex); 
	for(; d != NULL; d = d->next)
	{
		for(i = 0; d->service[i] != 0 && d->service[i] == service[i]; i++);
		if(d->service[i] == service[i])
			return d;
	}
	return NULL;
}

TestifyDiscovery *testify_discover_channel(char *string)
{
	uint8 service[33];
	testify_discover_decode(service, string, 32);
	return testify_discover_get(service);
}



void testify_discover_user_remove(TestifyDiscovery *discover, uint32 ip, uint16 port)
{
	uint i;
	for(i = 0; i < discover->peer_count; i++)
	{
		if(discover->peers_public[i].address.ip == ip && discover->peers_public[i].address.port == port && discover->peers_public[i].state != TESTIFY_DS_REMOVED)
		{
			discover->updated = TRUE;
			discover->peers_public[i].state = TESTIFY_DS_REMOVED;
			if(discover->peers_private[i].relay_port != 0)
				discover->ref_count--;
			return;
		}
	}
}


TestifyDiscovery *testify_discover_user_add(TestifyDiscovery *d, uint *user_id, char *service, uint32 ip, uint16 port, char *message, uint16 relay_port)
{
	uint8 buffer[513];
	uint i, j, k;
	if(d == NULL)
	{
		if(relay_port)
			return NULL;
		d = testify_discover_create(service, 0, "", TRUE, FALSE);
	}
	for(j = 0; j < d->peer_count && (d->peers_public[j].address.port != port || d->peers_public[j].address.ip != ip); j++);
	*user_id = j;
	if(j == d->peer_count)
	{
		if(d->peer_count > 0)
			d->peer_count += 0;
		if(d->peer_count == d->peer_allocated)
		{
			d->peer_allocated += 16;
			d->peers_public = realloc(d->peers_public, (sizeof *d->peers_public) * d->peer_allocated);
			d->peers_private = realloc(d->peers_private, (sizeof *d->peers_private) * d->peer_allocated);
		}	
		memcpy(d->peers_private[j].user_id, user_id, TESTIFY_DISCOVER_USER_ID_LENGTH);
		d->peers_private[j].relay_port = relay_port;
		if(relay_port != 0)
			d->ref_count++;		
		d->updated = TRUE;
		d->peers_public[j].address.ip = ip; 
		d->peers_public[j].address.port = port;
		d->peers_public[j].state = TESTIFY_DS_ADDED;
		d->peers_public[j].local = TRUE;
		d->peers_public[j].message[0] = 0;
		d->peer_count++;
	}
	if(j == 0)
		j += 0;

	d->peers_private[j].timeout = 0;
	if(message != NULL)
	{
		for(k = 0; k < TESTIFY_NETWORK_PEER_MESSAGE_LENGTH - 1 && message[k] != 0; k++)
			d->peers_public[j].message[k] = message[k];
		d->peers_public[j].message[k] = 0;
	}
	imagine_mutex_unlock(d->mutex);
	return d;
}

/*
TestifyDiscovery *testify_discover_parse_message_old(char *user, char *string)
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
		for(j = k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH && j < testify_discovery[i].peer_count; j++)
			for(k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH && user[k] == testify_discovery[i].users[j * 32 + k]; k++);
		
		if(j == testify_discovery[i].peer_count)
		{
			if(testify_discovery[i].peer_count == testify_discovery[i].peer_allocated)
			{
				testify_discovery[i].peer_allocated += 16;
				testify_discovery[i].peers_public = realloc(testify_discovery[i].peers_public, (sizeof *testify_discovery[i].peers_public) * testify_discovery[i].peer_allocated);
				testify_discovery[i].users = realloc(testify_discovery[i].users, (sizeof *testify_discovery[i].users) * TESTIFY_DISCOVER_USER_NAME_LENGTH * testify_discovery[i].peer_allocated);
			}		
			for(k = 0; k < TESTIFY_DISCOVER_USER_NAME_LENGTH; k++)
				testify_discovery[i].users[j * TESTIFY_DISCOVER_USER_NAME_LENGTH + k] = user[k];	
			testify_discovery[i].peer_count++;
		}
		testify_discovery[i].peers_public[j].address.ip = ip; 
		testify_discovery[i].peers_public[j].address.port = port;
		for(k = 0; k < TESTIFY_NETWORK_PEER_MESSAGE_LENGTH - 1 && buffer[7 + k] != 0; k++)
			testify_discovery[i].peers_public[j].message[k] = buffer[7 + k];
		testify_discovery[i].peers_public[j].message[k] = 0;
	}
	return &testify_discovery[i];
}
*/
#define testify_ipv4_address(a, b, c, d) (a * (256 * 256 * 256) + b * (256 * 256) + c * 256 + d)



void testify_discover_udp_pack(TestifyDiscovery *d, TestifyNetworkAddress *address, uint8 command, TestifyNetworkAddress *service_address, char *message)
{
	TestifyNetworkAddress a;
	THandle *handle;
	uint i, j;
	if(testify_discovery_udp_handle_port != NULL)
		handle = testify_discovery_udp_handle_port;
	else
		handle = testify_discovery_udp_handle_alt;
	testify_pack_uint8(handle, command, "Command");
	if(command & T_DUDPC_RELAY_BIT)
		testify_pack_uint32(handle, service_address->ip, "IP");
	testify_pack_uint16(handle, service_address->port, "Port");
	testify_pack_string(handle, d->service, "Service");
	testify_pack_string(handle, message, "Message");
	if(message[0] == 'o')
		i = 0;
	testify_network_datagram_send(handle, address);
}

void testify_discover_udp_broadcast(TestifyDiscovery *d, uint8 command, TestifyNetworkAddress *service_address, char *message)
{
	TestifyNetworkAddress a;
	THandle *handle;
	uint i, adapter_count;
	if(testify_discovery_udp_handle_port != NULL)
		handle = testify_discovery_udp_handle_port;
	else
		handle = testify_discovery_udp_handle_alt;
	adapter_count = testify_network_adapter_count();	
	for(i = 0; i < adapter_count; i++)
	{
		uint32 local;
		local = testify_network_adapter_ipv4_local_address(i);
		if(local != 0 && local != testify_ipv4_address(127, 0, 0, 1) && testify_network_ipv4_local(local))
		{
			a.ip = testify_network_adapter_ipv4_local_address(i) | ~testify_network_adapter_ipv4_mask(i);
			a.port = 5352;
			testify_pack_uint8(handle, command, "Command");
			if(command & T_DUDPC_RELAY_BIT)
				testify_pack_uint32(handle, service_address->ip, "IP");
			testify_pack_uint16(handle, service_address->port, "Port");
			testify_pack_string(handle, d->service, "Service");
			testify_pack_string(handle, message, "Message");
			if(message[0] == 'o')
				i += 0;
			testify_network_datagram_send(handle, &a);
		}
	}
}


void testify_discover_update_local(float delta)
{
	TestifyDiscovery *d = NULL;
	TestifyNetworkAddress a, a2, service_address;
	TestifyDiscoverUDPCommand command;
	THandle *handle;
	uint i, j, pos, id = -1, adapter_count, length;
	uint16 service_port;
	uint8 buffer[1500];
	char s[32], m[200];
	boolean local;


	handle = testify_discovery_udp_handle_port;
	if(handle == NULL)
	{
		handle = testify_discovery_udp_handle_port = testify_network_datagram_create(5352);
		if(testify_discovery_udp_handle_alt == NULL)
			printf("Tried to aquire main port %p\n", testify_discovery_udp_handle_port);
		if(testify_discovery_udp_handle_port != NULL)
			printf("Aquiring Port\n");
		if(handle != NULL && testify_discovery_udp_handle_alt != NULL)
		{
			printf("Releasing Alt Port\n");
			testify_free(testify_discovery_udp_handle_alt);
			testify_discovery_udp_handle_alt = NULL;
		}
	}
	if(handle == NULL)
	{
		handle = testify_discovery_udp_handle_alt;
		if(handle == NULL)
		{
			printf("Aquiring Port Alt\n");
			handle = testify_discovery_udp_handle_alt = testify_network_datagram_create(0);
		}
	}
	if(handle == NULL)
		return;

	if((length = testify_network_receive(handle, &a)) >= 11)
	{	
		for(i = 0; i < length; i++)
			buffer[i] = testify_unpack_uint8(handle, "Command");

		local = testify_network_ipv4_local(a.ip);
		if(local)
		{
			if(testify_discovery_udp_handle_port != handle && a.port != 5352)
				return;
			if(testify_discovery_udp_handle_port == handle && a.port == 5352)
				return;
			testify_network_address_lookup(&a, "localhost", a.port);
		}else if(testify_discovery_udp_handle_port == NULL && a.port != 5352)
			   return;
		service_address = a;
		command = buffer[0];
		if(command & T_DUDPC_RELAY_BIT)
		{
			if(!local)
				return;
			service_address.ip = ((uint32)buffer[1] << 24) | ((uint32)buffer[2] << 16) | ((uint32)buffer[3] << 8) | ((uint32)buffer[4]);
			service_address.port = ((uint32)buffer[5] << 8) | (uint32)buffer[6];
			command &= ~T_DUDPC_RELAY_BIT;
			pos = 7;
		}else
		{
			service_address.port = ((uint32)buffer[1] << 8) | (uint32)buffer[2];
			pos = 3;
		}		
		for(i = 0;; i++)
		{
			if(i >= 32)
				return;
			s[i] = buffer[pos++];
			if(s[i] == 0)
				break;
		}
		d = testify_discover_get(s);
		if(d != NULL && local && service_address.port == d->port)
			return;
		for(i = 0;; i++)
		{
			if(i >= 200)
				return;
			m[i] = buffer[pos++];
			if(m[i] == 0)
				break;
		}
		if(pos > length)
			return;
		if(command == T_DUDPC_RETIRE)
		{
			if(d == NULL)
				return;
			if(local && service_address.port == d->port)
				return;
			for(id = 0; ; id++)
			{
				if(id == d->peer_count)
					return;
				if(d->peers_public[id].address.ip == service_address.ip || d->peers_public[id].address.port == service_address.port)
				{
					imagine_mutex_lock(d->mutex);
					if(d->peers_public[id].state != TESTIFY_DS_REMOVED)
					{
						d->updated = TRUE;
						d->peers_public[id].state = TESTIFY_DS_REMOVED;
					}
					imagine_mutex_unlock(d->mutex);
					break;
				}
			}
		}else
			d = testify_discover_user_add(d, &id, s, service_address.ip, service_address.port, m, a.port);
		if(d != NULL)
		{
			if(local)
			{
				if(testify_discovery_udp_handle_port != NULL)
				{
					if(command == T_DUDPC_QUERY)
					{

						for(j = 0; j < d->peer_count; j++) /* answer any local query */
						{					
							if(j != id)
							{
								if(d->peers_private[j].relay_port != 0)
									testify_discover_udp_pack(d, &a, T_DUDPC_ANNOUNCE, &d->peers_public[j].address, d->peers_public[j].message);
								else
									testify_discover_udp_pack(d, &a, T_DUDPC_ANNOUNCE | T_DUDPC_RELAY_BIT, &d->peers_public[j].address, d->peers_public[j].message);
							}
						}

						if(d->port != 0)
						{
							a2.port = d->port;
							testify_discover_udp_pack(d, &a, T_DUDPC_ANNOUNCE, &a2, d->message);
						}
					}
					if(command == T_DUDPC_RETIRE)
					{
						for(j = 0; j < d->peer_count; j++) /* answer any local query */
							if(j != id && d->peers_private[j].relay_port != 0)
								testify_discover_udp_broadcast(d, T_DUDPC_RETIRE, &service_address, "");		
					}
				}
			}else
			{
				printf("non Local\n");
				if(command == T_DUDPC_QUERY)
				{
					for(j = 0; j < d->peer_count; j++) /* answer any global query */
						if(d->peers_private[j].relay_port != 0)
							testify_discover_udp_pack(d, &a, T_DUDPC_ANNOUNCE, &d->peers_public[j].address, d->peers_public[j].message);
					if(d->port != 0)
						testify_discover_udp_pack(d, &a, T_DUDPC_ANNOUNCE, d->port, d->message);
				}
			}
			if(command == T_DUDPC_QUERY)
				command = T_DUDPC_ANNOUNCE;
			if(testify_discovery_udp_handle_port != NULL)
			{
				for(j = 0; j < d->peer_count; j++) /* share any incomming command with local peers */
				{
					if(d->peers_private[j].relay_port != 0 && j != id)
					{
						testify_network_address_lookup(&a2, "localhost", d->peers_private[j].relay_port);
						testify_discover_udp_pack(d, &a2, command, &service_address, m);
					}
				}
			}
		}
	}
	/* send out ping commands */
	imagine_mutex_lock(testify_discover_list_mutex);
	d = testify_discover_list;
	imagine_mutex_unlock(testify_discover_list_mutex); 
	for(; d != NULL; d = d->next)
	{
		d->udp_ping_timer += delta;
		if(d->udp_ping_timer > 10)	
		{
			command = T_DUDPC_QUERY;
			d->udp_ping_timer = 0;
			if(testify_discovery_udp_handle_port != NULL)
			{
/*				if(d->port != 0)
				{
					service_address.port = d->port;
					testify_discover_udp_broadcast(d, T_DUDPC_QUERY, &service_address, d->message);
					command = T_DUDPC_ANNOUNCE;
				}
				for(j = 0; j < d->peer_count; j++)
				{
					if(d->peers_private[j].relay_port != 0)
					{
						testify_discover_udp_broadcast(d, command, &d->peers_public[j].address, d->peers_public[j].message);
						command = T_DUDPC_ANNOUNCE;
					}
				}
*/			}else
			{
				testify_network_address_lookup(&a, "localhost", 5352);
				service_address.port = d->port;
				testify_discover_udp_pack(d, &a, T_DUDPC_QUERY, &service_address, d->message);
			}
			break;
		}
	}
}



void testify_discover_tread_func(TestifyDiscovery *discover)
{
	uint i, new_seconds, new_fractions, old_seconds, old_fractions;
	TestifyDiscovery *d = NULL, **next = NULL;
	double delta;
fprintf(stderr, "X");	
	imagine_mutex_lock(testify_discover_list_mutex);
	imagine_current_time_get(&old_seconds, &old_fractions);
	while(TRUE)
	{
fprintf(stderr, "Y");
		imagine_mutex_unlock(testify_discover_list_mutex);
		imagine_current_time_get(&new_seconds, &new_fractions);
		delta = imagine_delta_time_compute(new_seconds, new_fractions, old_seconds, old_fractions); 
		old_seconds = new_seconds;
		old_fractions = new_fractions;
		testify_discover_update_local(delta);
		imagine_mutex_lock(testify_discover_list_mutex);
		next = &testify_discover_list;

fprintf(stderr, "Z");
		while(*next != NULL)
		{
			d = *next;

			if(d->ref_count == 0)
			{
fprintf(stderr, "Destroy");
				printf("Destroying discovery/n");
				*next = d->next;
				imagine_mutex_destroy(d->mutex);
				if(d->peers_public != NULL)
					free(d->peers_public);
				if(d->peers_private != NULL)
					free(d->peers_private);
				free(d);
				break;
			}
			imagine_mutex_lock(d->mutex);
			for(i = 0; i < d->peer_count; i++)
			{
fprintf(stderr, "Peer");
				d->peers_private[i].timeout += delta;
				if(d->peers_private[i].timeout > 20.0)
				{
					d->peers_public[i].state = TESTIFY_DS_REMOVED;
					d->updated = TRUE;
				}
			}
			imagine_mutex_unlock(d->mutex);
			next = &(*next)->next;
		}
fprintf(stderr, "Q");
		if(testify_discover_list == NULL)
		{
fprintf(stderr, "List");
			if(testify_discovery_udp_handle_port != NULL)
			{
				printf("Releaseing Port\n");
				testify_free(testify_discovery_udp_handle_port);
				testify_discovery_udp_handle_port = NULL;
			}
			if(testify_discovery_udp_handle_alt != NULL)
			{
				printf("Releaseing Alt Port\n");
				testify_free(testify_discovery_udp_handle_alt);
				testify_discovery_udp_handle_alt = NULL;
			}
			printf("Destroying thread\n");
			imagine_mutex_unlock(testify_discover_list_mutex); 
			return;
		}
fprintf(stderr, "W");
	}
} // WYZQW

void testify_discover_message_set(TestifyDiscovery *discovery, char *message)
{

//	imagine_mutex_unlock(discovery->mutex);
	
}

boolean testify_discover_updated(TestifyDiscovery *discovery)
{
	boolean updated;
	imagine_mutex_lock(discovery->mutex);
	updated = discovery->updated;
	imagine_mutex_unlock(discovery->mutex);
	return updated;
}


TestifyNetworkPeer *testify_discover_peers_get(TestifyDiscovery *discovery, uint *count)
{
	imagine_mutex_lock(discovery->mutex);
	*count = discovery->peer_count;
	return discovery->peers_public;
}

void testify_discover_peers_return(TestifyDiscovery *discovery)
{
	if(discovery->updated)
	{
		uint i;
		for(i = 0; i < discovery->peer_count; i++)
			if(discovery->peers_public[i].state == TESTIFY_DS_REMOVED)
				discovery->peers_public[i--] = discovery->peers_public[--discovery->peer_count];
		for(i = 0; i < discovery->peer_count; i++)
			discovery->peers_public[i].state = TESTIFY_DS_NONE;
		discovery->updated = FALSE;
	}
	imagine_mutex_unlock(discovery->mutex);
}

typedef enum{
	TESTIFY_NONE,
	TESTIFY_ADDED,
	TESTIFY_REMOVED,
	TESTIFY_COUNT
}TDiscoverEvent;



TestifyDiscovery *testify_discover_create(char *service, uint16 port, char *message, boolean local, boolean global)
{
	TestifyDiscovery *discovery = NULL, *prev;
	uint i, j;
fprintf(stderr, "sTART");	
	if(testify_discover_list_mutex == NULL)
	{
		testify_discover_id_create();
		testify_discover_list_mutex = imagine_mutex_create();
	}
fprintf(stderr, "0");	
	imagine_mutex_lock(testify_discover_list_mutex);
	for(discovery = testify_discover_list; discovery != NULL; discovery = discovery->next)
	{
		for(i = 0; discovery->service[i] != 0 && discovery->service[i] == service[i]; i++);
		if(discovery->service[i] == service[i])
		{
			imagine_mutex_unlock(testify_discover_list_mutex);
			imagine_mutex_lock(discovery->mutex);
			if(message != NULL)
			{
				for(j = 0; j < 200 - 1 && message[j] != 0; j++)
					discovery->message[j] = message[j];
				discovery->message[j] = 0;
			}else
				discovery->message[0] = 0;
			discovery->global = global;
			discovery->local = local;
			discovery->port = port;
			discovery->ref_count++;
			imagine_mutex_unlock(discovery->mutex);
			break;
		}
	}
	if(discovery == NULL)
		imagine_mutex_unlock(testify_discover_list_mutex);

fprintf(stderr, "1");	
	if(discovery == NULL)
	{
		discovery = malloc(sizeof *discovery);
		discovery->mutex = imagine_mutex_create();
		discovery->peers_public = NULL;
		discovery->peer_count = 0;
		discovery->peer_allocated = 0;
		discovery->service = f_text_copy_allocate(service);
		discovery->peers_private = NULL;
		discovery->channel[0] = 0;
		discovery->port = port;
		if(message == NULL)
		{
			discovery->message[0] = 0;
		}else
		{
			for(i = 0; i < 200 - 1 && message[i] != 0; i++)
				discovery->message[i] = message[i];
			discovery->message[i] = 0;
		}
		discovery->global = global;
		discovery->local = local;
		discovery->udp_ping_timer = 10000000; // Ping right away!
		discovery->joined = FALSE;
		discovery->ref_count = 1;
		imagine_mutex_lock(testify_discover_list_mutex);
		discovery->next = testify_discover_list;
		testify_discover_list = discovery;
		imagine_mutex_unlock(testify_discover_list_mutex);
	}

fprintf(stderr, "2");	
	if(discovery->next == NULL)
		imagine_thread(testify_discover_tread_func, NULL, "Testify discover thread");

fprintf(stderr, "3");	
	return discovery;
}


void testify_discover_destroy(TestifyDiscovery *discovery)
{
	TestifyNetworkAddress service_address, address;
	uint i;
	imagine_mutex_lock(discovery->mutex);
	service_address.port = discovery->port;
	testify_network_address_lookup(&address, "localhost", 5352);
	if(testify_discovery_udp_handle_port == NULL)
	{
		testify_discover_udp_pack(discovery, &address, T_DUDPC_RETIRE, &service_address, "");
	}else
	{
		testify_discover_udp_broadcast(discovery, T_DUDPC_RETIRE, &service_address, "");
		for(i = 0; i < discovery->peer_count; i++)
		{
			if(discovery->peers_private[i].relay_port != 0)
			{
				address.port = discovery->peers_private[i].relay_port;
				printf("retire send to %u\n", (uint)discovery->peers_private[i].relay_port);
				testify_discover_udp_pack(discovery, &address, T_DUDPC_RETIRE, &service_address, "");
			}
		}
	}
	discovery->ref_count--;
	discovery->port = 0;
	imagine_mutex_unlock(discovery->mutex);
}

TestifyDiscovery *testify_discover_parse_message(char *user, char *string)
{
	TestifyDiscovery *d;
	uint8 buffer[513];
	uint16 port;
	uint32 ip;
	uint i, j, adapter_count;
	i = testify_discover_decode(buffer, string, 512);
	buffer[i] = 0;
	imagine_mutex_lock(testify_discover_list_mutex);
	for(d = testify_discover_list; d != NULL; d = d->next)
	{
		for(j = 0; d->service[j] != 0 && d->service[j] == buffer[j]; j++);
		if(d->service[j] == buffer[j])
			break;
	}
	imagine_mutex_unlock(testify_discover_list_mutex);
	if(d == NULL)
		return NULL;

	if(buffer[j] == 4) /* IPv4 */
	{
		ip = buffer[j + 1] * (256 * 256 * 256) + buffer[j + 2] * (256 * 256) + buffer[j + 3] * (256) + buffer[j + 4];
		port = buffer[j + 5] * (256) + buffer[j + 6];
		adapter_count = testify_network_adapter_count();
		for(i = 0; i < adapter_count && testify_network_adapter_ipv4_global_address(i) != ip; i++);
		if(i == adapter_count)
		{
			buffer[j] = 0;
	//		return testify_discover_user_add(d, buffer, ip, port, &buffer[j + 7], 0);
		}else
			i = 0;
	}
	return NULL;
}

void testify_discover_id_creat()
{
	union {uint32 integer[2]; uint8 array[8];} merge;
	uint8 *read;
	uint i, j;
	imagine_current_time_get(&merge.integer[0], &merge.integer[1]);
	for(i = 0; i < 6; i++)
		testify_discover_id[(merge.integer[1] + i) % 6] = merge.array[i] ^ merge.array[i + 2];
	for(i = 0; i < testify_network_adapter_count(); i++)
	{
		read = testify_network_adapter_mac_address(i);
		for(j = 0; j < 6; j++)
			testify_discover_id[(merge.integer[0] + j) % 6] ^= read[j];
	}
}

THandle *testify_discover_global_connect()
{
	char *servers[11] = {"chat.freenode.net", "irc.dal.net", "irc.gamesurge.net", "open.ircnet.net", "irc.quakenet.org", "irc.rizon.net", "irc.swiftirc.net", "eu.undernet.org", "us.undernet.org", "irc.webchat.org", "irc.2600.net"};
	TestifyDiscovery *d;
	static THandle *handle = NULL;
	char *read, user_name[13] = {0}, rand[6], message[17 + 12 * 2];
	uint i;
	imagine_mutex_lock(testify_discover_list_mutex);
	for(d = testify_discover_list; d != NULL || d->global; d = d->next);
	imagine_mutex_unlock(testify_discover_list_mutex);
	if(d == NULL)
	{
		if(handle != NULL)
		{
			testify_free(handle);
			handle = NULL;
		}
		return NULL;
	}
	if(handle != NULL)
		return handle;
	if(testify_network_adapter_count() > 0)
	{
		for(i = 0; i < 11 && (handle = testify_network_stream_address_create(servers[i], 6667)) == NULL; i++);
		if(handle != NULL)
		{
			testify_discover_encode(user_name, testify_discover_id, 6, TRUE);
			sprintf(message, "NICK %s\r\n", user_name);
			testify_pack_sting_nulless(handle, message);
			sprintf(message, "USER %s 8 * : %s\r\n", user_name, user_name);
			testify_pack_sting_nulless(handle, message);
			testify_network_stream_send_force(handle);
		}
	}
	return handle;
}
#ifdef NOT_YET
void testify_discover_global_update(THandle *handle)
{
	TestifyDiscovery *d;
	char address[4];
	char *read, reply[512], code[8 + 2 + 1], user, *command, *name, *payload, *channel;
	static char buffer[512], user_name[13] = {0};
	static buffer_length = 0;
	static boolean init = FALSE, connected = FALSE;
	static uint32 ip_address = 0;
	uint32 tmp;
	uint i, j, length, adapter_count;

	/* Global discovery */
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
							d = testify_discover_channel(channel);
							if(d != NULL/* && ip_address != 0*/)
								testify_discover_pack_message(handle, d, name, 0, d->port, d->service);
						}
					}
				}
		//		testify_discover_pack_message(handle, TestifyDiscovery *discovery, name, uint32 ip, uint16 port, char *message)
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
			if(tmp / (256 * 256 * 256) % 256 != 192) // FIX ME
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
}
#endif