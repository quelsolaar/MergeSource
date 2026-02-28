#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

/*
 Need: 
	-Change size.
	-Delete.
	-Clone.
*/

extern void hxa_serialize_free_meta(HXAMeta *meta);

int hxa_util_meta_delete(HXAMeta **meta_data, hxa_uint32 *meta_data_count, HXAMeta *meta)
{
	unsigned int i;
	for(i = 0; i < *meta_data_count; i++)
	{
		if(&(*meta_data)[i] == meta)
		{
			hxa_serialize_free_meta(meta);
			for(i++; i < *meta_data_count; i++)
				(*meta_data)[i - 1] = (*meta_data)[i];
			(*meta_data_count)--;
			if((*meta_data_count) == 0)
			{
				free(*meta_data);
				*meta_data = NULL;
			}
			return TRUE;				
		}
		if((*meta_data)[i].type == HXA_MDT_META)
			if(hxa_util_meta_delete((HXAMeta **)&(*meta_data)[i].value.array_of_meta, &(*meta_data)[i].array_length, meta))
				return TRUE;
	}
	return FALSE;
}

HXAMeta *hxa_util_meta_clone(HXAMeta *meta_data, hxa_uint32 meta_data_count)
{
	HXAMeta *meta;
	unsigned int i, j;
	if(meta_data_count == 0)
		return NULL;
	meta = malloc((sizeof *meta_data) * meta_data_count);
	memcpy(meta, meta_data, (sizeof *meta_data) * meta_data_count);
	for(i = 0; i < meta_data_count; i++)
	{
		switch(meta[i].type)
		{
			case HXA_MDT_INT64 :
				meta[i].value.int64_value = malloc(sizeof(int64) * meta[i].array_length);
				memcpy(meta[i].value.int64_value, meta_data[i].value.int64_value, sizeof(int64) * meta[i].array_length);
			break;
			case HXA_MDT_DOUBLE :
				meta[i].value.double_value = malloc(sizeof(double) * meta[i].array_length);
				memcpy(meta[i].value.double_value, meta_data[i].value.double_value, sizeof(int64) * meta[i].array_length);
			break;
			case HXA_MDT_NODE :
				meta[i].value.node_value = malloc((sizeof *meta[i].value.node_value) * meta[i].array_length);
				memcpy(meta[i].value.node_value, meta_data[i].value.node_value, sizeof(int64) * meta[i].array_length);
			break;
			case HXA_MDT_TEXT :
				meta[i].value.text_value = malloc(sizeof(char) * meta[i].array_length);
				memcpy(meta[i].value.text_value, meta_data[i].value.text_value, sizeof(char) * meta[i].array_length);
			break;
			case HXA_MDT_BINARY :
				meta[i].value.int64_value = malloc(meta[i].array_length);
				memcpy(meta[i].value.int64_value, meta_data[i].value.int64_value, meta[i].array_length);
			break;
			case HXA_MDT_META :
				meta[i].value.array_of_meta = hxa_util_meta_clone(meta_data[i].value.array_of_meta, meta[i].array_length);
			break;
		}

	}
	return meta;
}


int hxa_util_meta_resize(HXAMeta *meta, unsigned int new_size)
{
	void *alloc = NULL;
	unsigned int i;
	switch(meta->type)
	{
		case HXA_MDT_INT64 :
			alloc = realloc(meta->value.int64_value, (sizeof *meta->value.int64_value) * new_size);
			if(alloc != NULL)
			{
				meta->value.int64_value = alloc;
				for(i = meta->array_length; i < new_size; i++)
					meta->value.int64_value[i] = 0; 
				meta->array_length = new_size;
			}
		break;
		case HXA_MDT_DOUBLE :
			alloc = realloc(meta->value.double_value, (sizeof *meta->value.double_value) * new_size);
			if(alloc != NULL)
			{
				meta->value.double_value = alloc;
				for(i = meta->array_length; i < new_size; i++)
					meta->value.double_value[i] = 0; 
				meta->array_length = new_size;
			}
		break;
		case HXA_MDT_NODE :
			alloc = realloc(meta->value.node_value, (sizeof *meta->value.node_value) * new_size);
			if(alloc != NULL)
			{
				meta->value.node_value = alloc;
				for(i = meta->array_length; i < new_size; i++)
					meta->value.node_value[i] = 0; 
				meta->array_length = new_size;
			}
		break;
		case HXA_MDT_TEXT :
			alloc = realloc(meta->value.text_value, (sizeof *meta->value.text_value) * new_size);
			if(alloc != NULL)
			{
				meta->value.text_value = alloc;
				for(i = meta->array_length; i < new_size; i++)
					meta->value.text_value[i] = 0; 
				meta->array_length = new_size;
			}
		break;
		case HXA_MDT_BINARY :
			alloc = realloc(meta->value.bin_value, (sizeof *meta->value.bin_value) * new_size);
			if(alloc != NULL)
			{
				meta->value.bin_value = alloc;
				for(i = meta->array_length; i < new_size; i++)
					meta->value.bin_value[i] = 0; 
				meta->array_length = new_size;
			}
		break;
		case HXA_MDT_META :			
			for(i = new_size; i < meta->array_length; i++)
				hxa_serialize_free_meta(&((HXAMeta *)meta->value.array_of_meta)[i]);
			alloc = realloc(meta->value.array_of_meta, sizeof(HXAMeta) * new_size);
			if(alloc != NULL)
			{
				meta->value.array_of_meta = alloc;
				for(i = meta->array_length; i < new_size; i++)
				{
					((HXAMeta *)meta->value.array_of_meta)[i].array_length = 0; 
					((HXAMeta *)meta->value.array_of_meta)[i].type = HXA_MDT_INT64; 
					((HXAMeta *)meta->value.array_of_meta)[i].name[0] = 0;
					((HXAMeta *)meta->value.array_of_meta)[i].value.int64_value = NULL; 
				}
				meta->array_length = new_size;
			}if(new_size < meta->array_length)
				meta->array_length = new_size;
		break;			
	}
	return alloc != NULL;
}



HXAMeta *hxa_util_meta_add(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, HXAMetaDataType type, void *data, unsigned int length, int copy)
{
	HXAMeta *m;
	unsigned int i, j;
	*meta_data = realloc(*meta_data, (sizeof **meta_data) * (*meta_data_count + 1));
	m = &(*meta_data)[(*meta_data_count)++];
	for(i = 0; i < HXA_NAME_MAX_LENGTH - 1 && name[i] != 0; i++)
		m->name[i] = name[i];
	m->name[i] = 0;
	m->type = type;
	if(data == NULL)
		copy = TRUE;
	switch(type)
	{
		case HXA_MDT_INT64 :
			if(copy)
			{
				m->value.int64_value = malloc((sizeof *m->value.int64_value) * length);
				if(data == NULL)
				{
					for(j = 0; j < length; j++)
						m->value.int64_value[j] = 0;
				}else
					memcpy(m->value.int64_value, data, (sizeof *m->value.int64_value) * length);
			}else
				m->value.int64_value = data;
		break;
		case HXA_MDT_DOUBLE :
			if(copy)
			{
				m->value.double_value = malloc((sizeof *m->value.double_value) * length);
				if(data == NULL)
				{
					for(j = 0; j < length; j++)
						m->value.double_value[j] = 0;
				}else
					memcpy(m->value.double_value, data, (sizeof *m->value.double_value) * length);
			}else
				m->value.int64_value = data;
		break;
		case HXA_MDT_NODE :
			if(copy)
			{
				m->value.node_value = malloc((sizeof *m->value.node_value) * length);
				if(data == NULL)
				{
					for(j = 0; j < length; j++)
						m->value.node_value[j] = 0;
				}else
					memcpy(m->value.node_value, data, (sizeof *m->value.node_value) * length);
			}else
				m->value.int64_value = data;
		break;
		case HXA_MDT_TEXT :
			if(data != NULL)
			{
				for(length = 0; ((char *)data)[length] != 0; length++);
				length++;
				if(copy)
				{
					m->value.text_value = malloc((sizeof *m->value.text_value) * length);
					memcpy(m->value.text_value, data, (sizeof *m->value.text_value) * length);
				}else
					m->value.text_value = data;
			}else
			{
				m->value.text_value = malloc((sizeof *m->value.text_value) * length);
				for(j = 0; j < length; j++)
					m->value.text_value[j] = 0;
			}
		break;
		case HXA_MDT_BINARY :
			if(copy)
			{
				m->value.bin_value = malloc((sizeof *m->value.bin_value) * length);
				if(data == NULL)
				{
					for(j = 0; j < length; j++)
						m->value.bin_value[j] = 0;
				}else
					memcpy(m->value.bin_value, data, (sizeof *m->value.bin_value) * length);
			}else
				m->value.bin_value = data;
		break;
		case HXA_MDT_META :
			if(copy)
			{
				if(data == NULL)
				{
					m->value.array_of_meta = NULL;
					length = 0;
				}else
				{
					m->value.array_of_meta = hxa_util_meta_clone((HXAMeta *)data, length);
				}
			}else
				m->value.array_of_meta = data;
		break;
	}
	m->array_length = length;
	return m;
}

HXAMeta *hxa_util_meta_set(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, HXAMetaDataType type, void *data, unsigned int length, int copy)
{
	HXAMeta *m;
	uint i, j;
	for(i = 0; i < *meta_data_count; i++)
	{
		m = &(*meta_data)[i];
		if(m->type == type)
		{

			for(j = 0; m->name[j] != 0 && m->name[j] == name[j]; j++);
			if(m->name[j] == name[j])
			{
				switch(type)
				{
					case HXA_MDT_INT64 :
						if(copy)
						{
							if(m->array_length != length)
							{
								free(m->value.int64_value);
								m->value.int64_value = malloc((sizeof *m->value.int64_value) * length);
							}
							if(data == NULL)
							{
								for(j = 0; j < length; j++)
									m->value.int64_value[j] = 0;
							}else
								memcpy(m->value.int64_value, data, (sizeof *m->value.int64_value) * length);
						}else
						{
							free(m->value.int64_value);
							m->value.int64_value = data;
						}
					break;
					case HXA_MDT_DOUBLE :
						if(copy)
						{
							if(m->array_length != length)
							{
								free(m->value.double_value);
								m->value.double_value = malloc((sizeof *m->value.double_value) * length);
							}
							if(data == NULL)
							{
								for(j = 0; j < length; j++)
									m->value.double_value[j] = 0;
							}else
								memcpy(m->value.double_value, data, (sizeof *m->value.double_value) * length);
						}else
						{
							free(m->value.double_value);
							m->value.double_value = data;
						}
					break;
					case HXA_MDT_NODE :
						if(copy)
						{
							if(m->array_length != length)
							{
								free(m->value.node_value);
								m->value.node_value = malloc((sizeof *m->value.node_value) * length);
							}
							if(data == NULL)
							{
								for(j = 0; j < length; j++)
									m->value.node_value[j] = 0;
							}else
								memcpy(m->value.node_value, data, (sizeof *m->value.node_value) * length);
						}else
						{
							free(m->value.node_value);
							m->value.node_value = data;
						}
					break;
					case HXA_MDT_TEXT :
						free(m->value.text_value);
						if(data != NULL)
						{
							for(length = 0; ((char *)data)[length] != 0; length++);
							length++;
							if(copy)
							{
								m->value.text_value = malloc((sizeof *m->value.text_value) * length);
								memcpy(m->value.text_value, data, (sizeof *m->value.text_value) * length);
							}else
								m->value.text_value = data;
						}else
						{
							m->value.text_value = malloc((sizeof *m->value.text_value) * length);
							for(j = 0; j < length; j++)
								m->value.text_value[j] = 0;
						}
					break;
					case HXA_MDT_BINARY :
						if(copy)
						{
							if(m->array_length != length)
							{
								free(m->value.bin_value);
								m->value.bin_value = malloc((sizeof *m->value.bin_value) * length);
							}
							if(data == NULL)
							{
								for(j = 0; j < length; j++)
									m->value.bin_value[j] = 0;
							}else
								memcpy(m->value.bin_value, data, (sizeof *m->value.bin_value) * length);
						}else
						{
							free(m->value.bin_value);
							m->value.bin_value = data;
						}
					break;
					case HXA_MDT_META :
						if(copy)
						{
							if(data == NULL)
							{
								m->value.array_of_meta = NULL;
								length = 0;
							}else
							{
								m->value.array_of_meta = hxa_util_meta_clone((HXAMeta *)data, length);
							}
						}else
							m->value.array_of_meta = data;
					break;
				}
				(*meta_data)[i].array_length = length;
				return &(*meta_data)[i];
			}
		}
	}
	return hxa_util_meta_add(meta_data, meta_data_count, name, type, data, length, copy);

}

void *hxa_util_meta_data_get(HXAMeta *meta_data, hxa_uint32 meta_data_count, char *name, HXAMetaDataType type, unsigned int *length, int recursive)
{
	void *output;
	unsigned int i, j;
	for(i = 0; i < meta_data_count; i++)
	{
		if(meta_data[i].type == type)
		{
			for(j = 0; meta_data[i].name[j] == name[j] && name[j] != 0; j++);
			if(meta_data[i].name[j] == name[j])
			{
				if(length != NULL)
					*length = meta_data[i].array_length;
				return meta_data[i].value.array_of_meta;
			}
			
		}
		if(recursive && meta_data[i].type == HXA_MDT_META)
			if((output = hxa_util_meta_data_get(meta_data[i].value.array_of_meta, meta_data[i].array_length, name, type, length, TRUE)) != NULL)
				return output;
	}
	if(length != NULL)
		*length = 0;
	return NULL;
}



HXAMeta *hxa_util_meta_get(HXAMeta *meta_data, hxa_uint32 meta_data_count, char *name, int recursive)
{
	void *output;
	unsigned int i, j;
	for(i = 0; i < meta_data_count; i++)
	{
		for(j = 0; meta_data[i].name[j] == name[j] && name[j] != 0; j++);
		if(meta_data[i].name[j] == name[j])
			return &meta_data[i];
	}
	if(recursive)
		for(i = 0; i < meta_data_count; i++)	
			if(meta_data[i].type == HXA_MDT_META)
				if((output = hxa_util_meta_get(meta_data[i].value.array_of_meta, meta_data[i].array_length, name, TRUE)) != NULL)
					return output;
	return NULL;
}


HXAMeta *hxa_util_meta_from_node_get(HXANode *node, char *name, int recursive)
{
	return hxa_util_meta_get(node->meta_data, node->meta_data_count, name, recursive);
}


unsigned int hxa_util_meta_data_get_next(HXAMeta *meta_data, hxa_uint32 start, hxa_uint32 meta_data_count, char *name, HXAMetaDataType type)
{
	unsigned int i, j;
	for(i = start; i < meta_data_count; i++)
	{
		if(meta_data[i].type == type)
		{
			for(j = 0; meta_data[i].name[j] == name[j] && name[j] != 0; j++);
			if(meta_data[i].name[j] == name[j])
				return i;			
		}
	}
	return -1;
}

void *hxa_util_meta_data_get_set(HXAMeta **meta_data, hxa_uint32 *meta_data_count, void *default_data, unsigned int *data_count, char *name, HXAMetaDataType type)
{
	unsigned int data_count_in;
	void *data;
	data_count_in = *data_count;
	data = hxa_util_meta_data_get(*meta_data, *meta_data_count, name, type, data_count, FALSE);
	if(data != NULL)
		return data;
	hxa_util_meta_add(meta_data, meta_data_count, name, type, default_data, data_count_in, TRUE);
	*data_count = data_count_in;
	return default_data;
}

HXAMeta *hxa_util_meta_level_get(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, int recursive)
{

	HXAMeta *meta;
	meta = hxa_util_meta_get(*meta_data, *meta_data_count, name, recursive);
	if(meta == NULL)
	{
		hxa_util_meta_add(meta_data, meta_data_count, name, HXA_MDT_META, NULL, 0, FALSE);
		meta = *meta_data;
		return &meta[*meta_data_count - 1];
	}
	return meta;
}