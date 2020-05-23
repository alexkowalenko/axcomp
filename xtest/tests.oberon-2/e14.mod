MODULE e14;
IMPORT Out;
VAR x: INTEGER;

BEGIN
    LOOP
        x := x + 1;
        Out.Int(x); Out.Ln;
        IF x = 10 THEN
            EXIT;
        END
    END
    RETURN 0
END e14.