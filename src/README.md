READ ME
=======
This directory contains the source code of the project; this includes both the matlab scripts/classes and the C++ code used by the validator. The jar file `parser.jar` contains the parser used to parse the formula files.

 * **+monitor_library**: folder containing the C++ classes that implement the MITL validator.
 * **+bin**: folder containing MATLAB scripts invoked during the system execution in order to generate the Simulink validator block.
 * **Parser.jar**: library of java classes used by the system in order to parse the input file. The archive includes both *.class* and *.java* files.
 * **run.bat**: launcher for Windows Operating Systems. Invoke 'run.bat <application input>' to execute the system.
 * **run.sh**: launcer for Unix-based operating Systems. Invoke 'run.bash <application input>' to execute the system.