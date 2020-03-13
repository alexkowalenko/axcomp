MODULE e11; (* REPEAT block *)

VAR x : INTEGER;
VAR y : INTEGER;

BEGIN
    x := 0;
    REPEAT 
        y := 0;
        REPEAT
            x := x + 1;
            y := y + 1;
        UNTIL y > 9;
    UNTIL x > 100;
    RETURN x; 

END e11.