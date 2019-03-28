function addmonitor(systemName, coord, syntaxTree)
    narginchk(3,3);
    validateattributes(systemName,{'char'},{'nonempty','row'});
    validateattributes(coord,{'double'},{'size',[1,4]});
    validateattributes(syntaxTree,{'bin.SyntaxNode'},{'nonempty','vector'});

    INPORT     = 'simulink/Sources/In1';
    OUTPORT    = 'simulink/Sinks/Out1';
    CONSTANT   = 'simulink/Sources/Constant';
    MUX        = 'simulink/Signal Routing/Mux';
    ADD        = 'simulink/Math Operations/Add';
    GAIN       = 'simulink/Math Operations/Gain';
    RELATION   = 'simulink/Logic and Bit Operations/Compare To Constant';
    S_FUNCTION = 'simulink/User-Defined Functions/S-Function';
    SUBSYSTEM  = 'built-in/SubSystem';

    S_FUNCTION_MEXFILE = 'monitor_sfun';
    MODEL_NAME         = [systemName, '/', syntaxTree.getFormulaName()];

    POSITION1=0;
    POSITION2=100;
    POSITION3=200;
    POSITION4=300;
    POSITION5=400;
    POSITION6=500;
    POSITION7=600;
    WIDTH=50;
    SPACE1=20;
    SPACE2=20;

    % Creazione del sistema
    subsystem = add_block(SUBSYSTEM, MODEL_NAME, 'Position', coord);

    % Aggiunta maschera
    mask = Simulink.Mask.create(subsystem);
    mask.addParameter('Evaluate','off','Tunable','off','Enabled','off','Visible','off');

    % Visita albero sintattico, costruzione e aggiunta dei blocchi predicati.
    [predicates,yposition] = AddPredicates(0, syntaxTree,[]);

    % Aggiunta MUX
    muxposition1 = 0;
    muxposition2 = yposition - SPACE2;
    mux = AddMUX(muxposition1,muxposition2,predicates);

    % Aggiunta S-Function
    sfunposition = (yposition - SPACE2 - WIDTH)/2;
    sfun = AddSFunction(sfunposition, mux, syntaxTree);

    % Aggiunta porta di output
    AddOutputPort(sfunposition, sfun);

%--------------------------------------------------------------------------
    function outport = AddOutputPort(yposition, sfun)
        position = [POSITION7 yposition POSITION7+WIDTH, yposition+WIDTH];
        
        outport = add_block(OUTPORT, strcat(MODEL_NAME,'/out'), 'Position', position);
        
        set_param(outport, 'OutDataTypeStr', 'boolean');
        set_param(outport, 'PortDimensions','1');
        set_param(outport, 'VarSizeSig','No');
        set_param(outport, 'SignalType','real');
        
        sfunports = get_param(sfun,'PortHandles');
        opports = get_param(outport,'PortHandles');

        add_line(MODEL_NAME, sfunports.Outport(1), opports.Inport(1));
    end


%--------------------------------------------------------------------------
    function mux = AddMUX(yposition1,yposition2, predicates)
        position = [POSITION5, yposition1 ,POSITION5+WIDTH/5, yposition2];

        mux = add_block(MUX,strcat(MODEL_NAME,'/MG_MUX'),'Inputs', ...
            num2str(length(predicates)), 'Position', position );
        
        
        muxports =  get_param(mux,'PortHandles');
        
        for index = 1:length(predicates)
            predports = get_param(predicates(index),'PortHandles');
            outport = predports.Outport(1);
            
            innport = muxports.Inport(index);
            add_line(MODEL_NAME,outport,innport);
        end
    end


%--------------------------------------------------------------------------
    function sfun = AddSFunction(yposition, mux, syntaxtree)
        muxports =  get_param(mux,'PortHandles');
        position = [POSITION6 yposition POSITION6+WIDTH, yposition+WIDTH];
        
        sfun = strcat(MODEL_NAME,'/MG_SFUNCTION');
        add_block(S_FUNCTION, sfun,'Position',position,'Parameters', bin.contree(syntaxtree));
        set_param(sfun,'FunctionName',S_FUNCTION_MEXFILE);
        
        sfunports = get_param(sfun,'PortHandles');
        add_line(MODEL_NAME, muxports.Outport(1), sfunports.Inport(1));
    end


%--------------------------------------------------------------------------
    function [predicates, newyposition] = AddPredicates(yposition, syntaxnode,lpredicates)
        import bin.SyntaxNode;
        
        nodetype = syntaxnode.getNodeType;
        prednum = length(lpredicates);
        predstr = num2str(prednum);
        
        
        if nodetype == SyntaxNode.BOOLEAN
            bool =  syntaxnode.getBooleanData;
            position = [POSITION4, yposition, POSITION4+WIDTH, yposition+WIDTH];
            
            block = add_block(CONSTANT,strcat(MODEL_NAME,'/MG_BOOL_', predstr, 'OutDataTypeStr', 'boolean'));
            set_param(block,'value', bool,'Position',position);
            
            %output
            predicates = [lpredicates,block];
            newyposition = yposition +WIDTH + SPACE2;
            
        elseif nodetype == SyntaxNode.PREDICATE
            [coefficients,variables,relation, constraint] = syntaxnode.getPredicateData;
            [block,newposition] = CreatePredicate(yposition, variables,coefficients, constraint, relation, prednum);
            
            predicates = [lpredicates,block];
            newyposition = newposition + SPACE2;
            
        else
            firstchild =[];
            secondchild = [];
            
            if nodetype == SyntaxNode.NOT
                firstchild = syntaxnode.getNotData;
                
            elseif nodetype == SyntaxNode.FUTURE
                [firstchild, ~] = syntaxnode.getFutureData;
                
            elseif nodetype == SyntaxNode.GLOBALLY
                [firstchild, ~] = syntaxnode.getGloballyData;
                
            elseif nodetype == SyntaxNode.AND
                [firstchild, secondchild] = syntaxnode.getAndData;
                
            elseif nodetype == SyntaxNode.OR
                [firstchild, secondchild] = syntaxnode.getOrData;
                
            elseif nodetype == SyntaxNode.UNTIL
                [firstchild, secondchild, ~] = syntaxnode.getUntilData;
            end
            
            [predicates,newyposition] = AddPredicates(yposition, firstchild,lpredicates);
            
            if ~isempty(secondchild)
                myposition = newyposition + SPACE2; 
                [predicates, newyposition]= AddPredicates(myposition, secondchild, predicates);
            end
        end
    end

    function [predicatehandle, newyposition] = CreatePredicate ( yposition, variables, coefficients,constraint, relation, prednum)
        
        oldposition = yposition;
        predstr =  char(string(prednum));
        
        cmpname = strcat(MODEL_NAME,'/MG_COMP',predstr);
        addname = strcat(MODEL_NAME,'/MG_SUM_', predstr);
        
        
        comp = add_block(RELATION, cmpname, 'const', char(constraint));
        set_param(comp, 'relop',char(relation));
        compports = get_param(comp,'PortHandles');
        
        inputs(1:length(variables)) = '+';
        add = add_block(ADD,addname, 'Inputs',inputs);
        addports = get_param(add,'PortHandles');
        
        add_line(MODEL_NAME,addports.Outport(1),compports.Inport(1));
        
        
        for i = 1:length(variables)
            variable = variables{i};
            coefficient = coefficients{i};
            
            inportname = strcat(MODEL_NAME,'/',variable);
            gainname = strcat(MODEL_NAME,'/MG_GAIN_',variable, predstr);
            
            %creazione blocco gain
            gainports = AddGain(yposition,gainname, inportname,coefficient);
            
            %aggiunta alla somma
            add_line(MODEL_NAME, gainports.Outport(1), addports.Inport(i));
            
            yposition = yposition+WIDTH+SPACE1;
        end
        
        predicatehandle = comp;
        newyposition = yposition-SPACE1;
        
        yposition = (oldposition + newyposition - WIDTH)/2;
        
        addposition = [POSITION3, yposition, POSITION3+WIDTH, yposition+WIDTH];
        cmpposition = [POSITION4, yposition, POSITION4+WIDTH, yposition+WIDTH];
        
        set_param(comp,'Position',cmpposition);
        set_param(add,'Position',addposition);
    end

    function gainports = AddGain(yposition, gainname, inportname, coeff)
        inport = getSimulinkBlockHandle(inportname);
        
        %position
        inportposition = [POSITION1, yposition, POSITION1+WIDTH, yposition+WIDTH];
        gainposition =[POSITION2, yposition, POSITION2+WIDTH, yposition+WIDTH];
        
        if inport == -1
            inport = add_block(INPORT, inportname, 'Position', inportposition);
            
            % set dei parametri della porta in input
            SetInportParameters(inport);
        end
        
        gain = add_block(GAIN,gainname, 'Gain', coeff,'Position',gainposition);
        
        gainports =  get_param(gain, 'PortHandles');
        inportports =  get_param(inport, 'PortHandles');
        
        add_line(MODEL_NAME, inportports.Outport(1), gainports.Inport(1));
    end

    function SetInportParameters(inport)
        set_param(inport, 'OutDataTypeStr', 'double');
        set_param(inport, 'PortDimensions','1');
        set_param(inport, 'VarSizeSig','No');
        set_param(inport, 'SignalType','real');
    end
end
