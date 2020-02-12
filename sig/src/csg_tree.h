#include "csg_prim.h"

/*!
	Only have to include this header to access functionality of CSG tree 
	This code provides functions to build a CSG object out of primitives

	*/

//operations for CSG Tree
typedef enum Operation { Union, Difference, Intersection };

//CSG Tree Node (from lecture)
struct csgnode{
	Operation op;
	csgnode* left;
	csgnode* right;
	/* example usage: o1 Difference (-) o2 = 'if o2 contributes to a ray, the contribution will NOT be considered'	*/


	CSGPrimitive* obj;
};

//functions to build and navigate CSG tree
