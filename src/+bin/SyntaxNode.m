classdef SyntaxNode
    
    properties(SetAccess = private)
        syntaxnode;
    end
    
    properties(Constant)
        BOOLEAN   = 0;
        PREDICATE = 1;
        NOT       = 2;
        AND       = 3;
        OR        = 4;
        FUTURE    = 5;
        GLOBALLY  = 6;
        UNTIL     = 7;
    end
    
    methods
        function this = SyntaxNode(syntaxnode)
            narginchk(1,1);
            nargoutchk(0,1);
            validateattributes(syntaxnode,{'syntaxtree.SyntaxNode'},{'nonempty','scalar'},1);         
            this.syntaxnode = syntaxnode;
        end
        
        function name = getFormulaName(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,1);
            
            tmp = this.syntaxnode.getData.get(DataFields.FORMULA_NAME);
            if isempty(tmp)
                error('Expected formula name to be present in the input node.');
            end
            name = char(tmp);
        end
        
        function nodetype = getNodeType(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            javanodetype =  this.syntaxnode.getNodeType();
            if javanodetype == NodeType.BOOLEAN
                nodetype = bin.SyntaxNode.BOOLEAN;
                
            elseif javanodetype == NodeType.PREDICATE
                nodetype = bin.SyntaxNode.PREDICATE;

            elseif javanodetype == NodeType.NOT
                nodetype = bin.SyntaxNode.NOT;
                
            elseif javanodetype == NodeType.AND
                nodetype = bin.SyntaxNode.AND;
                
            elseif javanodetype == NodeType.OR
                nodetype = bin.SyntaxNode.OR;
                
            elseif javanodetype == NodeType.FUTURE
                nodetype = bin.SyntaxNode.FUTURE;
                
            elseif javanodetype == NodeType.GLOBALLY
                nodetype = bin.SyntaxNode.GLOBALLY;
                
            elseif javanodetype == NodeType.UNTIL
                nodetype = bin.SyntaxNode.UNTIL;
                
            end
                
        end
        
        function bool =  getBooleanData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,1);
            
            if this.getNodeType ~= bin.SyntaxNode.BOOLEAN
                error('Expected caller to be a boolean predicate');
            else
                bool = char(this.syntaxnode.getData.get(DataFields.VALUE));
            end
        end
       
        function [coefficients,variables, relop, constraint] = getPredicateData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,4);
            
            if this.getNodeType ~= bin.SyntaxNode.PREDICATE
                error('Expected caller to be a predicate');
            else            
                variables = cellstr(char(this.syntaxnode.getData.get(DataFields.VARIABLES)));
                coefficients = cellstr(char(this.syntaxnode.getData.get(DataFields.COEFFICIENTS)));
                constraint = char(this.syntaxnode.getData.get(DataFields.CONSTRAINT));
                relop = char(this.syntaxnode.getData.get(DataFields.RELATION));
            end
        end
        
        function child = getNotData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,1);
            
            if this.getNodeType ~= bin.SyntaxNode.NOT
                error('Expected caller to be a not node');
            else
             child = bin.SyntaxNode(this.syntaxnode.getFirstChildNode);
            end
        end
        
        function [child1,child2] = getOrData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,2);
            
            if this.getNodeType ~= bin.SyntaxNode.OR
                error('Expected caller to be an or node');
            else
                child1 = bin.SyntaxNode(this.syntaxnode.getFirstChildNode);
                child2 =  bin.SyntaxNode(this.syntaxnode.getSecondChildNode);
            end
        end
        
        function [child1,child2] = getAndData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,2);
            
            if this.getNodeType ~= bin.SyntaxNode.AND
                error('Expected caller to be an and node');
            else
                child1 = bin.SyntaxNode(this.syntaxnode.getFirstChildNode);
                child2 =  bin.SyntaxNode(this.syntaxnode.getSecondChildNode);
            end
        end
        
        function [child,alpha] = getFutureData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,2);
            
            if this.getNodeType ~= bin.SyntaxNode.FUTURE
                error('Expected caller to be a future node');
            else
                child = bin.SyntaxNode(this.syntaxnode.getFirstChildNode);
                alpha = char(this.syntaxnode.getData.get(DataFields.ALPHA));
            end
        end
        
        function [child,alpha] = getGloballyData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,2);
            
            if this.getNodeType ~= bin.SyntaxNode.GLOBALLY
                error('Expected caller to be a globally node');
            else
                child = bin.SyntaxNode(this.syntaxnode.getFirstChildNode);
                alpha = char(this.syntaxnode.getData.get(DataFields.ALPHA));
            end
        end
        
        function [child1,child2,alpha] = getUntilData(this)
            import syntaxtree.*;
            import bin.SyntaxNode;
            
            narginchk(1,1);
            nargoutchk(0,3);
            
            if this.getNodeType ~= bin.SyntaxNode.UNTIL
                error('Expected caller to be an until node');
            else
                child1 = bin.SyntaxNode(this.syntaxnode.getFirstChildNode);
                child2 =  bin.SyntaxNode(this.syntaxnode.getSecondChildNode);
                alpha = char(this.syntaxnode.getData.get(DataFields.ALPHA));
            end
        end       
    end
end

