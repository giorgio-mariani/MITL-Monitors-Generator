#include <stdexcept>

#include "validators.h"
#include "misc.h"


BooleanValidatorNode::BooleanValidatorNode(bool value)
:state(value), lastUpdateTime(),
 currentUpdateTime(),computedValues(0.0,0.0)
 {}

void BooleanValidatorNode::start(RealType ts, const std::vector<BooleanType> &preds)
{
	currentUpdateTime = ts;
	lastUpdateTime = ts;
	computedValues.reset(ts,ts);
}

void BooleanValidatorNode::update(RealType t, const std::vector<BooleanType> &preds)
{
	if(t < currentUpdateTime)
		throw std::invalid_argument("Input time-step must be greater then or equal to the last input time-step.");

	lastUpdateTime = currentUpdateTime;
	currentUpdateTime = t;

	computedValues.reset(lastUpdateTime,currentUpdateTime);
	if (state)
		computedValues.addInterval(lastUpdateTime,currentUpdateTime);
}
