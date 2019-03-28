#include <vector>
#include <sstream>
#include <stdexcept>

#include "mex.h"
#include "validators.h"

std::stringstream& operator<<(std::ostream& stream, const Signal& s){
	Signal::const_iterator it = s.getBegin();

	stream<<"function:["<<s.getFirst() <<", "<< s.getLast()<<") -> {0,1}"<<"\n";
	stream<< "values: {";
	while (it != s.getEnd()){
		stream << *it << " ";
		it++;
	}
	stream<<"}";
	return stream;
}

std::stringstream& operator<<(std::ostream& stream, const Interval& h)
{
	return stream << "["<<h.leftLimit <<", "<<h.rightLimit<<")";
}



void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if(nrhs<2) mexErrMsgTxt("must have at least two inputs");

    try
    {
        Monitor formula(prhs[0]);

        if(!mxIsDouble(prhs[1])) mexErrMsgTxt("second input must be double array");
        RealType* timeseries = mxGetPr(prhs[1]);
        size_t len = mxGetNumberOfElements(prhs[1]);

        std::vector<BooleanType> preds(nrhs-1);

        for(int i=2;i<nrhs;i++){
        	if(!mxIsLogical(prhs[i])) mexErrMsgTxt("all input after the second must be logical arrays");
        	if(mxGetNumberOfElements(prhs[i]) < len) mexErrMsgTxt("all input after the second must be have at least the number of elements in the second array");

        	preds[i-2]=mxGetLogicals(prhs[i])[0];
        }


        Signal out(timeseries[0],timeseries[0]);
        formula.initialConditions(timeseries[0],preds);

        for (size_t i=1;i<len ;i++){

        	for(int j=2;j<nrhs;j++)
        		preds[j-2]=mxGetLogicals(prhs[j])[i];

            formula.extendTrace(timeseries[i],preds);
        }

        std::stringstream s;
        s << formula.formulaEvaluation() << std::endl << std::endl;
        mexPrintf(s.str().c_str());
    }
    catch (std::exception &e)
    {
        std::string msg("Error during execution:");
        msg +=e.what();
        mexErrMsgTxt(msg.c_str());
    }
}
