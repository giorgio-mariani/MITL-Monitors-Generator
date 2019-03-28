
#include <stdexcept>
#include <iostream>
#include <deque>

#include "misc.h"

using std::invalid_argument;

/**
\brief Create a signal with the given domain and constantly equal to zero.

\param first smallest element in the domain of the signal to create
\param last greatest element in the domain of the signal to create
\exception std::invalid_argument if *first* is greater than *last*

The created Signal has as domain the set *[first,last)* and is constantly equal to zero.
 */
Signal::Signal(RealType first, RealType last)
{
	if (first > last)
			throw invalid_argument("Signal: The value of the first input must be less than or equal to the value of the second input.");

	this->last = last;
	this->first = first;
}


/**
\brief increase the right limit of the the caller signal's domain.
\param newlast new domain right limit.
\exception std::invalid_argument if *newlast* is less than the right limit of *this*' domain.

Let *first* and *last* be the left limit and the right limit of *this*'domain before the method execution, then:
 - The domain of the *this* is the set *[first,newlast)*.
 - The values in the set *(last,newlast)* are all mapped to zero.
 */
void Signal::increaseLast(RealType newlast)
{
	if (newlast < last)
			throw invalid_argument("increaseLast: The value in input must be greater than or equal to the last value of the domain of the caller signal.");

	last = newlast;
}


/**
\brief increase the left limit of the the caller signal's domain.
\param newfirst  new domain left limit.

\exception std::invalid_argument if *newfirst* is less than the left limit of *this*' domain.

Let *first* and *last* be the left limit and the right limit of *this*'domain before the method execution, then:
- if \f$ newfirst \le last\f$ then the domain of the caller signal is the set *[newfirst,last)*.
- if \f$ newfirst > last\f$ then the domain of the caller signal is the set *[newfirst, newfirst)*.
 */
void Signal::increaseFirst(RealType newfirst)
{
	if (newfirst < first)
			throw invalid_argument("increaseFirst: The value of the input must be greater than or equal to the first value in the domain of the caller signal.");

	if(newfirst >= last){
		first = newfirst;
		last = first;
		intervals.clear();
	}
	else if (intervals.empty())
	{
		first = newfirst;
	}
	else
	{
		first = newfirst;

		// removing all the intervals that are now out of the domain and "cut" the interval at the boundary
		while(!intervals.empty()){
			Interval &h = intervals.front();

			// left is the left limit of h
			RealType left = h.leftLimit;

			// right is the right limit of h
			RealType right = h.rightLimit;

			// removing h form the intervals set
			intervals.pop_front();

			// re-adding h (eventually cut) to the set and terminating loop
			if (right > first)
			{
				if(left < first)
					left = first;

				intervals.push_front(Interval(left,right));
				break;
			}
		}
	}
}


/**
\brief add an interval to the preimage of *{1}* in the caller signal.
\param a left limit of the interval to add
\param b right limit of the interval to add
\exception std::invalid_argument
- if *a* is smaller  than the signal's domain left limit or *b* is greater than the right limit.
- if *a* is less than the left limit of the last interval that is preimage of *{1}*.

\note
if \f$a\le b\f$ then the set *[a,b)* is considered empty.

After method execution holds that:
- All the elements in *[a,b)* are now mapped to 1
- All the elements in the domain of the signal minus *[a,b)* are mapped to the value that they had before the execution of the method.
 */
void Signal::addInterval(const RealType a, const RealType b)
{
	// input interval (which now is not empty) is not within the given boundaries
	if (a < first || b > last)
		throw invalid_argument("addInterval: The set  of the values contained in between the first input and the second input must be a subset of the signal domain.");

	// input interval is empty
	if (a >= b)
		return;

	 Interval const add(a,b);

	if (intervals.empty())
		intervals.push_back(add);
	else
	{
		Interval &h = intervals.back();

		if (h.leftLimit > add.leftLimit)
			throw invalid_argument("add: The value of the first input must be greater than or equal to the left limit of the last interval in the signal.");
		else if (isMergeable(h, add))
			//h.rightLimit = add.rightLimit < h.rightLimit ?  h.rightLimit : add.rightLimit;
			h = merge(h, add); // FIXME unnecessary computation
		else
			intervals.push_back(add);
	}
}


/**
\brief reset the object values and domain
\param first first element of the new domain
\param last last element of the new domain
\exception std::invalid_argument if *first* is greater than last.

After the method execution holds:
- The domain of the calling signal is *[first,last)*
- The calling signal constantly maps each element in the domain to 0.
 */
void Signal::reset(RealType first,RealType last){
	if (first > last)
				throw invalid_argument("reset: The value of the first input must be less than or equal to the value of the second input.");

		this->last = last;
		this->first = first;
		this->intervals.clear();
}


/**
 \brief Concatenation between two Signal objects.
\param appendvalues signal that is going to be appended
\exception std::invalid_argument if at least one of the following conditions hold:
- The left limit of the first interval in *appendvalues* is less than the left limit of the last interval in *this*.
- Let *[c,d)* be the domain of *appendvalues* and *[a,b)* the domain of *this*, then  *c < a* or *d < b*.

Let be *[a,b)* the domain of the caller before the method execution, *[c,d)* the domain of 'appendvalues' and *P* the preimage of *{1}* in the caller signal before the method execution.
- The domain of the caller signal (*this*)  after the method execution is  *[a,d)*.
- The preimage of *{1}* in the caller signal  after the method execution is equal to *P* union the preimage of *{1}* in *appendvalues* (i.e. all the intervals in *appendvalues* are added to *this*).
 */
void Signal::append(const Signal &appendvalues)
{
	// check input conditions
	if(last > appendvalues.last)
		throw std::invalid_argument("The last domain value of the input signal must be greater than or equal to the last domain value of the caller signal.");

	if(first > appendvalues.first)
		throw std::invalid_argument("The first domain value of the input signal must be greater than or equal to the first domain value of the caller signal.");


	// update buffer last domain value
	increaseLast(appendvalues.getLast());

	// add necessary elements to the buffer
	Signal::const_iterator it = appendvalues.getBegin(), end = appendvalues.getEnd();

	while(it != end){
		addInterval(it->leftLimit,it->rightLimit);
		it++;
	}
}




