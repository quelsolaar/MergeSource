#include <stdio.h>
#include <string.h>
#define TESTIFY_INTERNAL
#include "testify.h"



/*
Localy a
*/

typedef enum{
	TESTIFY_DBC_ANNOUNCE, // Announce your existance, Send occationally. Responde only if new. Pass on to locals.
	TESTIFY_DBC_QUERY, // Same as ANNOUNCE but ALWAYS respond
	TESTIFY_DBC_REMOVE, // Announce your retirement. Pass on to locals.
	TESTIFY_DBC_CONNECT // Ask to connect to another peer.
}TestifyDiscoveBroadcastCommand;

#define TESTIFY_ADDRESS_SENDER 255

#define testify_ipv4_address(a, b, c, d) (a * (256 * 256 * 256) + b * (256 * 256) + c * 256 + d)

uint testify_discover_broadcast_send(TestifyDiscovery *discover, uint8 *data, uint length, boolean broadcast)
{
	TestifyAddress address;
	uint i, sent = 0;
	if(broadcast)
	{
		testify_address_lookup(&address, "broadcast", TESTIFY_TRANSPORT_IPV4, discover->local_discover_port);
		sent = testify_datagram_send(discover->local_handle, &address, data, length);
		for(i = 0; i < discover->peer_count; i++)
			if(discover->peers[i].address_local.transport == TESTIFY_TRANSPORT_NULL)
				sent = testify_datagram_send(discover->local_handle, &discover->peers[i].address_global, data, length);
	}
	if(discover->local_have_port)
	{
		testify_address_lookup(&address, "localhost", TESTIFY_TRANSPORT_IPV4, 0);
		for(i = 0; i < discover->local_port_count; i++)
		{
			if(discover->local_ports[i].port != discover->local_discover_port)
			{
				address.port = discover->local_ports[i].port;
				sent = testify_datagram_send(discover->local_handle, &address, data, length);
			}
		}
	}
	return sent;
}

uint testify_discover_broadcast_pack(TestifyDiscovery *discover, uint8 *data, TestifyDiscoveBroadcastCommand command)
{
	TestifyAddress addres_null;
	size_t i, length = 0;
	data[length++] = discover->obfuscate_salt++;
	data[length++] = command;
	if(discover->service_handle != NULL)
		addres_null.port = discover->service_handle->address.port;
	else
		addres_null.port = 0;	 
	addres_null.transport = TESTIFY_TRANSPORT_NULL;
	length += testify_address_serialize(&addres_null, &data[length]);
	if(discover->message != NULL)
		for(i = 0; 0 != discover->message[i] && length < 1000; i++)
			data[length++] = discover->message[i];
	data[length++] = 0;
	for(i = 0; i < discover->peer_count && length + TESTIFY_ADDRESS_MAX_SERIALIZE_SIZE * 2 < 1500; i++)
	{
		discover->peer_start = (discover->peer_start + 1) % discover->peer_count;
		if(discover->peers[discover->peer_start].peer.message[0] != 0 && discover->peers[discover->peer_start].address_global.transport != TESTIFY_TRANSPORT_NULL)
			length += testify_address_serialize(&discover->peers[discover->peer_start].address_global, data);
	}
	testify_obfuscate(discover->obfuscate_key, &data[1], length - 1, data[0]);
	return length;
}

TestifyHandle *testify_discover_broadcast_bind(TestifyDiscovery *discover, boolean *correct_port)
{
	TestifyHandle *handle;
	TestifyAddress address;
	address.transport = TESTIFY_TRANSPORT_IPV4;
	address.address.ipv4 = 0;
	address.port = discover->local_discover_port;
	handle = testify_create(&address, TESTIFY_PROTOCOL_DATAGRAM, TESTIFY_FLAG_BIND);
	if(handle == NULL)
		return NULL;
	testify_address(handle, 0, &address);
	*correct_port = discover->local_discover_port == address.port;
	return handle;
}

void testify_discover_broadcast_initialize(TestifyDiscovery *discover)
{
	TestifyAddress address;
	uint i;
	uint8 parts[2] = {0, 0};
	for(i = 0; discover->service[i] != 0; i++)
		parts[i & 1] ^= (discover->service[i] >> (i & 3)) | (discover->service[i] << (8 - (i & 3)));
	discover->local_discover_port = ((uint16)parts[1] << 8) | (uint16)parts[0];
	if(discover->local_discover_port < 2048)
		discover->local_discover_port = (256 * 256 - 1) - discover->local_discover_port;
	discover->local_timer = TESTIFY_DISCOVER_UDP_TIMEOUT * 3;
	discover->local_handle = testify_discover_broadcast_bind(discover, &discover->local_have_port);
	if(discover->local_handle == NULL)
	{
		TestifyAddress address;
		testify_address_lookup(&address, "any", TESTIFY_TRANSPORT_IPV4, 0);
		discover->local_handle = testify_create(&address, TESTIFY_PROTOCOL_DATAGRAM, 0);
		testify_address(discover->local_handle, 0, &address);
		discover->local_have_port = discover->local_discover_port == address.port;
	}
}

void testify_discover_broadcast_destroy(TestifyDiscovery *discover)
{
	uint8 data[2];
	uint size;
	data[0] = discover->obfuscate_salt++;
	data[1] = TESTIFY_DBC_REMOVE;
	testify_obfuscate(discover->obfuscate_key, &data[1], 1, data[0]);

	testify_discover_broadcast_send(discover, data, 2, TRUE);
	testify_destroy(discover->local_handle);
}

void testify_discover_broadcast_update(TestifyDiscovery *discover, float delta)
{
	TeestifyDiscoverPeerInternal *peer;
	TestifyAddress from, address;
	TestifyDiscoveBroadcastCommand command;
	TestifyHandle *handle;
	TestifyAddressType address_type, source_address_type;
	size_t size, i, j, pos, send_pos;
	uint timeout;
	uint8 data[1500], send_buffer[1500], *message;
	uint16 service_port, port;

	i = discover->local_timer;

	if(testify_status_flags_get() & TESTIFY_NS_LAN_ACCESS)
	{
		discover->local_timer += delta;	
		if(discover->local_timer > TESTIFY_DISCOVER_UDP_TIMEOUT - 1.0)
		{
			if(discover->local_timer < TESTIFY_DISCOVER_UDP_TIMEOUT * 2.0)
				size = testify_discover_broadcast_pack(discover, data, TESTIFY_DBC_ANNOUNCE);
			else
				size = testify_discover_broadcast_pack(discover, data, TESTIFY_DBC_QUERY);
			testify_discover_broadcast_send(discover, data, size, TRUE);
			discover->local_timer = 0.0;
		}
	}else
		discover->local_timer = TESTIFY_DISCOVER_UDP_TIMEOUT;

	if((uint)discover->local_timer / 4 != (uint)(discover->local_timer - delta) / 4)
	{
		if(!discover->local_have_port)
		{
			handle = testify_discover_broadcast_bind(discover, &discover->local_have_port);
			if(discover->local_have_port)
			{
				testify_destroy(discover->local_handle);
				printf("Aquiring port\n");
				discover->local_handle = handle;
				discover->updated = TRUE;
			}else if(handle != NULL)
			{
				testify_destroy(handle);
			}
		}else for(i = 0; i < discover->local_port_count; i++)
			if(++discover->local_ports[i].timer > TESTIFY_DISCOVER_UDP_TIMEOUT)
				discover->local_ports[i--] = discover->local_ports[--discover->local_port_count];
	}

	for(i = 0; i < discover->peer_count; i++)
	{
		discover->peers[i].timeout += delta;
		if(discover->peers[i].timeout > TESTIFY_DISCOVER_UDP_TIMEOUT)
		{
			/* If peer is port holder see if we can take it */
			if(discover->peers[i].timeout > TESTIFY_DISCOVER_UDP_TIMEOUT + 4.0)
			{
				discover->peers[i--] = discover->peers[--discover->peer_count];
				discover->updated = TRUE;
			}else if((uint)discover->peers[i].timeout != (uint)(discover->peers[i].timeout - delta))
			{
				size = testify_discover_broadcast_pack(discover, data, TESTIFY_DBC_QUERY);
				from = discover->peers[i].peer.address;
				from.port = discover->peers[i].discover_port;
				testify_datagram_send(discover->local_handle, &from, data, size);
			}
		}
	}
	while((size = testify_datagram_receive(discover->local_handle, &from, data, 1500)) >= 2)
	{
		int local;		
		source_address_type = testify_address_type(&from);
		if(source_address_type != TESTIFY_AT_LOOPBACK && testify_service_is_localhost(&from))
			source_address_type = TESTIFY_AT_LOOPBACK;
		testify_obfuscate(discover->obfuscate_key, &data[1], size - 1, data[0]);
		command = data[1] & 3;
		switch(source_address_type)
		{
			case TESTIFY_AT_LOOPBACK :
				if(!(discover->method_flags & TESTIFY_DMF_BROADCAST_LOCAL))
					return;
				if(from.port == discover->local_handle->address.port)
					return;
				testify_address_lookup(&from, "localhost", from.transport, from.port);
			break;
			case TESTIFY_AT_PRIVATE_NETWORK :
				if(!(discover->method_flags & TESTIFY_DMF_BROADCAST_LAN))
					return;
			break;
			case TESTIFY_AT_GLOBAL :
				if(!(discover->method_flags & TESTIFY_DMF_BROADCAST_WAN))
					return;
			break;
		}
		if(command == TESTIFY_DBC_QUERY)
		{
			i = testify_discover_broadcast_pack(discover, send_buffer, TESTIFY_DBC_ANNOUNCE);
			testify_datagram_send(discover->local_handle, &from, send_buffer, i);
		}
		pos = 2;
		if((data[1] >> 2) & 1) /* relay, read the real source first */
		{
			if(source_address_type == TESTIFY_AT_LOOPBACK && !discover->local_have_port)
			{
				discover->local_port_count = (data[1] >> 3);
				for(j = 0; j < discover->local_port_count; j++)
					testify_pack_littleendian16(&discover->local_ports[j], &data[pos + j * sizeof(uint16)]); 
				i = testify_address_deserialize(&from, &data[pos], size - pos);
				if(i == 0)
					break;
				pos += i;
			}else
				return;
		}
		if(discover->local_have_port) /* relay to anyone localy */
		{
			send_buffer[0] = discover->obfuscate_salt++;
			send_buffer[1] = command | (1 << 2) | (discover->local_port_count << 3); /* set relay bit */
			j = 2 + testify_address_serialize(&from, &send_buffer[2]);			
			for(i = 0; i < discover->local_port_count; i++)
				testify_pack_littleendian16(&send_buffer[j + i * sizeof(uint16)], &discover->local_ports[i]); 
			j += discover->local_port_count * sizeof(uint16);

			if(j + size < 1500)
			{
				memcpy(&send_buffer[j], &data[2], size - 2);
				j += size - 2;
				testify_obfuscate(discover->obfuscate_key, &send_buffer[1], j - 1, send_buffer[0]);
				testify_discover_broadcast_send(discover, send_buffer, i, FALSE);
			}
			if(source_address_type == TESTIFY_AT_LOOPBACK)
			{
				for(j = 0; j < discover->local_port_count && discover->local_ports[j].port != from.port; j++);
				if(j == discover->local_port_count)
				{			
					if(discover->local_port_count < TESTIFY_LOCAL_PORT_COUNT_MAX)
					{
						discover->local_ports[discover->local_port_count].port = from.port;
						discover->local_ports[discover->local_port_count++].timer = 0;
					}
				}else if(command == TESTIFY_DBC_REMOVE)
					discover->local_ports[j] = discover->local_ports[--discover->local_port_count];
				else
					discover->local_ports[j].timer = 0;
			}
		}

		if(command == TESTIFY_DBC_REMOVE)
		{
			for(i = 0; i < discover->peer_count; i++)
				if(testify_address_compare(&discover->peers[i].address_local, &from) ||
					testify_address_compare(&discover->peers[i].address_global, &from))
					discover->peers[i].timeout = TESTIFY_DISCOVER_UDP_TIMEOUT * 2.0;
			continue;
		}
		timeout = 0;

		i = testify_address_deserialize(&address, &data[pos], size - pos);
		if(i == 0)
			break;
		if(address.transport == TESTIFY_TRANSPORT_NULL)
		{
			port = address.port;
			address = from;
			address.port = port;
			address_type = source_address_type;
		}else
		{
			address_type = testify_address_type(&address);
			if(address_type != TESTIFY_AT_LOOPBACK && testify_service_is_localhost(&address))
				address_type = TESTIFY_AT_LOOPBACK;
		}
		pos += i;
		message = &data[pos];
		for(i = pos; i < size && data[i] != 0; i++);
		if(i == size)
			break;
		pos = i + 1;
		if(TESTIFY_AT_GLOBAL == address_type && !(discover->method_flags & TESTIFY_DMF_BROADCAST_WAN))
		{
			address_type = source_address_type;
			address = from;
		}
		if(message[0] != 0)
		{
			if(TESTIFY_AT_LOOPBACK == address_type && (discover->method_flags & TESTIFY_DMF_BROADCAST_LOCAL))
			{
				peer = testify_discover_peer_update(discover, &address, NULL, message, TESTIFY_DMF_BROADCAST_LOCAL, timeout);
				peer->discover_port = from.port;
			}else if(TESTIFY_AT_PRIVATE_NETWORK == address_type && (discover->method_flags & TESTIFY_DMF_BROADCAST_LAN))
				peer = testify_discover_peer_update(discover, &address, NULL, message, TESTIFY_DMF_BROADCAST_LAN, timeout);
			else if(TESTIFY_AT_GLOBAL == address_type && (discover->method_flags & TESTIFY_DMF_BROADCAST_WAN))
				peer = testify_discover_peer_update(discover, NULL, &address, message, TESTIFY_DMF_BROADCAST_WAN, timeout);
			if(peer != NULL)
				peer->discover_port = from.port;
		}
		
		while(pos <= size)
		{	


			message = NULL;
			i = testify_address_deserialize(&address, &data[pos], size - pos);
			if(i == 0)
				break;
			pos += i;
			timeout = TESTIFY_DISCOVER_UDP_TIMEOUT;
		}
	}
}

