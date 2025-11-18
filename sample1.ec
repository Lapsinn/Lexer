start

    # Declare variables for input and the result
    number inputNumber;
    number factorial = 1;

    # Get input from the user
    show "Enter a non-negative whole number: ";
    inputNumber = ask();

    # Check for invalid input (negative numbers)
    if inputNumber < 0 {
        show "Error: Factorial is not defined for negative numbers.";
    }

    # Handle the case for 0! = 1
    else if inputNumber == 0 {
        show "The factorial of 0 is 1.";
    }

    # Calculate factorial for positive numbers
    else {
        # Loop: initialization, condition, increment
        repeat (number i = inputNumber; i >= 1; prev i) {
            # Scale (multiplication assignment)
            factorial *= i;
        }

        # Output
        show inputNumber; 
    }

end
