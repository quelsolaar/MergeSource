
typedef struct{
	union{
		pgreal *real;
		uint32 *integer;
		uint	buffer_id;
	}array;
	uint length;
}PRenderArray;

extern void		p_array_init(void);

extern pgreal	*p_ra_get_array_real(PRenderArray *array, uint length);
extern uint		*p_ra_get_array_integer(PRenderArray *array, uint length);
extern void		p_ra_set_array_real(PRenderArray *array, pgreal *data, uint length);
extern void		p_ra_set_array_integer(PRenderArray *array, uint *data, uint length);

extern void		p_ra_clear_array(PRenderArray *array);
extern void		p_ra_free_array(PRenderArray *array);
extern void		p_ra_bind_vertex_array(PRenderArray *array);
extern void		p_ra_bind_normal_array(PRenderArray *array);
extern void		p_ra_bind_param_array(PRenderArray *array, uint id);
extern void		p_ra_unbind_param_array(uint count);
extern void		p_ra_bind_uv_array(PRenderArray *array);
extern void		p_ra_bind_color_array(PRenderArray *array);
extern void		p_ra_bind_reference_array(PRenderArray *array);
extern void		p_ra_draw(uint start, uint end);
