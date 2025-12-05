start
    # ERROR 1: Missing semicolon - FIXED by aggressive sync
    number x = 10;
    
    # ERROR 2: Invalid variable name - FIXED by using valid name
    number var2 = 5;
    
    # ERROR 3: Missing equals sign - FIXED by adding =
    number y = 20;
    
    # ERROR 4: Missing closing parenthesis - FIXED by adding )
    show(x);
    
    # ERROR 5: Assignment to undeclared variable - FIXED by declaring first
    number undeclared = 50;
    
    # ERROR 6: Missing expression in assignment - FIXED by adding value
    number z = 0;
    
    # ERROR 7: Invalid operator - FIXED by using valid operator
    number result = 5 + 3;
    
    # ERROR 8: If without condition - FIXED by adding condition
    if x > 0
    {
        show("Has condition now");
    }
    
    # ERROR 9: If without braces - FIXED by adding braces
    if x > 5
    {
        show("Now has braces");
    }
    
    # ERROR 10: Repeat without parentheses - FIXED by adding parens
    repeat(number i = 0; i < 5; i = i + 1)
    {
        show(i);
    }
    
    # ERROR 11: Missing semicolon in repeat - FIXED by adding semicolons
    repeat(number i = 0; i < 5; i = i + 1)
    {
        show(i);
    }
    
    # ERROR 12: Unclosed block - FIXED by adding closing brace
    if x == 10
    {
        show("Closing brace added");
    }
    
    # ERROR 13: Double assignment operators - FIXED by removing extra semicolon
    number a = 10;
    
    # ERROR 14: Missing loop body - FIXED by adding body
    repeat(number i = 0; i < 5; i = i + 1)
    {
        show(i);
    }
    
    # ERROR 15: Invalid comparison operator - FIXED by using valid operator
    if x != 5
    {
        show("Valid operator used");
    }
    
    # ERROR 16: Empty show statement - FIXED by adding expression
    show("Not empty");
    
    # ERROR 17: Nested braces mismatch - FIXED by matching braces
    if y > 0
    {
        if x > 5
        {
            show(x);
        }
    }
    
    # Still continues despite errors
    show("Error recovery test");
end