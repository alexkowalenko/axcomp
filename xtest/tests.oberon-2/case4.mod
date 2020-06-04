MODULE case4; (* CASE *)
IMPORT Out;
VAR c: CHAR;
    i: INTEGER;

BEGIN
    FOR i := 1 TO 4 DO
        c := CHR(64+ i);
        CASE c OF
            'A' : Out.String("A"); Out.Ln;
        |   'B', 'C' : Out.String("B,C"); Out.Ln;
        ELSE
            Out.String('D-Z'); Out.Ln;
        END
    END
    RETURN 0;
END case4.