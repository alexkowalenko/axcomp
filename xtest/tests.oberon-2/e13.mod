MODULE e13;
IMPORT Out;
VAR x: INTEGER;

BEGIN
    LOOP
        x := x + 1;
        Out.Int(x); Out.Ln;
        IF x # 10 THEN
            x := x;
        ELSE
            EXIT;
        END
    END
    RETURN 0
END e13.