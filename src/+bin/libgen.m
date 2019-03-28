function libgen(formula_file, library_dir, library_name, library_browser)

    % INPUT VARIABLES
    disp("-----------------------------------------------------------------");
    disp("output folder: "+library_dir);
    disp("formula file:  "+formula_file);
    disp("-----------------------------------------------------------------");

    LIB_DIR = library_dir; 
    FORMULA_FILE = formula_file;
    LIBRARY_NAME = library_name;
    LIBRARY_BROWSER_NAME = library_browser;

    SLBLOCKS_FUN = fullfile(LIB_DIR, 'slblocks.m');
    CSOURCE_DIR = fullfile(cd, '+monitor_library');
    JAVA_JAR = 'MITLparser.jar';
    PARSER_CLASS = 'parser.Grammar';
    EXCEPTION_CLASS = 'exceptions.UserException';

    % add parser jar file to the java classpath
    if ~exist(PARSER_CLASS,'class')
        javaaddpath(JAVA_JAR);
    end

    % recommended version of matlab

    % requirement: Simulink MUST be installed
    try 
        disp("Starting Simulink ...")
        start_simulink
        disp("Simulink loaded.");
    catch e
        if strcmp(e.identifier, "MATLAB:UndefinedFunction")
            msg = 'Requirement Error: Simulink not installed.';
            errorBehaviour(msg, 'Error');
        end
    end

    % requirement: there MUST be a C++ compiler available
    if isempty(mex.getCompilerConfigurations('C++'))
        msg = 'Requirement Error: Simulink not installed.';
        errorBehaviour(msg, "Error");
    end

    % requirement: there MUST not be a folder with the same name as the output
    % library
    if exist(LIB_DIR,'dir')
        msg = ['Error during library generation: ',LIB_DIR,' already exists.'];
        errorBehaviour(msg, 'Directory Error');
    end

    %======================================================================
    import bin.SyntaxNode; % node in the syntax tree

    % create empty library and parse input formula file -------------------
    try
        % creation and loading of the library.
        disp('Creating output library ...'); 
        mdl = new_system(LIBRARY_NAME,'Library');
        load_system(mdl);
        set_param(mdl,'EnableLBRepository','on');

        % parse input formula and create syntax tree
        disp('output library created.');
        disp('Parsing input formula.');
        syntaxtreeList = parser.Grammar.createSyntaxTreeFromFile(FORMULA_FILE);
        disp('Formula successfully parsed.');
    catch e
        % if necessary close model
        if exist('mdl', 'var')
            close_system(mdl, 0);
        end

        % display error (different behaviour if the error was raised by the Java parser)
        if  isa(e,'matlab.exception.JavaException') % check whether the exception was due to java 
            je = e.ExceptionObject;
            if isa(je, EXCEPTION_CLASS)
                errorBehaviour(char(je.getMessage),char(je.getId));
            end
        else 
            errorBehaviour(e.message,e.identifier);
        end
    end

    % create and add monitors to the library ------------------------------
    try
        % get the number of formulas in the formula file
        formulacount = syntaxtreeList.size();
        side = floor(sqrt(formulacount));

        yposition = 0;
        xposition = 0;
        width = 50;

        % add monitor in library for each formula
        for i = 1:formulacount
            % Wrap of the syntax tree in a MATLAB class
            syntaxtree = SyntaxNode(syntaxtreeList.get(i-1));
            
            disp("Adding monitor '"+syntaxtree.getFormulaName+"' to the library."); 
            coords = [xposition, yposition, xposition+width, yposition+width];

            if yposition < side
                yposition  = yposition + width*2;
            else
                yposition = 0;
                xposition = xposition + width*2;
            end

            % add block to the sub-system
            bin.addmonitor(LIBRARY_NAME, coords, syntaxtree );
        end
        
        % create output directory
        mkdir(LIB_DIR);
    catch e
        close_system(mdl, 0);
        errorBehaviour(e.message,e.identifier);
    end

    % compile mex function and store library to file ----------------------
    try    
        % compile file mex
        disp('Compiling S-Function ...'); 
        buildMEX(CSOURCE_DIR, LIB_DIR);

        % create slblocks.m
        disp('S-Function compiled.');
        disp('Creation of file ''slblock.m''.');
        fid = fopen(SLBLOCKS_FUN, 'wt' );
        if fid == -1
            errorBehaviour('Unable to create the slblock.m file','File Error');
        end

        fprintf( fid, 'function blkStruct = slblocks\n');
        fprintf( fid, strcat('Browser.Library = ''', LIBRARY_NAME, ''';\n') );
        fprintf( fid,  strcat('Browser.Name = ''', LIBRARY_BROWSER_NAME, ''';\n') );
        fprintf( fid, 'blkStruct.Browser = Browser;\n');

        status = fclose(fid);
        if status == -1
            errorBehaviour('Unable to close the slblock.m file','File Error');
        end

        % save library
        disp(['Saving Library with name''', LIBRARY_NAME, '''.']);
        save_system(mdl,fullfile(LIB_DIR, LIBRARY_NAME));    
    catch e
        rmdir(LIB_DIR,'s');
        close_system(mdl, 0);
        errorBehaviour(e.message,e.identifier);
    end

    % close resources -----------------------------------------------------
    close_system(mdl, 0);
    disp('Execution complete.');
    exit(0); % exit without errors
end

%==========================================================================
function errorBehaviour(msg,id)
    narginchk(1,2);
    nargoutchk(0,0);

    if nargin == 1
        id = 'Error';
    end

    validateattributes(msg,{'char'},{'vector'},1);
    validateattributes(id,{'char'},{'vector'},2);

    h = errordlg(msg,id,'modal');
    uiwait(h);
    exit(1);
end

function buildMEX(sourceDirectory, outputDirectory, debug, mexfun)
    narginchk(2,4);
    nargoutchk(0,0);

    switch nargin
        case 2
            debug = false;
            mexfun = false;
        case 3
            mexfun = false;
    end

    validateattributes(sourceDirectory, {'char'},{'row','nonempty'},1);
    validateattributes(outputDirectory, {'char'},{'row','nonempty'},2);
    validateattributes(debug, {'logical'},{'scalar','nonempty'},3);
    validateattributes(mexfun, {'logical'},{'scalar','nonempty'},4);

    COMP_DIR = sourceDirectory;
    OUTPUT_DIR = outputDirectory;
    HEADERS = ['-I',fullfile(COMP_DIR,'headers')];

    MEX_GATEWAY =       fullfile(COMP_DIR,'matlab','evaluate_formula.cpp');
    S_FUNCTION =        fullfile(COMP_DIR,'matlab','monitor_sfun.cpp');
    VALIDATOR_BUILDER = fullfile(COMP_DIR,'matlab','buildval.cpp');
    VALIDATOR =         fullfile(COMP_DIR,'validators','monitor.cpp');
    SIGNAL =            fullfile(COMP_DIR,'misc','Signal.cpp');
    INTERVAL =          fullfile(COMP_DIR,'misc','Interval.cpp');
    BOOL =              fullfile(COMP_DIR,'validators','boolvalidator.cpp');
    PREDICATE =         fullfile(COMP_DIR,'validators','predicatevalidator.cpp');
    NOT =               fullfile(COMP_DIR,'validators','notvalidator.cpp');
    OR =                fullfile(COMP_DIR,'validators','orvalidator.cpp');
    UNTIL =             fullfile(COMP_DIR,'validators','untilvalidator.cpp');

    if mexfun 
        main = MEX_GATEWAY;
    else
        main = S_FUNCTION;
    end

    if debug
        debugstr='-g';
    else
        debugstr='';
    end

    mex( debugstr, '-outdir',OUTPUT_DIR ,HEADERS,  ...
                        main, VALIDATOR_BUILDER, ...
                        VALIDATOR, SIGNAL, INTERVAL,BOOL, ...
                        PREDICATE, NOT, OR, UNTIL);
end