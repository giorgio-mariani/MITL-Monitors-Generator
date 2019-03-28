#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include <vector>

#include "type_transl.h"
#include "misc.h"
#include "mex.h"

// forward declarations
 class Monitor;
 class ValidatorNode;
 ValidatorNode* buildValidator(const mxArray*);

 /**
  \brief Class used to validate a Bounded LTL formula.

	This class monitor that a certain formula *f* given a certain trace \f$\mu\f$ is always valid.
	The trace is defined by the calling of the methods: initialConditions and extendTrace, while validity is checked with the method checkSafety.
  */
 class Monitor
 {
 private:
 	ValidatorNode *formula; /**< negative of the formula to be validated*/
 	Signal evaluation; /**< values of the formula so far*/
 	bool isstarted;	/**< whether or not the monitor has an been started*/

 public:
 	Monitor(const mxArray *);
 	~Monitor(void);

 	void initialConditions(RealType, const std::vector<BooleanType>&);
 	void extendTrace(RealType, const std::vector<BooleanType>&);

 	/**
 	 \brief returns the value where the formula is false.
 	 \returns the value where the formula is false.
 	 */
 	inline const Signal& formulaEvaluation(void) {return evaluation;}

 	/**
 	 \brief Check if the formula is ever false
 	 More precisely it is checked if for the instants between first trace instant and last trace instant - formula.mintime() the formula is ever false.
 	 \returns true if and only if the formula is true for all the instant between the first trace instant and last trace instant - formula.mintime().
 	 */
 	inline bool checkSafety(void) const {return evaluation.getIntervalCount() == 0;}

 	/**
 	 \brief check if the monitor is started
 	 \returns true if and only if initialConditions was already called on the monitor.
 	 */
 	inline bool isStarted(void) const {return isstarted;}
 };

/**
 \brief Interface used to validate a Bounded LTL formula.

 More precisely each class realizing ValidatorNode correspond to a certain syntactic operator of the BLTL grammar, and a set of objects
 (of type ValidatorNode) organized in a tree represent a syntax tree of a BLTL formula.
 So in a way we can define a formula from a tree of objects of type ValidatorNode, we call that formula the *implict formula* or simply the formula.

 Using the methods of the class is possible to compute the values of the implicit formula for a certain range of instants.
 */
 class ValidatorNode
 {
 public:
	 /**
	 \brief Method that initialize the values of the predicates in *this*.

	\warning
	start must not have been already invoked on *this* or any of its descendants.

	 \param ts first instant of the trace
	 \param preds value of the predicate in the formula from *ts* until the next call of update

	\exception invalid_argument if preds size is not big enough to contain the values of all the predicates in the formula (i.e. if it
	exists a descendant of *this* of type PredicateValidatorNode *p* with index >= preds.size)

	This method is used to define the value of the formula predicates descendant of *this*. Particularly  if *this* is of type PredicateValidatorNode
	then it will assume for the instant *ts* the value in preds[index of *this*].
  */
	 virtual void start(RealType ts, const std::vector<BooleanType> &preds) = 0;

	 /**
	\brief Method that update the instants where the formula is true.

	\warning
	start must have been already invoked on *this* and all of its descendants.

	\param t instant (the parameter must have a value greater than the one it had at the last call)
	\param preds value of the formula predicate for *t*

	\exception invalid_argument if preds size is not big enough to contain the values of all the predicates in the formula (i.e. if it
	exists a descendant of *this* of type PredicateValidatorNode *p* with index >= preds.size)

	This method update the state of *this* and all of its descendants so that the next call at
	compute will return the instants when the formula is true in the interval defined as follows:

	- Let \f$t_i\f$ be the value of *t* in the current call.
	- Let \f$t_s\f$ be the value of *ts* the value of the parameter *ts* during the invocation of start on *this*.
	- Let \f$t_{i-1}\f$ be the value of *t* in the last call or if no such call exists \f$t_s\f$).
	- Let \f$\delta\f$ be the value returned by the invocation of *this*.minTime().

	then  the interval for which the formula values are computed is (if \f$t_s\ge \delta\f$) \f$[t_{i-1} -\delta, t_i - \delta))\f$.

	More generally speaking the instants for which the formula is evaluated are:
	\f[
	 \big[\max(ts, t_{i-1} - \delta),\max(ts, t_i - \delta)\big)
	 \f]
	 */
	 virtual void update(RealType t, const std::vector<BooleanType> &preds) = 0;

	 /**
	\brief This methods returns the instants between the last two calls at update (offset by a certain value) where the implicit formula is valid.
	More precisely:
	- Let \f$t_i\f$ be the value of *t* in the last invocation of update on *this*.
	- Let \f$t_s\f$ be the value of *ts* the value of the parameter *ts* during the invocation of start on *this*.
	- Let \f$t_{i-1}\f$ be the value of *t* in the last call of update or if no such call exists \f$t_s\f$).
	- Let \f$\delta\f$ be the value returned by the invocation of *this*.minTime().

	then *this*.getValues() returns the instants in (if \f$t_s\ge \delta\f$) \f$[t_{i-1} -\delta, t_i - \delta))\f$ where the formula is true.
	More generally speaking the instants returned are in the interval:
	\f[
	\big[\max(ts, t_{i-1} - \delta),\max(ts, t_i - \delta)\big)
	\f]
	\returns the instants in \f$\big[\max(ts, t_{i-1} - \delta),\max(ts, t_i - \delta)\big)\f$ where the formula is valid. (the instants are returned as a Signal)
	*/
	 virtual const Signal& getValues(void) const = 0;

	 /**
	\brief This method return the minimum time that we need to "wait" in order to evaluate a certain instant.
	More precisely if we want to evaluate the implicit formula for a certain instant \f$t\f$ then we must  have the value of the predicates
	in the formula for at least the instants \f$[t,t+\delta)\f$, with \f$\delta\f$ the value returned by *this*.minTime().

	Another reason of why this value is interesting is the following:
	Let be \f$t_i\f$ the value of the parameter *t* in the i-th of update then, if \f$t_i\f$ is greater
	than the value returned by this method, the Signal returned by compute doesn't hav an empty domain (i.e. last > first).

	\returns the minimum time in order to be sure to be able to evaluate a certain instant.
	 */
	 virtual RealType minTime(void) const = 0;

	 /**
	  \brief Class destructor.
	  The only important thing about  the destructor is that it will deallocate not only the ValidatorNode caller but every descendant node also.
	  */
	 virtual ~ValidatorNode(void) {}
};

 class BooleanValidatorNode : public ValidatorNode{
 private:
	 bool state;
	 RealType lastUpdateTime;
	 RealType currentUpdateTime;
	 Signal computedValues;
 public:
	 BooleanValidatorNode(bool);
	 void start(RealType ts, const std::vector<BooleanType> &preds);
	 void update(RealType t, const  std::vector<BooleanType> &preds);

	 inline const Signal& getValues(void) const {return computedValues;}
	 inline RealType minTime(void) const {return RT_ZERO;}
 };

 class PredicateValidatorNode:public ValidatorNode
  {
  public:
   	typedef Signal::size_type predicate_index;

  private:
	 predicate_index index;	/*index used to get the node predicate value in the input vector*/
	 RealType lastUpdateTime;
	 RealType currentUpdateTime;
	 BooleanType lastvalue;
	 BooleanType currentValue;

  	// optimization fields
  	Signal computedValues;

  public:
  	PredicateValidatorNode(PredicateValidatorNode::predicate_index i);
  	void start(RealType ts, const std::vector<BooleanType> &preds);
  	void update(RealType t, const std::vector<BooleanType> &preds);

  	inline const Signal& getValues(void) const{return computedValues;};
  	inline RealType minTime(void) const {return RT_ZERO;};

  };

 class NotValidatorNode:public ValidatorNode
 {
 private:
 	ValidatorNode *child;

 	// optimization fields
 	double mintime;
 	Signal computedValues;

 public:
 	NotValidatorNode(ValidatorNode &c);
 	void start(RealType ts, const std::vector<BooleanType> &preds);
 	void update(RealType t, const std::vector<BooleanType> &preds);
  	~NotValidatorNode(void);

 	inline const Signal& getValues(void) const {return computedValues;};
 	inline RealType minTime(void) const {return mintime;};

 };

class OrValidatorNode:public ValidatorNode
{
private:
	ValidatorNode *firstchild;
	ValidatorNode *secondchild;
	Signal buffer;

	// optimization fields
	RealType mintime;
	Signal computedValues;

public:
	OrValidatorNode (ValidatorNode &child1, ValidatorNode &child2);
	void start(RealType ts, const std::vector<BooleanType> &preds);
	void update(RealType t, const std::vector<BooleanType> &preds);

	inline const Signal& getValues(void)  const {return computedValues;};
	inline RealType minTime(void) const {return mintime;};

  	~OrValidatorNode(void);
};

class UntilValidatorNode:public ValidatorNode
{
private:
	ValidatorNode *firstchild;
	ValidatorNode *secondchild;
	RealType alpha;
	Signal buffer1;
	Signal buffer2;

	// optimization fields
	RealType max;
	Signal computedValues;

public:
	UntilValidatorNode (ValidatorNode &child1, ValidatorNode &child2, RealType alpha);
	void start(RealType ts, const std::vector<BooleanType> &preds);
	void update(RealType t, const std::vector<BooleanType> &preds);
  	~UntilValidatorNode(void);

	inline const Signal& getValues(void)  const {return computedValues;};
	inline RealType minTime(void) const {return max + alpha;};

};
#endif
