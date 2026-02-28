 
#include "forge.h"
#include "r_include.h"
#include "relinquish.h"

typedef struct{
	void **pointers;
	uint alloctaed_count;
	uint alloctaed_alloctaed;
	uint freed;
	uint line;
	char *file;
	uint64 size;
	uint64 baseline;
	RelinquishAssetType type;
}RelinquishAsset;

RelinquishAsset *r_assets = NULL;
uint r_asset_count = 0;

void r_draw_asset_track(RelinquishAssetType type, void *pointer, size_t size, char *file, uint line)
{
	uint i;
	for(i = 0; i < r_asset_count; i++)
	{
		if(r_assets[i].file == file && r_assets[i].line == line)
		{
			if(r_assets[i].alloctaed_count == r_assets[i].alloctaed_alloctaed)
			{
				r_assets[i].alloctaed_alloctaed <<= 1;
				r_assets[i].pointers = realloc(r_assets[i].pointers, (sizeof *r_assets[i].pointers) * r_assets[i].alloctaed_alloctaed);
			}
			if(type == RELINQUISH_AT_TEXTURE)
			{
				union {void *pointer; uint32 integer[2];}tracker_handle;
				tracker_handle.pointer = pointer;
				tracker_handle.integer[1] = size;
				pointer = tracker_handle.pointer;
			}
			r_assets[i].pointers[r_assets[i].alloctaed_count++] = pointer;
			r_assets[i].size += size;
			return;
		}
	}
	if(r_asset_count % 64 == 0)
		r_assets = realloc(r_assets, (sizeof *r_assets) * (r_asset_count + 64));
	if(type == RELINQUISH_AT_TEXTURE)
	{
		union {void *pointer; uint32 integer[2];}tracker_handle;
		tracker_handle.pointer = pointer;
		tracker_handle.integer[1] = size;
		pointer = tracker_handle.pointer;
	}
	r_assets[r_asset_count].pointers = malloc((sizeof *r_assets[i].pointers) * 64);
	r_assets[r_asset_count].pointers[0] = pointer;
	r_assets[r_asset_count].alloctaed_count = 1;
	r_assets[r_asset_count].alloctaed_alloctaed = 64;
	r_assets[r_asset_count].freed = 0;
	r_assets[r_asset_count].line = line;
	r_assets[r_asset_count].file = file;
	r_assets[r_asset_count].size = size;
	r_assets[r_asset_count].type = type;
	r_assets[r_asset_count].baseline = 0;
	r_asset_count++;
}

void r_draw_asset_untrack(RelinquishAssetType type, void *pointer, size_t size)
{
	uint i, j;
	if(type != RELINQUISH_AT_TEXTURE)
	{
		for(i = 0; i < r_asset_count; i++)
		{
			if(r_assets[i].type == type)
			{
				for(j = 0; j < r_assets[i].alloctaed_count; j++)
				{
					if(r_assets[i].pointers[j] == pointer)
					{
						r_assets[i].pointers[j] = r_assets[i].pointers[--r_assets[i].alloctaed_count];
						r_assets[i].freed++;
						r_assets[i].size -= size;
						return;
					}
				}			
			}
		}
	}else
	{
		union {void *pointer; uint32 integer[2];}tracker_handle;
		for(i = 0; i < r_asset_count; i++)
		{
			if(r_assets[i].type == type)
			{
				for(j = 0; j < r_assets[i].alloctaed_count; j++)
				{
					tracker_handle.pointer = r_assets[i].pointers[j];
					tracker_handle.integer[1] = 0;
					if(tracker_handle.pointer == pointer)
					{
						tracker_handle.pointer = r_assets[i].pointers[j];
						r_assets[i].size -= tracker_handle.integer[1];
						r_assets[i].pointers[j] = r_assets[i].pointers[--r_assets[i].alloctaed_count];
						r_assets[i].freed++;
						return;
					}
				}			
			}
		}
	}
}



void r_draw_asset_print()
{
	char *asset_type_names[RELINQUISH_AT_COUNT]  = {"Shader", "Buffer", "Texture", "Frame buffer object"};
	uint i, j, count[RELINQUISH_AT_COUNT];
	uint64 sizes[RELINQUISH_AT_COUNT];

	for(i = 0; i < RELINQUISH_AT_COUNT; i++)
	{
		sizes[i] = 0;
		count[i] = 0;
	}
	for(i = 0; i < r_asset_count; i++)
	{
		count[r_assets[i].type] += r_assets[i].alloctaed_count;
		sizes[r_assets[i].type] += r_assets[i].size;
	}
	printf("Relinquish Asset usage:\n");
	for(i = 0; i < RELINQUISH_AT_COUNT; i++)
		printf("\t%ss: %u bytes: %llu\n", asset_type_names[i], count[i], sizes[i]);

	printf("Details:\n");
	for(i = 0; i < RELINQUISH_AT_COUNT; i++)
	{
		printf("%ss: %u bytes: %llu\n", asset_type_names[i], count[i], sizes[i]);
		for(j = 0; j < r_asset_count; j++)
		{
			if(r_assets[j].type == i && r_assets[j].baseline < r_assets[j].size)
			{
				printf("\t%s %u\n", r_assets[j].file, r_assets[j].line);
				printf("\t\tCount: %u\n", r_assets[j].alloctaed_count);
				printf("\t\tFreed: %u\n", r_assets[j].freed);
				printf("\t\tSize: %llu\n", r_assets[j].size);
			}
		}
	}

	i += 0;
}