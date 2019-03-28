#include <limits>
#include <string>
#include <sstream>
#include <stdexcept>

#include "mex.h"
#include "validators.h"

using std::string; using std::exception;

// defining the values of nodetype
#define MTS_PREDICATE 	0
#define	MTS_NOT 		1
#define	MTS_AND 		2
#define	MTS_OR 			3
#define	MTS_FUTURE  	4
#define	MTS_GLOBALLY  	5
#define	MTS_UNTIL 		6

//defining the fields of the syntax tree node
#define MTS_NODETYPE 		"NodeType"
#define MTS_PREDICATE_INDEX "PredicateIndex"
#define	MTS_ONLY_CHILD 		"ChildNode"
#define	MTS_FIRST_CHILD  	"FirstChildNode"
#define	MTS_SECOND_CHILD 	"SecondChildNode"
#define	MTS_ALPHA 			"Alpha"

static ValidatorNode* predicateBehaviour( const mxArray*);
static ValidatorNode* notBehaviour( const mxArray*);
static ValidatorNode* andBehaviour(const mxArray *formulatree);
static ValidatorNode* orBehaviour( const mxArray*);
static ValidatorNode* futureBehaviour(const mxArray *formulatree);
static ValidatorNode* globallyBehaviour(const mxArray *formulatree);
static ValidatorNode* untilBehaviour( const mxArray*);

static void checkError(bool, std::string);
static void getChildren(const mxArray * const formula, const mxArray **firstchild, const mxArray **secondchild);
static void getOnlyChild(const mxArray * const formula, const mxArray **const child);
static RealType getAlpha(const mxArray * const formula);


/*
 PRE-CONDITIONS buildValidator:
	error (pointer to char array) must not be NULL

POST-CONDITIONS buildValidator:
	Let val be the returned value, then:
		* val correctly simulates the input syntax tree.
 */
ValidatorNode *buildValidator( const  mxArray *formulatree)
{

	checkError(formulatree == NULL, "Null pointer exception.");
	checkError(!mxIsStruct(formulatree), "Input MATLAB object must be a structure.");
	checkError(!mxIsScalar(formulatree), "Input MATLAB object must be a scalar.");

	const mxArray *nodetypearr = mxGetField(formulatree, 0 ,MTS_NODETYPE);

	checkError(nodetypearr == NULL,"The field " MTS_NODETYPE " is not defined in the input structure.");
	checkError(!mxIsNumeric(nodetypearr),"The field " MTS_NODETYPE " must be a numeric type.");
	checkError(!mxIsScalar(nodetypearr), "The field " MTS_NODETYPE " must be a scalar.");

	int nodetype = int(mxGetScalar(nodetypearr));

	// variable that will point to the new allocated object that will be returned from the method call.
	ValidatorNode *val = NULL;

	switch(nodetype)
	{
	case MTS_PREDICATE:
		val = predicateBehaviour( formulatree);
		break;

	case MTS_NOT:
		val = notBehaviour( formulatree);
		break;

	case MTS_FUTURE:
		val = futureBehaviour(formulatree);
		break;

	case MTS_GLOBALLY:
		val = globallyBehaviour(formulatree);
		break;

	case MTS_AND:
		val = andBehaviour(formulatree);
		break;

	case MTS_OR:
		val = orBehaviour( formulatree);
		break;

	case MTS_UNTIL:
		val = untilBehaviour( formulatree);
		break;

	default:
			checkError(true,"Input node type is not valid.");
	}
	return val;
}

static ValidatorNode* predicateBehaviour(const mxArray *formulatree)
{
	checkError(formulatree == NULL,"The input pointer must not point to null.");

	const mxArray *pred_arr = mxGetField(formulatree, 0 ,MTS_PREDICATE_INDEX);

	checkError(pred_arr == NULL,"The field " MTS_PREDICATE_INDEX " is not defined in the input structure.");
	checkError(!mxIsNumeric(pred_arr),"The field " MTS_PREDICATE_INDEX " must be a numeric type.");
	checkError(!mxIsScalar(pred_arr),"The field " MTS_PREDICATE_INDEX " must be a scalar.");

	PredicateValidatorNode::predicate_index predindex = static_cast<PredicateValidatorNode::predicate_index>(mxGetScalar(pred_arr));
	return new PredicateValidatorNode(predindex);
}

static ValidatorNode* notBehaviour(const mxArray *formulatree)
{
	checkError(formulatree == NULL,"The input pointer must not point to null.");
	const mxArray * child = NULL;

	getOnlyChild(formulatree, &child);

	ValidatorNode *childval = buildValidator(child);
	ValidatorNode *out = NULL;
	out = new NotValidatorNode(*childval);

	return out;
}

static ValidatorNode* orBehaviour(const mxArray *formulatree)
{
	checkError(formulatree == NULL,"The input pointer must not point to null.");
	const mxArray * firstchild = NULL;
	const mxArray * secondchild = NULL;

	getChildren(formulatree, &firstchild, &secondchild);

	ValidatorNode* firstchildval = NULL;
	ValidatorNode* secondchildval = NULL;
	ValidatorNode* out =  NULL;

	// trying to build the output validator
	try
	{
		firstchildval = buildValidator(firstchild);
		secondchildval =  buildValidator(secondchild);
		out = new OrValidatorNode(*firstchildval, *secondchildval);
	}
	catch (exception &e) 	// de-allocating allocated resources
	{
		if (out != NULL)
			delete out;
		else
		{
			if (firstchildval != NULL)
				delete firstchildval;

			if (secondchildval != NULL)
				delete secondchildval;
		}
		throw;
	}

	return  out;
}

static ValidatorNode* andBehaviour(const mxArray *formulatree)
{
	checkError(formulatree == NULL,"The input pointer must not point to null.");
	const mxArray * firstchild = NULL;
	const mxArray * secondchild = NULL;

	getChildren(formulatree, &firstchild, &secondchild);

	ValidatorNode* firstchildval = NULL;
	ValidatorNode* secondchildval = NULL;
	ValidatorNode* out =  NULL;

	// trying to build the output validator
	try
	{
		firstchildval = buildValidator(firstchild);
		firstchildval = new NotValidatorNode(*firstchildval);

		secondchildval =  buildValidator(secondchild);
		secondchildval =  new NotValidatorNode(*secondchildval);

		out = new OrValidatorNode(*firstchildval,*secondchildval);
		out = new NotValidatorNode(*out);
	}
	catch (exception &e) 	// de-allocating allocated resources
	{
		if (out != NULL)
			delete out;
		else
		{
			if (firstchildval != NULL)
				delete firstchildval;

			if (secondchildval != NULL)
				delete secondchildval;
		}
		throw;
	}

	return  out;
}

static ValidatorNode* untilBehaviour(const mxArray *formulatree)
{
	checkError(formulatree == NULL,"The input pointer must not point to null.");
	const mxArray * firstchild = NULL;
	const mxArray * secondchild = NULL;

	getChildren(formulatree, &firstchild, &secondchild);

	ValidatorNode* firstchildval = NULL;
	ValidatorNode* secondchildval = NULL;
	ValidatorNode* out =  NULL;

	// trying to build the output validator
	try
	{
		firstchildval = buildValidator(firstchild);
		secondchildval =  buildValidator(secondchild);

		out = new UntilValidatorNode(*firstchildval,*secondchildval,getAlpha(formulatree));
	}
	catch (exception &e) 	// de-allocating allocated resources
	{
		if (out != NULL)
			delete out;
		else
		{
			if (firstchildval != NULL)
				delete firstchildval;

			if (secondchildval != NULL)
				delete secondchildval;
		}
		throw;
	}
	return out;
}

static ValidatorNode* futureBehaviour(const mxArray *formulatree)
{
	checkError(formulatree == NULL,"The input pointer must not point to null.");
	const mxArray * secondchild = NULL;

	getOnlyChild(formulatree, &secondchild);

	ValidatorNode* firstchildval = NULL;
	ValidatorNode* secondchildval = NULL;
	ValidatorNode* out =  NULL;

	// trying to build the output validator
	try
	{
		firstchildval = new BooleanValidatorNode(true);
		secondchildval =  buildValidator(secondchild);

		out = new UntilValidatorNode(*firstchildval,*secondchildval,getAlpha(formulatree));
	}
	catch (exception &e) 	// de-allocating allocated resources
	{
		if (out != NULL)
			delete out;
		else
		{
			if (firstchildval != NULL)
				delete firstchildval;

			if (secondchildval != NULL)
				delete secondchildval;
		}
		throw;
	}
	return out;
}

static ValidatorNode* globallyBehaviour(const mxArray *formulatree)
{
	checkError(formulatree == NULL,"The input pointer must not point to null.");
	const mxArray * secondchild = NULL;

	getOnlyChild(formulatree, &secondchild);

	ValidatorNode* firstchildval = NULL;
	ValidatorNode* secondchildval = NULL;
	ValidatorNode* out =  NULL;

	// trying to build the output validatorNode
	try
	{
		firstchildval = new BooleanValidatorNode(true);
		secondchildval =  buildValidator(secondchild);
		secondchildval = new NotValidatorNode(*secondchildval);

		out = new UntilValidatorNode(*firstchildval,*secondchildval,getAlpha(formulatree));
		out = new NotValidatorNode(*out);
	}
	catch (exception &e) 	// de-allocating allocated resources
	{
		if (out != NULL)
			delete out;
		else
		{
			if (firstchildval != NULL)
				delete firstchildval;

			if (secondchildval != NULL)
				delete secondchildval;
		}
		throw;
	}
	return out;
}


/*
 PRE-CONDITIONS getChildren:
	 * all the input pointers must not be null.
 	 * formula must point to a structure.
 	 * formula must point to a scalar array.
 	 * formula must have the fields 'FirstChildNode' and 'SecondChildNode'.

 POST-CONDITIONS getChildren:
 	 The following must be true after the function execution:

 	 	 * firstchild must point to a pointer pointing the value of the field 'FirstChildNode'
 	 	 * secondchild must point to a pointer pointing the value of the field 'SecondChildNode'
 */
static void getChildren(const mxArray * const formula, const mxArray **firstchild, const mxArray **secondchild)
{
	checkError(formula == NULL,"The first input pointer must not point to null.");
	checkError(firstchild == NULL,"The second input pointer must not point to null.");
	checkError(secondchild == NULL,"The third input pointer must not point to null.");

	checkError(!mxIsStruct(formula), "The first input pointer must point to a structure.");
	checkError(!mxIsScalar(formula),"The first input pointer must point to a scalar structure.");

	*firstchild = mxGetField(formula, 0 ,MTS_FIRST_CHILD);
	*secondchild =  mxGetField(formula, 0 ,MTS_SECOND_CHILD);

	checkError(*secondchild == NULL,"The field " MTS_FIRST_CHILD " is not defined in the input structure.");
	checkError(*secondchild == NULL,"The field " MTS_SECOND_CHILD " is not defined in the input structure.");
}

/*
 PRE-CONDITIONS getOnlyChild:
	 * all the input pointers must not be to null.
 	 * formula must point to a structure.
 	 * formula must point to a scalar array.
 	 * formula must have the field 'ChildNode'.

 POST-CONDITIONS getOnlyChild:
 	 The following must be true after the function execution:

 	 	 child must point to a pointer pointing the value of the field 'ChildNode'
 */
static void getOnlyChild(const mxArray * const formula, const mxArray **const child)
{
	checkError(formula == NULL,"The first input pointer must not point to null.");
	checkError(child == NULL,"The second input pointer must not point to null.");

	checkError(!mxIsStruct(formula), "The first input pointer must point to a structure.");
	checkError(!mxIsScalar(formula),"The first input pointer must point to a scalar structure.");

	*child = mxGetField(formula, 0 ,MTS_ONLY_CHILD);
	checkError(*child == NULL,"The field " MTS_ONLY_CHILD " is not defined in the input structure.");
}

/*
 PRE-CONDITIONS getAlpha:
	 * formula must not be to null.
 	 * formula must point to a structure.
 	 * formula must point to a scalar structure.
 	 * formula must have the field 'Alpha'.

 POST-CONDITIONS getAlpha:
 	 The following must be true after the function execution:

 	 	 the returned value is equal to the value in the field 'Alpha'.
 */
static RealType getAlpha(const mxArray * const formula)
{
	checkError(formula == NULL,"The first input pointer must not point to null.");
	checkError(!mxIsStruct(formula), "The first input pointer must point to a structure.");
	checkError(!mxIsScalar(formula),"The first input pointer must point to a scalar structure.");

	const mxArray *alpha_arr = mxGetField(formula, 0 ,MTS_ALPHA);

	checkError(alpha_arr == NULL,"The field " MTS_ALPHA " is not defined in the input structure.");
	checkError(!mxIsNumeric(alpha_arr),"The field " MTS_ALPHA " must be a numeric type.");
	checkError(!mxIsScalar(alpha_arr),"The field " MTS_ALPHA " must be a scalar.");

	RealType alpha = static_cast<RealType> (mxGetScalar(alpha_arr));
	return alpha;
}

static void checkError(const bool condition, std::string errstring)
{
	if(condition)
		throw std::invalid_argument(errstring);
}
