#include <algorithm>
#include <stdexcept>

#include "misc.h"
#include "validators.h"


NotValidatorNode::NotValidatorNode(ValidatorNode &c):child(&c), computedValues(0.0,0.0)
{
	mintime = child->minTime();
}

void NotValidatorNode::start(RealType ts, const std::vector<BooleanType> &preds)
{
	computedValues.reset(ts,ts);
	child->start(ts,preds);
}

void NotValidatorNode::update(RealType t, const std::vector<BooleanType> &preds)
{
	child->update(t,preds);
	const Signal &s=child->getValues();

	RealType first = s.getFirst();
	RealType last = s.getLast();

	computedValues.reset(first,last);

	// get iterator on first and last elements (which behaviors are not undefined since there are elements in the signal)
	Signal::const_iterator it = s.getBegin(), end = s.getEnd();

	// first element in the signal domain
	RealType it_right = first;

	// first element in the first interval
	RealType it_left = first;

	while(it != end){
		it_left = it->leftLimit;
		computedValues.addInterval(it_right,it_left);
		it_right = it->rightLimit;
		it++;
	}
	computedValues.addInterval(it_right,last);
}



NotValidatorNode::~NotValidatorNode(void)
{
	delete child;
}
