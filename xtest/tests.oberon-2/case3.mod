MODULE case3; (* CASE *)
IMPORT Out;
VAR c : CHAR;

BEGIN
    FOR i := 1 TO 4 DO
        c := CHR(64+ i);
        CASE c OF
            'A' : Out.String("A"); Out.Ln;
        |   'B' : Out.String("B"); Out.Ln;
        |   'C' : Out.String("C"); Out.Ln;
        ELSE
            Out.String('D-Z'); Out.Ln;
        END
    END
    RETURN 0;
END case3.