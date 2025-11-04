number grade set to 85;

if grade greaterequal 90 then {
    show "Grade: A";
    show "Excellent work!";
}
else if grade greaterequal 80 then {
    show "Grade: B";
    show "Good job!";
}
else if grade greaterequal 70 then {
    show "Grade: C";
    show "You passed.";
}
else if grade greaterequal 60 then {
    show "Grade: D";
    show "Needs improvement.";
}
else {
    show "Grade: F";
    show "Failed. Please try again.";
}