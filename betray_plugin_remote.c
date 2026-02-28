

#include "betray_plugin_api.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "testify.h"


typedef enum{
	B_RAC_PING,
	B_RAC_DISCONECT,
	B_RAC_BUTTON,
	B_RAC_AXIS,
	B_RAC_POINTER
}BetrayRemoteCommand;

typedef enum{
	B_RAT_BUTTON,
	B_RAT_POINTER,
	B_RAT_AXIS
}BetrayRemoteAllocationType;

typedef struct{
	BetrayRemoteAllocationType type;
	char name[64];
	uint axis;
	uint user_id;
	uint device_id;
}BetrayRemoteAllocation;

typedef struct{
	UnravelAddress address;
	 BetrayRemoteAllocation *allocations;
	 uint allocation_count;
	 float timer;
}BetrayRemoteConnection;

BetrayRemoteConnection *betray_remote_connection;
uint betray_remote_connection_count;
THandle *betray_remote_connection_handle = NULL;

uint controller_plugin_connection_find(TestifyNetworkAddress *address)
{
	uint i;
	for(i = 0; i < betray_remote_connection_count; i++)
	{
		if(betray_remote_connection[i].address == *address)
		{
			betray_remote_connection[0].timer = 0;
			return i;
		}
	}
	if(betray_remote_connection_count % 16 == 0)
		betray_remote_connection = realloc(betray_remote_connection, (sizeof *betray_remote_connection) * (betray_remote_connection_count + 16));

	betray_remote_connection[betray_remote_connection_count].address = *address;
	betray_remote_connection[betray_remote_connection_count].allocations = NULL;
	betray_remote_connection[betray_remote_connection_count].allocation_count = 0;
	betray_remote_connection[betray_remote_connection_count].timer = 0;
	return betray_remote_connection_count++;
}

void controller_plugin_callback_main(BInputState *input)
{
	TestifyNetworkAddress *from;
	uint size, id, user_id, device_id, allocation_id;

	while((size = testify_network_receive(betray_remote_connection_handle, &from)) != 0)
	{
		if(size > 0)
		{
			switch(testify_unpack_uint8(betray_remote_connection_handle, "command"))
			{
				case B_RAC_PING :
					testify_pack_string(betray_remote_connection_handle, "Pong", "Reply");
					testify_network_datagram_send(betray_remote_connection_handle, &from); 
				break;
				case B_RAC_DISCONECT :
					id = controller_plugin_connection_find(&from);
					betray_remote_connection[id].timer = 10000000;
				break;
				case B_RAC_BUTTON :
					id = controller_plugin_connection_find(&from);
					user_id = testify_unpack_uint32(betray_remote_connection_handle, "user_id");
					device_id = testify_unpack_uint32(betray_remote_connection_handle, "device_id");
					allocation_id = testify_unpack_uint32(betray_remote_connection_handle, "allocation_id");

					testify_network_datagram_send(betray_remote_connection_handle, &from); 
				break;
				case B_RAC_AXIS :
				break;
				case B_RAC_POINTER :
				break;
			}
		}
	}
	for(i = 0; i <  )
	{
					if(betray_remote_connection[id].allocations != NULL)
						free(betray_remote_connection[id].allocations);
					betray_remote_connection[id] = betray_remote_connection[--betray_remote_connection_count];	
	
	}
}

void betray_plugin_init(void)
{
	betray_remote_connection_handle = testify_network_datagram_create(2242);
	if(betray_remote_connection_handle != NULL)
		betray_plugin_callback_set_main(controller_plugin_callback_main);
}