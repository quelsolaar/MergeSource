#define MAX_TESS_TABLE_GENS_PER_COPMPUTE 1000000

typedef enum {
    TESS_POLYGON_TRI = 3,
	TESS_POLYGON_QUAD = 4
} TessPolygonType;

typedef struct{
	TessPolygonType type; /*tri or quad */
	pgreal			corner[4][4]; /* vertex number , andt the values it is combined by*/
	uint			edge[4]; /* what edge is it +5 = internal edge */
	uint			level_of_edge[4]; /* the tesselation level of the edges */ 
}weight_polygon;

typedef struct{
	pgreal vertical0;
	pgreal horizontal0;
	pgreal vertical1;
	pgreal horizontal1;
	pgreal vertical2;
	pgreal horizontal2;
	pgreal vertical3;
	pgreal horizontal3;
}weight_square;


#define	TESSELATION_TABLE_MAX_LEVEL 8

typedef struct{	
	pgreal	*vertex_influence; /*[array] vertex number , andt the values it is combined by in series of 3 or 4*/
	uint	*index; /*[array] the reference offset for the final index */
	uint	*reference; /*[array] the vertex order of ref form the rop data */
	uint	*normals;
	uint	edges[5];
	uint	vertex_count; /* number of vertex in the tesselated polygon */
	uint	element_count; /* number of elements in the tesselated polygon */
}PTessTableElement;


extern void p_sds_ts_init_weight_polygon(TessPolygonType type, weight_polygon *polygon);
extern void p_sds_ts_copy_vertex(weight_polygon *read, pgreal *target, uint vertex1);
extern void p_sds_ts_divide_edge(weight_polygon *read, pgreal *target, uint vertex1, uint vertex2);
extern void p_sds_ts_create_middel_vertex(weight_polygon *read, pgreal *target);
extern void p_sds_ts_tesselate_weight_polygon(weight_polygon *read, weight_polygon *write, uint level);
extern uint p_sds_ts_corner_split(weight_polygon *polygon, weight_square *square, uint tess0, uint tess1);
extern uint p_sds_ts_middle_split(weight_polygon *polygon, weight_square *square, uint h_tess, uint v_tess);
extern uint p_sds_ts_edge_split(weight_polygon *polygon, weight_square *square, uint tess_0, uint tess_1);
extern uint p_sds_ts_pow_level(uint level);
extern uint p_sds_ts_lowest_level(uint level0, uint level1);
extern uint p_sds_ts_init_weight_quad(weight_polygon *polygon, uint level3, uint level2, uint level1, uint level0);
extern uint p_sds_ts_calculate_tri_polyon_count(uint edge_level0, uint edge_level1, uint edge_level2);
extern void p_sds_ts_split_polygon(weight_polygon *read, weight_polygon *write, uint edge);
extern weight_polygon *p_sds_ts_split_tri_edges(weight_polygon *read, weight_polygon *write1, uint in_count, uint out_count, uint *level);

extern uint p_get_max_tess_level(void);
extern PTessTableElement *get_dynamic_table_tri(uint base_level, uint *edge);
extern PTessTableElement *get_dynamic_table_quad(uint base_level, uint *edge);

extern uint get_dynamic_table_tri_level(uint base_level, PTessTableElement *table, uint edge);
extern uint get_dynamic_table_quad_level(uint base_level, PTessTableElement *table, uint edge);

void p_geo_table_sort_edges(PTessTableElement *t, uint *edges, uint corners);
void p_geo_table_edge_count(PTessTableElement *t, uint *edges, uint corners);

void p_geo_table_gen_normals(PTessTableElement *t, uint corners);

