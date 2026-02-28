

typedef struct{
	pgreal						value;
	uint32						vertex;
}PDependElement;

typedef struct{
	uint16			length;
	pgreal			sum;
	PDependElement	*element;
}PDepend;

extern void		p_sds_add_depend(PDepend *dep, PDepend *add, pgreal mult);
 extern PDepend	*p_sds_allocate_depend_first(uint length);
extern PDepend	*p_sds_allocate_depend(uint length);
extern void		p_sds_free_depend(PDepend *dep, uint length);

typedef struct{
	uint *ref;
	uint *neighbor;
	pgreal *crease;
	uint tri_length;
	uint quad_length;			
	uint *base_neighbor;	
	uint base_tri_count;
	uint base_quad_count;
	uint poly_per_base;
	uint open_edges;
	PDepend *vertex_dependency;
	uint vertex_dependency_length;
	uint vertex_count;
	uint control_vertex_count;
	uint geometry_version;
	void *next;	
	uint stage[2];
	uint level;
	uint version;
}PPolyStore;

extern PPolyStore	*p_sds_create(uint *ref, uint ref_count, pgreal *vertex, uint vertex_count, uint version);
extern void			p_sds_stage_count_poly(PPolyStore *mesh, uint *ref, uint ref_count, pgreal *vertex, uint vertex_count, pgreal default_crease);
extern void			p_sds_stage_clean_poly(PPolyStore *mesh, uint *ref, uint ref_count, pgreal *vertex, uint vertex_count);
extern void			p_sds_stage_clean_poly_cerease(PPolyStore *mesh, uint *ref, uint ref_count, pgreal *vertex, uint vertex_count, uint *crease);
extern void			p_sds_compute_neighbor(PPolyStore *mesh);
extern PPolyStore	*p_sds_allocate_next(PPolyStore *mesh);
extern void			p_sds_free(PPolyStore *mesh, boolean limited);
extern void			p_sds_final_clean(PPolyStore *mesh);

extern void			sds_test_draw(PPolyStore *mesh, pgreal *vertex);

extern PPolyStore	*compute(PPolyStore *mesh, uint *ref, uint ref_count, pgreal *vertex, uint vertex_count, uint32 *edge_crease, uint version);
extern void			sds_2_test(void);

extern pgreal		p_sds_get_crease(PPolyStore *mesh, uint edge);
extern uint			p_sds_get_corner_next(PPolyStore *mesh, uint corner, int move);
extern uint			p_sds_get_middle(PPolyStore *old_mesh, uint poly);
extern void			p_sds_add_polygon(PPolyStore *old_mesh, PDepend *dep, uint poly, pgreal weight);
extern uint			p_sds_get_edge(PPolyStore *old_mesh, uint edge);
extern float		p_sds_divide(PPolyStore *mesh);

//extern PPolyStore	*p_sds_get_mesh(ENode *node);
