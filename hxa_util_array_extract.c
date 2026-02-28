#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "hxa.h"
#include "hxa_utils.h"
#include "forge.h"

#define FALSE 0
#define TRUE !FALSE

typedef signed char hxa_int8;
typedef unsigned short hxa_uint16;
typedef signed short hxa_int16;

HXALayer *hxa_layer_find_by_name(HXALayerStack *stack, char *name)
{
	unsigned int i, j;
	for(i = 0; i < stack->layer_count; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == stack->layers[i].name[j]; j++);
		if(name[j] == stack->layers[i].name[j])
			return &stack->layers[i];
	}
	return NULL;	
}

HXALayer *hxa_layer_find_by_name_and_type(HXALayerStack *stack, char *name, HXALayerDataType type)
{
	unsigned int i, j;
	for(i = 0; i < stack->layer_count; i++)
	{
		if(stack->layers[i].type == type)
		{
			for(j = 0; name[j] != 0 && name[j] == stack->layers[i].name[j]; j++);
			if(name[j] == stack->layers[i].name[j])
				return &stack->layers[i];
		}
	}
	return NULL;	
}


extern short hxa_float32_to_float16(float value);

int hxa_vertex_array_convert_corner(HXALayer *layer, HxATypeConvert output_type, hxa_uint8 *output, size_t length, size_t stride, unsigned int component)
{
	size_t i, read_stride;
	hxa_int8 *read8;
	hxa_int8 i8;
	hxa_uint8 ui8;
	hxa_int16 i16;
	hxa_uint16 ui16;
	hxa_int32 i32, *read32;
	hxa_uint32 ui32;
	float f32, *readf32;
	double f64, *readf64;


	if(component >= layer->components)
		return FALSE;

	read_stride = layer->components;
	switch(layer->type)
	{
		case HXA_LDT_UINT8 :
			read8 = &layer->data.uint8_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)read8[i * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)read8[i * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)read8[i * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)read8[i * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)read8[i * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)read8[i * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)read8[i * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)read8[i * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)read8[i * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
		case HXA_LDT_INT32 :
			read32 = &layer->data.int32_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)read32[i * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)read32[i * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)read32[i * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)read32[i * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)read32[i * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)read32[i * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)read32[i * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)read32[i * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)read32[i * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
		case HXA_LDT_FLOAT :
			readf32 = &layer->data.float_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)readf32[i * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)readf32[i * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)readf32[i * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)readf32[i * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)readf32[i * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)readf32[i * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)readf32[i * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)readf32[i * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)readf32[i * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
		case HXA_LDT_DOUBLE :
			readf64 = &layer->data.double_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)readf64[i * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)readf64[i * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)readf64[i * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)readf64[i * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)readf64[i * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)readf64[i * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)readf64[i * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)readf64[i * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)readf64[i * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
	}
	return TRUE;
}

int hxa_vertex_array_convert_vertex(HXALayer *layer, hxa_int32 *reference, HxATypeConvert output_type, hxa_uint8 *output, size_t length, size_t stride, unsigned int component)
{
	size_t i, read_stride;
	hxa_uint8 *read8;
	hxa_int8 i8;
	hxa_uint8 ui8;
	hxa_int16 i16;
	hxa_uint16 ui16;
	hxa_int32 i32, *read32, r;
	hxa_uint32 ui32;
	float f32, *readf32;
	double f64, *readf64;


	if(component >= layer->components)
		return FALSE;
	read_stride = layer->components;
	switch(layer->type)
	{
		case HXA_LDT_UINT8 :
			read8 = &layer->data.uint8_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{				
						r = reference[i];
						if(r < 0)
							r = -r - 1;
						i8 = (hxa_int8)read8[r * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{							
						r = reference[i];
						if(r < 0)
							r = -r - 1;			
						ui8 = (hxa_uint8)read8[r * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{									
						r = reference[i];
						if(r < 0)
							r = -r - 1;	
						i16 = (hxa_int16)read8[r * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{									
						r = reference[i];
						if(r < 0)
							r = -r - 1;	
						ui16 = (hxa_uint16)read8[r * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{								
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i32 = (hxa_int32)read8[r * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{				
						r = reference[i];
						if(r < 0)
							r = -r - 1;						
						ui32 = (hxa_uint32)read8[r * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						r = reference[i];
						if(r < 0)
							r = -r - 1;				
						ui16 = hxa_float32_to_float16((float)read8[r * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{				
						r = reference[i];
						if(r < 0)
							r = -r - 1;						
						f32 = (float)read8[r * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{					
						r = reference[i];
						if(r < 0)
							r = -r - 1;			
						f64 = (double)read8[r * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
		case HXA_LDT_INT32 :
			read32 = &layer->data.int32_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;								
						i8 = (hxa_int8)read32[r * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;								
						ui8 = (hxa_uint8)read32[r * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;					
						i16 = (hxa_int16)read32[r * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;			
						ui16 = (hxa_uint16)read32[r * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i32 = (hxa_int32)read32[r * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui32 = (hxa_uint32)read32[r * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui16 = hxa_float32_to_float16((float)read32[r * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						f32 = (float)read32[r * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						f64 = (double)read32[r * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
		case HXA_LDT_FLOAT :
			readf32 = &layer->data.float_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i8 = (hxa_int8)readf32[r * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui8 = (hxa_uint8)readf32[r * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i16 = (hxa_int16)readf32[r * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui16 = (hxa_uint16)readf32[r * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i32 = (hxa_int32)readf32[r * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui32 = (hxa_uint32)readf32[r * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui16 = hxa_float32_to_float16((float)readf32[r * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						f32 = (float)readf32[r * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						f64 = (double)readf32[r * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
		case HXA_LDT_DOUBLE :
			readf64 = &layer->data.double_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i8 = (hxa_int8)readf64[r * read_stride];
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui8 = (hxa_uint8)readf64[r * read_stride];
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i16 = (hxa_int16)readf64[r * read_stride];
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui16 = (hxa_uint16)readf64[r * read_stride];
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						i32 = (hxa_int32)readf64[r * read_stride];
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui32 = (hxa_uint32)readf64[r * read_stride];
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						ui16 = hxa_float32_to_float16((float)readf64[r * read_stride]);
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						f32 = (float)readf64[r * read_stride];
						memcpy(&output[i * stride], &f32, sizeof(float));
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{
						r = reference[i];
						if(r < 0)
							r = -r - 1;		
						f64 = (double)readf64[r * read_stride];
						memcpy(&output[i * stride], &f64, sizeof(double));
					}
				break;
			}
		break;
	}
	return TRUE;
}



int hxa_vertex_array_convert_face(HXALayer *layer, hxa_int32 *reference, HxATypeConvert output_type, hxa_uint8 *output, size_t length, size_t stride, unsigned int component)
{
	size_t i, j, read_stride;
	hxa_uint8 *read8;
	hxa_int8 i8;
	hxa_uint8 ui8;
	hxa_int16 i16;
	hxa_uint16 ui16;
	hxa_int32 i32, *read32;
	hxa_uint32 ui32;
	float f32, *readf32;
	double f64, *readf64;

	if(component >= layer->components)
		return FALSE;
	j = 0;
	read_stride = layer->components;
	switch(layer->type)
	{
		case HXA_LDT_UINT8 :
			read8 = &layer->data.uint8_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)read8[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)read8[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						j += read_stride;
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)read8[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)read8[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						j += read_stride;
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)read8[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)read8[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)read8[j]);
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint32));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint32));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)read8[j * read_stride];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f32, sizeof(float));
						memcpy(&output[i * stride], &f32, sizeof(float));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)read8[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f64, sizeof(double));
						memcpy(&output[i * stride], &f64, sizeof(double));
						j += read_stride;
					}
				break;
			}
		break;
		case HXA_LDT_INT32 :
			read32 = &layer->data.int32_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)read32[j];
						for(; reference[i] >=0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)read32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						j += read_stride;
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)read32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)read32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						j += read_stride;
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)read32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)read32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)read32[j]);
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)read32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f32, sizeof(float));
						memcpy(&output[i * stride], &f32, sizeof(float));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)read32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f64, sizeof(double));
						memcpy(&output[i * stride], &f64, sizeof(double));
						j += read_stride;
					}
				break;
			}
		break;
		case HXA_LDT_FLOAT :
			readf32 = &layer->data.float_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)readf32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)readf32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						j += read_stride;
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)readf32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)readf32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						j += read_stride;
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)readf32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)readf32[i * read_stride];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)readf32[j]);
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)readf32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f32, sizeof(float));
						memcpy(&output[i * stride], &f32, sizeof(float));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)readf32[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f64, sizeof(double));
						memcpy(&output[i * stride], &f64, sizeof(double));
						j += read_stride;
					}
				break;
			}
		break;
		case HXA_LDT_DOUBLE :
			readf64 = &layer->data.double_data[component];
			switch(output_type)
			{
				case HXA_TC_INT8 :
					for(i = 0; i < length; i++)
					{						
						i8 = (hxa_int8)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						memcpy(&output[i * stride], &i8, sizeof(hxa_int8));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT8 :
					for(i = 0; i < length; i++)
					{						
						ui8 = (hxa_uint8)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						memcpy(&output[i * stride], &ui8, sizeof(hxa_uint8));
						j += read_stride;
					}
				break;
				case HXA_TC_INT16 :
					for(i = 0; i < length; i++)
					{						
						i16 = (hxa_int16)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						memcpy(&output[i * stride], &i16, sizeof(hxa_int16));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = (hxa_uint16)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						j += read_stride;
					}
				break;
				case HXA_TC_INT32 :
					for(i = 0; i < length; i++)
					{						
						i32 = (hxa_int32)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						memcpy(&output[i * stride], &i32, sizeof(hxa_int32));
						j += read_stride;
					}
				break;
				case HXA_TC_UINT32 :
					for(i = 0; i < length; i++)
					{						
						ui32 = (hxa_uint32)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						memcpy(&output[i * stride], &ui32, sizeof(hxa_uint32));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT16 :
					for(i = 0; i < length; i++)
					{						
						ui16 = hxa_float32_to_float16((float)readf64[j]);
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						memcpy(&output[i * stride], &ui16, sizeof(hxa_uint16));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT32 :
					for(i = 0; i < length; i++)
					{						
						f32 = (float)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f32, sizeof(float));
						memcpy(&output[i * stride], &f32, sizeof(float));
						j += read_stride;
					}
				break;
				case HXA_TC_FLOAT64 :
					for(i = 0; i < length; i++)
					{						
						f64 = (double)readf64[j];
						for(; reference[i] >= 0 && i + 1 < length; i++)
							memcpy(&output[i * stride], &f64, sizeof(double));
						memcpy(&output[i * stride], &f64, sizeof(double));
						j += read_stride;
					}
				break;
			}
		break;
	}
	f_debug_mem_check_bounds();
}



void *hxa_util_array_extract(HXANode *node, size_t vertex_stride, size_t *vertex_param_offsets, unsigned int *vertex_param_types, char **vertex_param_names, hxa_uint8 *vertex_component, void ** defaults, unsigned int param_count)
{
	HXALayer *layer;
	hxa_uint8 *output;
	size_t offset = 0;
	unsigned int param, *reference;
	output = malloc(node->content.geometry.edge_corner_count * vertex_stride);
	reference = node->content.geometry.corner_stack.layers->data.int32_data;
	for(param = 0; param < param_count; param++)
	{
		if(vertex_param_offsets != NULL)
			offset = vertex_param_offsets[param];
f_debug_mem_check_bounds();
		if((layer = hxa_layer_find_by_name(&node->content.geometry.corner_stack, vertex_param_names[param])) == NULL ||
			!hxa_vertex_array_convert_corner(layer, vertex_param_types[param], &output[offset], node->content.geometry.edge_corner_count, vertex_stride, vertex_component[param]))
		{
f_debug_mem_check_bounds();
			if((layer = hxa_layer_find_by_name(&node->content.geometry.vertex_stack, vertex_param_names[param])) == NULL ||
				!hxa_vertex_array_convert_vertex(layer, reference, vertex_param_types[param], &output[offset], node->content.geometry.edge_corner_count, vertex_stride, vertex_component[param]))
			{
f_debug_mem_check_bounds();
				if((layer = hxa_layer_find_by_name(&node->content.geometry.face_stack, vertex_param_names[param])) == NULL ||
					!hxa_vertex_array_convert_face(layer, reference, vertex_param_types[param], &output[offset], node->content.geometry.edge_corner_count, vertex_stride, vertex_component[param]))
				{

				}
				f_debug_mem_check_bounds();
			}
		}
	}
	return output;
}