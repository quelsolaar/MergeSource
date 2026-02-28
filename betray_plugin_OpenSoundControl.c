
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "betray_plugin_api.h"
#include "testify.h"

typedef enum{
	BETRAY_OSCIT_BUTTON,
	BETRAY_OSCIT_AXIS_1D,
	BETRAY_OSCIT_AXIS_2D,
	BETRAY_OSCIT_AXIS_3D,
	BETRAY_OSCIT_POINTER_BUTTON_COUNT_ZERO,
	BETRAY_OSCIT_COUNT
}BPOSCInputType;

typedef struct{
	char name[32];
	uint betray_id;
	BPOSCInputType type;
}BPOSCInput;

typedef struct{
	char device_name[32];
	uint32 ip;
	uint16 port;
	BPOSCInput *input;
	uint input_count;
	uint user_id;
	uint device_id;
}BPOSCDevice;


uint betray_plugin_osc_port_setting;
uint16 betray_plugin_osc_network_port = 0;
THandle *betray_plugin_osc_network_handle = NULL;

BPOSCDevice *betray_plugin_osc_devices = NULL; 
uint betray_plugin_osc_devices_count = 0; 



BPOSCInput *betray_osc_input_get(BPOSCDevice *device, char *string, BPOSCInputType type)
{
	BPOSCInput *input;
	uint i, j;
	input = device->input;
	for(i = 0; i < device->input_count; i++)
	{
		if(input->type == type)
		{
			for(j = 0; string[j] == input->name[j] && input->name[j] != 0; j++);
			if(input->name[j] == 0 && (string[j] == 0 || string[j] == '/' || j == 32 - 1))
				return input;
		}
		input++;
	}
	if(device->input_count % 16 == 0)
		device->input = realloc(device->input, (sizeof *device->input) * (device->input_count + 16));
	input = &device->input[device->input_count++];
	for(i = 0; i < 32 - 1 && string[i] != 0; i++)
		input->name[i] = string[i];
	input->name[i] = 0;
	input->type = type;
	input->betray_id;
	switch(type)
	{
		case BETRAY_OSCIT_BUTTON :
			input->betray_id = betray_plugin_button_allocate(-1, string);
		break;
		case BETRAY_OSCIT_AXIS_1D :
			input->betray_id = betray_plugin_axis_allocate(device->user_id, device->device_id, string, B_AXIS_UNDEFINED, 1);
		break;
		case BETRAY_OSCIT_AXIS_2D :
			input->betray_id = betray_plugin_axis_allocate(device->user_id, device->device_id, string, B_AXIS_UNDEFINED, 1);
		break;
		case BETRAY_OSCIT_AXIS_3D :
			input->betray_id = betray_plugin_axis_allocate(device->user_id, device->device_id, string, B_AXIS_UNDEFINED, 1);
		break;
		default :
			input->betray_id = betray_plugin_pointer_allocate(device->user_id, device->device_id, type - BETRAY_OSCIT_POINTER_BUTTON_COUNT_ZERO, 0, 0, -1, NULL, "OSC Pointer", TRUE);
		break;
	}
	return input;
}

BPOSCInput *betray_osc_devive_input_get(uint32 ip, uint16 port, char *device_string, char *id_string, char *name, BPOSCInputType type)
{
	BPOSCDevice *devices;
	char *osc = "OSC ", buffer[32 + 4];
	uint i, j;
	for(i = 0; i < betray_plugin_osc_devices_count; i++)
	{
		if(betray_plugin_osc_devices[i].ip == ip && betray_plugin_osc_devices[i].port == port)
		{
			for(j = 0; device_string[j] == betray_plugin_osc_devices[i].device_name[j] && betray_plugin_osc_devices[i].device_name[j] != 0; j++);
			if(betray_plugin_osc_devices[i].device_name[j] == device_string[j])
			{	
				if(string[j] != 0 && string[j + 1] != 0)
					name = &string[j + 1];
				return betray_osc_input_get(&betray_plugin_osc_devices[i], name, type);
			}
		}
	}
	if(betray_plugin_osc_devices_count % 16 == 0)
		betray_plugin_osc_devices = realloc(betray_plugin_osc_devices, (sizeof *betray_plugin_osc_devices) * (betray_plugin_osc_devices_count + 16));
	devices = &betray_plugin_osc_devices[betray_plugin_osc_devices_count++];

	devices->device_name[32];
	devices->ip = ip;
	devices->port = port;
	devices->input = NULL;
	devices->input_count = 0;
	for(i = 0; i < betray_plugin_osc_devices_count - 1 && betray_plugin_osc_devices[i].ip != ip; i++);
	if(i < betray_plugin_osc_devices_count - 1)
		devices->user_id = betray_plugin_osc_devices[i].user_id;
	else
		devices->user_id = betray_plugin_user_allocate();
	for(i = 0; i < 4; i++)
		buffer[i] = osc[i];
	for(i = 0; i < 31 && device_string[i] != 0; i++)
		buffer[i + 4] = device_string[i];
	buffer[i + 4] = 0;
	devices->device_id = betray_plugin_input_device_allocate(devices->user_id, buffer);
	return betray_osc_input_get(devices, name, type);
}


BPOSCInput *betray_osc_parse(uint32 ip, uint16 port, char *string, uint length)
{
	char address[32] = {0}, *types, text[64];
	BPOSCInputType type;
	uint i, j, float_count = 0, true_false_count = 0;
	float vector[16];
	boolean true_false[8];
	for(i = 0; i < length;)
	{
		if(string[i] == '/')
		{		
			for(j = 0; string[i + j + 1] != 0 && j < 32 - 1 && i + j + 1 < length; j++);
				address[j] = string[i + j + 1];
			for(; string[i] != 0; i++);
			i = ((i + 3) / 4) * 4;
		}else if(string[i] == ',')
		{	
			types = &string[++i];
			for(;string[i] != 0 ;i++)
			i = ((i + 3) / 4) * 4;
			for(j = 0; types[j] != 0; j++)
			{
				switch(types[j])
				{
					case 'i' :
					{
						if(i + 4 <= length)
							break;
						i += 4;
					}
					break;
					case 'f' :
					{
						union{uint8 integer[4]; float real;}merge;
						if(i + 4 <= length)
							break;
						merge.integer[0] = string[i++]; 
						merge.integer[1] = string[i++]; 
						merge.integer[2] = string[i++]; 
						merge.integer[3] = string[i++];					
						if(float_count < 16)
							vector[float_count] = merge.real;
						float_count++;
					}
					break;
					case 's' :
					{
						if(text[0] == 0)
						{
							for(j = 0; string[i + j] != 0 && j < 64 - 1; j++)
								text[j] = string[i + j];
							text[j] = 0;
						}else
							j = 0;
						for(; string[i + j] != 0; j++);
						i += j;
						i = ((i + 3) / 4) * 4;
					}
					break;
					case 'b' :
					{
						union{uint8 integer; uint8 bytes[4];}merge;
						if(i + 4 <= length)
							break;
						merge.bytes[0] = string[i++]; 
						merge.bytes[1] = string[i++]; 
						merge.bytes[2] = string[i++]; 
						merge.bytes[3] = string[i++];
						i += ((merge.integer + 3) / 4) * 4;
					}
					break;
					default :
						while(types[j] != 0)
							j++;
						j--;
					break;
				}
			}
			if(float_count == 2 && true_false_count > 0)
			{
				betray_osc_devive_input_get(ip, port, char *string, char *name, BETRAY_OSCIT_POINTER_BUTTON_COUNT_ZERO)
			}
		}else
			i++;
	}
}

void controller_plugin_callback_main(BInputState *input)
{
	static FILE *file = NULL;
	char buffer[1500];
	if(file == NULL)
	file = fopen("OSC_Debug_output.txt", "w");

	if(betray_plugin_osc_network_port != betray_settings_number_int_get(betray_plugin_osc_port_setting))
	{
		betray_plugin_osc_network_port = betray_settings_number_int_get(betray_plugin_osc_port_setting);
		if(betray_plugin_osc_network_handle != NULL)
			testify_free(betray_plugin_osc_network_handle);
		betray_plugin_osc_network_handle = testify_network_datagram_create(betray_plugin_osc_network_port);
	}
	
	if(betray_plugin_osc_network_handle != NULL)
	{
		TestifyNetworkAddress from;
		uint i, size;
		if((size = testify_network_receive(betray_plugin_osc_network_handle, &from)) != 0)
		{
			char buffer[32];
			testify_network_debug_address_print(&from, buffer);
			fprintf(file, "Got message containing %u bytes from %s\n", size, buffer);
			for(i = 0; i < 1500 && i < size; i++)
				buffer[i] = testify_unpack_uint8(betray_plugin_osc_network_handle, "bytes");
			f_fprint_raw(file, buffer, size);
		
			//	BPOSCInput *betray_osc_devive_input_get(uint32 ip, uint16 port, char *string, BPOSCInputType type)

		/*	if(betray_plugin_osc_button_id == -1)
				betray_plugin_osc_button_id = betray_plugin_button_allocate(-1, "Our OSC BUTTON!");

			betray_plugin_button_set(betray_plugin_osc_user_id, betray_plugin_osc_button_id, BETRAY_BUTTON_PLAY_PAUSE, -1);
			exit(0);*/
		}
	}
}

void betray_plugin_init(void)
{
	betray_plugin_callback_set_main(controller_plugin_callback_main);
	betray_plugin_osc_port_setting = betray_settings_create(BETRAY_ST_NUMBER_INT, "OSC network port", 0, NULL); 
	betray_settings_number_int_set(betray_plugin_osc_port_setting, 8888);
}