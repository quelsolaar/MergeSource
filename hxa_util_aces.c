#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

char *hxa_aces_type_names[] = {"box2i",
							"chlist",
							"chromaticities",
							"compression",
							"double",
							"float",
							"half",
							"int",
							"lineOrder",
							"keycode",
							"rational",
							"short",
							"string",
							"stringVector",
							"timecode",
							"unsignedChar",
							"unsignedInt",
							"unsignedLong",
							"unsignedShort",
							"v2f",
							"v3f"};
unsigned int hxa_aces_type_min_size[] = {16, /* HXA_ACES_ATTRIBUTE_TYPE_BOX2I */
									18, /* HXA_ACES_ATTRIBUTE_TYPE_CHLIST */
									4 * 8, /* HXA_ACES_ATTRIBUTE_TYPE_CHROMATICITIES */
									1, /* HXA_ACES_ATTRIBUTE_TYPE_COMPRESSION */
									8, /* HXA_ACES_ATTRIBUTE_TYPE_DOUBLE */
									4, /* HXA_ACES_ATTRIBUTE_TYPE_FLOAT */
									2, /* HXA_ACES_ATTRIBUTE_TYPE_HALF */
									4, /* HXA_ACES_ATTRIBUTE_TYPE_INT */
									1, /* HXA_ACES_ATTRIBUTE_TYPE_LINEORDER */
									4 * 7, /* HXA_ACES_ATTRIBUTE_TYPE_KEYCODE */
									4 * 2, /* HXA_ACES_ATTRIBUTE_TYPE_RATIONAL */
									2, /* HXA_ACES_ATTRIBUTE_TYPE_SHORT */
									2, /* HXA_ACES_ATTRIBUTE_TYPE_STRING */
									2, /* HXA_ACES_ATTRIBUTE_TYPE_STRINGVECTOR */
									4 * 4, /* HXA_ACES_ATTRIBUTE_TYPE_TIMECODE */
									1, /* HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDCHAR */
									4, /* HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDINT */
									8, /* HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDLONG */
									2, /* HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDSHORT */
									4 * 2, /* HXA_ACES_ATTRIBUTE_TYPE_V2F */
									4 * 3, /* HXA_ACES_ATTRIBUTE_TYPE_V3F */
									0}; /* HXA_ACES_ATTRIBUTE_TYPE_UNKNOWN */



typedef enum{
	HXA_ACES_ATTRIBUTE_TYPE_BOX2I,
	HXA_ACES_ATTRIBUTE_TYPE_CHLIST,
	HXA_ACES_ATTRIBUTE_TYPE_CHROMATICITIES,
	HXA_ACES_ATTRIBUTE_TYPE_COMPRESSION,
	HXA_ACES_ATTRIBUTE_TYPE_DOUBLE,
	HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	HXA_ACES_ATTRIBUTE_TYPE_HALF,
	HXA_ACES_ATTRIBUTE_TYPE_INT,
	HXA_ACES_ATTRIBUTE_TYPE_LINEORDER,
	HXA_ACES_ATTRIBUTE_TYPE_KEYCODE,
	HXA_ACES_ATTRIBUTE_TYPE_RATIONAL,
	HXA_ACES_ATTRIBUTE_TYPE_SHORT,
	HXA_ACES_ATTRIBUTE_TYPE_STRING,
	HXA_ACES_ATTRIBUTE_TYPE_STRINGVECTOR,
	HXA_ACES_ATTRIBUTE_TYPE_TIMECODE,
	HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDCHAR,
	HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDINT,
	HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDLONG,
	HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDSHORT,
	HXA_ACES_ATTRIBUTE_TYPE_V2F,
	HXA_ACES_ATTRIBUTE_TYPE_V3F,
	HXA_ACES_ATTRIBUTE_TYPE_COUNT,
	HXA_ACES_ATTRIBUTE_TYPE_UNKNOWN = HXA_ACES_ATTRIBUTE_TYPE_COUNT,
	HXA_ACES_ATTRIBUTE_TYPE_UNUSED
}HxAACESAttributeTypes;

typedef enum{
    HXA_ACES_ACPT_UINT32 = 0,
    HXA_ACES_ACPT_HALF16 = 1,
    HXA_ACES_ACPT_FLOAT32 = 2,
}HxAACESAttributeClistPixelType;

typedef struct{
	char name[256];
	HxAACESAttributeClistPixelType pixelType;
	unsigned int pLinear;
	int xSampling;
	int ySampling;
}HxAACESAttributeClist;

typedef struct{
	HxAACESAttributeTypes type;
	char name[256];
	union{
		struct{
			int x_min;
			int y_min;
			int x_max;
			int y_max;
		}box2i;
		struct{
			unsigned int length;
			HxAACESAttributeClist *list;
		}chlist;
		struct{
			float redX;
			float redY;
			float greenX;
			float greenY;
			float blueX;
			float blueY;
			float whiteX;
			float whiteY;
		}chromaticities;
		unsigned char compression;
		double type_double;
		float type_float;
		unsigned short type_half;
		unsigned int type_int;
		unsigned char lineOrder;
		struct{
			int filmMfCCode;
			int filmType;
			int prefix;
			int count;
			int perfOffset;
			int perfsPerFrame;
			int perfsPerCount;
		}keycode;
		struct{
			int n;
			unsigned int b;
		}rational;
		short type_short;
		struct{
			char *string;
			unsigned int length;
		}string;
		struct{
			char *string;
			unsigned int length;
		}stringVector;
		struct{
			unsigned int timeAndFlags;
			unsigned int userData;
		}timecode;
		unsigned char unsignedChar;
		unsigned int unsignedInt;
		unsigned long unsignedLong;
		unsigned short unsignedShort;
		float v2f[2];
		float v3f[3];
		struct{
			char *type_name;
			unsigned int length;
			void *data;
		}unknown;
	}data;
}HxAACESAttributeType;


typedef struct{
	char name[256];
	unsigned int channel_count;
	union{
		unsigned short *half_buffer;
		float *float_buffer;
	}pixels;
}HxAACESLayer;

typedef struct{
	HxAACESAttributeType *attributes;
	unsigned int attribute_count;
	unsigned int attribute_allocate;
	HxAACESLayer *layers;
	unsigned int layer_count;
}HxAACESImage;

typedef struct{
	char *name;
	HxAACESAttributeTypes type;
}HxAACESAttributePredefined;

typedef struct{
	unsigned int layer;
	unsigned int channel;
	HxAACESAttributeClistPixelType type;
}HxAChannelRead;

//"endOfHeaderOffset", HXA_ACES_ATTRIBUTE_TYPE_UNSIGNED_LONG
//"free", HXA_ACES_ATTRIBUTE_TYPE_STRING,
//"headerChecksum", HXA_ACES_ATTRIBUTE_TYPE_STRING,
//"imageChecksum", HXA_ACES_ATTRIBUTE_TYPE_STRING,

#define HXA_ACES_ATTRIBUTES_PREDEFINED 54
#define HXA_ACES_ATTRIBUTES_REQUIRED_OPENEXR 8
#define HXA_ACES_ATTRIBUTES_REQUIRED_ACES 11
#define HXA_ACES_ATTRIBUTES_IGNORE 4

HxAACESAttributePredefined hxa_aces_attribute_predefined[] = {
	/* OpenEXR Required attributes.*/
	"channels", HXA_ACES_ATTRIBUTE_TYPE_CHLIST,
	"compression", HXA_ACES_ATTRIBUTE_TYPE_COMPRESSION,
	"dataWindow", HXA_ACES_ATTRIBUTE_TYPE_BOX2I,
	"displayWindow", HXA_ACES_ATTRIBUTE_TYPE_BOX2I,
	"lineOrder", HXA_ACES_ATTRIBUTE_TYPE_LINEORDER,
	"pixelAspectRatio", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"screenWindowCenter", HXA_ACES_ATTRIBUTE_TYPE_V2F,
	"screenWindowWidth", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	/* ACES Required attributes.*/
	"acesImageContainerFlag", HXA_ACES_ATTRIBUTE_TYPE_INT,
	"adoptedNeutral", HXA_ACES_ATTRIBUTE_TYPE_V2F,
	"chromaticities", HXA_ACES_ATTRIBUTE_TYPE_CHROMATICITIES,
	/* ignore the following*/
	"endOfHeaderOffset", HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDLONG,
	"free", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"headerChecksum", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"imageChecksum", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	/* make sure the following has right name/type combo */
	"altitude", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"aperture", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"cameraFirmwareVersion", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"cameraIPAddress", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"cameraLabel", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"cameraMake", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"cameraModel", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"cameraOrientation", HXA_ACES_ATTRIBUTE_TYPE_V3F,
	"comments", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"convergenceDistance", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"creator", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"expTime", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"focalLength", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"focus", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"framesPerSecond", HXA_ACES_ATTRIBUTE_TYPE_RATIONAL,
	"imageCounter", HXA_ACES_ATTRIBUTE_TYPE_INT,
	"imageRotation", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"interocularDistance", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"isoSpeed", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"keyCode", HXA_ACES_ATTRIBUTE_TYPE_KEYCODE,
	"latitude", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"lensAttributes", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"lensMake", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"lensModel", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"lensSerialNumber", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"longitude", HXA_ACES_ATTRIBUTE_TYPE_FLOAT,
	"multiView", HXA_ACES_ATTRIBUTE_TYPE_STRINGVECTOR,
	"originalImageFlag", HXA_ACES_ATTRIBUTE_TYPE_INT, // Support
	"owner", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"recorderFirmwareVersion", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"recorderMake", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"recorderModel", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"recorderSerialNumber", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"reelName", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"storageMediaSerialNumber", HXA_ACES_ATTRIBUTE_TYPE_STRING,
	"timeCode", HXA_ACES_ATTRIBUTE_TYPE_TIMECODE,
	"timecodeRate", HXA_ACES_ATTRIBUTE_TYPE_INT, 
	"utcOffset", HXA_ACES_ATTRIBUTE_TYPE_FLOAT, 
	"uuid", HXA_ACES_ATTRIBUTE_TYPE_STRING};

#define hxa_aces_copy16(a, b) memcpy(a, b, 2)
#define hxa_aces_copy32(a, b) memcpy(a, b, 4)
#define hxa_aces_copy64(a, b) memcpy(a, b, 8)

void hxa_util_aces_header_attribute_free(HxAACESAttributeType *attrib_array, unsigned int attrib_count)
{
	unsigned int i;
	for(i = 0; i < attrib_count; i++)
	{
		switch(attrib_array[i].type)
		{
			case HXA_ACES_ATTRIBUTE_TYPE_CHLIST :
				free(attrib_array[i].data.chlist.list);
			break;
			case HXA_ACES_ATTRIBUTE_TYPE_STRING :
				free(attrib_array[i].data.string.string);
			break;
			case HXA_ACES_ATTRIBUTE_TYPE_STRINGVECTOR :
				free(attrib_array[i].data.stringVector.string);
			break;
			case HXA_ACES_ATTRIBUTE_TYPE_UNKNOWN :
				free(attrib_array[i].data.unknown.type_name);
			break;
		}
	}
	free(attrib_array);
}

int hxa_util_aces_header_parse(HxAACESAttributeType **attrib_array, unsigned int *attrib_count, unsigned int *attrib_allocated, char *type, char *name, int length, unsigned char *data, int silent)
{
	HxAACESAttributeTypes aces_type;
	unsigned int i, pos = 0, pre_define;
	HxAACESAttributeType *attribute;
	for(aces_type = 0; aces_type < HXA_ACES_ATTRIBUTE_TYPE_COUNT; aces_type++)
	{
		for(i = 0; hxa_aces_type_names[aces_type][i] == type[i] && type[i] != 0; i++);
		if(hxa_aces_type_names[aces_type][i] == type[i])
			break;
	}
	for(pre_define = 0; pre_define < HXA_ACES_ATTRIBUTES_PREDEFINED; pre_define++)
	{
		for(i = 0; hxa_aces_attribute_predefined[pre_define].name[i] == name[i] && name[i] != 0; i++);
		if(hxa_aces_attribute_predefined[pre_define].name[i] == name[i])
		{
			if(hxa_aces_attribute_predefined[pre_define].type != aces_type)
			{
				if(!silent)
					printf("HxA Error: Predefined attibute %s found to have wrong type: %s. Standard requires attribute %s to be of type %s\n", name, hxa_aces_type_names[aces_type], name, hxa_aces_type_names[hxa_aces_attribute_predefined[pre_define].type]);
				return FALSE;
			}
			break;
		}
	}
	if(pre_define >= HXA_ACES_ATTRIBUTES_REQUIRED_OPENEXR && pre_define < HXA_ACES_ATTRIBUTES_REQUIRED_OPENEXR + HXA_ACES_ATTRIBUTES_IGNORE)
		return TRUE;

	if(hxa_aces_type_min_size[aces_type] > length)
	{
		if(!silent)
			printf("HxA Error: Attibute %s of type %s only has %u bytes of storage but require at least %u bytes.\n", name, type, length, hxa_aces_type_min_size[aces_type]);
		return FALSE;
	}
	if(pre_define < HXA_ACES_ATTRIBUTES_REQUIRED_OPENEXR)
		attribute = &(*attrib_array)[pre_define];
	else if(*attrib_count < *attrib_allocated)
		attribute = &(*attrib_array)[(*attrib_count)++];
	else
	{
		*attrib_allocated += 16;
		*attrib_array = realloc(*attrib_array, (sizeof *attribute) * *attrib_allocated);			
		attribute = &(*attrib_array)[(*attrib_count)++];
	}

	for(i = 0; name[i] != 0; i++)
		attribute->name[i] = name[i];
	attribute->name[i] = 0;
	attribute->type = aces_type;
	switch(aces_type)
	{
		case HXA_ACES_ATTRIBUTE_TYPE_BOX2I :
			hxa_aces_copy32(&attribute->data.box2i.x_min, &data[0]);
			hxa_aces_copy32(&attribute->data.box2i.y_min, &data[4]);
			hxa_aces_copy32(&attribute->data.box2i.x_max, &data[8]);
			hxa_aces_copy32(&attribute->data.box2i.y_max, &data[12]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_CHLIST :
			attribute->data.chlist.length = 0;
			attribute->data.chlist.list = malloc((sizeof *attribute->data.chlist.list) * (length / (2 + 4 + 4 + 4 + 4)));
			while(data[pos] != 0)
			{
				for(i = 0; i < 255 && pos + 16 < length && data[pos] != 0; i++)
					attribute->data.chlist.list[attribute->data.chlist.length].name[i] = data[pos++];
				if(i >= 255 || pos + 16 + 1 > length)
				{
					if(!silent)
						printf("HxA Error: Attibute %s of type clist is not correctly null terminated\n", name);
					return FALSE;
				}
				pos++;
				attribute->data.chlist.list[attribute->data.chlist.length].name[i] = 0;
				hxa_aces_copy32(&attribute->data.chlist.list[attribute->data.chlist.length].pixelType, &data[pos]);
				hxa_aces_copy32(&attribute->data.chlist.list[attribute->data.chlist.length].pLinear, &data[pos + 4]);
				hxa_aces_copy32(&attribute->data.chlist.list[attribute->data.chlist.length].xSampling, &data[pos + 8]);
				hxa_aces_copy32(&attribute->data.chlist.list[attribute->data.chlist.length].ySampling, &data[pos + 12]);
				pos += 16;
				attribute->data.chlist.length++;
			}
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_CHROMATICITIES :
			hxa_aces_copy32(&attribute->data.chromaticities.redX, &data[0]);
			hxa_aces_copy32(&attribute->data.chromaticities.redY, &data[4]);
			hxa_aces_copy32(&attribute->data.chromaticities.greenX, &data[8]);
			hxa_aces_copy32(&attribute->data.chromaticities.greenY, &data[12]);
			hxa_aces_copy32(&attribute->data.chromaticities.blueX, &data[16]);
			hxa_aces_copy32(&attribute->data.chromaticities.blueY, &data[20]);
			hxa_aces_copy32(&attribute->data.chromaticities.whiteX, &data[24]);
			hxa_aces_copy32(&attribute->data.chromaticities.whiteY, &data[28]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_COMPRESSION :
			attribute->data.compression = data[0];
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_DOUBLE :
			hxa_aces_copy64(&attribute->data.type_double, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_FLOAT :
			hxa_aces_copy32(&attribute->data.type_float, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_HALF :
			hxa_aces_copy16(&attribute->data.box2i.x_min, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_INT :
			hxa_aces_copy32(&attribute->data.type_int, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_LINEORDER :
			attribute->data.lineOrder = data[0];
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_KEYCODE :
			hxa_aces_copy32(&attribute->data.keycode.filmMfCCode, &data[0]);
			hxa_aces_copy32(&attribute->data.keycode.filmType, &data[4]);
			hxa_aces_copy32(&attribute->data.keycode.prefix, &data[8]);
			hxa_aces_copy32(&attribute->data.keycode.count, &data[12]);
			hxa_aces_copy32(&attribute->data.keycode.perfOffset, &data[16]);
			hxa_aces_copy32(&attribute->data.keycode.perfsPerFrame, &data[20]);
			hxa_aces_copy32(&attribute->data.keycode.perfsPerCount, &data[24]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_RATIONAL :
			hxa_aces_copy32(&attribute->data.rational.n, &data[0]);
			hxa_aces_copy32(&attribute->data.rational.b, &data[4]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_SHORT :
			hxa_aces_copy16(&attribute->data.type_short, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_STRING :
			attribute->data.string.length = length;
			attribute->data.string.string = malloc(length + 1);
			memcpy(attribute->data.string.string, data, length);
			attribute->data.string.string[length] = 0;
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_STRINGVECTOR :
			attribute->data.string.length = length;
			attribute->data.string.string = malloc(length + 1);
			memcpy(attribute->data.string.string, data, length);
			attribute->data.string.string[length] = 0;
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_TIMECODE :
			hxa_aces_copy32(&attribute->data.timecode.timeAndFlags, &data[0]);
			hxa_aces_copy32(&attribute->data.timecode.userData, &data[4]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDCHAR :
			attribute->data.unsignedChar = data[0];
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDINT :
			hxa_aces_copy32(&attribute->data.unsignedInt, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDLONG :
			hxa_aces_copy64(&attribute->data.unsignedLong, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_UNSIGNEDSHORT :
			hxa_aces_copy16(&attribute->data.unsignedShort, &data[0]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_V2F :
			hxa_aces_copy32(&attribute->data.v2f[0], &data[0]);
			hxa_aces_copy32(&attribute->data.v2f[1], &data[4]);
		return TRUE;
		case HXA_ACES_ATTRIBUTE_TYPE_V3F :
			hxa_aces_copy32(&attribute->data.v3f[0], &data[0]);
			hxa_aces_copy32(&attribute->data.v3f[1], &data[4]);
			hxa_aces_copy32(&attribute->data.v3f[2], &data[8]);
		return TRUE;
		case  HXA_ACES_ATTRIBUTE_TYPE_UNKNOWN :
			for(i = 0; type[i] != 0; i++);
			attribute->data.unknown.type_name = malloc(++i + length);
			memcpy(attribute->data.unknown.type_name, type, i);
			attribute->data.unknown.data = &attribute->data.unknown.type_name[i];
			attribute->data.unknown.length = length;
			memcpy(attribute->data.unknown.data, data, length);
		return TRUE;
	}
	return FALSE;
}

char *hxa_util_aces_file_load(char *file_name, size_t *size)
{
	char *buffer;
	unsigned int allocation, i;
	FILE *f;
	f = fopen(file_name, "rb");
	if(f == NULL)
	{
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	allocation = ftell(f);
	rewind(f);
	buffer = malloc(allocation + 1);
	memset(buffer, 0, allocation + 1);
	fread(buffer, 1, allocation, f);
	i = ftell(f);
	fclose(f);
	buffer[allocation] = 0;
	if(size != NULL)
		*size = allocation;
	return buffer;
}


float hxa_util_aces_float16_to_float32(unsigned short value)
{
	union { int integer; float real; }convert;
	int sign, exponent, sig;
	sign = (value >> 15) & 0x00000001;
	exponent = (value >> 10) & 0x0000001f;
	sig = value & 0x000003ff;

	if (exponent == 0)
	{
		if (sig == 0)
		{
			convert.integer = sign << 31;
			return convert.real;
		}
		else
		{
			while (!(sig & 0x00000400))
			{
				sig <<= 1;
				exponent -= 1;
			}
			exponent += 1;
			sig &= ~0x00000400;
		}
	}
	else if (exponent == 31)
	{
		if (sig == 0)
		{
			convert.integer = (sign << 31) | 0x7f800000;
			return convert.real;
		}
		else
		{
			convert.integer = (sign << 31) | 0x7f800000 | (sig << 13);
			return convert.real;
		}
	}

	exponent = exponent + (127 - 15);
	sig = sig << 13;

	convert.integer = (sign << 31) | (exponent << 23) | sig;
	return convert.real;
}

int hxa_util_aces_scanline(HxAACESImage *image, unsigned char *data, size_t size, unsigned int x, unsigned int y, HxAChannelRead *channel_reads, int silent, int convert)
{
	unsigned int i, j, pos = 0, line, line_size, stride;
	unsigned short *uint16_read, *uint16_write;
	float *float32_read, *float32_write;
	if(size < 8)
	{
		if(!silent)
			printf("HxA Error: .\n");
		return FALSE;			
	}
	hxa_aces_copy32(&line, &data[pos]);
	hxa_aces_copy32(&line_size, &data[pos + 4]);
	pos += 8;
	for(i = 0; i < image->attributes[0].data.chlist.length; i++)
	{
	/*	channel_reads[i].layer;
		channel_reads[i].channel;
		channel_reads[i].type;*/
		stride = image->layers[channel_reads[i].layer].channel_count;
		float32_write = &image->layers[channel_reads[i].layer].pixels.float_buffer[stride * line * x + channel_reads[i].channel];
		uint16_read = &data[pos];
		for(j = 0; j < x; j++)
		{
			*float32_write = hxa_util_aces_float16_to_float32(*uint16_read++);
			float32_write += stride;
		}
		pos += x * sizeof(short);
	}	
	return TRUE;		
}

void hxa_util_aces_free(HxAACESImage *image)
{
	unsigned int i;
	hxa_util_aces_header_attribute_free(image->attributes, image->attribute_count);
	for(i = 0; i < image->layer_count; i++)
		if(image->layers[i].pixels.float_buffer != NULL)
			free(image->layers[i].pixels.float_buffer);
	free(image->layers);
	free(image);
}



void hxa_util_aces_channel_list(HxAACESImage *file, HxAACESAttributeClist *list, unsigned int list_length, HxAChannelRead *reads, unsigned int silent)
{
	char channel_names[] = {'R', 'G', 'B', 'A', 'X', 'Y', 'Z'};
	int period;
	unsigned int i, j, k, channel;
	file->layers = malloc((sizeof *file->layers) * list_length);
	file->layer_count = 0;	 
	for(i = 0; i < list_length; i++)
	{
		period = FALSE;
		channel = 0;
		for(j = 0; list[i].name[j] != 0; j++)	
			if(list[i].name[j] == '.')
				period = TRUE;
		if(j == 1 || period)
		{
			for(k = 0; k < 7; k++)
			{
				if(list[i].name[j - 1] == channel_names[k])
				{
					period = TRUE;
					list[i].name[j - 1] = 0;
					channel = k % 4;
					break;
				}
			}
		}
		if(period)
		{
			for(j = 0; j < file->layer_count; j++)
			{
				for(k = 0; list[i].name[k] != 0 && list[i].name[k] == file->layers[j].name[k]; k++);
				if(list[i].name[k] == file->layers[j].name[k])
					break;
			}
		}else
			j = file->layer_count;
		if(j == file->layer_count)
		{
			file->layer_count++;
			for(k = 0; k < list[i].name[k]; k++)
				file->layers[j].name[k] = list[i].name[k];
			file->layers[j].name[k] = 0;
			file->layers[j].channel_count = channel + 1;
		}
		else
			if(channel + 1 > file->layers[j].channel_count) 
				file->layers[j].channel_count = channel + 1;

		reads[i].layer = j;
		reads[i].channel = channel;
		reads[i].type = list[i].pixelType;
	}
	for(i = 0; i < file->layer_count; i++)
	{
		file->layers[i].pixels.float_buffer = malloc(sizeof(float) * file->layers[j].channel_count * (unsigned int)(file->attributes[2].data.box2i.x_max + 1 - file->attributes[2].data.box2i.x_min) * (unsigned int)(file->attributes[2].data.box2i.y_max + 1 - file->attributes[2].data.box2i.y_min));
	}
}


HxAACESImage *hxa_util_aces_load(char *file_path, int silent)
{
	unsigned char *data;
	size_t size, pos;
	unsigned long long offset;
	unsigned int i, magic_number = 0, version = 0, x, y;
	HxAChannelRead *channel_reads, channel_reads_buffer[64];
	HxAACESAttributeType *attrib_array;
	HxAACESImage *image;
	unsigned int attribute_count, attrib_allocated = HXA_ACES_ATTRIBUTES_PREDEFINED;
	attribute_count = HXA_ACES_ATTRIBUTES_REQUIRED_ACES;
	data = hxa_util_aces_file_load(file_path, &size);
	if(data == NULL)
	{
		if(!silent)
			printf("HxA Error: File %s not found.\n", file_path);
		return FALSE;
	}
	if(size < 4)
	{
		if(!silent)
			printf("HxA Error: Unexpected end of file found.\n");
		return FALSE;
	}
	hxa_aces_copy32(&magic_number, data);
	if(magic_number != 20000630)
	{
		if(!silent)
			printf("HxA Error: Filw is not a valid OpenEXR/ACES file (wrong magic number).\n");
		return FALSE;
	}
	pos = 4;
	hxa_aces_copy32(&version, &data[pos]);

/*	if(magic_number != 20000630)
	{
		if(!silent)
			printf("HxA Error: Filw is not a valid OpenEXR/ACES file (wrong magic number).\n");
		return FALSE;
	}*/
	pos += 4;
	attrib_array = malloc((sizeof *attrib_array) * attrib_allocated);
	for(i = 0; i < HXA_ACES_ATTRIBUTES_REQUIRED_ACES; i++)
		attrib_array[i].type = HXA_ACES_ATTRIBUTE_TYPE_UNUSED;
	while(data[pos] != 0)
	{
		char name[256], type[256];
		int length;
		for(i = 0; data[pos] != 0; i++)
		{
			if(i == 255)
			{
				name[255] = 0; 
				if(!silent)
					printf("HxA Error: File attribute <%s> is not termnated\n", name);
				hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
				return NULL;
			}
			name[i] = data[pos++];
		}
		name[i] = 0;
		pos++;
		for(i = 0; data[pos] != 0; i++)
		{
			if(i == 255)
			{
				type[255] = 0; 
				if(!silent)
					printf("HxA Error: File attribute %s has a non termnated type <%s>\n", name, type);
				hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
				return NULL;
			}
			type[i] = data[pos++];
		}
		type[i] = 0;
		pos++;
		memcpy(&length, &data[pos], 4);
		if(length < 0 || length > 1024 * 1024 + 4 - pos)
		{
			if(!silent)
				printf("HxA Error: Unexpected end of file at attribute %s with length <%u>\n", name, length);
			hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
			return NULL;
		}
		pos += 4;
		if(!hxa_util_aces_header_parse(&attrib_array, &attribute_count, &attrib_allocated, type, name, length, &data[pos], silent))
		{
			hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
			return NULL;
		}	
		printf("name %s type %s\n", name, type);
		pos += length;
	}
	pos++;
	if(attrib_array[0].type != HXA_ACES_ATTRIBUTE_TYPE_CHLIST)
	{
		if(!silent)
			printf("HxA Error: no chanel list found in file.\n");
		hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
		return NULL;
	}
	if(attrib_array[1].type != HXA_ACES_ATTRIBUTE_TYPE_COMPRESSION)
	{
		if(!silent)
			printf("HxA Error: no compression attribute found in file.\n");
		hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
		return NULL;
	}
	if(attrib_array[1].data.compression != 0)
	{
		if(!silent)
			printf("HxA Error: The compression (%u) in this file is not supported by ACES. ACES Only supports uncompressed files (0).\n", (unsigned int)attrib_array[1].data.compression);
		hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
		return NULL;
	}
	if(attrib_array[2].type != HXA_ACES_ATTRIBUTE_TYPE_BOX2I)
	{
		if(!silent)
			printf("HxA Error: no data window found in file.\n");
		hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
		return NULL;
	}
	if(attrib_array[2].data.box2i.x_max < attrib_array[2].data.box2i.x_min || attrib_array[2].data.box2i.y_max < attrib_array[2].data.box2i.y_min)
	{
		if(!silent)
			printf("HxA Error: File has negative image size.\n");
		hxa_util_aces_header_attribute_free(attrib_array, attribute_count);
		return NULL;
	}
	image = malloc(sizeof *image);
	image->attributes = attrib_array;
	image->attribute_allocate = attrib_allocated;
	image->attribute_count = attribute_count;
	x = (unsigned int)(attrib_array[2].data.box2i.x_max + 1 - attrib_array[2].data.box2i.x_min);
	y = (unsigned int)(attrib_array[2].data.box2i.y_max + 1 - attrib_array[2].data.box2i.y_min);
	if(attrib_array[0].data.chlist.length <= 64)
		channel_reads = channel_reads_buffer;
	else
		channel_reads = malloc((sizeof *channel_reads) * attrib_array[0].data.chlist.length);
	hxa_util_aces_channel_list(image, attrib_array[0].data.chlist.list, attrib_array[0].data.chlist.length, channel_reads, silent);

	if(pos + y * 8 > size)
	{
		if(!silent)
			printf("HxA Error: Unexpected end of file in scanline table.\n");
		hxa_util_aces_free(image);
		return NULL;
	}

	for(i = 0; i < y; i++)
	{
		{
			unsigned char values[8];
			int j;
			hxa_aces_copy64(&values, &data[pos + i * 8]);
			printf("offset %u [%u + %u * 8]", pos + i * 8, pos, i);
			for(j = 0; j < 8; j++)
				printf(" %u", (unsigned int)values[j]);
			printf("\n");
		}

		hxa_aces_copy64(&offset, &data[pos + i * 8]);
		if(size < offset)
		{
			if(!silent)
				printf("HxA Error: Scanline table points to a position outside file\n");
			hxa_util_aces_free(image);
			return NULL;
		}
		if(!hxa_util_aces_scanline(image, &data[offset], size - offset, x, y, channel_reads, silent, FALSE))
		{
			if(!silent)
				printf("HxA Error: File has negative image size.\n");
			hxa_util_aces_free(image);
			return NULL;
		}
	}
	if(channel_reads != channel_reads_buffer)
		free(channel_reads);
	return image;
}


void *hxa_util_aces_load_test(char *file_path, int silent, unsigned int *x, unsigned int *y)
{
	HxAACESImage *image;
	image = hxa_util_aces_load(file_path, TRUE);	
	*x = (unsigned int)(image->attributes[2].data.box2i.x_max + 1 - image->attributes[2].data.box2i.x_min);
	*y = (unsigned int)(image->attributes[2].data.box2i.y_max + 1 - image->attributes[2].data.box2i.y_min);
	return image->layers[0].pixels.float_buffer;
}