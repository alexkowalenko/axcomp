MODULE case5;
IMPORT Out;

BEGIN
    FOR i := 1 TO 5 DO
        CASE i OF
            1 .. 2 : Out.String("A"); Out.Ln;
        ELSE
            Out.String('D-Z'); Out.Ln;
        END
    END
    RETURN 0
END case5.