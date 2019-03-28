#include <algorithm>
#include <stdexcept>

#include "misc.h"
#include "validators.h"

static void computeUnion(const Signal &signal1,const Signal &signal2,Signal &computedvalues)
{
	Signal::const_iterator it1 = signal1.getBegin(), end1 = signal1.getEnd();
	Signal::const_iterator it2= signal2.getBegin(), end2 = signal2.getEnd();

	RealType first = std::min(signal1.getFirst(), signal2.getFirst());
	RealType last = std::min(signal1.getLast(), signal2.getLast());
	computedvalues.reset(first, last);

	while(it1 != end1 && it2 != end2){
		Interval i1 = *it1;
		Interval i2 = *it2;
		Interval add = i1;

		if(i1.rightLimit <= i2.rightLimit){
			add = i1;
			it1++;
		}
		else{
			add = i2;
			it2++;
		}

		if (add.leftLimit > last)
			return;

		if (add.rightLimit > last)
			add.rightLimit = last;

		computedvalues.addInterval(add.leftLimit,add.rightLimit);
	}

	while(it1!=end1){
		Interval add = *it1;

		if (add.leftLimit > last)
			return;

		if (add.rightLimit > last)
			add.rightLimit = last;

		computedvalues.addInterval(add.leftLimit,add.rightLimit);
		it1++;
	}

	while(it2!=end2){
		Interval add = *it2;

		if (add.leftLimit > last)
			return;

		if (add.rightLimit > last)
			add.rightLimit = last;

		computedvalues.addInterval(add.leftLimit,add.rightLimit);
		it2++;
	}
}



OrValidatorNode::OrValidatorNode (ValidatorNode &child1, ValidatorNode &child2): buffer(0.0,0.0), computedValues(0.0,0.0)
	{
		RealType fmt = child1.minTime(), smt = child2.minTime();

		if (fmt <= smt)
		{
			firstchild = &child1;
			secondchild = &child2;
			mintime = smt;
		}
		else
		{
			firstchild = &child2;
			secondchild = &child1;
			mintime = fmt;
		}
	}

void OrValidatorNode::start(RealType ts, const std::vector<BooleanType> &preds)
{
	 // setting  the state of the object
	 buffer.reset(ts,ts);
	 computedValues.reset(ts,ts);

	 // calls on the recursive structure (starting the whole sub-tree)
	 firstchild->start(ts,preds);
	 secondchild->start(ts,preds);
}

void OrValidatorNode::update(RealType t, const std::vector<BooleanType> &preds)
{
	// function calls on the recursive structure (updating the whole sub-tree)
	firstchild->update(t,preds);
	secondchild->update(t,preds);

	const Signal &s1 = firstchild -> getValues();
	const Signal &s2 = secondchild -> getValues();

	// update of the buffer's values
	buffer.append(s1);

	// computing union between buffer and the second child computed signal
	computeUnion(buffer, s2, computedValues);

	// increasing the buffer first domain (decreasing the buffer actual size)
	RealType buffstart = computedValues.getLast();
	buffer.increaseFirst(buffstart);
}



OrValidatorNode::~OrValidatorNode(void)
{
	delete firstchild;
	delete secondchild;
}






