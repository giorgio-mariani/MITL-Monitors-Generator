#include <algorithm>
#include <stdexcept>
#include<iostream>	//FIXME remove me

#include "type_transl.h"
#include "misc.h"

using std::min;using std::max; using std::invalid_argument;



/**
\brief Create an interval with given limits.
\param a left limit of the interval to create.
\param b right limit of the interval to create.
\exception std::invalid_argument if a is greater than or equal to b
 */
Interval::Interval(const RealType a, const RealType b ):leftLimit(a),rightLimit(b)
{
	if(a >= b)
		throw std::invalid_argument("Interval: The first input must be less than the second input.");
}



/**
\brief Check if the union between two given intervals is also an interval.
\param h1 interval
\param h2 interval
\returns true if *h1* union *h2* is still an interval, otherwise false.
 */
bool isMergeable(const Interval &h1, const Interval &h2)
{
	const Interval *min, *max;
	if ( h1.leftLimit <= h2.leftLimit){
		min = &h1;
		max = &h2;
	}
	else{
		min = &h2;
		max = &h1;
	}
	return min->rightLimit >= max->leftLimit;
}



/**
\brief Check if a certain value is contained in a given interval.
\param h interval
\param v real value
\returns true if *v* is contained in *h*, otherwise false.
 */
inline bool contains(const Interval &h, RealType v){return h.leftLimit <= v  && v < h.rightLimit;}



/** \brief Compute the union between two adjacent or intersect intervals.
 	\param h1 first interval to be merged.
 	\param h2 second interval to be merged.

 	\returns the interval representing the union between h1 and h2, if the union is not an interval than an exception is thrown.
 	\exception std::invalid_argument the union between h1 and h2 is not an interval
 */
Interval merge(const Interval &h1, const Interval &h2)
{
	if (!isMergeable(h1,h2))
		throw invalid_argument("merge: The two input intervals can not be merged!");

	return Interval(min(h1.leftLimit,h2.leftLimit ), max(h1.rightLimit,h2.rightLimit));
}
