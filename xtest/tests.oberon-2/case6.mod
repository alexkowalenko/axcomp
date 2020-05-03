MODULE case6;
IMPORT Out;

BEGIN
    FOR i := 1 TO 15 DO
        Out.Int(i); Out.Char(' ');
        CASE i OF
            1 : Out.String("x 1"); Out.Ln;
            | 2 .. 4 : Out.String("x 2..4"); Out.Ln;
            | 5, 8 :  Out.String("x 5, 8"); Out.Ln;
            | 10, 12 .. 14 : Out.String("x 10, 12..14"); Out.Ln;
        ELSE
            Out.String("ELSE"); Out.Ln;
        END
    END
    RETURN 0
END case6.