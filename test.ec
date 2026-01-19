#keywords
and
ask
bool
const
continue
decimal
else
end
false
if
letter
not
null
number
or
repeat
return
show
sizeof
stop
true
while
word

#reserved_words
exit
goto
import
loop
main

#noise_words
each
of
then
also

#assignment_operators
=
+=
-=
*=
/=
%=
~=

#arithmetic_operators
+
-
*
/
%
~
^

#comparison_operators
==
!=
>
<
>=
<=

#logical_operators
&&
||
!

#special_character
?
:

#literals
123          
45.67        
"Hello"      
'A'          

#identifiers
x
value
finalAnswer
count_1

# ===============================
# Global declarations for testing
# ===============================

const number PI = 3.14159;
number global_x = 10;

number add(number a, number b) {
    return a + b;
}

number square(number x) {
    return x * x;
}

number factorial(number n) {
    if (n <= 1) return 1;
    else return n * factorial(n - 1);
}

number sum3(number a, number b, number c) {
    return a + b + c;
}

# ===============================
# Main program: all tests in one
# ===============================

start
    # Test 1: literals
    show(42);
    show(3.14);
    show("hello");
    show('a');
    show(true);
    show(false);
    show(null);

    # Test 2: identifiers and assignment
    number x = 5;
    decimal y = 2.5;
    letter c = 'z';
    bool flag = true;

    show(x);
    x += 3;
    show(x);

    x -= 2;
    show(x);

    x *= 4;
    show(x);

    x /= 2;
    show(x);

    x %= 5;
    show(x);

    # Test 3: arithmetic expressions
    show(2 + 3);
    show(10 - 4);
    show(5 * 6);
    show(20 / 4);
    show(20 ~ 3);
    show(10 % 3);

    show(2 + 3 * 4);
    show(2 * 3 + 4);
    show(2 * 3 ^ 2);
    show(2 + 3 * 4 ^ 2 - 1);

    # Test 4: unary operators
    show(-5);
    show(+5);
    show(!true);
    show(--5);
    show(-(5 + 3));

    # Test 5: relational operators
    show(5 == 5);
    show(5 != 3);
    show(10 > 5);
    show(3 < 10);
    show(10 >= 10);
    show(5 <= 10);
    show(5 + 3 > 2);

    # Test 6: boolean logic
    show(true && false);
    show(true || false);
    show(!false);
    show(true || false && false);
    show(!true && true);
    show(5 > 3 && 2 < 4);
    show(5 > 3 && 2 < 4 || !true);
    show((true || false) && true);

    # Test 7: SHOW with various expressions
    show(x);
    show(global_x);
    show(PI);
    show(5 + 3 * 2);
    show("Value of x:");
    show(x);

    # Test 8: ASK input
    hello = ask(number);
    hello = ask(decimal);
    hello = ask(letter);
    hello = ask(bool);

    # Test 9: if else
    if (x > 5) {
        show("x > 5");
    } else {
        show("x <= 5");
    }

    if (5 > 3 && 2 < 4) {
        show("complex condition true");
    }

    if (x == 1) {
        show(1);
    } else if (x == 2) {
        show(2);
    } else {
        show(3);
    }

    if (true) {
        show("block if works");
    }

    if (true) show("single statement if");

    # Test 10: repeat loops
    repeat(number i = 0; i < 3; i += 1) {
        show(i);
    }

    number j = 0;
    repeat(j = 0; j < 3; j += 1) {
        show(j);
    }

    repeat(number k = 0; k < 5 && k != 3; k += 1) {
        show(k);
    }

    repeat(number a = 0; a < 2; a += 1) {
        repeat(number b = 0; b < 2; b += 1) {
            show(a);
        }
    }

    # Test 11: continue and stop
    repeat(number m = 0; m < 5; m += 1) {
        if (m == 2) continue;
        if (m == 4) stop;
        show(m);
    };

    # Test 12: local variables and blocks
    {
        number inner = 10;
        show(inner);
    }

    # Test 13: function calls
    show(add(2, 3));
    show(square(4));
    show(factorial(5));
    show(sum3(1, 2, 3));

    show(add(add(1, 2), 3));
    show(square(add(2, 3)));

    # Test 14: return statements
    # Parsed via function bodies above

    # Test 15: mixed statement list
    number t = 5;
    show(t);
    t += 2;
    if (t > 5) show(t);
    repeat(number r = 0; r < t; r += 1) {
        show(r);
    };

end
