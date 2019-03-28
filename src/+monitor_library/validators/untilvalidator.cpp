#include <algorithm>
#include <stdexcept>

#include "misc.h"
#include "validators.h"



using std::max;
using std::min;



/*
PRE-CONDITIONS:
 	 alpha must be greater than zero.

POST-CONDITIONS:
	the output interval h correctly represent the unitary Until with h1 and h2 as input and as parameter the value alpha.

	the unitary until is defined as follow:
		let [a,b), [c,d) be two non-empty intervals.

		* if b < c then return the interval [c,d)
		* if b >= c and a < c then return  [a,b)U[c,d) intersect [c-alpha,d)
		* otherwise return [c,d)
 */
static Interval unitaryUntil(const Interval &h1, const Interval &h2, RealType alpha){

	if (alpha < 0)
		throw std::invalid_argument("unitaryUntil: The third parameter must be greater than zero.");

	const RealType &a = h1.leftLimit, &b=h1.rightLimit;
	const RealType &c = h2.leftLimit, &d=h2.rightLimit;


	if(b >= c && a < c)
	{
		RealType leftlimit = max(c - alpha, a);
		return Interval(leftlimit, d);
	}
	else
		return h2;
}


/*
PRE-CONDITIONS:
	* alpha must be greater than zero.
	* the two input signal must have the same first domain value (not controller because of double imprecision).

POST-CONDITIONS:
 	 untilvalues correctly contains the signal representing the until evaluation for each t in [first,last), with:
 	 	 first = first value of the domain of signal1 (or equivalently, signal2).
 	 	 last = max between the last value of signal1 and signal2, minus alpha.
 */
static void computeUntil(const Signal &signal1, const Signal &signal2, Signal &untilvalues, RealType alpha)
{
	if (alpha < 0)
			throw std::invalid_argument("computeUntil: The alpha parameter must be greater than zero.");

	RealType newfirst = min(signal1.getFirst(), signal2.getFirst());
	RealType newlast = min(signal1.getLast(), signal2.getLast()) - alpha;

	// the two input signal are not long enough to be able to compute the until
	if (newfirst > newlast)
		newlast = newfirst;

	untilvalues.reset(newfirst,newlast);

	Signal::const_iterator it1 = signal1.getBegin(), end1 = signal1.getEnd();
	Signal::const_iterator it2 = signal2.getBegin(), end2 = signal2.getEnd();

	while(it2 != end2)
	{
		Interval add = *it2;
		while(it1 != end1 && !isMergeable(*it1,*it2))
			it1++;

		it2++;

		// iterator's interval is valid
		if (it1 != end1)
			 add = unitaryUntil(*it1,add,alpha);

		// if the interval to be added is out of the interested domain (all the interval after this are going to be greater so we can safely return from the function).
		if (add.leftLimit >= newlast)
			return;

		// if the interval to be added is at the boundary of the interested domain ( note that the interval is always inside the left boundary, but not necessarily it is inside the right boundary, hence the control).
		if (add.rightLimit >= newlast)
			untilvalues.addInterval(add.leftLimit, newlast);
		else
			untilvalues.addInterval(add.leftLimit,add.rightLimit);
	}
}



// METHODS--------------------------------------------------------------------------------------------------

UntilValidatorNode::UntilValidatorNode (ValidatorNode &child1, ValidatorNode &child2, RealType a)
: firstchild(&child1),	secondchild(&child2),
  alpha(a),				buffer1(0.0,0.0),
  buffer2(0.0,0.0),		computedValues(0.0,0.0)
{
	if (alpha <= 0)
		throw std::invalid_argument("UntilValidator: alpha parameter must be greater than zero.");

	RealType fmt = child1.minTime(), smt = child2.minTime();
	max = (fmt < smt? smt:fmt);
}

void UntilValidatorNode::start(RealType ts, const std::vector<BooleanType> &preds)
{
	 // setting  the state of the object
	 buffer1.reset(ts,ts);
	 buffer2.reset(ts,ts);

	 computedValues.reset(ts,ts);

	 // calls on the recursive structure (starting the whole sub-tree)
	 firstchild->start(ts,preds);
	 secondchild->start(ts,preds);
}

void UntilValidatorNode::update(RealType t, const std::vector<BooleanType> &preds)
{
	// function calls on the recursive structure (updating the whole sub-tree)
	firstchild->update(t,preds);
	secondchild->update(t,preds);

	const Signal &tmp1 = firstchild->getValues();
	const Signal &tmp2 = secondchild->getValues();

	// update of the buffers' values
	buffer1.append(tmp1);
	buffer2.append(tmp2);

	// computing until and saving values in computedvalues
	computeUntil(buffer1,buffer2,computedValues,alpha);

	// defining the new starts of the buffers
	RealType buffstart = computedValues.getLast();

	// increasing the buffer first domain value
	buffer1.increaseFirst(buffstart);
	buffer2.increaseFirst(buffstart);
}




UntilValidatorNode::~UntilValidatorNode(void)
{
	delete firstchild;
	delete secondchild;
}
