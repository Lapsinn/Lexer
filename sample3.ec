start

    # Declaration Statements (Data Types) 
    number Num1 = 10;
    decimal dval = 1.5;
    letter char = 'A';
    word w_msg = "Test";

    # Value Setting Operators 
    num1 += 5; 
    num1 -= 2;
    num1 *= 3;
    num1 /= 4;
    num1 %= 2;

    # Logic Operators
    show w_msg;
    num1 = ask();
    
    if (num1 > 5 && num1 <= 10) { 
        # Math Operators 
        number result = num1 + 2
        number result = num1 - 1 
        number result = num1 * 3 
        number result = num1 / 2 
        number result = num1 % 1 
        number result = num1 ~ 1 
        number result = num1 ^ 2;

        # Control Flow Keywords 
        repeat 5 times { 
            if result == 0 { 
                stop; 
            }
            if (result != 10) {
                continue;
            }
        }
    } 
    else if (num1 <= 5 or num1 > 10) {
        show "Condition failed.";
    }

    # Second Loop Example
    while (num1 < 10) { 
        num1++;
    }

    # Final Keywords
    else {
        show "Final check.";
    }
    
    exit; 

end