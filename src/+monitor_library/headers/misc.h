#ifndef MISC_H_
#define MISC_H_

#include <deque>
#include "type_transl.h"
#include <iostream>

// Interval struct definitions-------------------------------------------------------------

/**
\brief structure representing an half closed interval [a,b), with a < b.
 */
struct Interval {
	RealType leftLimit;	/**< left limit of the interval*/
	RealType rightLimit; 	/**< right limit of the interval*/

	Interval(RealType, RealType);
};

bool isMergeable(const Interval &,const Interval &);
bool contains(const Interval &, RealType);
Interval merge(const Interval &, const Interval &);


// Signal class definitions------------------------------------------------------------------

/**
class representing a function \f$f:[a,b)->\{1,0\}\f$, with \f$a \le b\f$ (if \f$a=b\f$ then *f* is the empty function), such as:
	- The preimage of \f${1}\f$ is an union of finite intervals *[x,y)*, with *x < y*.
	- The preimage of \f${0}\f$ is an union of finite intervals *[x,y)*, with *x < y*.


\note
We can represent such a function with the pair \f$(D, I)\f$, with:
- *D* equal to the domain of the function.
- *I* the set of disjunct intervals in the preimage of *{1}*.

example:
--------
Lets consider the function \f$f:[1,5) -> \{1,0\}\f$, with:
- \f$f(x) = 1\f$ if \f$ x \in [1,2) \cup [4.5,5)\f$
- \f$f(x) = 0\f$ if \f$ x \in [2,4.5)\f$

Then the pair \f$(D, I)\f$ representing it is:
- \f$D = [1,5)\f$
- \f$I = {[1,2), [4.5,5)}\f$
*/
class Signal {

public:
	typedef  std::deque<Interval>::size_type size_type;
	typedef std::deque<Interval>::const_iterator const_iterator;

private:
	RealType first; ///< smallest element in the signal domain
	RealType last; ///< greatest element in the signal domain
	std::deque<Interval> intervals; ///< queue that contains the intervals representing the preimage of {1}

public:
	Signal(RealType, RealType);
	void increaseFirst(const RealType);
	void increaseLast(const RealType);
	void addInterval(const RealType, const RealType);
	void reset(RealType, RealType);
	void append(const Signal&);


	/**
	\brief return the left limit of the domain.
	\return the smallest element in the caller Signal domain (i.e. the left limit of the domain).*/
	inline RealType getFirst(void)const{return first;};

	/**
	\brief return the right limit of the domain.
	\return the greatest element in the caller Signal domain (i.e. the right limit of the domain).*/
	inline RealType getLast(void)const{return last;};

	/**
	\brief return the number of disjunct intervals in the preimage of *{1}*.*/
	inline Signal::size_type getIntervalCount(void) const {return intervals.size();};

	/**
	\brief return the iterator to the first interval in the preimage of *{1}*.
	The iterator will iterate on the intervals in the preimage of *{1}*.*/
	inline Signal::const_iterator getBegin(void) const {return intervals.begin();};

	/**
	\brief return the iterator pointing to the interval after the last interval in the preimage of *{1}*.
	The iterator value is undefined, use this iterator to check if another iterator has finished to iterate over the signal.*/
	inline Signal::const_iterator getEnd(void) const {return intervals.end();};

};

#endif
