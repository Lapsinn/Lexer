start

    # Declare variables for input and the result
    number inputNumber;
    number factorial set to 1;

    # Get input from the user
    show "Enter a non-negative whole number: ";
    set inputNumber to ask();

    # Check for invalid input (negative numbers)
    if inputNumber less 0 {
        show "Error: Factorial is not defined for negative numbers.";
    }

    # Handle the case for 0! = 1
    else if inputNumber is 0 {
        show "The factorial of 0 is 1.";
    }

    # Calculate factorial for positive numbers
    else {
        # Loop: initialization, condition, increment
        repeat (number i set to inputNumber; i greaterequal 1; prev i) {
            # Scale (multiplication assignment)
            scale factorial by i;
        }

        # Output
        show inputNumber; 
    }

end
