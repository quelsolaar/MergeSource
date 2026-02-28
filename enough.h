/*
 * This is the header for the Enough library. Enough is a storage API written on top of Verse;
 * it saves application authors from having to do the work of defining and managing data structures
 * to hold a "mirror" of a Verse host's contents. Note that Enough does not contain any functions
 * for sending data to the host, use the plain Verse API for that.
*/

#if !defined(ENOUGH_H)
#define	ENOUGH_H
#include "verse.h"
#include "forge.h"
#include "e_types.h"

#define E_CDC_COUNT 16

typedef enum{
	E_CDC_CREATE,
	E_CDC_STRUCT,
	E_CDC_DATA,
	E_CDC_DESTROY
}ECustomDataCommand;

/* verse connection ----------------------------------------------------------------------------------------------------------------*/


extern void enough_init(void);

extern uint		e_vc_connect(const char *server_address, const char *name, const char *pass, const uint8 *host_id);
extern boolean	e_vc_check_connected(void);
extern boolean	e_vc_check_connected_slot(uint	connection);
extern boolean	e_vc_check_accepted_slot(uint connection);
extern char *	e_vc_check_rejected_slot(uint connection);

extern void		e_vc_disconnect(uint	connection);
extern void		e_vc_disconnect_all(void);
extern void		e_vc_set_current_active_connection(uint connection);
extern void		e_vc_connection_update(uint connection, uint time);
extern void		e_vc_set_auto_subscribe(VNodeType type, boolean set);

/* verse node storage ----------------------------------------------------------------------------------------------------------------*/

typedef void ENode;

extern ENode *		e_ns_get_node(uint connection, uint node_id);
extern ENode *		e_ns_get_node_next(uint id, uint connection, VNodeType type);
extern ENode *		e_ns_get_node_avatar(uint connection);
extern ENode *		e_ns_get_node_link(const ENode *parent, uint node_id);

extern uint			e_ns_get_node_count(uint connection, VNodeType type);
extern boolean		e_search_node(ENode *node, char *search);

extern uint			e_ns_get_node_id(const ENode *node);
extern VNodeType	e_ns_get_node_type(const ENode *node);
extern VNodeOwner	e_ns_get_node_owner(const ENode *node);
extern char *		e_ns_get_node_name(ENode *node);
extern uint			e_ns_get_node_connection(const ENode *node);

extern uint			e_ns_get_node_version_struct(const ENode *node);
extern uint			e_ns_get_node_version_data(const ENode *node);
extern void			e_ns_update_node_version_struct(ENode *node);
extern void			e_ns_update_node_version_data(ENode *node);
extern uint			e_ns_get_global_version(uint connection, VNodeType type);

extern void			e_ns_set_custom_data(ENode *node, uint slot, void *data);
extern void			e_ns_set_custom_func(uint slot, VNodeType type, void (*func)(ENode *node, ECustomDataCommand command));
extern void *			e_ns_get_custom_data(ENode *node, uint slot);

extern ENode *		e_ns_get_node_selected(uint connection, VNodeType type);
extern void			e_ns_set_node_selected(uint connection, uint id, VNodeType type);
extern uint			e_ns_get_node_selected_id(uint connection, VNodeType type);
extern void			e_ns_set_node_create_func(void (* func)(uint connection, uint id, VNodeType type, void *user), void *user);

extern char *			e_ns_get_tag_group(const ENode *node, uint16 group_id);
extern uint16			e_ns_get_next_tag_group(const ENode *node, uint16 group_id);
extern uint16			e_ns_get_next_tag(const ENode *node, uint16 group_id, uint16 tag_id);
extern char *			e_ns_get_tag_name(const ENode *node, uint16 group_id, uint16 tag_id);
extern VNTagType	e_ns_get_tag_type(const ENode *node, uint16 group_id, uint16 tag_id);
extern VNTag *		e_ns_get_tag(const ENode *node, uint16 group_id, uint16 tag_id);

extern void			e_ns_get_tag_by_name(const ENode *node, char *name, uint16 *group_id, uint16 *tag_id);
extern VNTag		*e_ns_get_tag_by_type(const ENode *node, char *name, VNTagType type, uint16 *group_id, uint16 *tag_id);
extern uint16		e_ns_get_group_by_name(const ENode *node, char *group_name);
extern VNTag		*e_ns_get_tag_by_name_and_group(const ENode *node, char *group_name, char *tag_name, uint16 *group_id, uint16 *tag_id, VNTagType *tag_type);
extern VNTag		*e_ns_get_tag_by_type_and_group(const ENode *node, char *group_name, char *tag_name, VNTagType type, uint16 *group_id, uint16 *tag_id);

extern void e_matrix_to_quaternionsf(float *matrix, VNQuat32 *quaternion);
extern void e_matrix_to_quaternionsd(double *matrix, VNQuat64 *quaternion);
extern void e_quaternions_to_matrixf(float *matrix, VNQuat32 *quaternion);
extern void e_quaternions_to_matrixd(double *matrix, VNQuat64 *quaternion);

/* object node storage ----------------------------------------------------------------------------------------------------------------*/

typedef void EObjLink;

extern void			e_nso_get_pos(ENode *node, double *pos, double *speed, double *accelerate, double *drag_normal, double *drag, uint32 *time);
extern void			e_nso_get_rot(ENode *node, VNQuat64 *rot, VNQuat64 *speed, VNQuat64 *accelerate, VNQuat64 *drag_normal, double *drag, uint32 *time);
extern void			e_nso_get_pos_time(ENode *node, double *pos, uint32 time_s, uint32 time_f);
extern void			e_nso_get_rot_time(ENode *node, VNQuat64 *rot, uint32 time_s, uint32 time_f);
extern void			e_nso_get_scale(ENode *node, double *scale);
extern void			e_nso_get_rot_matrix(ENode *node, double *matrix, uint32 time_s, uint32 time_f);
extern void			e_nso_get_matrix(ENode *node, double *matrix, uint32 time_s, uint32 time_f);

extern void			e_nso_get_light(ENode *node, double *light);

extern EObjLink *	e_nso_get_link(ENode *node, uint16  id);
extern EObjLink *	e_nso_get_next_link(ENode *node, uint16  id);
extern uint16		e_nso_get_link_id(EObjLink *link);
extern VNodeID		e_nso_get_link_node(EObjLink *link);
extern char *		e_nso_get_link_name(EObjLink *link);
extern uint32		e_nso_get_link_target_id(EObjLink *link);

extern void			e_nso_get_anim_time(EObjLink *link, uint32 *time_s, uint32 *time_f);

extern void			e_nso_get_anim_pos(EObjLink *link, double *pos);
extern void			e_nso_get_anim_speed(EObjLink *link, double *speed);
extern void			e_nso_get_anim_accel(EObjLink *link, double *accel);
extern void			e_nso_get_anim_scale(EObjLink *link, double *scale);
extern void			e_nso_get_anim_scale_speed(EObjLink *link, double *scale_speed);
extern boolean		e_nso_get_anim_active(EObjLink *link);
extern void			e_nso_get_anim_evaluate_pos(EObjLink *link, double *pos, uint32 time_s, uint32 time_f);
extern void			e_nso_get_anim_evaluate_scale(EObjLink *link, double *scale, uint32 time_s, uint32 time_f);


extern char *		e_nso_get_method_group(ENode *node, uint16 group_id);
extern uint16		e_nso_get_next_method_group(ENode *node, uint16 group_id);
extern char *		e_nso_get_method(ENode *node, uint16 group_id, uint16 method_id);
extern uint16		e_nso_get_next_method(ENode *node, uint16 group_id, uint16 method_id);
extern uint			e_nso_get_method_param_count(ENode *node, uint16 group_id, uint16 method_id);
extern char **		e_nso_get_method_param_names(ENode *node, uint16 group_id, uint16 method_id);
extern VNOParamType *	e_nso_get_method_param_types(ENode *node, uint16 group_id, uint16 method_id);

extern boolean		e_nso_get_hide(ENode *node);
/*
extern double		e_nso_evaluate_anim_handle_single(EOAnimhandle *handle, uint seconds, uint fractions);
extern double		e_nso_evaluate_anim_handle_mult(double *output, EOAnimhandle *handle, uint seconds, uint fractions);
*/
/* geometry node storage ----------------------------------------------------------------------------------------------------------------*/

typedef void EGeoLayer;

extern EGeoLayer *	e_nsg_get_layer_by_name(ENode *g_node, const char *name);
extern EGeoLayer *	e_nsg_get_layer_by_id(ENode *g_node,  uint layer_id);
extern EGeoLayer *	e_nsg_get_layer_by_type(ENode *g_node, VNGLayerType type, const char *name);
extern EGeoLayer *	e_nsg_get_layer_by_fragment(ENode *g_node, const char *name);
extern EGeoLayer *	e_nsg_get_layer_next(ENode *g_node, uint layer_id);

extern EGeoLayer *	e_nsg_get_layer_crease_vertex_layer(ENode *g_node);
extern char *		e_nsg_get_layer_crease_vertex_name(ENode *g_node);
extern uint32		e_nsg_get_layer_crease_vertex_value(ENode *g_node);

extern EGeoLayer *	e_nsg_get_layer_crease_edge_layer(ENode *g_node);
extern char *		e_nsg_get_layer_crease_edge_name(ENode *g_node);
extern uint32		e_nsg_get_layer_crease_edge_value(ENode *g_node);

extern void		*	e_nsg_get_layer_data(ENode *g_node, EGeoLayer *layer);
extern VNGLayerType e_nsg_get_layer_type(EGeoLayer *layer);
extern uint			e_nsg_get_layer_id(EGeoLayer *layer);
extern uint			e_nsg_get_layer_version(EGeoLayer *layer);
extern uint			e_nsg_get_vertex_length(ENode *g_node);
extern uint			e_nsg_get_polygon_length(ENode *g_node);
extern char	*		e_nsg_get_layer_name(EGeoLayer *layer);

extern void			e_nsg_get_center(ENode *node, egreal *center);
extern void			e_nsg_get_bounding_box(ENode *node, egreal *high_x, egreal *low_x, egreal *high_y, egreal *low_y, egreal *high_z, egreal *low_z);
extern egreal		e_nsg_get_size(ENode *node);


extern uint			e_nsg_find_empty_vertex_slot(ENode *node, uint start);
extern uint			e_nsg_find_empty_polygon_slot(ENode *node, uint start);

extern uint16		e_nsg_get_bone_by_weight(ENode *g_node, const char *name);
extern uint16		e_nsg_get_bone_next(ENode *g_node, uint16 bone_id);

extern char	*		e_nsg_get_bone_weight(ENode *g_node, uint16 bone_id);
extern char	*		e_nsg_get_bone_reference(ENode *g_node, uint16 bone_id);
extern uint16		e_nsg_get_bone_parent(ENode *g_node, uint16 bone_id);
extern void			e_nsg_get_bone_pos32(ENode *g_node, uint16 bone_id, float *pos);
extern void			e_nsg_get_bone_pos64(ENode *g_node, uint16 bone_id, double *pos);
extern void			e_nsg_get_bone_rot32(ENode *g_node, uint16 bone_id, VNQuat32 *rot);
extern void			e_nsg_get_bone_rot64(ENode *g_node, uint16 bone_id, VNQuat64 *rot);

extern char *		e_nsg_get_bone_pos_label(ENode *g_node, uint16 bone_id);
extern char *		e_nsg_get_bone_rot_label(ENode *g_node, uint16 bone_id);
extern char *		e_nsg_get_bone_scale_label(ENode *g_node, uint16 bone_id);

/*
extern void			e_nsg_get_bone_matrix32(ENode *o_node, ENode *g_node, uint16 bone_id, float *matrix);
extern void			e_nsg_get_bone_matrix64(ENode *o_node, ENode *g_node, uint16 bone_id, double *matrix);
*/


/* material node storage ----------------------------------------------------------------------------------------------------------------*/

extern VNMFragmentID	e_nsm_get_fragment_next(ENode *node, VNMFragmentID id);
extern VMatFrag *	e_nsm_get_fragment(ENode *node, VNMFragmentID id);
extern VNMFragmentType	e_nsm_get_fragment_type(ENode *node, VNMFragmentID id);

extern VNMFragmentID	e_nsm_get_fragment_color_front(ENode *node);
extern VNMFragmentID	e_nsm_get_fragment_color_back(ENode *node);
extern VNMFragmentID	e_nsm_get_fragment_color_particles(ENode *node);
extern VNMFragmentID	e_nsm_get_fragment_color_displacement(ENode *node);
extern VNMFragmentID	e_nsm_find_empty_slot(ENode *node, VNMFragmentID id);

extern uint			e_nsm_get_fragment_count(ENode *node);
extern uint			e_nsm_get_fragment_version(ENode *node, VNMFragmentID id);

extern void			e_nsm_set_custom_data(ENode *node, VNMFragmentID frag, uint slot, void *data);
extern void			e_nsm_set_custom_func(uint slot, void (*func)(ENode *node, VNMFragmentID frag, ECustomDataCommand command));
extern void *			e_nsm_get_custom_data(ENode *node, VNMFragmentID frag, uint slot);

/* loop protection */

extern boolean 			e_nsm_enter_fragment(ENode *node, VNMFragmentID id);
extern void			e_nsm_leave_fragment(ENode *node, VNMFragmentID id);


/* bitmap node storage ----------------------------------------------------------------------------------------------------------------*/

typedef void EBitLayer;

extern EBitLayer *	e_nsb_get_layer_by_name(ENode *node, const char *name);
extern EBitLayer *	e_nsb_get_layer_by_id(ENode *node, uint layer_id);
extern EBitLayer *	e_nsb_get_layer_by_type(ENode *node, VNBLayerType type, const char *name);
extern EBitLayer *	e_nsb_get_layer_next(ENode *node, uint layer_id);

extern void *		e_nsb_get_layer_data(ENode *node, EBitLayer *layer);
extern uint			e_nsb_get_layer_id(EBitLayer *layer);
extern char *		e_nsb_get_layer_name(EBitLayer *layer);
extern VNBLayerType e_nsb_get_layer_type(EBitLayer *layer);
extern uint			e_nsb_get_layer_version(EBitLayer *layer);

extern void			e_nsb_fill_buffer_uint8(ENode *node, EBitLayer *layer, uint8 *buffer, uint stride);
extern void			e_nsb_fill_buffer_uint16(ENode *node, EBitLayer *layer, uint16 *buffer, uint stride);
extern void			e_nsb_fill_buffer_float(ENode *node, EBitLayer *layer, float *buffer, uint stride);
extern void			e_nsb_fill_buffer_double(ENode *node, EBitLayer *layer, double *buffer, uint stride);

extern void			e_nsb_image_set_float(ENode *node, EBitLayer *layer, float *buffer, uint stride);

typedef void EBMHandle;

extern ebreal		e_nsb_get_aspect(ENode *node);
extern void			e_nsb_get_size(ENode *node, uint *x, uint *y, uint *z);
extern EBMHandle	*e_nsb_get_image_handle(VNodeID node_id, const char *layer_r, const char *layer_g, const char *layer_b);
extern EBMHandle	*e_nsb_get_empty_handle(void);
extern void			e_nsb_evaluate_image_handle_tile(EBMHandle *handle, ebreal *output, ebreal u, ebreal v, ebreal w);
extern void			e_nsb_evaluate_image_handle_clamp(EBMHandle *handle, ebreal *output, ebreal u, ebreal v, ebreal w);
extern void			e_nsb_destroy_image_handle(EBMHandle *handle);

/* curve node storage ----------------------------------------------------------------------------------------------------------------*/

typedef void ECurve;

extern ECurve *			e_nsc_get_curve_by_name(ENode *c_node, const char *name);
extern ECurve *			e_nsc_get_curve_by_id(ENode *c_node, uint curve_id);
extern ECurve *			e_nsc_get_curve_next(ENode *c_node, uint curve_id);

extern char *			e_nsc_get_curve_name(ECurve *curve);
extern uint				e_nsc_get_curve_id(ECurve *curve);
extern uint				e_nsc_get_curve_length(ECurve *curve);
extern uint				e_nsc_get_curve_dimensions(ECurve *curve);
extern uint				e_nsc_get_curve_point_count(ECurve *curve);

extern uint				e_nsc_get_point_next(ECurve *curve, uint point_id);
extern uint				e_nsc_get_point_order(ECurve *curve, uint point_nr);
extern uint				e_nsc_get_point_pos(ECurve *curve, uint point_id);

extern void				e_nsc_get_point(ECurve *curve, uint point_id, real64 *pre_value, uint32 *pre_pos, real64 *value, real64 *pos, real64 *post_value, uint32 *post_pos);
extern void				e_nsc_get_point_double(ECurve *curve, uint point_id, real64 *pre_value, real64 *pre_pos, real64 *value, real64 *pos, real64 *post_value, real64 *post_pos);
extern void				e_nsc_get_segment(ECurve *curve, uint segment_nr, uint axis, real64 *point_0, real64 *point_1, real64 *point_2, real64 *point_3);

extern double				e_nsc_evaluate_curve(ECurve *curve, double *output, double pos);

extern void				e_nsc_send_c_key_set(ENode *node, ECurve *curve, uint32 key_id, real64 *pre_value, real64 *pre_pos, real64 *value, real64 *pos, real64 *post_value, real64 *post_pos);

/* Text node storage ----------------------------------------------------------------------------------------------------------------*/


typedef void ETextBuffer;

extern char *			e_nst_get_language(ENode *t_node);
extern ETextBuffer *	e_nst_get_buffer_by_name(ENode *node, char *name);
extern ETextBuffer *	e_nst_get_buffer_by_id(ENode *node,  uint buffer_id);
extern ETextBuffer *	e_nst_get_buffer_next(ENode *node, uint buffer_id);
extern char *			e_nst_get_buffer_data(ENode *node, ETextBuffer *buffer);
extern uint				e_nst_get_buffer_data_length(ENode *node, ETextBuffer *buffer);
extern uint				e_nst_get_buffer_id(ETextBuffer *buffer);
extern char *			e_nst_get_buffer_name(ETextBuffer *buffer);
extern uint				e_nst_get_buffer_version(ETextBuffer *buffer);


/* Audio node storage ----------------------------------------------------------------------------------------------------------------*/

typedef void EAudioBuffer;
typedef void EAudioStream;

extern EAudioBuffer *	e_nsa_get_buffer_by_name(ENode *node, char *name);
extern EAudioBuffer *	e_nsa_get_buffer_by_id(ENode *node,  uint buffer_id);
extern EAudioBuffer *	e_nsa_get_buffer_next(ENode *node, uint buffer_id);
extern char *			e_nsa_get_buffer_data(ENode *node, EAudioBuffer *buffer);
extern uint				e_nsa_get_buffer_data_length(ENode *node, EAudioBuffer *buffer);
extern uint				e_nsa_get_buffer_id(EAudioBuffer *buffer);
extern char *			e_nsa_get_buffer_name(EAudioBuffer *buffer);
extern VNABlockType		e_nsa_get_buffer_type(EAudioBuffer *buffer);
extern double			e_nsa_get_buffer_frequency(EAudioBuffer *buffer);
extern uint				e_nsa_get_buffer_version(EAudioBuffer *buffer);

extern EAudioStream *	e_nsa_get_stream_by_name(ENode *node, char *name);
extern EAudioStream *	e_nsa_get_stream_by_id(ENode *node,  uint stream_id);
extern EAudioStream *	e_nsa_get_stream_next(ENode *node, uint stream_id);
extern uint				e_nsa_get_stream_id(EAudioStream *stream);
extern char *			e_nsa_get_stream_name(EAudioStream *stream);
extern uint				e_nsa_get_stream_version(EAudioStream *stream);

#endif
