/*
 * @file Ray.h
 *
 * Combination of a point & vector.
 *
 * Created on: Aug 19, 2010
 * @author Andrew Ford
 */

#ifndef RAY_H_
#define RAY_H_

#include "Point.h"
#include "Vector.h"

typedef struct
{
    Point location;
    Vector direction;
} Ray;

#endif /* RAY_H_ */
