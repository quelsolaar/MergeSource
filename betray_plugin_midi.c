


#include "forge.h"
#include "betray_plugin_api.h"
#include <windows.h>
#include <stdio.h>

typedef struct{
		float value;
		boolean updated;
		uint betray_axis_id;
		char name[64];
}BetrayPluginMidiSlider;

typedef struct{
		boolean events[8];
		uint event_count;
		uint betray_button_id;
		char name[64];
}BetrayPluginMidiButton;

typedef struct{
	BetrayPluginMidiSlider sliders[256];
	BetrayPluginMidiButton buttons[256];
	char device_name[64];
	uint user_id;
}MidiDevice;

MidiDevice *betray_plugin_midi_devices;
uint betray_plugin_midi_device_count;
void *betray_plugin_midi_mutex = NULL;



void betray_plugin_midi_file_name(char *buffer, MidiDevice *d)
{
	char device_name[64];
	uint i;
	for(i = 0; i < 63 && d->device_name[i] != 0; i++)
	{
		if(d->device_name[i] <= ' ')
			device_name[i] = '_';
		else
			device_name[i] = d->device_name[i];
	}
	device_name[i] = 0;
	sprintf(buffer, "betray_midi_%s_settings.txt", device_name);
}

void betray_plugin_midi_save()
{
	MidiDevice *d;
	char file_name[128];
	FILE *f;
	uint i, j;
	for(i = 0; i < betray_plugin_midi_device_count; i++)
	{
		d = &betray_plugin_midi_devices[i];
		betray_plugin_midi_file_name(file_name, d);
		f = fopen(file_name, "w");
		if(f != NULL)
		{
			for(j = 0; j < 256; j++)
				if(d->buttons[j].betray_button_id != -1)
					fprintf(f, "Button_%u : %s\n", j, d->buttons[j].name);
			for(j = 0; j < 256; j++)
				if(d->sliders[j].betray_axis_id != -1)
					fprintf(f, "Slider_%u : %s\n", j, d->sliders[j].name);
			fclose(f);
		}
	}
}


void betray_plugin_midi_load(MidiDevice *d)
{
	char file_name[128], *buffer, *button = "Button_", *slider = "Slider_";
	uint i, j, id;

	betray_plugin_midi_file_name(file_name, d);
	buffer = f_text_load(file_name, NULL);
	if(buffer == NULL)
		return;
	for(i = 0; buffer[i] != 0; i++)
	{
		if(buffer[i] == 'B')
		{
			for(j = 0; buffer[i + j] == button[j] && buffer[j] != 0; j++);
			if(button[j] == 0)
			{
				id = 0;
				for(i += j; buffer[i] >= '0' && buffer[i] <= '9'; i++)
				{
					id *= 10;
					id += buffer[i] - '0';
				}
				if(id < 256)
				{
					for(;(buffer[i] <= ' ' || buffer[i] == ':') && buffer[i] != '\n' && buffer[i] != 0; i++);
					if(buffer[i] == 0)
					{						
						free(buffer);
						return;
					}
					if(buffer[i] != '\n')
					{
						for(j = 0; j < 63 && buffer[i + j] != '\n' && buffer[i + j] != 0; j++)
							d->buttons[id].name[j] = buffer[i + j];
						d->buttons[id].name[j] = 0;
						i += j - 1;
						d->buttons[id].betray_button_id = betray_plugin_button_allocate(-1, d->buttons[id].name);
					}
				}
			}
		}
		if(buffer[i] == 'S')
		{
			for(j = 0; buffer[i + j] == slider[j] && slider[j] != 0; j++);
			if(slider[j] == 0)
			{
				id = 0;
				for(i += j; buffer[i] >= '0' && buffer[i] <= '9'; i++)
				{
					id *= 10;
					id += buffer[i] - '0';
				}
				
				if(id < 256)
				{
					for(;(buffer[i] <= ' ' || buffer[i] == ':') && buffer[i] != '\n' && buffer[i] != 0; i++);
					if(buffer[i] == 0)
					{						
						free(buffer);
						return;
					}
					
					if(buffer[i] != '\n')
					{
						for(j = 0; j < 63 && buffer[i + j] != '\n' && buffer[i + j] != 0; j++)
							d->sliders[id].name[j] = buffer[i + j];
						d->sliders[id].name[j] = 0;
						i += j - 1;
						d->sliders[id].betray_axis_id = betray_plugin_axis_allocate(d->user_id, d->device_id, d->sliders[id].name, B_AXIS_SLIDER, 1);		
					}
				}
			}
		}
	}
	free(buffer);
}




void CALLBACK betray_plugin_midi_callback(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	MidiDevice *device;
	imagine_mutex_lock(betray_plugin_midi_mutex); 
	device = &betray_plugin_midi_devices[dwInstance];
	switch(wMsg) {
	case MIM_OPEN:
		printf("wMsg=MIM_OPEN\n");
		break;
	case MIM_CLOSE:
		printf("wMsg=MIM_CLOSE\n");
		break;
	case MIM_DATA:
		{
			float value;
			uint id;
			id = (dwParam1 >> 8) & 0xff;
			if((uint32)(dwParam1 & 0xff) == (uint32)0x90)
			{

				printf("Button down %u\n", (dwParam1 >> 8) & 0xff);
				if(device->buttons[id].event_count < 8)
					device->buttons[id].events[device->buttons[id].event_count++] = TRUE;
			}
			if((uint32)(dwParam1 & 0xff) == (uint32)0x80) // button up
			{
				printf("Button up %u\n", (dwParam1 >> 8) & 0xff);
				if(device->buttons[id].event_count < 8)
					device->buttons[id].events[device->buttons[id].event_count++] = FALSE;
			}
			if((uint32)(dwParam1 & 0xff) == (uint32)0xb0) // axis
			{
				device->sliders[id].updated = TRUE;
				device->sliders[id].value = (float)(dwParam1 & 0x7f0000) / (float)(0x7f0000);
				printf("Axis %u %f\n", (dwParam1 >> 8) & 0xff, device->sliders[id].value);
			}
		}
		break;
	case MIM_LONGDATA:
		printf("wMsg=MIM_LONGDATA\n"); 
		break;
	case MIM_ERROR:
		printf("wMsg=MIM_ERROR\n");
		break;
	case MIM_LONGERROR:
		printf("wMsg=MIM_LONGERROR\n");
		break;
	case MIM_MOREDATA:
		printf("wMsg=MIM_MOREDATA\n");
		break;
	default:
		printf("wMsg = unknown\n");
		break;
	}
	imagine_mutex_unlock(betray_plugin_midi_mutex); 
}

void midi_plugin_callback_main(BInputState *input)
{
	MidiDevice *device;
	uint i, j, k;	
	imagine_mutex_lock(betray_plugin_midi_mutex); 
	for(i = 0; i < betray_plugin_midi_device_count; i++)
	{
		device = &betray_plugin_midi_devices[i];
		for(j = 0; j < 256; j++)
		{
			if(device->buttons[j].event_count != 0)
			{
				if(device->buttons[j].betray_button_id == -1)
				{
					device->buttons[j].betray_button_id = betray_plugin_button_allocate(-1, device->buttons[j].name);
					betray_plugin_midi_save();
				}
				for(k = 0; k < device->buttons[j].event_count; k++)
					betray_plugin_button_set(device->user_id, device->buttons[j].betray_button_id, device->buttons[j].events[k], -1);
				device->buttons[j].event_count = 0;
			}
			if(device->sliders[j].updated)
			{
				if(device->sliders[j].betray_axis_id == -1)
				{
					device->sliders[j].betray_axis_id = betray_plugin_axis_allocate(device->user_id, device->device_id, device->sliders[j].name, B_AXIS_SLIDER, 1);
					betray_plugin_midi_save();
				}		
				betray_plugin_axis_set(device->sliders[j].betray_axis_id, device->sliders[j].value, 0.0, 0.0);
				device->sliders[j].updated = FALSE;
			}
		}
	}
	imagine_mutex_unlock(betray_plugin_midi_mutex); 
}

void betray_plugin_init(void)
{
	MIDIINCAPSW lpMidiInCaps;
	HMIDIIN hMidiDevice = NULL;
	DWORD nMidiPort = 0;
	uint count, i, j, k, l;
	MMRESULT rv;
	char buffer[64];

	betray_plugin_callback_set_main(midi_plugin_callback_main);

	betray_plugin_midi_mutex = imagine_mutex_create();
	imagine_mutex_lock(betray_plugin_midi_mutex); 
	count = midiInGetNumDevs();
	betray_plugin_midi_devices = malloc((sizeof *betray_plugin_midi_devices) * count);
	betray_plugin_midi_device_count = 0;
	for(i = 0; i < count; i++)
	{
		midiInGetDevCaps(i, &lpMidiInCaps, sizeof lpMidiInCaps);		
		for(j = 0;  j < 63 && lpMidiInCaps.szPname[j] != 0; j++)
			betray_plugin_midi_devices[betray_plugin_midi_device_count].device_name[j] = lpMidiInCaps.szPname[j];
		betray_plugin_midi_devices[betray_plugin_midi_device_count].device_name[j] = 0;
		betray_plugin_midi_devices[betray_plugin_midi_device_count].user_id = betray_plugin_user_allocate();
		betray_plugin_midi_devices[betray_plugin_midi_device_count].device_id = betray_plugin_input_device_allocate(betray_plugin_midi_devices[betray_plugin_midi_device_count].user_id, lpMidiInCaps.szPname);

		for(j = 0; j < 256; j++)
		{
			sprintf(buffer, " Button %u", j);
			for(k = 0; betray_plugin_midi_devices[betray_plugin_midi_device_count].device_name[k] != 0; k++)
				betray_plugin_midi_devices[betray_plugin_midi_device_count].buttons[j].name[k] = betray_plugin_midi_devices[betray_plugin_midi_device_count].device_name[k];
			for(l = 0; k + l < 63 && buffer[l] != 0; l++)
				betray_plugin_midi_devices[betray_plugin_midi_device_count].buttons[j].name[k + l] = buffer[l];
			betray_plugin_midi_devices[betray_plugin_midi_device_count].buttons[j].name[k + l] = 0;
			betray_plugin_midi_devices[betray_plugin_midi_device_count].buttons[j].betray_button_id = -1;
			betray_plugin_midi_devices[betray_plugin_midi_device_count].buttons[j].event_count = 0;

			sprintf(buffer, " Slider %u", j);
			for(k = 0; betray_plugin_midi_devices[betray_plugin_midi_device_count].device_name[k] != 0; k++)
				betray_plugin_midi_devices[betray_plugin_midi_device_count].sliders[j].name[k] = betray_plugin_midi_devices[betray_plugin_midi_device_count].device_name[k];
			for(l = 0; k + l < 63 && buffer[l] != 0; l++)
				betray_plugin_midi_devices[betray_plugin_midi_device_count].sliders[j].name[k + l] = buffer[l];
			betray_plugin_midi_devices[betray_plugin_midi_device_count].sliders[j].name[k + l] = 0;
			betray_plugin_midi_devices[betray_plugin_midi_device_count].sliders[j].betray_axis_id = -1;
			betray_plugin_midi_devices[betray_plugin_midi_device_count].sliders[j].updated = FALSE;
		}
		betray_plugin_midi_load(&betray_plugin_midi_devices[betray_plugin_midi_device_count]);
		rv = midiInOpen(&hMidiDevice, nMidiPort, (DWORD)(void*)betray_plugin_midi_callback, betray_plugin_midi_device_count, CALLBACK_FUNCTION);
		if(rv == MMSYSERR_NOERROR)
		{
			midiInStart(hMidiDevice);
			betray_plugin_midi_device_count++;
		}
	}
	imagine_mutex_unlock(betray_plugin_midi_mutex); 
}

/*
int  text_usb()
{
    unsigned int count, i;
    MIDIINCAPS inputCapabilities;

	count = midiInGetNumDevs();
	printf("count %u\n", count);
    for(i = 0; i < count; i++)
	{
       midiInGetDevCaps(i, &inputCapabilities, sizeof(inputCapabilities));
       printf("Hardware: %s\n", inputCapabilities.szPname);
	   inputCapabilities.
    }
	i = 0;
}
/*
int text_usb()
{
	HDEVINFO                         hDevInfo;
	SP_DEVICE_INTERFACE_DATA         DevIntfData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA DevIntfDetailData;
	SP_DEVINFO_DATA                  DevData;
	HANDLE								handle; 

	DWORD dwSize, dwType, dwMemberIdx;
	HKEY hKey;
	BYTE lpData[1024];
	// We will try to get device information set for all USB devices that have a
	// device interface and are currently present on the system (plugged in).
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	
	if (hDevInfo != INVALID_HANDLE_VALUE)
	{
		int i, j;
		// Prepare to enumerate all device interfaces for the device information
		// set that we retrieved with SetupDiGetClassDevs(..)
		DevIntfData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		dwMemberIdx = 0;
		
		// Next, we will keep calling this SetupDiEnumDeviceInterfaces(..) until this
		// function causes GetLastError() to return  ERROR_NO_MORE_ITEMS. With each
		// call the dwMemberIdx value needs to be incremented to retrieve the next
		// device interface information.

		
		
	//	while(GetLastError() != ERROR_NO_MORE_ITEMS)
		for(i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_USB_DEVICE, i, &DevIntfData); i++)
		{
			void *h = NULL;
			printf("loop\n");

			DevData.cbSize = sizeof(DevData);
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &DevIntfData, NULL, 0, &dwSize, NULL);
			DevIntfDetailData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
			DevIntfDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if(SetupDiGetDeviceInterfaceDetail(hDevInfo, &DevIntfData, DevIntfDetailData, dwSize, &dwSize, &DevData))
			{
				{
					char buffer[256];
					int i;
					for(i = 0; i < 256 && DevIntfDetailData->DevicePath[i] != 0; i++)
						buffer[i] = DevIntfDetailData->DevicePath[i];
					buffer[i] = 0;
					printf("Device %s\n", buffer);
				}


	
				handle = CreateFile(DevIntfDetailData->DevicePath,
								GENERIC_WRITE | GENERIC_READ, 
								FILE_SHARE_WRITE | FILE_SHARE_READ,
								NULL, OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 
								NULL);
				printf("handle %p\n", handle);
				if(handle == NULL)
					return;
				printf("A Error.\n GetLastError=%x\n", GetLastError());
		//		ERROR_INVALID_HANDLE
				if((j = WinUsb_Initialize(handle, &h)))
				{
					printf("WORKING!\n");

				}
				CloseHandle(handle);
				printf("B Error.\n GetLastError=%x\n", GetLastError());
			}
			HeapFree(GetProcessHeap(), 0, DevIntfDetailData);

			// Continue looping
		//	SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_USB_DEVICE, ++dwMemberIdx, &DevIntfData);
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);
	}
	exit(0);
	return 0;
}*/