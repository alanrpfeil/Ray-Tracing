#include "csg_prim.h"
#include <algorithm>


/*! 
	algorithm from:
	https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection 
*/
bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1)
{
	float discr = b * b - 4 * a * c; // (b^2 - 4ac)
	if (discr < 0) return false; //no solution, no intersection

	else if (discr == 0) x0 = x1 = -0.5 * b / a; // one solution
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);

	return true;
}

/*! 
	algorithm from:
	https://www.geeksforgeeks.org/check-whether-a-point-lies-inside-a-sphere-or-not/
*/
bool CSGPrimitive::contains(const GsPnt& p) const
{
	float x1 = gs_pow( (p.x - center.x), 2 );
	float y1 = gs_pow( (p.y - center.y), 2 );
	float z1 = gs_pow( (p.z - center.z), 2 );
	
	float dist = x1 + y1 + z1;
	float inside = gs_pow(ra, 2);
	gsout << dist << " :: " << inside << gsnl;

	return ( x1 + y1 + z1 <= gs_pow(ra,2) ) ? true : false;
}


/*
	algorithm from:
	https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
*/
bool CSGPrimitive::intersects(const GsVec& orig, const GsVec& dir, const CSGPrimitive* object) const
{
	float p0, p1; //points of intersection

	GsVec L = object->center - orig; //component from ray origin to sphere center
	float a = dot(dir, dir); 
	float b = 2 * dot(dir, L);
	float c = dot(L,L) - gs_pow(object->ra, 2);
	if (!solveQuadratic(a,b,c,p0,p1)) return false;

	//edge cases
	if (p0 < 0) {
		p0 = p1; // if p0 is negative, let's use p1 instead (p0 is behind ray)
		if (p0 < 0) return false; // both p0 and p1 are negative (both points are behind ray)
	}

	//p0 = point of intersection
	return true;
}
