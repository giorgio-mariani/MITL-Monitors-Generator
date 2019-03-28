
/* the name of the S-function.*/
#define S_FUNCTION_NAME  monitor_sfun
#define S_FUNCTION_LEVEL 2

/*
 * Need to include simstruc.h for the definition of the SimStruct and
 * its associated macro definitions.
 *
 * The following headers are included by matlabroot/simulink/include/simstruc.h
 * when compiling as a MEX file:
 *
 *   matlabroot/extern/include/tmwtypes.h    - General types, e.g. real_T
 *   matlabroot/extern/include/mex.h         - MATLAB MEX file API routines
 *   matlabroot/extern/include/matrix.h      - MATLAB MEX file API routines
 *
 * The following headers are included by matlabroot/simulink/include/simstruc.h
 * when compiling your S-function with RTW:
 *
 *   matlabroot/extern/include/tmwtypes.h    - General types, e.g. real_T
 *   matlabroot/rtw/c/libsrc/rt_matrx.h      - Macros for MATLAB API routines
 *
 */
#include "simstruc.h"
#include "validators.h"

#include <vector>
#include <stdexcept>
using std::vector;using std::exception;

/* Error handling
 * --------------
 *
 * You should use the following technique to report errors encountered within
 * an S-function:
 *
 *       ssSetErrorStatus(S,"error encountered due to ...");
 *       return;
 *
 * Note that the 2nd argument to ssSetErrorStatus must be persistent memory.
 * It cannot be a local variable in your procedure. For example the following
 * will cause unpredictable errors:
 *
 *      mdlOutputs()
 *      {
 *         char msg[256];         {ILLEGAL: to fix use "static char msg[256];"}
 *         sprintf(msg,"Error due to %s", string);
 *         ssSetErrorStatus(S,msg);
 *         return;
 *      }
 *
 * The ssSetErrorStatus error handling approach is the suggested alternative
 * to using the mexErrMsgTxt function.  MexErrMsgTxt uses "exception handling"
 * to immediately terminate S-function execution and return control to
 * Simulink. In order to support exception handling inside of S-functions,
 * Simulink must setup exception handlers prior to each S-function invocation.
 * This introduces overhead into simulation.
 *
 * If you do not call mexErrMsgTxt or any other routines that cause exceptions,
 * then you should use SS_OPTION_EXCEPTION_FREE_CODE S-function option.  This
 * is done by issuing the following command in the mdlInitializeSizes function:
 *
 *      ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
 *
 * Setting this option, will increase the performance of your S-function by
 * allowing Simulink to bypass the exception handling setup that is usually
 * performed prior to each S-function invocation.  Extreme care must be taken
 * to verify that your code is exception free when using the
 * SS_OPTION_EXCEPTION_FREE_CODE option.  If your S-function generates
 * an exception when this option is set, unpredictable results will occur.
 *
 * Exception free code refers to code which never "long jumps". Your S-function
 * is not exception free if it contains any routine which when called has
 * the potential of long jumping. For example mexErrMsgTxt throws an exception
 * (i.e. long jumps) when called, thus ending execution of your S-function.
 * Use of mxCalloc may cause unpredictable problems in the event of a memory
 * allocation error since mxCalloc will long jump. If memory allocation is
 * needed, you should use the stdlib.h calloc routine directly and perform
 * your own error handling.
 *
 * All mex* routines have the potential of long jumping (i.e. throwing an
 * exception). In addition several mx* routines have the potential of
 * long jumping. To avoid any difficulties, only the routines which get
 * a pointer or determine the size of parameters should be used. For example
 * the following will never throw an exception: mxGetPr, mxGetData,
 * mxGetNumberOfDimensions, mxGetM, mxGetN, mxGetNumberOfElements.
 *
 * If all of your "run-time" methods within your S-function are exception
 * free, then you can use the option:
 *      ssSetOptions(S, SS_OPTION_RUNTIME_EXCEPTION_FREE_CODE);
 * The other methods in your S-function need not be exception free. The
 * run-time methods include any of the following:
 *    mdlGetTimeOfNextVarHit, mdlOutputs, mdlUpdate, and mdlDerivatives
 *
 * Warnings & Printf's
 * -------------------
 *   You can use ssWarning(S,msg) to display a warning.
 *    - When the S-function is compiled via mex for use with Simulink,
 *      ssWarning equates to mexWarnMsgTxt.
 *    - When the S-function is used with Real-Time Workshop,
 *      ssWarning(S,msg) equates to
 *        printf("Warning: in block '%s', '%s'\n", ssGetPath(S),msg);
 *      if the target has stdio facilities, otherwise it becomes a comment and
 *      is disabled.
 *
 *   You can use ssPrintf(fmt, ...) to print a message.
 *    - When the S-function is compiled via mex for use with Simulink,
 *      ssPrintf equates to mexPrintf.
 *    - When the S-function is used with Real-Time Workshop,
 *      ssPrintf equates to printf, if the target has stdio facilities,
 *      otherwise it becomes a call to a empty function (rtPrintfNoOp).
 *    - In the case of Real-Time Workshop which may or may not have stdio
 *      facilities, to generate the most efficient code use:
 *         #if defined(SS_STDIO_AVAILABLE)
 *            ssPrintf("my message ...");
 *         #endif
 *    - You can also use this technique to do other standard I/O related items,
 *      such as:
 *         #if defined(SS_STDIO_AVAILABLE)
 *             if ((fp=fopen(file,"w")) == NULL) {
 *                ssSetErrorStatus(S,"open failed");
 *                return;
 *             }
 *             ...
 *         #endif
 */

/*====================*
 * S-function methods *
 *====================*/

/*
 * Level 2 S-function methods
 * --------------------------
 *    Notation:  "=>" indicates method is required.
 *                [method] indicates method is optional.
 *
 *    Note, many of the methods below are only available for use in level 2
 *    C-MEX S-functions.
 *
 * Model Initialization in Simulink
 * --------------------------------
 *=> mdlInitializeSizes         -  Initialize SimStruct sizes array
 *
 *   [mdlSetInputPortFrameData] -  Optional method. Check and set input and
 *                                 output port frame data attributes.
 *
 *       NOTE: An S-function cannot use mdlSetInput(Output)PortWidth and
 *       mdlSetInput(Output)PortDimensionInfo at the same time. It can use
 *       either a width or dimension method, but not both.
 *
 *   [mdlSetInputPortWidth]     -  Optional method. Check and set input and
 *                                 optionally other port widths.
 *   [mdlSetOutputPortWidth]    -  Optional method. Check and set output
 *                                 and optionally other port widths.
 *
 *   [mdlSetInputPortDimensionInfo]
 *                              -  Optional method. Check and set input and
 *                                 optionally other port dimensions.
 *   [mdlSetOutputPortDimensionInfo]
 *                              -  Optional method. Check and set output
 *                                 and optionally other port dimensions.
 *   [mdlSetDefaultPortDimensionInfo]
 *                               - Optional method. Set dimensions of all
 *                                 input and output ports that have unknown
 *                                 dimensions.
 *
 *   [mdlSetInputPortSampleTime] - Optional method. Check and set input
 *                                 port sample time and optionally other port
 *                                 sample times.
 *   [mdlSetOutputPortSampleTime]- Optional method. Check and set output
 *                                 port sample time and optionally other port
 *                                 sample times.
 *=> mdlInitializeSampleTimes   -  Initialize sample times and optionally
 *                                 function-call connections.
 *
 *   [mdlSetInputPortDataType]    - Optional method. Check and set input port
 *                                  data type. See SS_DOUBLE to SS_BOOLEAN in
 *                                  simstruc_types.h for built-in data types.
 *   [mdlSetOutputPortDataType]   - Optional method. Check and set output port
 *                                  data type. See SS_DOUBLE to SS_BOOLEAN in
 *                                  simstruc_types.h for built-in data types.
 *   [mdlSetDefaultPortDataTypes] - Optional method. Set data types of all
 *                                  dynamically typed input and output ports.
 *
 *   [mdlInputPortComplexSignal]  - Optional method. Check and set input
 *                                  port complexity attribute (COMPLEX_YES,
 *                                  COMPLEX_NO).
 *   [mdlOutputPortComplexSignal] - Optional method. Check and set output
 *                                  port complexity attribute (COMPLEX_YES,
 *                                  COMPLEX_NO).
 *   [mdlSetDefaultPortComplexSignals]
 *                                - Optional method. Set complex signal flags
 *                                  of all input and output ports who
 *                                  have their complex signals set to
 *                                  COMPLEX_INHERITED (dynamic complexity).
 *
 *   [mdlSetWorkWidths]         -  Optional method. Set the state, iwork,
 *                                 rwork, pwork, dwork, etc sizes.
 *
 *   [mdlStart]                 -  Optional method. Perform actions such
 *                                 as allocating memory and attaching to pwork
 *                                 elements.
 *
 *   [mdlInitializeConditions]  -  Initialize model parameters (usually
 *                                 states). Will not be called if your
 *                                 S-function does not have an initialize
 *                                 conditions method.
 *
 *   ['constant' mdlOutputs]    -  Execute blocks with constant sample
 *                                 times. These are only executed once
 *                                 here.
 *
 *   [mdlSetSimState]           -  Optional method. Load the complete simulation
 *                                 state for this block, which is called when
 *                                 starting the simulation from an initial
 *                                 simulation state and this s-function has set
 *                                 its ssSetSimStateCompliance to
 *                                 USE_CUSTOM_SIM_STATE. See also mdlGetSimState
 *
 * Model simulation loop in Simulink
 * ---------------------------------
 *   [mdlCheckParameters]       -  Optional method. Will be called at
 *                                 any time during the simulation loop when
 *                                 parameters change.
 *   SimulationLoop:
 *        [mdlProcessParameters]   -  Optional method. Called during
 *                                    simulation after parameters have been
 *                                    changed and verified to be okay by
 *                                    mdlCheckParameters. The processing is
 *                                    done at the "top" of the simulation loop
 *                                    when it is safe to process the changed
 *                                    parameters.
 *        [mdlGetTimeOfNextVarHit] -  Optional method. If your S-function
 *                                    has a variable step sample time, then
 *                                    this method will be called.
 *        [mdlInitializeConditions]-  Optional method. Only called if your
 *                                    S-function resides in an enabled
 *                                    subsystem configured to reset states,
 *                                    and the subsystem has just enabled.
 *     => mdlOutputs               -  Major output call (usually updates
 *                                    output signals).
 *        [mdlUpdate]              -  Update the discrete states, etc.
 *
 *        Integration (Minor time step)
 *          [mdlDerivatives]         -  Compute the derivatives.
 *          Do
 *            [mdlOutputs]
 *            [mdlDerivatives]
 *          EndDo - number of iterations depends on solver
 *          Do
 *            [mdlOutputs]
 *            [mdlZeroCrossings]
 *          EndDo - number of iterations depends on zero crossings signals
 *        EndIntegration
 *
 *   EndSimulationLoop
 *
 *   [mdlGetSimState]  -  Optional method. Called to get the complete simulation
 *                        state for this block if the model is configured to
 *                        save its final simulation state and this
 *                        S-Function has set its ssSetSimStateCompliance to
 *                        USE_CUSTOM_SIM_STATE. See also mdlSetSimState
 *
 *   => mdlTerminate   -  End of model housekeeping - free memory, etc.
 *
 * Model initialization for code generation (rtwgen)
 * -------------------------------------------------
 *   <Initialization. See "Model Initialization in Simulink" above>
 *
 *   [mdlRTW]                   -  Optional method.  Only called when
 *                                 generating code to add information to the
 *                                 model.rtw file which is used by the
 *                                 Real-Time Workshop.
 *
 *   mdlTerminate               -  End of model housekeeping - free memory,
 *                                 etc.
 *
 * Noninlined S-function execution in Real-Time Workshop
 * -----------------------------------------------------
 *   1) The results of most initialization methods are 'compiled' into
 *      the generated code and many methods are not called.
 *   2) Noninlined S-functions are limited in several ways, for example
 *      parameter must be real (non-complex) double vectors or strings. More
 *      capability is provided via the Target Language Compiler.  See the
 *      Target Language Compiler Reference Guide.
 *
 * => mdlInitializeSizes        -  Initialize SimStruct sizes array
 * => mdlInitializeSampleTimes  -  Initialize sample times and optionally
 *                                 function-call connections.
 *   [mdlInitializeConditions]  -  Initialize model parameters (usually
 *                                 states). Will not be called if your
 *                                 S-function does not have an initialize
 *                                 conditions method.
 *   [mdlStart]                 -  Optional method. Perform actions such
 *                                 as allocating memory and attaching to pwork
 *                                 elements.
 *   ExecutionLoop:
 *     => mdlOutputs            -  Major output call (usually updates
 *                                 output signals).
 *        [mdlUpdate]           -  Update the discrete states, etc.
 *
 *        Integration (Minor time step)
 *          [mdlDerivatives]         -  Compute the derivatives.
 *          Do
 *            [mdlOutputs]
 *            [mdlDerivatives]
 *          EndDo - number of iterations depends on solver
 *          Do
 *            [mdlOutputs]
 *            [mdlZeroCrossings]
 *          EndDo - number of iterations depends on zero crossings signals
 *   EndExecutionLoop
 *   mdlTerminate               -  End of model housekeeping - free memory,
 *                                 etc.
 */

/*=====================================*
 * Configuration and execution methods *
 *=====================================*/

static const int_T formulaParamIdx = 0;
static const int_T formulaPtrIdx = 0;
static const int_T vectorPtrIdx = 1;


static inline Monitor*& getFormulaPtr(SimStruct *S)
{
    Monitor** tmp =  (Monitor**)(ssGetPWork(S)+formulaPtrIdx);
    return *tmp;
}
static inline vector<boolean_T>*& getVectorPtr(SimStruct *S)
{
    vector<boolean_T>** tmp =  (vector<boolean_T>**)(ssGetPWork(S)+vectorPtrIdx);
    return *tmp;
}

static inline boolean_T& getOutputPortSig(SimStruct *S)
{
	return static_cast<boolean_T*>(ssGetOutputPortSignal(S,0))[0];
}

static inline InputPtrsType getInputPortSig(SimStruct *S) {return ssGetInputPortSignalPtrs(S,0);}
static inline int_T getInputPortWidth(SimStruct *S) {return ssGetInputPortWidth(S,0);}


/* Function: mdlInitializeSizes ===============================================
 * Abstract:
 *    The sizes information is used by Simulink to determine the S-function
 *    block's characteristics (number of inputs, outputs, states, etc.).
 */
static void mdlInitializeSizes(SimStruct *S)
{
    int_T nInputPorts  = 1;  /* number of input ports  */
    int_T nOutputPorts = 1;  /* number of output ports */
    int_T needsInput   = 1;  /* direct feed through    */

    int_T inputPortIdx  = 0;
    int_T outputPortIdx = 0;


    ssSetNumSFcnParams(S, 1);  /* Number of expected parameters */
    int_T formulaIdx = 0;

    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        /*
         * If the the number of expected input parameters is not equal
         * to the number of parameters entered in the dialog box return.
         * Simulink will generate an error indicating that there is a
         * parameter mismatch.
         */
        return;
    }

    ssSetSFcnParamTunable(S, formulaIdx, 0); /* First input parameter is not tunable*/

    /* Register the number and type of states the S-Function uses*/
    ssSetNumContStates( S, 0);   /* number of continuous states*/
    ssSetNumDiscStates( S, 0);   /* number of discrete states*/

    /*
     * Configure the input ports. First set the number of input ports.
     */
    if (!ssSetNumInputPorts(S, nInputPorts)) return;

    /*
     * Set input port dimensions for each input port index starting at 0.
     * The following options summarize different ways for setting the input
     * port dimensions.
     *
     * (1) If the input port dimensions are unknown, use
     *     ssSetInputPortDimensionInfo(S, inputPortIdx, DYNAMIC_DIMENSION))
     *
     * (2) If the input signal is an unoriented vector, and the input port
     *     width is w, use
     *     ssSetInputPortVectorDimension(S, inputPortIdx, w)
     *     w (or width) can be DYNAMICALLY_SIZED or greater than 0.
     *     This is equivalent to ssSetInputPortWidth(S, inputPortIdx, w).
     *
     * (3) If the input signal is a matrix of dimension mxn, use
     *     ssSetInputPortMatrixDimensions(S, inputPortIdx, m, n)
     *     m and n can be DYNAMICALLY_SIZED or greater than zero.
     *
     * (4) Otherwise use:
     *     ssSetInputPortDimensionInfo(S, inputPortIdx, dimsInfo)
     *     This function can be used to fully or partially initialize the port
     *     dimensions. dimsInfo is a structure containing width, number of
     *     dimensions, and dimensions of the port.
     */
    if(!ssSetInputPortVectorDimension(S, inputPortIdx, DYNAMICALLY_SIZED)) return;
    ssSetInputPortDataType(S,inputPortIdx,SS_BOOLEAN);

    /*
     * Set direct feedthrough flag (1=yes, 0=no).
     * A port has direct feedthrough if the input is used in either
     * the mdlOutputs or mdlGetTimeOfNextVarHit functions.
     * See sfuntmpl_directfeed.txt.
     */
    ssSetInputPortDirectFeedThrough(S, inputPortIdx, needsInput);

    /* Configure the output ports. First set the number of output ports.*/
    if (!ssSetNumOutputPorts(S, nOutputPorts)) return;

    /*
     * Set output port dimensions for each output port index starting at 0.
     * See comments for setting input port dimensions.
     */
    if(!ssSetOutputPortVectorDimension(S, outputPortIdx, 1)) return;
    ssSetOutputPortDataType(S,outputPortIdx,SS_BOOLEAN);


    /*
     * Set the number of sample times. This must be a positive, nonzero
     * integer indicating the number of sample times or it can be
     * PORT_BASED_SAMPLE_TIMES. For multi-rate S-functions, the
     * suggested approach to setting sample times is via the port
     * based sample times method. When you create a multirate
     * S-function, care needs to be taking to verify that when
     * slower tasks are preempted that your S-function correctly
     * manages data as to avoid race conditions. When port based
     * sample times are specified, the block cannot inherit a constant
     * sample time at any port.
     */
    ssSetNumSampleTimes(S, 1);   /* number of sample times                */

    /* Set size of the work vectors.*/
    ssSetNumRWork( S, 0);  /* number of real work vector elements   */
    ssSetNumIWork( S, 0);  /* number of integer work vector elements*/
    ssSetNumPWork( S, 2);  /* number of pointer work vector elements*/
    ssSetNumModes( S, 0);  /* number of mode work vector elements   */
    ssSetNumNonsampledZCs( S, 0);   /* number of nonsampled zero crossings   */

    /* Specify the sim state compliance to be same as a built-in block */
    /* see sfun_simstate.c for example of other possible settings */
    ssSetSimStateCompliance(S, DISALLOW_SIM_STATE);

    /*
     * All options have the form SS_OPTION_<name> and are documented in
     * matlabroot/simulink/include/simstruc.h. The options should be
     * bitwise or'd together as in
     *   ssSetOptions(S, (SS_OPTION_name1 | SS_OPTION_name2))
     */
    ssSetOptions( S, 0);   /* general options (SS_OPTION_xx)*/
}



/* Function: mdlInitializeSampleTimes =========================================
 * Abstract:
 *
 *    This function is used to specify the sample time(s) for your S-function.
 *    You must register the same number of sample times as specified in
 *    ssSetNumSampleTimes. If you specify that you have no sample times, then
 *    the S-function is assumed to have one inherited sample time.
 *
 *    The sample times are specified as pairs "[sample_time, offset_time]"
 *    via the following macros:
 *      ssSetSampleTime(S, sampleTimePairIndex, sample_time)
 *      ssSetOffsetTime(S, offsetTimePairIndex, offset_time)
 *    Where sampleTimePairIndex starts at 0.
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, CONTINUOUS_SAMPLE_TIME );   	/* continuos sample time*/
    ssSetOffsetTime(S, 0, FIXED_IN_MINOR_STEP_OFFSET); 	/* fixed during minor timesteps*/
}


#define MDL_START  /* Change to #undef to remove function */
#if defined(MDL_START)
  /* Function: mdlStart =======================================================
   * Abstract:
   *    This function is called once at start of model execution. If you
   *    have states that should be initialized once, this is the place
   *    to do it.
   */
  static void mdlStart(SimStruct *S)
  {
	  const mxArray *formulaMex = ssGetSFcnParam(S, formulaParamIdx); /* Get input parameter's pointer*/

		 Monitor *&formulaPtr = getFormulaPtr(S);
		 vector<boolean_T> *&vectorPtr = getVectorPtr(S);

	  formulaPtr = NULL;
	  vectorPtr = NULL;

	  try{
		  formulaPtr = new Monitor(formulaMex);
	  }
	  catch(exception &e)
	  {
		  mexErrMsgTxt(e.what());
	  }
	  vectorPtr = new vector<boolean_T>(ssGetInputPortWidth(S,0));
  }
#endif





/* Function: mdlOutputs =======================================================
 * Abstract:
 *    In this function, you compute the outputs of your S-function
 *    block. Generally outputs are placed in the output vector(s),
 *    ssGetOutputPortSignal.
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{
	 Monitor *&formulaPtr = getFormulaPtr(S);	/* get formula pointer*/
	 vector<boolean_T> *&vectorPtr = getVectorPtr(S);	/* get predicate vector pointer*/

	 InputPtrsType inputs = getInputPortSig(S);	/* input values*/

	 /* Updating the vector -------------------------------------------------------*/
	 vector<boolean_T>::iterator it = vectorPtr->begin(),end = vectorPtr->end();
	 for(int_T i=0; i<getInputPortWidth(S); i++, it++)
	 {
		 mxAssert(it != end,"Error in mdlOutputs: Input width different than vector size");
		 boolean_T b = *(static_cast<const boolean_T* const>(inputs[i]));
		 *it = b;
	 }

	 /* Updating the formula validator---------------------------------------------*/
	 try{
		 if(formulaPtr->isStarted())
			 formulaPtr->extendTrace(ssGetT(S), *vectorPtr);
		 else
			 formulaPtr->initialConditions(ssGetT(S),*vectorPtr);

		 /* Updating the output---------------------------------------------*/
		 boolean_T &y  = getOutputPortSig(S);
		 y = !(formulaPtr->checkSafety());
	 }
	 catch(exception &e)
	 {
		 mexErrMsgTxt(e.what());
	 }
}

/* Function: mdlTerminate =====================================================
 * Abstract:
 *    In this function, you should perform any actions that are necessary
 *    at the termination of a simulation.  For example, if memory was allocated
 *    in mdlStart, this is the place to free it.
 */
static void mdlTerminate(SimStruct *S)
{
	 Monitor *&formulaPtr = getFormulaPtr(S);
	 vector<boolean_T> *&vectorPtr = getVectorPtr(S);

	if (formulaPtr != NULL)
	{
		delete formulaPtr;
		formulaPtr = NULL;
	}

	if (vectorPtr != NULL)
	{
		delete vectorPtr;
		vectorPtr = NULL;
	}
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
