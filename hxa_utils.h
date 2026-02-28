#include "hxa.h"

#ifndef HXA_UTIL
#define HXA_UTIL

/* ------- disk stoarge ------------
Functions for saving and loading HxA files to and from disk. */

extern HXAFile	*hxa_load(char *file_name, int silent); /* Load a Hxa file in to memory.*/
extern int		hxa_save(char *file_name, HXAFile *data); /* Save a HxA structure to disk. */


extern HXAFile *hxa_unserialize(hxa_uint8 *buffer, size_t size, int silent); /* take a buffere and turn it in to a HXAFile */
extern size_t	hxa_serialize_size(HXAFile *data);  /* Get the size needed to seialize a HXAFile */
extern void		hxa_serialize(hxa_uint8 *buffer, HXAFile *data); /* Serialize a HXAFile in to a buffer */
extern int		hxa_serialize_save(char *file_name, HXAFile *data); /* use the sertialization implementation to save a HXAFile to disk */



extern void		hxa_util_primitive_cube(HXAFile *file, double x, double y, double z);

extern void			hxa_util_node_clone_content(HXANode *node_to, HXANode *node_from);
extern HXAFile	*hxa_util_clone_file(HXAFile *file);

extern void		hxa_util_free_node_content(HXANode *node); /* frees the content of a node but does not free the node pointer itself */
extern void		hxa_util_free_file(HXAFile *file); /* Frees an entire HxA structure. */

/* ------- validation ------------
Validates that the content of HxA structure. */
extern int		hxa_util_validate(HXAFile *file, int silent); /* Validates the HxA file to contain valid data. */
extern int		hxa_util_validate_pow(HXAFile *file, int silent); /* Validate that all images are of Power of two resolution. */


/* ------- validation ------------
Validates that the content of HxA structure. */
extern void		hxa_print(HXAFile *file, int data); /* Prints the content of a Hxa structure to stdout in a human readable form. If data is set to TRUE, Array content will be printed, otherwise not.*/
extern void		hxa_print_meta_find(HXAFile *file, char *key); 

extern HXAMeta	*hxa_util_meta_add(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, HXAMetaDataType type, void *data, unsigned int length, int copy); /* Adds a Meta data entry to a node. If copy is set to TRUE the function will allocate the memory needed to store the data and copy the data to it. If copy is set to FALSE the user provided pointer will be used by the structure.*/
extern HXAMeta	*hxa_util_meta_set(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, HXAMetaDataType type, void *data, unsigned int length, int copy); /* Adds a Meta data entry to a node. If copy is set to TRUE the function will allocate the memory needed to store the data and copy the data to it. If copy is set to FALSE the user provided pointer will be used by the structure.*/
extern HXAMeta	*hxa_util_meta_get(HXAMeta *meta_data, hxa_uint32 meta_data_count, char *name, int recursive); /* Searches for a Meta tag with a specific type and name and returns a pointer to it. It will also write the*/
extern HXAMeta	*hxa_util_meta_level_get(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, int recursive);
extern unsigned int hxa_util_meta_get_next(HXAMeta *meta_data, hxa_uint32 start, hxa_uint32 meta_data_count, char *name, HXAMetaDataType type);
extern void		*hxa_util_meta_data_get(HXAMeta *meta_data, hxa_uint32 meta_data_count, char *name, HXAMetaDataType type, unsigned int *length, int recursive);
extern void		hxa_util_free_meta(HXAMeta *meta);

extern int		hxa_util_meta_delete(HXAMeta **meta_data, hxa_uint32 *meta_data_count, HXAMeta *meta);
extern int		hxa_util_meta_resize(HXAMeta *meta, unsigned int new_size);
extern HXAMeta	*hxa_util_meta_clone(HXAMeta *meta_data, hxa_uint32 meta_data_count);
extern HXAMeta	*hxa_util_meta_add(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, HXAMetaDataType type, void *data, unsigned int length, int copy);
extern HXAMeta	*hxa_util_meta_set(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, HXAMetaDataType type, void *data, unsigned int length, int copy);
extern void		*hxa_util_meta_data_get(HXAMeta *meta_data, hxa_uint32 meta_data_count, char *name, HXAMetaDataType type, unsigned int *length, int recursive);
extern HXAMeta	*hxa_util_meta_from_node_get(HXANode *node, char *name, int recursive);
extern unsigned int hxa_util_meta_data_get_next(HXAMeta *meta_data, hxa_uint32 start, hxa_uint32 meta_data_count, char *name, HXAMetaDataType type);
extern void		*hxa_util_meta_data_get_set(HXAMeta **meta_data, hxa_uint32 *meta_data_count, void *default_data, unsigned int *data_count, char *name, HXAMetaDataType type);




#define			hxa_ref(ref, a) (ref[a] >= 0 ? ref[a] : (-ref[a] - 1)) /* A macro that converts all ref values to positive references. */
extern int		hxa_corner_get_next(int *ref, unsigned int corner); /* Returns the position of the next reference value in a polygon from one corner. */
extern int		hxa_corner_get_previous(int *ref, unsigned int corner); /* Returns the position of the next reference value in a polygon from one corner.  */
extern int		hxa_edge_get_next(int *ref, unsigned int *neighbour, unsigned int edge);
extern int		hxa_edge_get_previous(int *ref, unsigned int *neighbour, unsigned int edge);
extern unsigned int *hxa_neighbour_node(HXANode *node); /* Generates edge adjacensy information for a geometry mesh. */
extern unsigned int *hxa_neighbour_node_repair(HXANode *node, unsigned int *n);
extern void		hxa_neighbour_file(HXAFile *file); /* Generates neighbour data for all geometry nodes in a HxA structure. */

extern HXAFile *hxa_util_merge(HXAFile *file_a, HXAFile *file_b); /* Merges two HxA structures in to one. */


extern void		hxa_util_convert_layer_float_to_double(HXALayer *layer, unsigned int count); /* Converts a layer from single precission float to double precission floats. */
extern void		hxa_util_convert_layer_double_to_float(HXALayer *layer, unsigned int count); /* Converts a layer from double precission float to single precission floats. */
extern void		hxa_util_convert_stack_float_to_double(HXALayerStack *stack, unsigned int count); /* Converts a layer stack from single precission float to double precission floats. */
extern void		hxa_util_convert_stack_double_to_float(HXALayerStack *stack, unsigned int count); /* Converts a layer stack from double precission float to single precission floats. */
extern void		hxa_util_convert_node_float_to_double(HXANode *node); /* Converts all layer in a node from single precission float to double precission floats. */
extern void		hxa_util_convert_node_double_to_float(HXANode *node); /* Converts all layer in a node from double precission float to single precission floats. */

extern void		hxa_util_node_vertex_purge(HXANode *node); /* removes any unused vertices from a node */
extern void		hxa_util_vertex_purge(HXAFile *file); /* removes any unused vertices from a structure */

extern void		hxa_util_normal_corner(HXANode *node);

extern void		hxa_util_triangulate_node(HXANode *node, unsigned int max_sides); /* Splits all n-gons with more sides then max_sides in to triangles. */


typedef enum{
	HXA_TC_INT8,
	HXA_TC_UINT8,
	HXA_TC_INT16,
	HXA_TC_UINT16,
	HXA_TC_INT32,
	HXA_TC_UINT32,
	HXA_TC_FLOAT16,
	HXA_TC_FLOAT32,
	HXA_TC_FLOAT64,
	HXA_TC_COUNT
}HxATypeConvert;

extern void	   *hxa_util_array_extract(HXANode *node, size_t vertex_stride, size_t *vertex_param_offsets, unsigned int *vertex_param_types, char **vertex_param_names, hxa_uint8 *vertex_component, void ** defaults, unsigned int param_count);


extern void		hxa_corner_to_vertex(HXANode *node); /**/

extern void		hxa_close_node(HXANode *node); /* Closes all holes in a polygon mesh */
extern void		hxa_close_file(HXAFile *file); /* Closes all holes in all polygon meshes */



extern HXAFile *hxa_util_true_type_load(char *file_name); /* loads a Truetype font and converts in in to polygon nodes. */


extern int hxa_load_png(HXAFile *file, char *file_name);
extern HXAFile *hxa_util_fbx_load(char *file_name, HXAFile *hxa_file);

typedef enum{
	HXA_UAET_INT8,
	HXA_UAET_UINT8,
	HXA_UAET_INT16,
	HXA_UAET_UINT16,
	HXA_UAET_INT32,
	HXA_UAET_UINT32,
	HXA_UAET_HALF16,
	HXA_UAET_FLOAT32,
	HXA_UAET_DOUBLE64,
	HXA_UAET_COUNT
}HxAUtilArrayExportTypes;

unsigned char *hxa_type_vertex_convert(HXANode *node, unsigned char *buffer, unsigned int param_count, char **param_names, HxAUtilArrayExportTypes *param_types, unsigned int *param_dimentions);
unsigned char *hxa_type_reference_convert(HXANode *node, unsigned char *buffer, unsigned int param_count, char **param_names, HxAUtilArrayExportTypes *param_types, unsigned int *param_dimentions);

#endif