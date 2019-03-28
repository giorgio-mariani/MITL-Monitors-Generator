#include "validators.h"
#include "mex.h"


Monitor::Monitor(const mxArray *formulatree):formula(NULL),evaluation(0,0),isstarted(false)
{
	ValidatorNode *f_ptr = buildValidator(formulatree);
	formula = new NotValidatorNode(*f_ptr);
}

Monitor::~Monitor()
{
	delete formula;
}

void Monitor::initialConditions(RealType ts, const std::vector<BooleanType> &preds)
{
	formula->start(ts,preds);
	evaluation.reset(ts,ts);
	isstarted = true;
}

void Monitor::extendTrace(RealType ts, const std::vector<BooleanType> &preds)
{
	formula->update(ts,preds);
	evaluation.append(formula->getValues());
}
