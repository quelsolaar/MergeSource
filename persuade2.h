/* Undef this to disable use of Persuade in the various applications. The symbol
 * actually cared about by the other code is PERSUADE_H, so that is wrapped here
 * by the PERSUADE_ENABLED symbol for ease of disabling/enabling.
 *
 * Typically, we set this externally in the Makefile, since there are linking
 * considerations too.
*/

#include "forge.h"

#define PERSUADE_ENABLED

#if defined PERSUADE_ENABLED

#if !defined PERSUADE_H
#define	PERSUADE_H

#define PERSUADE_FLOAT

#ifdef PERSUADE_DOUBLE
#define PERSUADE_ILLEGAL_VERTEX         1.7976931348623158e+308
typedef double pgreal;
#endif

#ifdef PERSUADE_FLOAT
#define PERSUADE_ILLEGAL_VERTEX         3.402823466e+38f
typedef float pgreal;
#endif

#ifndef PERSUADE_INTERNAL

typedef void PMesh;
typedef void PPolyStore;
extern void persuade_init(uint max_tesselation_level); /* initialize persuade and set the maximum SDS level*/



extern PPolyStore *persuade_mesh_create(uint *ref, uint ref_count, pgreal *vertex, uint vertex_count, uint *edge_crease, double default_crease, uint levels); /* use to create a drawabel version of a geometry node */

extern void		persuade_mesh_destroy(PMesh *mesh); /* Destroy the same*/
extern PMesh	*persuade2_lod_create(PPolyStore *smesh, uint *ref, pgreal *vertex, float compexity, uint force_tess_level, float *eye);
extern void		persuade_lod_destroy(PMesh *mesh);
extern void		persuade2_lod_vertex_shape(PMesh *mesh, pgreal *render_vertex, uint output_stride, pgreal *cvs);
extern void		persuade2_lod_normal_shape(PMesh *mesh, pgreal *render_normal, uint normal_stride, pgreal *render_vertex, uint vertex_stride);
extern uint		persuade2_lod_vertex_length_get(PMesh *mesh);
//void persuade2_lod_shape(PMesh *mesh, pgreal *vertex);

extern uint		*p_rm_get_reference(PMesh *mesh);
extern uint		p_rm_get_ref_length(PMesh *mesh);

#endif		/* PERSUADE_INTERNAL */
#endif		/* PERSUADE_H */
#endif		/* PERSUADE_ENABLED */
