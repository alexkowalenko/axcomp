MODULE case_2; (* CASE *)
IMPORT Out;
VAR i: INTEGER;
BEGIN
    FOR i := 1 TO 4 DO
        CASE i OF
            1 : Out.String('One'); Out.Ln;
        |   2 : Out.String('Two'); Out.Ln;
        |   3 : Out.String('More'); Out.Ln;
        END
    END
    RETURN 0;
END case_2.