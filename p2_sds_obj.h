typedef struct{
	uint32	material;
	uint32  material_version;
	uint	render_end;
	uint	quad_end;
	uint	tri_end;
	uint	vertex_start;
	uint	id;
	uint16	layer;
}PMeshMaterial;

typedef struct{
	uint	version;
	pgreal	*data;
	pgreal	weight[3];
}PMeshLayer;

typedef enum{
	PDM_NONE, /* no displacement mapping */
	PDM_NORMALIZED, /* normalized propperly */
	PDM_QUICK, /* only normalized once, fast for animation */
	PDM_DYNAMIC, /* inclues */
}PDispMode;

typedef enum{
	PAN_NONE,
	PAN_PLAY,
	PAN_STOPPED,
	PAN_PLAY_BUT_BROKEN
}PAnimMode;

typedef struct{
	uint geometry_id;
	uint geometry_version;
	uint stage;
	uint sub_stages[4];
	void *temp;
	void *next;
	struct{
		void **tess; /* the selected tesselation */
		uint32 *order_node; /* how the render base polys refers to the geometry node polys */
		uint32 *order_temp_mesh; /* how the PMesh polys refers to the geometry mesh polygs (deleted once used)*/
		uint32 *order_temp_mesh_rev; /* how the geometry mesh polys refers to the PMesh polys (deleted once used)*/
		uint32 *order_temp_poly_start; /* start of each polygons vertices*/
		uint tri_count; /* number of control tris */
		uint quad_count; /* number of control quads */
		pgreal factor;
		uint force;
		pgreal eye[3];
		pgreal (*edge_tess_func)(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay);
	}tess;
	struct{
		pgreal *vertex_array; /* the subdivided vertex array */
		pgreal *normal_array; /* the subdivided normal array */
		uint32 vertex_count; /* the number of subdivided vertex array */
		uint32 *reference; /* the final reference array */
		uint32 element_count; /* the length of the reference array */
		PMeshMaterial *mat; /*the materials and their ranges */
		uint mat_count; /* number of ranges */
		boolean open_edges;
		boolean shadows;
	}render;
	struct{
		uint32	*reference;
		pgreal	*weight;
		uint32	*ref_count;
		uint	length;
		uint	length_temp;
		uint	length_temp2;
		uint	length_temp3;
	}depend;
	struct{
		uint32	*normal_ref;	
		pgreal	*normals;
	//	pgreal	*displacement;
	//	PDispMode mode;
		pgreal	*draw_normals;
	}normal;
	struct{
		void		*array;
		uint		array_count;
		uint		*version;
		uint		data_version;
	}param;
}PMesh;

extern pgreal	p_sds_edge_tesselation_global_func(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay);
extern pgreal	p_sds_edge_tesselation_local_func(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay);

/* p_sds_obj.c */

extern void		p_lod_compute_vertex_array(pgreal *vertex, uint output_stride, uint vertex_count, const uint *ref_count, const uint *reference,  const pgreal *weight, const pgreal *cvs);
extern void		p_lod_compute_normal_array(pgreal *normals, uint normal_stride, uint vertex_count, const uint *normal_ref, const pgreal *vertex, uint vertex_stride);
extern void		p_lod_compute_displacement_array(pgreal *vertex, uint vertex_count, const pgreal *normals, const pgreal *displacement);

/* p_sds_obj_edge.c*/

extern void		p_lod_create_normal_ref_and_shadow_skirts(PPolyStore *smesh, PMesh *mesh);

/* p_sds_obj_displace.c */

//extern void		p_lod_create_displacement_array(ENode *g_node, ENode *o_node, PMesh *mesh, uint base_level);
//extern void		p_lod_update_displacement_array(ENode *g_node, ENode *o_node, PMesh *mesh, uint base_level);

/* p_sds_obj_sort.c */

//extern void		p_lod_gap_count(ENode *node, PPolyStore *smesh, PMesh *mesh, ENode *o_node);

/* p_sds_obj_tess.c */


//extern void		p_lod_set_view_pos(double *view_cam);
//extern pgreal	p_lod_compute_lod_level(ENode *o_node, ENode *g_node, uint32 time_s, uint32 time_f);
//extern boolean	p_lod_compute_lod_update(ENode *o_node, ENode *g_node, uint32 time_s, uint32 time_f, pgreal factor);


extern void		p_lod_select_tesselation(PMesh *mesh, PPolyStore *smesh, pgreal *cvs);

/* p_sds_obj_param.c */

//extern void		p_lod_create_layer_param(ENode *g_node, PMesh *mesh);
//extern boolean	p_lod_update_layer_param(ENode *g_node, PMesh *mesh);

//extern boolean p_lod_update_shadow(ENode *g_node, PMesh *mesh);

/* p_sds_obj_edge_normal.c */

extern void		p_lod_compute_vertex_normals(PPolyStore *smesh, PMesh *mesh);

/* p_sds_obj_anim.c */

//extern boolean	p_lod_anim_bones_update_test(PMesh *mesh, ENode *o_node, ENode *g_node);
//extern boolean	p_lod_anim_scale_update_test(PMesh *mesh, ENode *o_node);
//extern boolean	p_lod_anim_layer_update_test(PMesh *mesh, ENode *o_node, ENode *g_node);

//extern void		p_lod_anim_vertex_array(pgreal *vertex, uint cv_count, PMesh *mesh, ENode *g_node);



//extern double p_anim_evaluate_anim(ENode *o_node, double *output, uint seconds, uint fractions, char *name, double default_value);
//extern void p_anim_vertex_array(pgreal *vertex, uint cv_count, PMesh *mesh, ENode *g_node);
