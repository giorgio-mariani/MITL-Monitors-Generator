function syntaxtreestr = contree( syntaxtree )
    narginchk(1,1);
    nargoutchk(0,1);

    % validate classe in input
    validateattributes(syntaxtree,{'bin.SyntaxNode'},{'scalar'});

    PREDICATE   = '0';
    NOT         = '1';
    AND         = '2';
    OR          = '3';
    FUTURE      = '4';
    GLOBALLY    = '5';
    UNTIL       = '6';

    NODE_TYPE 		= '''NodeType''';
    PREDICATE_INDEX = '''PredicateIndex''';
    ONLY_CHILD 		= '''ChildNode''';
    FIRST_CHILD  	= '''FirstChildNode''';
    SECOND_CHILD 	= '''SecondChildNode''';
    ALPHA 			= '''Alpha''';

    syntaxtreestr = convertRecursive(syntaxtree,0);

    function [outnode,nextindex] = convertRecursive(syntaxnode, predicateindex)
        import bin.SyntaxNode;
        nodetype = syntaxnode.getNodeType;
        
        if nodetype == SyntaxNode.BOOLEAN || nodetype == SyntaxNode.PREDICATE
            outnode = addPredicateNode(num2str(predicateindex));
            nextindex  = predicateindex + 1;
            
        elseif nodetype == SyntaxNode.NOT
            child = syntaxnode.getNotData;
            [node,nextindex] = convertRecursive(child,predicateindex);
            outnode = addNotNode(node);
            
        elseif nodetype == SyntaxNode.FUTURE
            [firstchild, alpha] = syntaxnode.getFutureData;
            [node,nextindex] = convertRecursive(firstchild,predicateindex);
            outnode = addFutureNode(node,alpha);
            
        elseif nodetype == SyntaxNode.GLOBALLY
            [firstchild, alpha] = syntaxnode.getGloballyData;
            [node,nextindex] = convertRecursive(firstchild,predicateindex);
            outnode = addGloballyNode(node,alpha);
            
            
        elseif nodetype == SyntaxNode.AND
            [firstchild, secondchild] = syntaxnode.getAndData;
            
            [nodeleft,midindex] = convertRecursive(firstchild, predicateindex);
            [noderight,nextindex] = convertRecursive(secondchild, midindex);
            
            outnode = addAndNode(nodeleft,noderight);
            
        elseif nodetype == SyntaxNode.OR
            [firstchild, secondchild] = syntaxnode.getOrData;
            
            [nodeleft,midindex] = convertRecursive(firstchild, predicateindex);
            [noderight,nextindex] = convertRecursive(secondchild, midindex);
            
            outnode = addOrNode(nodeleft,noderight);
            
        elseif nodetype == SyntaxNode.UNTIL
            [firstchild, secondchild, alpha] = syntaxnode.getUntilData;
            
            [nodeleft,midindex] = convertRecursive(firstchild, predicateindex);
            [noderight,nextindex] = convertRecursive(secondchild, midindex);
            
            outnode = addUntilNode(nodeleft,noderight,alpha);
        end
    end

    function node = addPredicateNode(index_str)
        node = ['struct(' , NODE_TYPE, ',', PREDICATE ,',', PREDICATE_INDEX, ',',index_str,')'];
    end

    function node = addNotNode(child_str)
        node = ['struct(' , NODE_TYPE, ',',NOT,',', ONLY_CHILD ,',', child_str ,')'];
    end

    function node = addAndNode(child_str1, child_str2)
        node = ['struct(' , NODE_TYPE,',',AND,',', FIRST_CHILD ,',', child_str1 ,',', SECOND_CHILD ,',', child_str2,')'];
    end

    function node = addOrNode(child_str1, child_str2)
        node = ['struct(' , NODE_TYPE,',',OR,',', FIRST_CHILD ,',', child_str1 ,',', SECOND_CHILD ,',', child_str2,')'];
    end

    function node = addFutureNode(child_str1, alpha_str)
        node = ['struct(', NODE_TYPE,',',FUTURE,',', ONLY_CHILD ,',', child_str1 ,',', ALPHA ,',', alpha_str,')'];
    end

    function node = addGloballyNode(child_str1, alpha_str)
        node = ['struct(', NODE_TYPE ,',', GLOBALLY ,',', ONLY_CHILD ,',', child_str1 ,',', ALPHA ,',', alpha_str,')'];
    end

    function node = addUntilNode(child_str1,child_str2,alpha_str)
        node = ['struct(', NODE_TYPE ,',', UNTIL ,',', FIRST_CHILD ,',', child_str1 ,',', SECOND_CHILD ,',', child_str2,',''Alpha'',', alpha_str,')'];
    end
end
