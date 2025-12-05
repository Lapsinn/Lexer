start
    number x = 10;
    number y = 5;
    decimal result = 0.0;
    
    result = x + y * 2;
    show(result);
    
    show("The value of x is: ");
    show(x);
    
    number a = 20;
    number b = 3;
    decimal division = a / b;
    show(division);
    
    number power = 2 ^ 8;
    show(power);
    
    number modulo = 17 % 5;
    show(modulo);
    
    if (x > 5)
    {
        show("x is greater than 5");
    }
    else
    {
        show("x is not greater than 5");
    }
    
    if (y <= 10)
    {
        show("y is less than or equal to 10");
    }
    
    if (a >= 15 && b < 5)
    {
        show("Both conditions are true");
    }
    
    if (x == 10 || y == 3)
    {
        show("At least one condition is true");
    }
    
    repeat(number i = 0; i < 5; i += 1)
    {
        show(i);
    }
    
    repeat(number i = 0; i < 10; i += 1)
    {
        if (i == 5)
        {
            stop;
        }
        
        if (i == 2)
        {
            continue;
        }
        
        show(i);
    }
    
    repeat(number row = 1; row <= 3; row += 1)
    {
        repeat(number col = 1; col <= 2; col += 1)
        {
            show(row);
            show(col);
        }
    }
    
    number var1 = 100;
    number var2 = 200;
    decimal total = var1 + var2;
    show(total);
    
    decimal price = 50.00;
    price += 10.50;
    show(price);
    
    price -= 5.25;
    show(price);
    
    price *= 2;
    show(price);
    
    price /= 3;
    show(price);
    
    decimal complex = (10 + 5) * (3 - 1);
    show(complex);
    
    show("Program completed successfully");
end