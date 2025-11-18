start

    # Declaration Statements (Data Types) 
    number num1 set to 10;
    decimal dval set to 1.5;
    letter char set to 'A';
    word w_msg set to "Test";

    # Value Setting Operators 
    increase num1 by 5; 
    decrease num1 by 2;
    scale num1 by 3;
    split num1 by 4;
    remainder num1 by 2;

    # Single Value Operators 
    next num1; 
    prev num1; 
    set dval to pos dval; 
    set dval to neg dval;

    # Input/Output & Logic Operators
    show w_msg;
    set num1 to ask();
    
    if (num1 greater 5 and num1 lessequal 10) { 
        # Math Operators 
        number result set to num1 plus 2 min 1 mul 3 div 2 mod 1 idiv 1 pow 2;

        # Control Flow Keywords 
        repeat 5 times { 
            if result is 0 { 
                stop; 
            }
            if not (result isnt 10) {
                continue;
            }
        }
    } 
    else if (num1 lessequal 5 or num1 greater 10) {
        show "Condition failed.";
    }

    # Second Loop Example
    while (num1 less 10) { 
        next num1;
    }

    # Final Keywords
    else {
        show "Final check.";
    }
    
    exit; 

end