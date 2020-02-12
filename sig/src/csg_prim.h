#pragma once
#include <sig/gs_primitive.h>


/*
	TODO
		support for shapes { Sphere, Box }
		support for { interesction, difference, union }
*/


class CSGPrimitive : public GsPrimitive
{
public:

	/*! Given a point, determine whether it lies inside the object */
	bool contains(const GsPnt& p) const;

	/*! Given a ray, determine whether it intersect the object */
	bool intersects(const GsVec& orig, const GsVec& dir, const CSGPrimitive* object) const;

private: 

};



