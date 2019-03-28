#include <algorithm>
#include <stdexcept>

#include "misc.h"
#include "validators.h"



PredicateValidatorNode::PredicateValidatorNode(predicate_index i)
:index(i),
 lastUpdateTime(),
 currentUpdateTime(),
 lastvalue(),
 currentValue(),
 computedValues(0.0,0.0)
{}


void PredicateValidatorNode::start(RealType ts, const std::vector<BooleanType> &preds)
{
	if(index >= preds.size())
		throw std::invalid_argument("start: Index of the predicate must be less then the input predicate vector's size.");

	currentUpdateTime = ts;
	lastUpdateTime = ts;
	currentValue = preds[index];
	lastvalue = currentValue;
	computedValues.reset(ts,ts);
}

void PredicateValidatorNode::update(RealType t, const std::vector<BooleanType> &preds)
{
	if(index >= preds.size())
			throw std::invalid_argument("Index of the predicate is must be less then the input predicate vector's size.");

	if(t < currentUpdateTime)
		throw std::invalid_argument("Input time-step must be greater then or equal to the last input time-step.");

	lastUpdateTime = currentUpdateTime;
	currentUpdateTime = t;
	lastvalue = currentValue;
	currentValue = preds[index];

	computedValues.reset(lastUpdateTime,currentUpdateTime);
	if (lastvalue)
		computedValues.addInterval(lastUpdateTime,currentUpdateTime);
}
