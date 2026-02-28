#include "la_includes.h"

#include "la_particle_fx.h"
#include "la_tool.h"

typedef struct{
	double	x;
	double	y;
	double	z;
	uint16 layer;
}UNDOMoveVertex;

typedef struct{
	uint32	a;
	uint32	b;
	uint32	c;
	uint32	d;
}UNDOCreatePolygon;

typedef struct{
	uint32	a;
	uint32	b;
	uint32	c;
	uint32	d;
}UNDOSetCrease;


typedef struct{
	uint32	a;
	uint32	b;
}UNDOSetEdge;

typedef struct{
	char	group[16];
	char	tag[16];	
	double	vec[3];
	double	select;
}UNDOTag;

typedef enum{
	UNDOET_EMPTY,
	UNDOET_STOPP,
	UNDOET_VERTEX,
	UNDOET_POLYGON,
	UNDOET_CREASE,
	UNDOET_SELECT,
	UNDOET_EDGE,
	UNDOET_TAG_CREATE,
	UNDOET_TAG_MOVE,
	UNDOET_TAG_GROUP,
	UNDOET_TAG_NAME,
	UNDOET_TAG_SELECT,
}UNDOEventType;

typedef struct{
	UNDOEventType	type;
	uint			id;
	union {
		UNDOMoveVertex		vertex;
		UNDOCreatePolygon	polygon;
		UNDOSetCrease		crease;
		UNDOSetEdge			edge;
		double				select;
		UNDOTag				tag;
	} event[2];
}UNDOEvent;

struct{
	UNDOEvent	*event;
	uint		length;
	uint		undos;
	uint		redos;
	uint		pos;
	uint32		g_node;
	uint32		ref_length;
	uint32		*ref;
	uint		structure_version;
	boolean		crease_local;
	uint32		*crease;
	uint16		crease_layer;	
	uint32		vertex_length;
	double		*vertex;
	uint16		vertex_layer;
	uint		vertex_version;
//	uint		layer_id;
//	char		layer_name[17];
	boolean		select_local;
	double		*select;
	uint16		select_layer;
	uint		select_version;
	uint32		edge_count;
	uint32		edge_alocated;
	uint32		*edges;
	uint		edge_version;
	uint32		slot[2];
	boolean		vertex_delete;
	double		distance;
	double		grid_center[3];
	double		grid_size;
	boolean		grid_active;
	UNDOTag		*tags;
	uint		tag_count;
	UNDOTag		pending_tag;
	uint		tags_version;
	uint		tag_type_in_tag;
	uint		tag_type_in_group;
	char		tag_type_in_text[16];
	uint		tag_type_in_cursor;
	uint		reference_node;
	boolean		group_update;
}UNDOGlobal;

/*
typedef struct{
	char	group[16];
	char	tag[16];	
	double	vec[3];
	double	select;
}UNDOTag;

extern UNDOTag	*udg_get_tags(uint *count);
extern void		udg_create_tag(double *vec);
extern void		udg_destory_tag(double *vec);
extern void		udg_rename_tag(uint tag, char name);
extern void		udg_rename_group(uint tag, char name);
extern void		udg_move_tag(uint tag, double *vec);
extern void		udg_select_tag(uint tag, double	select);
*/


extern void udg_create_func(uint connection, uint id, VNodeType type, void *user);

void init_undo(void)
{
	uint i;
	UNDOGlobal.length = imagine_setting_integer_get("UNDO_MEMORY_FOOT_PRINT", 5000000, "Ammount of memory dedicated to undo buffer.") / sizeof(UNDOEvent);
	UNDOGlobal.event = malloc((sizeof *UNDOGlobal.event) * UNDOGlobal.length);
	UNDOGlobal.undos = 0;
	UNDOGlobal.redos = 0;
	UNDOGlobal.pos = 0;
	UNDOGlobal.g_node = 0;
	UNDOGlobal.vertex_delete = FALSE;
	UNDOGlobal.distance = 0.01;
	UNDOGlobal.select_local = FALSE;
	UNDOGlobal.select = NULL;
	for(i = 0; i < UNDOGlobal.length; i++)
		UNDOGlobal.event[i].type = UNDOET_STOPP;
	UNDOGlobal.grid_active = FALSE;
	UNDOGlobal.grid_center[0] = 0;
	UNDOGlobal.grid_center[1] = 0;
	UNDOGlobal.grid_center[2] = 0;
	UNDOGlobal.grid_size = 0.01;
	e_ns_set_node_create_func(udg_create_func, NULL);
	UNDOGlobal.slot[0] = 0;
	UNDOGlobal.slot[1] = 0;


	UNDOGlobal.tags = NULL;
	UNDOGlobal.tag_count = 0;
	UNDOGlobal.pending_tag.group[0] = 0;
	UNDOGlobal.tag_type_in_tag = -1;
	UNDOGlobal.tag_type_in_group = -1;
	UNDOGlobal.tag_type_in_text[0] = 0;
	UNDOGlobal.tag_type_in_cursor = 0;
	UNDOGlobal.reference_node = 0;
	UNDOGlobal.group_update = FALSE;
}

UNDOTag	*udg_get_tags(uint *count)
{
	*count = UNDOGlobal.tag_count;
	return UNDOGlobal.tags;
}

void udg_get_tag_pos(uint tag, double *pos)
{
	if(tag < UNDOGlobal.tag_count)
	{
		pos[0] = UNDOGlobal.tags[tag].vec[0];
		pos[1] = UNDOGlobal.tags[tag].vec[1];
		pos[2] = UNDOGlobal.tags[tag].vec[2];
	}
}
void udg_set_grid(boolean active)
{
	UNDOGlobal.grid_active = active;
}

void udg_set_grid_snap(double *center, double size)
{
	UNDOGlobal.grid_size = size;
	UNDOGlobal.grid_center[0] = center[0];
	UNDOGlobal.grid_center[1] = center[1];
	UNDOGlobal.grid_center[2] = center[2];
	UNDOGlobal.grid_active = TRUE;
}

boolean udg_get_grid_snap(double *center, double *size)
{
	if(size != NULL)
		*size = UNDOGlobal.grid_size;
	if(center != NULL)
	{
		center[0] = UNDOGlobal.grid_center[0];
		center[1] = UNDOGlobal.grid_center[1];
		center[2] = UNDOGlobal.grid_center[2];
	}
	return UNDOGlobal.grid_active;
}

uint create_id;

void udg_create_new_modeling_node(void)
{
	verse_send_node_create(0, V_NT_GEOMETRY, 0);
	create_id = 0;
}

uint32 udg_get_modeling_node(void)
{
    return UNDOGlobal.g_node;
}

void udg_create_new_modeling_layer(void)
{
	ENode *node;
	EGeoLayer *layer;
	uint count = 0;
	char name[32];
	node = e_ns_get_node(0, udg_get_modeling_node());
	if(node != NULL	&& V_NT_GEOMETRY == e_ns_get_node_type(node))
		for(layer = e_nsg_get_layer_next(node, 0); layer != NULL; layer = e_nsg_get_layer_next(node, e_nsg_get_layer_id(layer) + 1))
			if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer))
				count++;
	sprintf(name, "AltLayer%u", count);
	verse_send_g_layer_create(UNDOGlobal.g_node, -1, name, VN_G_LAYER_VERTEX_XYZ, 0, 0);

}


void udg_set_modeling_layer(uint layer_id)
{
	UNDOGlobal.vertex_layer = layer_id;
	UNDOGlobal.vertex_version += 3343535;
}

egreal *udg_get_base_layer(void)
{
	ENode *g_node;
	EGeoLayer *layer;
	if(UNDOGlobal.vertex_layer == 0)
		return NULL;
	g_node = e_ns_get_node(0, UNDOGlobal.g_node);
	layer = e_nsg_get_layer_by_id(g_node, 0);
	return e_nsg_get_layer_data(g_node, layer);
}

void udg_create_func(uint connection, uint id, VNodeType type, void *user)
{
	if(type == V_NT_GEOMETRY)
	{
		verse_send_g_layer_create(id, (VLayerID) ~0u, "crease", VN_G_LAYER_POLYGON_CORNER_UINT32, 0, 0);
		verse_send_g_layer_create(id, (VLayerID) ~0u, "select", VN_G_LAYER_VERTEX_REAL, 0, 0);
		verse_send_g_crease_set_edge(id, "crease", 0);
		UNDOGlobal.g_node = id;
	}
}

void udg_destroy_node(uint32 node_id)
{
	verse_send_node_destroy(node_id);
}


void udg_clone_node(uint32 node_id)
{
/*	ENode *original_node, *copy_node;
	EGeoLayer *layer;
	uint32 *ref, *crease;
	Point *vertex;
	uint32 i, j, id, length, vertex_length;
	original_node = e_ns_get_node(0, e_nso_get_link(e_ns_get_node(0, node_id), V_NT_GEOMETRY));
	node_id = e_nso_get_link(e_ns_get_node(0, udg_create_new_modeling_node()), V_NT_GEOMETRY);
	copy_node = e_ns_get_node(0, node_id);

	ref = e_nsg_get_layer_data(original_node, e_nsg_get_layer_by_id(original_node, 1));
	crease = e_nsg_get_layer_data(original_node, e_nsg_get_layer_by_type(original_node, VN_G_LAYER_POLYGON_CORNER_UINT32, "crease"));
	length = e_nsg_get_polygon_length(original_node);
	vertex_length = e_nsg_get_vertex_length(original_node);
	id = e_nsg_get_layer_id(e_nsg_get_layer_by_type(copy_node, VN_G_LAYER_POLYGON_CORNER_UINT32, "crease"));
	vertex = e_nsg_get_layer_data(original_node, e_nsg_get_layer_by_id(original_node, 0));	
	for(i = 0; i < vertex_length; i++)
	{
		if(vertex[i].x < 1.9)
			verse_send_g_vertex_set_xyz_real64(node_id, 0, i, vertex[i].x, vertex[i].y , vertex[i].z);
	}	
	for(i = 0; i < length; i++)
	{
		if(ref[i * 4] < vertex_length)
		{
			verse_send_g_polygon_set_corner_uint32(node_id, i, 0, ref[i * 4], ref[i * 4 + 1], ref[i * 4 + 2], ref[i * 4 + 3]);
			verse_send_g_polygon_set_corner_uint32(node_id, i, id, crease[i * 4], crease[i * 4 + 1], crease[i * 4 + 2], crease[i * 4 + 3]);
		}
	}
	*/
}

void execute_event(UNDOEvent *event, uint direction)
{


	switch(event->type)
	{
		
		case UNDOET_VERTEX :
			if(event->event[direction].vertex.layer == 65535)
				verse_send_g_vertex_delete_real64(UNDOGlobal.g_node, event->id);
			else
			{
				if(UNDOGlobal.vertex_length > event->id && UNDOGlobal.vertex[event->id * 3] == V_REAL64_MAX)
					UNDOGlobal.select[event->id] = 0;
				verse_send_g_vertex_set_xyz_real64(UNDOGlobal.g_node, event->event[direction].vertex.layer, event->id, event->event[direction].vertex.x, event->event[direction].vertex.y, event->event[direction].vertex.z);
			}
		break;
		case UNDOET_POLYGON :
			if(event->event[direction].polygon.a == -1)
				verse_send_g_polygon_delete(UNDOGlobal.g_node, event->id);
			else
				verse_send_g_polygon_set_corner_uint32(UNDOGlobal.g_node, 1, event->id, event->event[direction].polygon.a, event->event[direction].polygon.b, event->event[direction].polygon.c, event->event[direction].polygon.d);
		break;
		case UNDOET_CREASE :
			verse_send_g_polygon_set_corner_uint32(UNDOGlobal.g_node, UNDOGlobal.crease_layer, event->id, event->event[direction].crease.a, event->event[direction].crease.b, event->event[direction].crease.c, event->event[direction].crease.d);
		break;
		case UNDOET_SELECT :
			if(UNDOGlobal.select_local)
			{
				UNDOGlobal.select[event->id] = event->event[direction].select;
				UNDOGlobal.select_version++;
			}
			else
				verse_send_g_vertex_set_real64(UNDOGlobal.g_node, UNDOGlobal.select_layer, event->id, event->event[direction].select);
		break;
		case UNDOET_EDGE :
			if(event->event[direction].edge.a != -1)
			{
				if(UNDOGlobal.edge_count == UNDOGlobal.edge_alocated)
				{
					UNDOGlobal.edge_alocated += 16; 
					UNDOGlobal.edges = realloc(UNDOGlobal.edges, (sizeof *UNDOGlobal.edges) * 2 * UNDOGlobal.edge_alocated);
				}
				UNDOGlobal.edges[UNDOGlobal.edge_count * 2] = event->event[direction].edge.a;
				UNDOGlobal.edges[UNDOGlobal.edge_count * 2 + 1] = event->event[direction].edge.b;
				UNDOGlobal.edge_count++;
				UNDOGlobal.edge_version++;
			}else
			{

				if(UNDOGlobal.edge_count != 0)
				{
			/*		uint i;
					--UNDOGlobal.edge_count;
					UNDOGlobal.edge_version++;
					for(i = 0; i < UNDOGlobal.edge_count; i++)
					{
						if((event->event[direction].edge.a == UNDOGlobal.edges[event->id * 2] && event->event[direction].edge.b == UNDOGlobal.edges[event->id * 2 + 1]) && (event->event[direction].edge.a == UNDOGlobal.edges[event->id * 2 + 1] && event->event[direction].edge.b == UNDOGlobal.edges[event->id * 2 ]))
						{
							UNDOGlobal.edges[event->id * 2] = UNDOGlobal.edges[UNDOGlobal.edge_count * 2];
							UNDOGlobal.edges[event->id * 2 + 1] = UNDOGlobal.edges[UNDOGlobal.edge_count * 2 + 1];
							break;
						}
					}*/
					UNDOGlobal.edge_count--;
					UNDOGlobal.edges[event->id * 2] = UNDOGlobal.edges[UNDOGlobal.edge_count * 2];
					UNDOGlobal.edges[event->id * 2 + 1] = UNDOGlobal.edges[UNDOGlobal.edge_count * 2 + 1];
					UNDOGlobal.edge_version++;
				/*	UNDOGlobal.vertex_delete = TRUE;*/
				}
			}
		break;	

		case UNDOET_TAG_CREATE :
		{
			uint16 group_id = (uint16)-1; 
			uint16 tag_id = (uint16)-1, tag_2 = (uint16)-1;
			VNTag tag;
			ENode *node;
			node = e_ns_get_node(0, UNDOGlobal.g_node);
printf("A\n");
			if(event->event[direction].tag.group[0] == 0)
			{
				printf("B\n");
				e_ns_get_tag_by_type_and_group(node, event->event[direction].tag.group, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &group_id, &tag_id);
				group_id = e_ns_get_group_by_name(node, event->event[direction].tag.group);
				if(group_id != (uint16) -1 && tag_id != (uint16) -1)
				{
					printf("C\n");
					for(tag_2 = e_ns_get_next_tag(node, group_id, 0); tag_2 == tag_id; tag_2 = e_ns_get_next_tag(node, group_id, tag_2 + 1));
					if(tag_2 == (uint16)-1)
						verse_send_tag_group_destroy(UNDOGlobal.g_node, group_id);
					else
						verse_send_tag_destroy(UNDOGlobal.g_node, group_id, tag_id);
				}
printf("D\n");
			}else
			{

				printf("E %s\n", event->event[direction].tag.group);
				e_ns_get_tag_by_type_and_group(node, event->event[direction].tag.group, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &group_id, &tag_id);
				group_id = e_ns_get_group_by_name(node, event->event[direction].tag.group);
				printf("group_id, %u tag_id %u\n", (uint32)group_id, (uint32)tag_id);
				if(group_id != (uint16)-1)
				{
					printf("F\n");
					tag.vreal64_vec3[0] = event->event[direction].tag.vec[0];
					tag.vreal64_vec3[1] = event->event[direction].tag.vec[1];
					tag.vreal64_vec3[2] = event->event[direction].tag.vec[2];
					verse_send_tag_create(UNDOGlobal.g_node, group_id, (uint16)-1, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &tag);
				}else
				{
					printf("G\n");
					verse_send_tag_group_create(UNDOGlobal.g_node, (uint16)-1, event->event[direction].tag.group);
					UNDOGlobal.pending_tag = event->event[direction].tag;
				}
			}
printf("H\n");
		}
		break;
		case UNDOET_TAG_MOVE :
		{
			uint16 group_id; 
			uint16 tag_id;
			VNTag tag;
			ENode *node;
			uint i;
			printf("UNDOET_TAG_MOVE\n");
			node = e_ns_get_node(0, UNDOGlobal.g_node);
			group_id = e_ns_get_group_by_name(node, event->event[direction].tag.group);
			if(group_id != (uint16) -1)
			{
				char *name;
				tag_id = event->id;
				name = e_ns_get_tag_name(node, group_id, tag_id);
				for(i = 0; name[i] != 0 && name[i] == event->event[direction].tag.tag[i]; i++);
				if(name[i] != event->event[direction].tag.tag[i])
					e_ns_get_tag_by_type_and_group(node, event->event[direction].tag.group, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &group_id, &tag_id);
				if(group_id != (uint16) -1 && tag_id != (uint16) -1)
				{
					tag.vreal64_vec3[0] = event->event[direction].tag.vec[0];
					tag.vreal64_vec3[1] = event->event[direction].tag.vec[1];
					tag.vreal64_vec3[2] = event->event[direction].tag.vec[2];
					printf("verse_send_tag_create %f %f %f\n", tag.vreal64_vec3[0], tag.vreal64_vec3[1], tag.vreal64_vec3[2]);
					verse_send_tag_create(UNDOGlobal.g_node, group_id, tag_id, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &tag);
				}
			}
		}
		break;
		case UNDOET_TAG_NAME :
		{
			uint16 group_id; 
			uint16 tag_id;
			VNTag tag;
			ENode *node;
			printf("UNDOET_TAG_NAME\n");
			node = e_ns_get_node(0, UNDOGlobal.g_node);
			e_ns_get_tag_by_type_and_group(node, event->event[1 - direction].tag.group, event->event[1 - direction].tag.tag, VN_TAG_REAL64_VEC3, &group_id, &tag_id);
			if(group_id != (uint16) -1 && tag_id != (uint16) -1)
			{
				tag.vreal64_vec3[0] = event->event[direction].tag.vec[0];
				tag.vreal64_vec3[1] = event->event[direction].tag.vec[1];
				tag.vreal64_vec3[2] = event->event[direction].tag.vec[2];
				verse_send_tag_create(UNDOGlobal.g_node, group_id, tag_id, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &tag);
			}
		}
		break;
		case UNDOET_TAG_SELECT :
		{
			uint i, j;
			for(i = 0;	UNDOGlobal.tag_count; i++)
			{
				for(j = 0; UNDOGlobal.tags[i].group[j] == event->event[direction].tag.group[j] && UNDOGlobal.tags[i].group[j] != 0; j++);
				if(UNDOGlobal.tags[i].group[j] == event->event[direction].tag.group[j])
				{
					for(j = 0; UNDOGlobal.tags[i].tag[j] == event->event[direction].tag.tag[j] && UNDOGlobal.tags[i].tag[j] != 0; j++);
					if(UNDOGlobal.tags[i].tag[j] == event->event[direction].tag.tag[j])
						UNDOGlobal.tags[i].select = event->event[direction].tag.select;
				}
			}
		}
		break;
/*	UNDOET_TAG_CREATE,
	UNDOET_TAG_MOVE,
	UNDOET_TAG_GROUP,
	UNDOET_TAG_NAME,
	UNDOET_TAG_SELECT,
  
	
	  ENode *node;
			uint i, other;
			uint16 group_id; 
			uint16 tag_id;
			VNTag tag;
			other = 1 - direction; 
			node = e_ns_get_node(0, UNDOGlobal.g_node);
			printf("A\n");
			for(i = 0; event->event[other].tag.group[i] == event->event[direction].tag.group[i] && event->event[other].tag.group[i] != 0; i++);
			if(event->event[other].tag.group[i] == event->event[direction].tag.group[i])
			{
				printf("B\n");
				e_ns_get_tag_by_type_and_group(node, event->event[other].tag.group, event->event[other].tag.tag, VN_TAG_REAL64_VEC3, &group_id, &tag_id);
				if(tag_id != -1 && (tag.vreal64_vec3[0] != event->event[direction].tag.vec[0] ||
									tag.vreal64_vec3[1] != event->event[direction].tag.vec[1] ||
									tag.vreal64_vec3[2] != event->event[direction].tag.vec[2]))
				{
					printf("C\n");
					tag.vreal64_vec3[0] = event->event[direction].tag.vec[0];
					tag.vreal64_vec3[1] = event->event[direction].tag.vec[1];
					tag.vreal64_vec3[2] = event->event[direction].tag.vec[2];
					verse_send_tag_create(UNDOGlobal.g_node, group_id, tag_id, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &tag);
				}
			}else
			{
				printf("D\n");
				group_id = e_ns_get_group_by_name(node, event->event[direction].tag.group);
				if(group_id != -1)
				{
					printf("E %u %s\n", group_id, event->event[direction].tag.group);
					for(tag_id = e_ns_get_next_tag(node, group_id, 0); tag_id != (uint16)-1; tag_id = e_ns_get_next_tag(node, group_id, tag_id + 1))
					{
						char *name;
						printf("F\n");
						name = e_ns_get_tag_name(node, group_id, tag_id);
						for(i = 0; name[i] != 0  && name[i] == event->event[other].tag.tag[i]; i++);
						if(name[i] == event->event[other].tag.tag[i])
							break;
					}
				//	if(tag_id == (uint16)-1)
				//	{
						printf("G\n");
					//	verse_send_tag_group_create(UNDOGlobal.g_node, group_id, event->event[direction].tag.group);
						tag.vreal64_vec3[0] = event->event[direction].tag.vec[0];
						tag.vreal64_vec3[1] = event->event[direction].tag.vec[1];
						tag.vreal64_vec3[2] = event->event[direction].tag.vec[2];
						verse_send_tag_create(UNDOGlobal.g_node, 0, tag_id, event->event[direction].tag.tag, VN_TAG_REAL64_VEC3, &tag);
				/*	}else
					{
						printf("H\n");
						verse_send_tag_group_create(UNDOGlobal.g_node, -1, event->event[direction].tag.group);
						UNDOGlobal.pending_tag = event->event[direction].tag;
						e_ns_get_tag_by_name_and_group(node, event->event[direction].tag.group, event->event[direction].tag.tag, &group_id, &tag_id);
						verse_send_tag_destroy(UNDOGlobal.g_node, group_id, tag_id);
					}
				}else
				{
					printf("I\n");
					verse_send_tag_group_create(UNDOGlobal.g_node, -1, event->event[direction].tag.group);
					UNDOGlobal.pending_tag = event->event[direction].tag;
				}
				printf("J\n");
			}*/

	}
}

void udg_undo_geometry(void)
{
	if(UNDOGlobal.undos > 0)
	{
		uint i;
		i = UNDOGlobal.pos;
		while(UNDOGlobal.event[--UNDOGlobal.pos % UNDOGlobal.length].type != UNDOET_STOPP)
		{
			if(UNDOGlobal.pos == -1)
				UNDOGlobal.pos = UNDOGlobal.length;
			execute_event(&UNDOGlobal.event[UNDOGlobal.pos], 0);
			
		}
	}
	UNDOGlobal.undos--;
	UNDOGlobal.redos++;
}

void udg_redo_geometry(void)
{
	if(UNDOGlobal.redos > 0)
	{
		uint i;
		i = UNDOGlobal.pos;

		while(UNDOGlobal.event[++UNDOGlobal.pos].type != UNDOET_STOPP)
		{
			if(UNDOGlobal.pos == -1)
				UNDOGlobal.pos = UNDOGlobal.length;
			execute_event(&UNDOGlobal.event[UNDOGlobal.pos], 1);
		}
	}
	UNDOGlobal.undos++;
	UNDOGlobal.redos--;
}


UNDOEvent * get_next_undo_event(void)
{
	if(++UNDOGlobal.pos == UNDOGlobal.length)
		UNDOGlobal.pos = 0;
	return &UNDOGlobal.event[UNDOGlobal.pos];
}



boolean udg_check_g_node(uint32 node_id)
{
	ENode *g_node;

	g_node = e_ns_get_node(0, node_id);
	if(g_node != NULL && e_ns_get_node_type(g_node) == V_NT_GEOMETRY)
	{
		EGeoLayer *vertex, *polygon, *crease;
		vertex = e_nsg_get_layer_by_id(g_node, 0),
		polygon = e_nsg_get_layer_by_id(g_node, 1);
		crease = e_nsg_get_layer_crease_edge_layer(g_node);

		if(polygon == NULL || vertex == NULL || crease == NULL)
			return FALSE;
		if(e_nsg_get_layer_data(g_node, vertex) == NULL || 
			e_nsg_get_layer_data(g_node, polygon) == NULL || 
			e_nsg_get_layer_data(g_node, crease) == NULL)
			return FALSE;
	//	if(UNDOGlobal.select_local != TRUE && e_nsg_get_layer_data(g_node, e_nsg_get_layer_by_type(g_node, VN_G_LAYER_VERTEX_REAL, "select")) == NULL)
	//		return FALSE;
		return TRUE;
	}
	return FALSE;
}

boolean udg_update_geometry(void)
{
	ENode *g_node;
	EGeoLayer *layer;
	uint i;

	if((g_node = deceive_get_edit_node()) != NULL)
		UNDOGlobal.g_node = e_ns_get_node_id(g_node);
	if(udg_check_g_node(UNDOGlobal.g_node))
	{
		g_node = e_ns_get_node(0, UNDOGlobal.g_node);
		UNDOGlobal.structure_version = e_ns_get_node_version_struct(g_node);
		UNDOGlobal.ref_length = e_nsg_get_polygon_length(g_node);
		UNDOGlobal.ref = e_nsg_get_layer_data(g_node, e_nsg_get_layer_by_id(g_node, 1));
		layer = e_nsg_get_layer_crease_edge_layer(g_node);
		if(layer != NULL)
		{
			UNDOGlobal.crease = e_nsg_get_layer_data(g_node, layer);
			UNDOGlobal.crease_layer = e_nsg_get_layer_id(layer);	
		}else
		{
			UNDOGlobal.crease = NULL;
			UNDOGlobal.crease_layer = 0;				
		}
		layer = e_nsg_get_layer_by_type(g_node, VN_G_LAYER_VERTEX_REAL, "select");
		if(layer != NULL)
		{
			uint i, new_selection;
			if(UNDOGlobal.select_local == TRUE)
				free(UNDOGlobal.select);
			new_selection = UNDOGlobal.select_version != e_nsg_get_layer_version(layer);
			UNDOGlobal.select_version = e_nsg_get_layer_version(layer);
			UNDOGlobal.select = e_nsg_get_layer_data(g_node, layer);
			UNDOGlobal.select_layer = e_nsg_get_layer_id(layer);
			UNDOGlobal.select_local = FALSE;			
			UNDOGlobal.vertex_length = e_nsg_get_vertex_length(g_node);
		}else
		{
			if(UNDOGlobal.select_local != TRUE)
			{
				if(e_nsg_get_vertex_length(g_node) != 0)
					UNDOGlobal.select = malloc((sizeof *UNDOGlobal.select) * e_nsg_get_vertex_length(g_node));
				else
					UNDOGlobal.select = NULL;
			}
			if(UNDOGlobal.vertex_length != e_nsg_get_vertex_length(g_node))
			{
				if(e_nsg_get_vertex_length(g_node) != 0)
				{
					UNDOGlobal.select = realloc(UNDOGlobal.select, (sizeof *UNDOGlobal.select) * e_nsg_get_vertex_length(g_node));
					for(i = UNDOGlobal.vertex_length; i < e_nsg_get_vertex_length(g_node); i++)
						UNDOGlobal.select[i] = 0;
				}else if(UNDOGlobal.select != NULL)
				{
					free(UNDOGlobal.select);
					UNDOGlobal.select = NULL;
				}
			}
			UNDOGlobal.select_local = TRUE;
		}
		UNDOGlobal.vertex_length = e_nsg_get_vertex_length(g_node);
		layer = e_nsg_get_layer_by_id(g_node, UNDOGlobal.vertex_layer);
		if(layer == NULL)
			layer = e_nsg_get_layer_by_id(g_node, 0);	
		UNDOGlobal.vertex_version = e_nsg_get_layer_version(layer);
		UNDOGlobal.vertex = e_nsg_get_layer_data(g_node, layer);
		UNDOGlobal.vertex_layer = e_nsg_get_layer_id(layer);
		if(UNDOGlobal.select != NULL)
		{
			for(i = 0; i < UNDOGlobal.vertex_length && (UNDOGlobal.select[i] < 0.01 || UNDOGlobal.vertex[i * 3] == V_REAL64_MAX); i++);
			if(la_t_tm_hiden())
			{
				if(i != UNDOGlobal.vertex_length)
				{
					egreal sum = 0, pos[3] = {0, 0, 0};
					for(; i < UNDOGlobal.vertex_length; i++)
					{
						if(UNDOGlobal.select[i] > 0.01 && UNDOGlobal.vertex[i * 3] != V_REAL64_MAX)
						{
							sum += UNDOGlobal.select[i];
							pos[0] += UNDOGlobal.vertex[i * 3] * UNDOGlobal.select[i];
							pos[1] += UNDOGlobal.vertex[i * 3 + 1] * UNDOGlobal.select[i];
							pos[2] += UNDOGlobal.vertex[i * 3 + 2] * UNDOGlobal.select[i];
						}
					}
					la_t_tm_place(pos[0] / sum, pos[1] / sum, pos[2] / sum);
					la_t_tm_hide(FALSE);
				}
			}else if(i == UNDOGlobal.vertex_length)
				la_t_tm_hide(TRUE);
		}
		{
			uint16 group_id, tag_id;
			uint i;
			char *name;
			VNTag *tag;
			UNDOGlobal.tags_version = e_ns_get_node_version_data(g_node);
			i = 0;
			for(group_id = e_ns_get_next_tag_group(g_node, 0); group_id != (uint16)-1; group_id = e_ns_get_next_tag_group(g_node, group_id + 1))
			{
				name = e_ns_get_tag_group(g_node, group_id);
				for(i = 0; UNDOGlobal.pending_tag.group[i] != 0 && UNDOGlobal.pending_tag.group[i] == name[i]; i++);
				if(UNDOGlobal.pending_tag.group[i] == name[i])
				{
					VNTag t;
					t.vreal64_vec3[0] = UNDOGlobal.pending_tag.vec[0];
					t.vreal64_vec3[1] = UNDOGlobal.pending_tag.vec[1];
					t.vreal64_vec3[2] = UNDOGlobal.pending_tag.vec[2];
					verse_send_tag_create(UNDOGlobal.g_node, group_id, -1, UNDOGlobal.pending_tag.tag, VN_TAG_REAL64_VEC3, &t);
					UNDOGlobal.pending_tag.group[0] = 0;
				}
				for(tag_id = e_ns_get_next_tag(g_node, group_id, 0); tag_id != (uint16)-1; tag_id = e_ns_get_next_tag(g_node, group_id, tag_id + 1))
					if(VN_TAG_REAL64_VEC3 == e_ns_get_tag_type(g_node, group_id, tag_id))
						i++;
			}

			if(UNDOGlobal.tag_count != i)
			{
				if(UNDOGlobal.tags != NULL)
					free(UNDOGlobal.tags);
				UNDOGlobal.tags = NULL;
				UNDOGlobal.tag_count = i;
				if(UNDOGlobal.tag_count != 0)
				{
					UNDOGlobal.tags = malloc((sizeof *UNDOGlobal.tags) * UNDOGlobal.tag_count);
					for(i = 0; i < UNDOGlobal.tag_count; i++)
						UNDOGlobal.tags[i].select = 0;
				}
			}
			if(UNDOGlobal.tag_count != 0)
			{	
				UNDOGlobal.tag_count = 0;
				for(group_id = e_ns_get_next_tag_group(g_node, 0); group_id != (uint16)-1; group_id = e_ns_get_next_tag_group(g_node, group_id + 1))
				{
					for(tag_id = e_ns_get_next_tag(g_node, group_id, 0); tag_id != (uint16)-1; tag_id = e_ns_get_next_tag(g_node, group_id, tag_id + 1))
					{
						if(VN_TAG_REAL64_VEC3 == e_ns_get_tag_type(g_node, group_id, tag_id))
						{	
							name = e_ns_get_tag_group(g_node, group_id);
							for(i = 0; name[i] != 0; i++)
								UNDOGlobal.tags[UNDOGlobal.tag_count].group[i] = name[i];
							UNDOGlobal.tags[UNDOGlobal.tag_count].group[i] = 0;
							name = e_ns_get_tag_name(g_node, group_id, tag_id);
							for(i = 0; name[i] != 0; i++)
								UNDOGlobal.tags[UNDOGlobal.tag_count].tag[i] = name[i];
							UNDOGlobal.tags[UNDOGlobal.tag_count].tag[i] = 0;
							tag = e_ns_get_tag(g_node, group_id, tag_id);
							UNDOGlobal.tags[UNDOGlobal.tag_count].vec[0] = tag->vreal64_vec3[0];
							UNDOGlobal.tags[UNDOGlobal.tag_count].vec[1] = tag->vreal64_vec3[1];
							UNDOGlobal.tags[UNDOGlobal.tag_count].vec[2] = tag->vreal64_vec3[2];
							UNDOGlobal.tag_count++;
						}
					}
				}
				
			}
		}
		return TRUE;
	}
	return FALSE;
}

void udg_set_modeling_node(uint32 node_id)
{
	ENode *g_node;
	if((g_node = e_ns_get_node(0, node_id)) != NULL)
	{
		UNDOGlobal.g_node = node_id;
		UNDOGlobal.structure_version = 0;
		UNDOGlobal.vertex_version = 0;
		UNDOGlobal.select_version = 0;
		UNDOGlobal.edge_version = 0;
		UNDOGlobal.tags_version = 0;

		if(e_nsg_get_layer_by_type(g_node, VN_G_LAYER_POLYGON_CORNER_UINT32, "crease") == NULL)
			verse_send_g_layer_create(node_id, -1, "crease", VN_G_LAYER_POLYGON_CORNER_UINT32, 0, 0);
/*		if(e_nsg_get_layer_by_type(g_node, VN_G_LAYER_VERTEX_REAL, "select") == NULL)
			verse_send_g_layer_create(node_id, -2, "select", VN_G_LAYER_VERTEX_REAL, 0, 0);
*/		verse_send_g_crease_set_edge(node_id, "crease", 0);
		udg_update_geometry();
	}
}
uint udg_get_version(boolean	structure, boolean vertex, boolean select, boolean edge, boolean tags)
{
	uint output = 0;
	if(structure)
		output += UNDOGlobal.structure_version;
	if(vertex)
		output += UNDOGlobal.vertex_version;
	if(select)
		output += UNDOGlobal.select_version;
	if(edge)
		output += UNDOGlobal.edge_version;
	if(tags)
		output += UNDOGlobal.tags_version;
	return output;
}

void udg_get_geometry(uint32 *vertex_count, uint32 *polygon_count, double **vertex, uint32 **ref, uint32 **crease)
{
	if(vertex_count != NULL)
		*vertex_count = UNDOGlobal.vertex_length;
	if(polygon_count != NULL)
		*polygon_count = UNDOGlobal.ref_length;
	if(vertex != NULL)
		*vertex = UNDOGlobal.vertex;
	if(ref != NULL)
		*ref = UNDOGlobal.ref;
	if(crease != NULL)
		*crease = UNDOGlobal.crease;
}

void udg_vertex_set(uint32 id, double *state, double x, double y, double z)
{
	UNDOEvent *event;
//	int32 grid;
	UNDOGlobal.group_update = TRUE;
	event = get_next_undo_event();

	if(x != x || y != y || z != z)
	{
		uint i;
		i = 0;	
	}

	event->type = UNDOET_VERTEX;
	event->id = id;

	if(state == NULL)
		event->event[0].vertex.layer = 65535;
	else
		event->event[0].vertex.layer = UNDOGlobal.vertex_layer;
/*	if(UNDOGlobal.grid_active);
	{
		UNDOGlobal.grid_center[0] = 0;
		UNDOGlobal.grid_center[1] = 0;
		UNDOGlobal.grid_center[2] = 0;
		x = (double)((uint)(x / UNDOGlobal.grid_size)) * UNDOGlobal.grid_size;
		y = (double)((uint)(y / UNDOGlobal.grid_size)) * UNDOGlobal.grid_size;
		z = (double)((uint)(z / UNDOGlobal.grid_size)) * UNDOGlobal.grid_size;
	}*/
	event->event[1].vertex.layer = UNDOGlobal.vertex_layer;
	if(id < UNDOGlobal.vertex_length && state != NULL)
	{
		event->event[0].vertex.x = state[0];
		event->event[0].vertex.y = state[1];
		event->event[0].vertex.z = state[2];
	}
	event->event[1].vertex.x = x;
	event->event[1].vertex.y = y;
	event->event[1].vertex.z = z;
/*	if(UNDOGlobal.grid_active)
	{	
		grid = (uint32)(2147483647.0 * UNDOGlobal.grid_size);
		event->event[1].vertex.x = (event->event[1].vertex.x / grid) * grid;
		event->event[1].vertex.y = (event->event[1].vertex.y / grid) * grid;
		event->event[1].vertex.z = (event->event[1].vertex.z / grid) * grid;
	}*/
	execute_event(event, 1);
}
void udg_vertex_move(uint32 id, double x, double y, double z)
{
	if(x != x || y != y || z != z)
	{
		uint i;
		i = 0;	
	}
/*	if(x > 1000 || x < -1000)
	{
		uint *a = NULL;
		*a = 0;
	}
*/	verse_send_g_vertex_set_xyz_real64(UNDOGlobal.g_node, UNDOGlobal.vertex_layer, id, x, y, z);
}


void udg_vertex_delete(uint32 id)
{
	UNDOEvent *event;
	event = get_next_undo_event();
	event->type = UNDOET_VERTEX;
	event->id = id;
	event->event[0].vertex.layer = UNDOGlobal.vertex_layer;
	event->event[1].vertex.layer = 65535;
	event->event[0].vertex.x = UNDOGlobal.vertex[id * 3];
	event->event[0].vertex.y = UNDOGlobal.vertex[id * 3 + 1];
	event->event[0].vertex.z = UNDOGlobal.vertex[id * 3 + 2];
	event->event[1].vertex.x = V_REAL64_MAX;
	event->event[1].vertex.y = V_REAL64_MAX;
	event->event[1].vertex.z = V_REAL64_MAX;
	execute_event(event, 1);
}


void udg_get_vertex_pos(double *pos, uint vertex_id)
{
	if(vertex_id < UNDOGlobal.vertex_length)
	{
		vertex_id *= 3;
		pos[0] = UNDOGlobal.vertex[vertex_id++];
		pos[1] = UNDOGlobal.vertex[vertex_id++];
		pos[2] = UNDOGlobal.vertex[vertex_id++];
	}
}


void udg_polygon_set(uint32 id, uint32 a, uint32 b, uint32 c, uint32 d)
{
	UNDOEvent *event;
	UNDOGlobal.group_update = TRUE;
	event = get_next_undo_event();
	event->type = UNDOET_POLYGON;
	event->id = id;
	if(id < UNDOGlobal.ref_length)
	{
		event->event[0].polygon.a = UNDOGlobal.ref[id * 4];
		event->event[0].polygon.b = UNDOGlobal.ref[id * 4 + 1];
		event->event[0].polygon.c = UNDOGlobal.ref[id * 4 + 2];
		event->event[0].polygon.d = UNDOGlobal.ref[id * 4 + 3];
	}else
		event->event[0].polygon.a = -1;
	event->event[1].polygon.a = a;
	event->event[1].polygon.b = b;
	event->event[1].polygon.c = c;
	event->event[1].polygon.d = d;
	execute_event(event, 1);
}

void udg_polygon_delete(uint32 id)
{
	UNDOEvent *event;
	lo_pfx_wiggle_line_add(UNDOGlobal.vertex[UNDOGlobal.ref[id * 4] * 3 + 0],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4] * 3 + 1],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4] * 3 + 2],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 1] * 3 + 0],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 1] * 3 + 1],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 1] * 3 + 2], TRUE);
	lo_pfx_wiggle_line_add(UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 1] * 3 + 0],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 1] * 3 + 1],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 1] * 3 + 2],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 0],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 1],
								UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 2], TRUE);
	if(UNDOGlobal.ref[id * 4 + 3] < UNDOGlobal.ref_length)
	{
		lo_pfx_wiggle_line_add(UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 0],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 1],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 2],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 3] * 3 + 0],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 3] * 3 + 1],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 3] * 3 + 2], TRUE);
		lo_pfx_wiggle_line_add(UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 3] * 3 + 0],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 3] * 3 + 1],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 3] * 3 + 2],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4] * 3 + 0],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4] * 3 + 1],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4] * 3 + 2], TRUE);
	
	}else
		lo_pfx_wiggle_line_add(UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 0],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 1],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 2] * 3 + 2],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 0] * 3 + 0],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 0] * 3 + 1],
									UNDOGlobal.vertex[UNDOGlobal.ref[id * 4 + 0] * 3 + 2], TRUE);
	if(UNDOGlobal.crease != 0)
	{
		event = get_next_undo_event();
		event->type = UNDOET_CREASE;
		event->id = id;
		event->event[0].crease.a = UNDOGlobal.crease[id * 4];
		event->event[0].crease.b = UNDOGlobal.crease[id * 4 + 1];
		event->event[0].crease.c = UNDOGlobal.crease[id * 4 + 2];
		event->event[0].crease.d = UNDOGlobal.crease[id * 4 + 3];
		event->event[1].crease.a = UNDOGlobal.crease[id * 4];
		event->event[1].crease.b = UNDOGlobal.crease[id * 4 + 1];
		event->event[1].crease.c = UNDOGlobal.crease[id * 4 + 2];
		event->event[1].crease.d = UNDOGlobal.crease[id * 4 + 3];
	}
	event = get_next_undo_event();
	event->type = UNDOET_POLYGON;
	event->id = id;
	event->event[0].polygon.a = UNDOGlobal.ref[id * 4];
	event->event[0].polygon.b = UNDOGlobal.ref[id * 4 + 1];
	event->event[0].polygon.c = UNDOGlobal.ref[id * 4 + 2];
	event->event[0].polygon.d = UNDOGlobal.ref[id * 4 + 3];
	event->event[1].polygon.a = -1;
	event->event[1].polygon.b = -1;
	event->event[1].polygon.c = -1;
	event->event[1].polygon.d = -1;
	{
		double pos_a[3];
		double pos_b[3];
		double pos_c[3];
//		printf("polygon %u %u %u %u \n", event->event[0].polygon.a, event->event[0].polygon.b, event->event[0].polygon.c, event->event[0].polygon.d);
		udg_get_vertex_pos(pos_a, event->event[0].polygon.a);
		udg_get_vertex_pos(pos_b, event->event[0].polygon.b);
//		la_pfx_create_dust_line(pos_a, pos_b);
		udg_get_vertex_pos(pos_c, event->event[0].polygon.c);
//		la_pfx_create_dust_line(pos_c, pos_b);
//		printf("vertex %f %f %f\n", pos_a[0], pos_a[1], pos_a[2]);
//		printf("vertex %f %f %f\n", pos_b[0], pos_b[1], pos_b[2]);
//		printf("vertex %f %f %f\n", pos_c[0], pos_c[1], pos_c[2]);

		if(event->event[0].polygon.d < UNDOGlobal.vertex_length)
		{
			udg_get_vertex_pos(pos_b, event->event[0].polygon.d);
//			printf("vertex %f %f %f\n", pos_b[0], pos_b[1], pos_b[2]);
//			la_pfx_create_dust_line(pos_c, pos_b);
//			la_pfx_create_dust_line(pos_a, pos_b);
		}//else
//			la_pfx_create_dust_line(pos_c, pos_a);
	}
	UNDOGlobal.vertex_delete = TRUE;
	execute_event(event,1);
}

void udg_crease_set(uint32 id, uint32 a, uint32 b, uint32 c, uint32 d)
{
	UNDOEvent *event;
	if(UNDOGlobal.crease == NULL)
		return;
	UNDOGlobal.group_update = TRUE;
	event = get_next_undo_event();
	event->type = UNDOET_CREASE;
	event->id = id;
	if(id < UNDOGlobal.ref_length)
	{
		event->event[0].crease.a = UNDOGlobal.crease[id * 4];
		event->event[0].crease.b = UNDOGlobal.crease[id * 4 + 1];
		event->event[0].crease.c = UNDOGlobal.crease[id * 4 + 2];
		event->event[0].crease.d = UNDOGlobal.crease[id * 4 + 3];
	}else
	{
		event->event[0].crease.a = 0;
		event->event[0].crease.b = 0;
		event->event[0].crease.c = 0;
		event->event[0].crease.d = 0;
	}
	event->event[1].crease.a = a;
	event->event[1].crease.b = b;
	event->event[1].crease.c = c;
	event->event[1].crease.d = d;
	execute_event(event, 1);
}

void udg_create_tag(double *vec)
{
	ENode *g_node;
	UNDOEvent *event;
	char *name = "group"; 
	uint16 group_id;
	uint i;
	event = get_next_undo_event();
	event->type = UNDOET_TAG_CREATE;
	event->id = -1;

	event->event[0].tag.group[0] = 0;
	event->event[0].tag.tag[0] = 0;
	
	g_node = e_ns_get_node(0, UNDOGlobal.g_node);
	if((group_id = e_ns_get_next_tag_group(g_node, 0)) != (uint16)-1)
		name = e_ns_get_tag_group(g_node, group_id);

	for(i = 0; name[i] != 0; i++)
		event->event[1].tag.group[i] = name[i];
	event->event[1].tag.group[i] = 0;
	name = "tag";
	for(i = 0; name[i] != 0; i++)
		event->event[1].tag.tag[i] = name[i];
	event->event[1].tag.tag[i] = 0;
	event->event[1].tag.vec[0] = vec[0];
	event->event[1].tag.vec[1] = vec[1];
	event->event[1].tag.vec[2] = vec[2];
	event->event[1].tag.select = 0;
	execute_event(event, 1);
}

void udg_destroy_tag(uint tag)
{
	UNDOEvent *event;
	event = get_next_undo_event();
	event->type = UNDOET_TAG_CREATE;
	event->id = -1;
	event->event[1].tag.group[0] = 0;
	event->event[1].tag.tag[0] = 0;
	event->event[0].tag = UNDOGlobal.tags[tag];
	execute_event(event, 1);
}

void udg_move_tag(uint tag, double *vec)
{
	UNDOEvent *event;
	event = get_next_undo_event();
	event->type = UNDOET_TAG_MOVE;
	event->id = tag;
	event->event[0].tag = UNDOGlobal.tags[tag];
	event->event[1].tag = UNDOGlobal.tags[tag];
	event->event[1].tag.vec[0] = vec[0];
	event->event[1].tag.vec[1] = vec[1];
	event->event[1].tag.vec[2] = vec[2];
	execute_event(event, 1);
}

/*	UNDOGlobal.tag_type_in_tag = -1;
	UNDOGlobal.tag_type_in_group = -1;
	UNDOGlobal.tag_type_in_text[0] = 0;
	UNDOGlobal.tag_type_in_cursor = 0;
*/
void udg_rename_tag_func(void *user, boolean cancel)
{
	UNDOEvent *event;
	uint i;
	if(cancel || UNDOGlobal.tag_type_in_text[0] == 0)
		return;
	event = get_next_undo_event();
	event->type = UNDOET_TAG_NAME;
	event->id = -1;
	event->event[1].tag = UNDOGlobal.tags[UNDOGlobal.tag_type_in_tag];
	event->event[0].tag = UNDOGlobal.tags[UNDOGlobal.tag_type_in_tag];
	for(i = 0; UNDOGlobal.tag_type_in_text[i] != 0; i++)
		event->event[1].tag.tag[i] = UNDOGlobal.tag_type_in_text[i];
	event->event[1].tag.tag[i] = 0;
	execute_event(event, 1);
}

void udg_rename_group_func(void *user, boolean cancel)
{
	UNDOEvent *event;
	uint i;
	if(cancel || UNDOGlobal.tag_type_in_text[0] == 0)
		return;
	event = get_next_undo_event();
	event->type = UNDOET_TAG_GROUP;
	event->id = -1;
	event->event[1].tag = UNDOGlobal.tags[UNDOGlobal.tag_type_in_tag];
	event->event[0].tag = UNDOGlobal.tags[UNDOGlobal.tag_type_in_tag];
	for(i = 0; UNDOGlobal.tag_type_in_text[i] != 0; i++)
		event->event[1].tag.group[i] = UNDOGlobal.tag_type_in_text[i];
	event->event[1].tag.group[i] = 0;
	execute_event(event, 1);
}

void udg_rename_tag(uint tag)
{
//	uint i;
//	for(i = 0; event->event[tag].tag.tag[i] != 0; i++)
//		type_in_text[i] = event->event[tag].tag.tag[i];
	UNDOGlobal.tag_type_in_cursor = 0;
	UNDOGlobal.tags[tag].tag[0] = 0;
//	betray_start_type_in(UNDOGlobal.tags[tag].tag, 16, udg_rename_tag_func, &UNDOGlobal.tag_type_in_cursor, NULL);
}
void udg_rename_group(uint tag)
{
//	uint i;
//	for(i = 0; event->event[tag].tag.tag[i] != 0; i++)
//		type_in_text[i] = event->event[tag].tag.tag[i];
	UNDOGlobal.tag_type_in_cursor = 0;
	UNDOGlobal.tags[tag].group[0] = 0;
//	betray_start_type_in(UNDOGlobal.tags[tag].group, 16, udg_rename_tag_func, &UNDOGlobal.tag_type_in_cursor, NULL);
}
extern void		betray_end_type_in_mode(boolean cancel);

void udg_select_tag(uint tag, double select)
{
/*	UNDOEvent *event;
	event = get_next_undo_event();
	event->type = UNDOET_TAG_SELECT;
	event->id = -1;
	event->event[1].tag = UNDOGlobal.tags[tag];
	event->event[0].tag = UNDOGlobal.tags[tag];
	event->event[1].tag.select = select;
	execute_event(event, 1);
*/
	UNDOGlobal.tags[tag].select = select;
}

void udg_end_event(void)
{
	if(UNDOGlobal.event[UNDOGlobal.pos].type == UNDOET_STOPP)
		return;
	if(++UNDOGlobal.pos == UNDOGlobal.length);
		UNDOGlobal.pos = 0;
	UNDOGlobal.event[UNDOGlobal.pos].type = UNDOET_STOPP;
}

void udg_set_select(uint vertex, double value)
{
	UNDOEvent *event;
	if(vertex < UNDOGlobal.vertex_length && UNDOGlobal.vertex[vertex * 3] != V_REAL64_MAX)
	{
		event = get_next_undo_event();
		event->type = UNDOET_SELECT;
		event->id = vertex;
		event->event[0].select = UNDOGlobal.select[vertex];
		event->event[1].select = value;
		execute_event(event, 1);
	}
}

double udg_get_select(uint vertex)
{
	return UNDOGlobal.select[vertex];
}

void udg_clear_select(double value)
{
	uint i;
	for(i = 0; i < UNDOGlobal.vertex_length; i++)
		udg_set_select(i, value);
	for(i = 0; i < UNDOGlobal.tag_count; i++)
		UNDOGlobal.tags[i].select = 0;
}

uint32 udg_find_empty_slot_vertex(void)
{
	UNDOGlobal.slot[0] = e_nsg_find_empty_vertex_slot(e_ns_get_node(0, UNDOGlobal.g_node), UNDOGlobal.slot[0] + 1);

	return UNDOGlobal.slot[0];	
}

uint32 udg_find_empty_slot_polygon(void)
{
	UNDOGlobal.slot[1] = e_nsg_find_empty_polygon_slot(e_ns_get_node(0, UNDOGlobal.g_node), UNDOGlobal.slot[1] + 1);
	return UNDOGlobal.slot[1];	
}

void udg_create_edge(uint vertex0, uint vertex1)
{
	UNDOEvent *event;
	event = get_next_undo_event();
	event->type = UNDOET_EDGE;
	event->id = UNDOGlobal.edge_count;
	event->event[0].edge.a = -1;
	event->event[0].edge.b = -1;
	event->event[1].edge.a = vertex0;
	event->event[1].edge.b = vertex1;
/*	if(UNDOGlobal.edge_count == UNDOGlobal.edge_alocated)
	{
		UNDOGlobal.edge_alocated += 16; 
		UNDOGlobal.edges = realloc(UNDOGlobal.edges, (sizeof *UNDOGlobal.edges) * 2 * UNDOGlobal.edge_alocated);
	}
	UNDOGlobal.edges[UNDOGlobal.edge_count * 2] = vertex0;
	UNDOGlobal.edges[UNDOGlobal.edge_count * 2 + 1] = vertex1;
	UNDOGlobal.edge_count++;
	UNDOGlobal.edge_version++;*/
	execute_event(event, 1);
}

uint *udg_get_edge_data(uint *count)
{
	*count = UNDOGlobal.edge_count;
	return UNDOGlobal.edges;
}

void udg_destroy_edge(uint id)
{

	if(UNDOGlobal.edge_count > id && UNDOGlobal.edge_count != 0)
	{
		UNDOEvent *event;
		event = get_next_undo_event();
		event->type = UNDOET_EDGE;
		event->id = id;
		event->event[0].edge.a = UNDOGlobal.edges[id * 2];
		event->event[0].edge.b = UNDOGlobal.edges[id * 2 + 1];
		event->event[1].edge.a = -1;
		event->event[1].edge.b = -1;
		lo_pfx_wiggle_line_add(UNDOGlobal.vertex[UNDOGlobal.edges[id * 2] * 3 + 0],
								UNDOGlobal.vertex[UNDOGlobal.edges[id * 2] * 3 + 1],
								UNDOGlobal.vertex[UNDOGlobal.edges[id * 2] * 3 + 2],
								UNDOGlobal.vertex[UNDOGlobal.edges[id * 2 + 1] * 3 + 0],
								UNDOGlobal.vertex[UNDOGlobal.edges[id * 2 + 1] * 3 + 1],
								UNDOGlobal.vertex[UNDOGlobal.edges[id * 2 + 1] * 3 + 2], TRUE);
		execute_event(event, 1);
	}
}

void udg_destroy_all_edges(void)
{
	UNDOGlobal.edge_count = 0;
	UNDOGlobal.edge_version++;
}

void udg_set_distance(uint vertex_a, uint vertex_b)
{
	double a[3], b[3];
	udg_get_vertex_pos(a, vertex_a);
	udg_get_vertex_pos(b, vertex_b);
	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
	UNDOGlobal.distance = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

double udg_get_distance(void)
{
	return UNDOGlobal.distance;
}

void undo_event_done(void)
{
	if(UNDOGlobal.event[UNDOGlobal.pos].type == UNDOET_STOPP)
		return;
/*	if(UNDOGlobal.vertex_delete)
	{
		uint *array, i;
		array = malloc((sizeof *array) * UNDOGlobal.vertex_length);
		for(i = 0; i < UNDOGlobal.vertex_length; i++)
			array[i] = 0;
		for(i = 0; i < UNDOGlobal.edge_count; i++)
		{
			array[UNDOGlobal.edges[i * 2]]++;
			array[UNDOGlobal.edges[i * 2 + 1]]++;
		}
		for(i = 0; i < UNDOGlobal.ref_length; i++)
		{
			if(UNDOGlobal.ref[i * 4] < UNDOGlobal.vertex_length && UNDOGlobal.ref[i * 4 + 1] < UNDOGlobal.vertex_length && UNDOGlobal.ref[i * 4 + 2] < UNDOGlobal.vertex_length)
			{
				array[UNDOGlobal.ref[i * 4]]++;
				array[UNDOGlobal.ref[i * 4 + 1]]++;
				array[UNDOGlobal.ref[i * 4 + 2]]++;
				if(UNDOGlobal.ref[i * 4 + 3] < UNDOGlobal.vertex_length)
					array[UNDOGlobal.ref[i * 4 + 3]]++;
			}
		}
		i = UNDOGlobal.pos;
		while(UNDOGlobal.event[--i].type != UNDOET_STOPP)
		{
			if(i == -1)
				i = UNDOGlobal.length;
			if(UNDOGlobal.event[i].type == UNDOET_POLYGON && UNDOGlobal.event[i].event[1].polygon.a == -1 && UNDOGlobal.ref[UNDOGlobal.event[i].id * 4] < UNDOGlobal.vertex_length)
			{
				if(UNDOGlobal.event[i].event[0].polygon.a < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[0].polygon.a]--;
				if(UNDOGlobal.event[i].event[0].polygon.b < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[0].polygon.b]--;
				if(UNDOGlobal.event[i].event[0].polygon.c < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[0].polygon.c]--;
				if(UNDOGlobal.event[i].event[0].polygon.d < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[0].polygon.d]--;
				if(UNDOGlobal.event[i].event[1].polygon.a < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[1].polygon.a]++;
				if(UNDOGlobal.event[i].event[1].polygon.b < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[1].polygon.b]++;
				if(UNDOGlobal.event[i].event[1].polygon.c < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[1].polygon.c]++;
				if(UNDOGlobal.event[i].event[1].polygon.d < UNDOGlobal.vertex_length)
					array[UNDOGlobal.event[i].event[1].polygon.d]++;
			}
		}
		for(i = 0; i < UNDOGlobal.vertex_length; i++)
			if(array[i] == 0 && UNDOGlobal.vertex[i * 3] != V_REAL64_MAX)
				udg_vertex_delete(i);
		free(array);
	}*/
	if(++UNDOGlobal.pos == UNDOGlobal.length)
		UNDOGlobal.pos = 0;
	UNDOGlobal.event[UNDOGlobal.pos].type = UNDOET_STOPP;
	UNDOGlobal.undos++;
	UNDOGlobal.redos = 0;
	UNDOGlobal.vertex_delete = FALSE;
	if(UNDOGlobal.group_update)
		la_t_group_store();
	UNDOGlobal.group_update = FALSE;
}

boolean	udg_get_reference_geometry(uint32 *vertex_count, double **vertex)
{
//	UNDOGlobal.reference_node
	// ENode *		e_ns_get_node(0, UNDOGlobal.reference_node);
	ENode *node;
	node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY);
	if(node != NULL)
	{
		EGeoLayer *layer;
		layer = e_nsg_get_layer_by_id(node, 0);
		if(layer != NULL)
		{
			*vertex = e_nsg_get_layer_data(node, layer);
			*vertex_count = e_nsg_get_vertex_length(node);
			return TRUE;
		}
	}
	return FALSE;
}

uint udg_get_reference_version()
{
	ENode *node;
	node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY);
	if(node != NULL)
	{
		EGeoLayer *layer;
		layer = e_nsg_get_layer_by_id(node, 0);
		if(layer != NULL)
			return e_nsg_get_layer_version(layer);
	}
	return 0;
}
