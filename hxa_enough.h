#include "enough.h"
#include "hxa.h"
/* Tiny library that connects Enough (verse storage layer) with HxA */

extern void enough_to_hxa_node_geometry(ENode *e_node, HXANode *haxa_node); /* convert from a Enough Geometry node to HxA in memory geometry node. */
extern void enough_to_hxa_one(char *file_name, ENode *e_node); /* Save a Enough geometry node to a HxA file. */
extern void enough_to_hxa_all(char *file_name); /* Save all Enough gemoetry nodes to a HxA file.*/