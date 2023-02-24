#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

typedef enum {
	HXA_PNG_CT_GRAYSCALE = 0,
	HXA_PNG_CT_RGB = 2,
	HXA_PNG_CT_INDEXED = 3,
	HXA_PNG_CT_GRAYSCALE_AND_ALPHA = 4,
	HXA_PNG_CT_RGB_AND_ALPHA = 6
}HxAPNGChannelTypes;

/*
A palette is a 3 byte per entry list at the length of chunk.
*/

extern int hxa_inflate(unsigned char* output, size_t* output_length, unsigned char* input, size_t input_length);

void hxa_load_unpack_uint8(unsigned char* input_buffer, unsigned char* output_buffer, unsigned int x, unsigned int y, unsigned int channels)
{
	unsigned int i, j, k, read = 0, write = 0, acces_a, acces_b, acces_c;
	unsigned char tmp[4];
	if (input_buffer[read] == 2)
		input_buffer[read] = 0; // Up @ 1st Row == None
	if (input_buffer[read] == 3)
		input_buffer[read] = 5; // Average @ 1st Row == Unique Case
	if (input_buffer[read] == 4)
		input_buffer[read] = 1; // Paeth @ 1st Row == Add
	for (i = 0; i < y; i++)
	{
		switch (input_buffer[read++])
		{
		case 0: // None
			memcpy(&output_buffer[write], &input_buffer[read], x * channels);
			write += channels * x;
			read += channels * x;
			break;
		case 1: // Add
			acces_a = write;
			for (j = 0; j < channels; j++)
				output_buffer[write++] = input_buffer[read++];
			for (; j < x * channels; j++)
				output_buffer[write++] = input_buffer[read++] + output_buffer[acces_a++];
			break;
		case 2: // Up
			acces_b = write - x * channels;
			for (j = 0; j < x * channels; j++)
				output_buffer[write++] = input_buffer[read++] + output_buffer[acces_b++];
			break;
		case 3: // Average
			acces_a = write;
			acces_b = write - x * channels;
			for (j = 0; j < channels; j++)
				output_buffer[write++] = input_buffer[read++] + output_buffer[acces_b++] / 2;
			for (; j < x * channels; j++)
				output_buffer[write++] = input_buffer[read++] + (output_buffer[acces_a++] + output_buffer[acces_b++]) / 2;
			break;
		case 5: // Average @ 1st Row
			acces_a = write;
			for (j = 0; j < channels; j++)
				output_buffer[write++] = input_buffer[read++];
			for (; j < x * channels; j++)
				output_buffer[write++] = input_buffer[read++] + output_buffer[acces_a++] / 2;
			break;
		case 4: // Paeth
			acces_b = write;
			acces_a = write - x * channels;
			acces_c = write - x * channels;
			for (j = 0; j < channels; j++)
				output_buffer[write++] = input_buffer[read++] + output_buffer[acces_a++];
			for (; j < x * channels; j++)
			{
				int p, a, b, c, ap, bp, cp;
				a = output_buffer[acces_a++];
				b = output_buffer[acces_b++];
				c = output_buffer[acces_c++];
				p = a + b - c;
				ap = abs(p - a);
				bp = abs(p - b);
				cp = abs(p - c);
				if (ap <= bp && ap <= cp)
					p = a;
				else if (bp <= cp)
					p = b;
				else
					p = c;
				output_buffer[write++] = input_buffer[read++] + p;
			}
			break;
		default:
			printf("Error");
			break;
		}
	}
}


unsigned int hxa_load_unpack_interlaced_uint8(unsigned char* input_buffer, unsigned char* output_buffer, unsigned int size_x, unsigned int size_y, unsigned int offset_x, unsigned int offset_y, unsigned int stride_x, unsigned int stride_y, unsigned int channels)
{
	unsigned int i, j, k, read = 0, write = 0, acces_a, acces_b, acces_c;
	unsigned char tmp[4];
	if (input_buffer[read] >= 2)
		input_buffer[read] = 0;
	if (size_x <= offset_x || size_y <= offset_y)
		return;
	for (i = offset_y; i < size_y; i += stride_y)
	{
		write = (i * size_x + offset_x) * channels;
		switch (input_buffer[read++])
		{
		case 0:
			for (j = offset_x; j < size_x; j += stride_x)
			{
				for (k = 0; k < channels; k++)
					output_buffer[write + k] = input_buffer[read++];
				write += channels * stride_x;
			}
			break;
		case 1:
			acces_a = write;
			for (j = 0; j < channels; j++)
				output_buffer[write + j] = input_buffer[read++];
			write += stride_x * channels;
			for (j = stride_x + offset_x; j < size_x; j += stride_x)
			{
				for (k = 0; k < channels; k++)
					output_buffer[write + k] = input_buffer[read++] + output_buffer[acces_a + k];
				write += stride_x * channels;
				acces_a += channels * stride_x;
			}
			break;
		case 2:
			acces_b = write - size_x * channels * stride_y;
			for (j = offset_x; j < size_x; j += stride_x)
			{
				for (k = 0; k < channels; k++)
					output_buffer[write + k] = input_buffer[read++] + output_buffer[acces_b + k];
				write += stride_x * channels;
				acces_b += channels * stride_x;
			}
			break;
		case 3:
			acces_a = write;
			acces_b = write - size_x * channels * stride_y;
			for (j = 0; j < channels; j++)
				output_buffer[write + k] = input_buffer[read++] + output_buffer[acces_b + j] / 2;
			acces_b += channels * stride_x;
			write += channels * stride_x;
			for (j = stride_x + offset_x; j < size_x; j += stride_x)
			{
				for (k = 0; k < channels; k++)
					output_buffer[write + k] = input_buffer[read++] + (output_buffer[acces_a + k] + output_buffer[acces_b + k]) / 2;
				write += channels * stride_x;
				acces_a += channels * stride_x;
				acces_b += channels * stride_x;
			}
			break;
		case 4:
			acces_b = write;
			acces_a = write - size_x * channels * stride_y;
			acces_c = write - size_x * channels * stride_y;
			for (k = 0; k < channels; k++)
				output_buffer[write + k] = input_buffer[read++] + output_buffer[acces_a + k];
			acces_a += channels * stride_x;
			write += channels * stride_x;
			for (j = stride_x + offset_x; j < size_x; j += stride_x)
			{
				for (k = 0; k < channels; k++)
				{
					int p, a, b, c, ap, bp, cp;
					a = output_buffer[acces_a + k];
					b = output_buffer[acces_b + k];
					c = output_buffer[acces_c + k];
					p = a + b - c;
					ap = abs(p - a);
					bp = abs(p - b);
					cp = abs(p - c);
					if (ap <= bp && ap <= cp)
						p = a;
					else if (bp <= cp)
						p = b;
					else
						p = c;
					output_buffer[write + k] = input_buffer[read++] + p;
				}
				write += channels * stride_x;
				acces_a += channels * stride_x;
				acces_b += channels * stride_x;
				acces_c += channels * stride_x;
			}
			break;
		}
	}
	return read;
}

extern void hxa_png_crc_compute(unsigned char* output, unsigned char* read_buffer, int length);

const unsigned int MOD_ADLER = 65521;

unsigned int adler32(unsigned char* data, size_t len)
{
	unsigned int a = 1, b = 0;
	size_t index;

	// Process each byte of the data in order
	for (index = 0; index < len; ++index)
	{
		a = (a + data[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}

	return (b << 16) | a;
}



char* hxa_load_png_file(char* file_name, size_t* size)
{
	char* buffer;
	unsigned int allocation, i;
	FILE* f;
	f = fopen(file_name, "rb");
	if (f == NULL)
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
	if (size != NULL)
		*size = allocation;
	return buffer;
}


int hxa_load_png(HXAFile* file, char* file_name)
{
	char* name = HXA_CONVENTION_SOFT_LAYER_COLOR;
	unsigned int channels[] = { 1, 0, 3, 3, 2, 0, 4 };
	unsigned int x, y, i, j, meta_data_used = 0, packed_size = 0, debug_alloc;
	unsigned char bits, compression, filter, interlace, * meta_data;
	union { unsigned char bytes[4]; unsigned int integet; }crc;
	HxAPNGChannelTypes color;
	HXANode* node;
	union { char text[5]; unsigned int type; }type;
	size_t size, pos, chunk_length, bitmap_size;
	unsigned char* data, * unprocessed_data, * image_data;
	data = hxa_load_png_file(file_name, &size);
	if (data == NULL)
	{
		//	printf("HxA Error: Could not open file %s\n", file_name);
		return FALSE;
	}
	//	printf("%s\n", file_name);
	//	f_print_raw(data, 50);
	type.text[4] = 0;
	chunk_length = data[11] + data[10] * 256 + data[9] * 256 * 256 + data[8] * 256 * 256 * 256;
	x = data[19] + data[18] * 256 + data[17] * 256 * 256 + data[16] * 256 * 256 * 256;
	y = data[23] + data[22] * 256 + data[21] * 256 * 256 + data[20] * 256 * 256 * 256;
	bits = data[24];
	color = data[25];
	compression = data[26]; // Must be 0 Deflate
	filter = data[27]; // Must be 0
	interlace = data[28]; // 0, regular 1 adam7 8 * 8

	hxa_png_crc_compute(crc.bytes, &data[12], chunk_length + 4);
	/*		if(crc.bytes[0] != data[12 + chunk_length + 4] ||
				crc.bytes[1] != data[12 + chunk_length + 5] ||
				crc.bytes[2] != data[12 + chunk_length + 6] ||
				crc.bytes[3] != data[12 + chunk_length + 7])
				i += 0;*/


	meta_data = malloc(size);
	bitmap_size = x * y * channels[color] * bits / 8;
	unprocessed_data = malloc(bitmap_size + y);
	for (i = 0; i < bitmap_size + y; i++)
		unprocessed_data[i] = 0;
	for (pos = 8 + 8 + chunk_length + 4; pos + 8 < size; pos += chunk_length + 12)
	{
		chunk_length = data[pos + 3] + data[pos + 2] * 256 + data[pos + 1] * 256 * 256 + data[pos] * 256 * 256 * 256;
		memcpy(&type.type, &data[pos + 4], 4);
		hxa_png_crc_compute(crc.bytes, &data[pos + 4], chunk_length + 4);
		if (crc.bytes[0] != data[pos + chunk_length + 8] ||
			crc.bytes[1] != data[pos + chunk_length + 9] ||
			crc.bytes[2] != data[pos + chunk_length + 10] ||
			crc.bytes[3] != data[pos + chunk_length + 11])
			i += 0;

		//	printf("chunk_length %u - %s %u\n", chunk_length, type.text, type.type);

		/*	{
				printf("Adler file %s:\n", type.text);
				f_print_raw(&data[pos + 4 + chunk_length], 12);
				printf("CRC:\n");
				hxa_png_crc_compute(crc.bytes, &data[pos + 4], chunk_length + 4);
				printf("CRC type included:\n");
				f_print_raw(crc.bytes, 4);
			}*/
		if (type.type == 1413563465)
		{
			//		printf("header:\n");
				//	exit(0);
			for (i = 0; i < chunk_length; i++)
				data[packed_size++] = data[pos + 8 + i];
			/*		printf("File CRC:\n");
					f_print_raw(&data[pos + 0 + i], 8);
					printf("\n");
					f_print_raw(&data[pos + 8 + i], 4);
					printf("\n");
					f_print_raw(&data[pos + 12 + i], 16);
					printf("\n");*/
					/*		printf("nibbles %u %u\n", data[0] % 16, data[0] / 16);
							printf("nibbles %u %u %u\n", data[1] % 16, (data[1] / 16) % 2, (data[1] / 32) % 8);
							hxa_inflate(image_data, bitmap_size, &i, data, chunk_length);
					f_print_raw(data, size);
							exit(0);*/
		}
		else
		{
			//	printf("header: %u\n", chunk_length);
			//	f_print_raw(data, 64);
			memcpy(&meta_data[meta_data_used], &data[pos], chunk_length + 12);
			meta_data_used += chunk_length + 12;
		}
	}
	meta_data = realloc(meta_data, meta_data_used);
	//	f_print_raw(data, packed_size);
	hxa_inflate(unprocessed_data, &bitmap_size, &data[2], packed_size - 6);
	//	exit(0);
	/*	{
		//	unsigned char crc[4];
			union{unsigned char bytes[4]; unsigned int integet;}crc;
			printf("Unprocessed:\n");
			f_print_raw(unprocessed_data, bitmap_size);
			crc.integet = adler32(unprocessed_data, bitmap_size);
		//	hxa_png_crc_compute(crc, &unprocessed_data, bitmap_size);
			printf("Adler:\n");
			f_print_raw(crc.bytes, 4);
			hxa_png_crc_compute(crc.bytes, data, packed_size);
			printf("CRC:\n");
			f_print_raw(crc.bytes, 4);
			i = 0;
		}*/

		//	f_print_raw(unprocessed_data, 400);
	image_data = malloc(x * y * channels[color]);
	for (i = 0; i < x * y * channels[color]; i++)
		image_data[i] = 0;
	if (interlace == 1)
	{
		i = 0;
		i += hxa_load_unpack_interlaced_uint8(&unprocessed_data[i], image_data, x, y, 0, 0, 8, 8, channels[color]);
		i += hxa_load_unpack_interlaced_uint8(&unprocessed_data[i], image_data, x, y, 4, 0, 8, 8, channels[color]);
		i += hxa_load_unpack_interlaced_uint8(&unprocessed_data[i], image_data, x, y, 0, 4, 4, 8, channels[color]);
		i += hxa_load_unpack_interlaced_uint8(&unprocessed_data[i], image_data, x, y, 2, 0, 4, 4, channels[color]);
		i += hxa_load_unpack_interlaced_uint8(&unprocessed_data[i], image_data, x, y, 0, 2, 2, 4, channels[color]);
		i += hxa_load_unpack_interlaced_uint8(&unprocessed_data[i], image_data, x, y, 1, 0, 2, 2, channels[color]);
		i += hxa_load_unpack_interlaced_uint8(&unprocessed_data[i], image_data, x, y, 0, 1, 1, 2, channels[color]);
	}
	else
		hxa_load_unpack_uint8(unprocessed_data, image_data, x, y, channels[color]);
	free(data);
	free(unprocessed_data);
	file->node_count++;
	file->node_array = realloc(file->node_array, (sizeof * file->node_array) * file->node_count);
	node = &file->node_array[file->node_count - 1];
	node->type = HXA_NT_IMAGE;
	node->content.image.type = HXA_IT_2D_IMAGE;
	node->content.image.resolution[0] = x;
	node->content.image.resolution[1] = y;
	node->content.image.resolution[2] = 1;
	node->content.image.image_stack.layer_count = 1;
	node->content.image.image_stack.layers = malloc(sizeof * node->content.image.image_stack.layers);
	node->content.image.image_stack.layers->components = channels[color];
	node->content.image.image_stack.layers->type = HXA_LDT_UINT8;
	node->content.image.image_stack.layers->data.uint8_data = image_data;
	for (i = 0; name[i] != 0; i++)
		node->content.image.image_stack.layers->name[i] = name[i];
	node->content.image.image_stack.layers->name[i] = 0;
	node->meta_data_count = 2;
	node->meta_data = malloc((sizeof * node->meta_data) * 2);
	name = "name";
	for (i = 0; name[i] != 0; i++)
		node->meta_data[0].name[i] = name[i];
	node->meta_data[0].name[i] = 0;
	node->meta_data[0].type = HXA_MDT_TEXT;
	for (i = 0; file_name[i] != 0; i++);

	node->meta_data[0].value.text_value = malloc(i + 1);
	for (; i != 0 && file_name[i] != '/' && file_name[i] != '\\'; i--);
	if (file_name[i] == '/' || file_name[i] == '\\')
		i++;
	for (j = 0; file_name[i + j] != 0 && file_name[i + j] != '.'; j++)
		node->meta_data[0].value.text_value[j] = file_name[i + j];
	node->meta_data[0].value.text_value[j] = 0;
	node->meta_data[0].array_length = j;
	name = "png_cunks";
	for (i = 0; name[i] != 0; i++)
		node->meta_data[1].name[i] = name[i];
	node->meta_data[1].name[i] = 0;
	node->meta_data[1].type = HXA_MDT_BINARY;
	node->meta_data[1].array_length = meta_data_used;
	node->meta_data[1].value.bin_value = meta_data;
	return TRUE;
}

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void hxa_png_crc_compute_table(void)
{
	unsigned long c;
	int n, k;

	for (n = 0; n < 256; n++)
	{
		c = (unsigned long)n;
		for (k = 0; k < 8; k++)
		{
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[n] = c;
	}
	crc_table_computed = 1;
}

void hxa_png_crc_compute(unsigned char* output, unsigned char* read_buffer, int length)
{
	unsigned int crc = 0xffffffffL, i;
	hxa_png_crc_compute_table();
	for (i = 0; i < length; i++)
		crc = crc_table[(crc ^ read_buffer[i]) & 0xff] ^ (crc >> 8);
	crc = crc ^ 0xffffffffL;
	output[0] = (crc >> 24) & 0xFF;
	output[1] = (crc >> 16) & 0xFF;
	output[2] = (crc >> 8) & 0xFF;
	output[3] = crc & 0xFF;
}

channels[] = { 1, 0, 3, 3, 2, 0, 4 };

int hxa_debug_print_buffer(unsigned char* pixels, unsigned int size, char* name)
{
	unsigned int i, j, k;

	printf("%s\n", name);
	for (i = 0; i < size;)
	{
		if (pixels[i] == 127)
		{
			for (j = k = 0; j + i + 3 < size; j += 4)
			{
				if (pixels[i + j + 0] != 127 &&
					pixels[i + j + 1] != 127 &&
					pixels[i + j + 2] != 127 &&
					pixels[i + j + 3] != 127)
					break;
			}
			if (j > 4)
			{
				printf("127 streak: %u - %u\n", i, j);
			}
			i += j;
		}
		else
			i++;
	}
	for (i = 0; i < size;)
	{
		for (j = k = 0; j + i < size && k < 6; j++)
		{
			if (pixels[i + j] == 0 || pixels[i + j] == 255)
				k++;
			else
				k = 0;
		}
		if (j > 16)
		{
			printf("in data %u - %u -", i, j);
			for (k = 0; k < j; k++)
				printf(" %u", pixels[i + k]);
			printf("\n");
		}
		i += j;
	}
}

int hxa_save_png(unsigned char* pixels, unsigned int channels, unsigned int x, unsigned int y, char* file_name)
{
	FILE* file;
	unsigned int i, j, pixel_length, data_length, file_length, compression_left, write_pos, row_pos, adler_a = 1, adler_b = 0;
	unsigned short s;
	char* data, channel_code[] = { 1, 0, 4, 2, 6 };
	unsigned char value, header[] = { 137,
			'P',
			'N',
			'G',
			13,
			10,
			26,
			10,
			0, /* Chunk length */
			0,
			0,
			13,
			'I',
			'H',
			'D',
			'R',
			0, /* size X */
			0,
			0,
			0,
			0, /* size y */
			0,
			0,
			40,
			8, /* bits */
			2, /* Color */
			0, /* Compression */
			0, /* Filter */
			0,
			0, /* CRC */
			0,
			0,
			0,
			0, /* Chunk length */
			0,
			0,
			0,
			'I', /*Chunk type*/
			'D',
			'A',
			'T' }; /* Interlace (0 == None)*/
	if (!crc_table_computed)
		hxa_png_crc_compute_table();
	file = fopen(file_name, "wb");
	if (file == NULL)
		return FALSE;
	pixel_length = (x * channels + 1) * y;
	data_length = pixel_length + (1 + pixel_length / 0xFFFF) * 5 + 6;
	file_length = sizeof(header) + 4 + data_length + 2 + 10;
	data = malloc(file_length);
	memcpy(data, header, sizeof(header));
	data[16] = (x >> 24) & 0xFF;
	data[17] = (x >> 16) & 0xFF;
	data[18] = (x >> 8) & 0xFF;
	data[19] = x & 0xFF;
	data[20] = (y >> 24) & 0xFF;
	data[21] = (y >> 16) & 0xFF;
	data[22] = (y >> 8) & 0xFF;
	data[23] = y & 0xFF;
	data[25] = channel_code[channels];

	hxa_png_crc_compute(&data[29], &data[12], 17);
	data[33] = (data_length >> 24) & 0xFF;
	data[34] = (data_length >> 16) & 0xFF;
	data[35] = (data_length >> 8) & 0xFF;
	data[36] = data_length & 0xFF;
	write_pos = sizeof(header);
	data[write_pos++] = 120; // deflate compression
	data[write_pos++] = 31 - (120 * 256) % 31; // no dictionary, fastest compression. check sum.



	for (i = row_pos = compression_left = 0; i < y;)
	{
		static int debug_add = -1;
		if (compression_left == 0)
		{
			if ((y - i) * (x * channels + 1) - row_pos > 0xFFFF)
			{
				data[write_pos++] = 0; /* uncompressed */
				data[write_pos++] = 0xFF; /* length */
				data[write_pos++] = 0xFF;
				data[write_pos++] = 0; /* inv legth */
				data[write_pos++] = 0;
				compression_left = 0xFFFF;
			}
			else
			{
				s = (y - i) * (x * channels + 1) - row_pos;
				//			printf("expected length: %u\n", s);
				data[write_pos++] = 1; /* Last uncompressed uncompressed */
				data[write_pos++] = s & 0xFF; /* length */
				data[write_pos++] = (s >> 8) & 0xFF;
				s = ~s;
				data[write_pos++] = s & 0xFF; /* length */
				data[write_pos++] = (s >> 8) & 0xFF;
				compression_left = 0xFFFF;
				debug_add = 0;
			}
		}
		if (row_pos == 0)
		{
			if (debug_add != 0)
				debug_add++;
			//		printf("A added %u -> %u\n", 1, debug_add);
			data[write_pos++] = 0;
			adler_a = (adler_a + 0) % MOD_ADLER;
			adler_b = (adler_b + adler_a) % MOD_ADLER;
			compression_left--;
			row_pos++;
		}//else
		{
			if (compression_left >= 1 + x * channels - row_pos)
			{

				if (debug_add != 0)
					debug_add += 1 + x * channels - row_pos;
				//	printf("B added %u -> %u\n", 1 + x * channels - row_pos, debug_add);
				for (j = 0; j < 1 + x * channels - row_pos; j++)
				{
					value = pixels[i * x * channels + row_pos - 1 + j];
					adler_a = (adler_a + value) % MOD_ADLER;
					adler_b = (adler_b + adler_a) % MOD_ADLER;
					data[write_pos + j] = value;
				}
				/*
				memcpy(&data[write_pos], &pixels[i * x * channels + row_pos], x * channels - row_pos);*/
				write_pos += j;
				compression_left -= 1 + x * channels - row_pos;
				row_pos = 0;
				i++;
			}
			else
			{
				if (debug_add != 0)
					debug_add += compression_left;
				//		printf("C added %u -> %u\n", compression_left, debug_add);
				for (j = 0; j < compression_left; j++)
				{
					value = pixels[i * x * channels + row_pos - 1 + j];
					adler_a = (adler_a + value) % MOD_ADLER;
					adler_b = (adler_b + adler_a) % MOD_ADLER;
					data[write_pos + j] = value;
				}
				/*	memcpy(&data[write_pos], &pixels[i * x * channels + row_pos], compression_left);
				*/	write_pos += compression_left;
				row_pos += compression_left;
				compression_left = 0;
			}
		}
	}

	adler_a = (adler_b << 16) | adler_a;
	data[write_pos++] = (adler_a >> 24) & 0xFF;
	data[write_pos++] = (adler_a >> 16) & 0xFF;
	data[write_pos++] = (adler_a >> 8) & 0xFF;
	data[write_pos++] = adler_a & 0xFF;
	hxa_png_crc_compute(&data[write_pos], &data[37], write_pos - 37);
	write_pos += 4;
	data[write_pos++] = 0;
	data[write_pos++] = 0;
	data[write_pos++] = 0;
	data[write_pos++] = 0;
	data[write_pos++] = 73;
	data[write_pos++] = 69;
	data[write_pos++] = 78;
	data[write_pos++] = 68;
	data[write_pos++] = 174;
	data[write_pos++] = 66;
	data[write_pos++] = 96;
	data[write_pos++] = 130;
	fwrite(data, 1, file_length, file);
	//	printf("saving!!!\n");
	//	f_print_raw(data, 50);
	fclose(file);
	free(data);
	return TRUE;
}