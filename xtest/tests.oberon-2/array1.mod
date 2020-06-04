MODULE array1; (* Multi-dimension arrays *)
IMPORT Out;
VAR x : ARRAY 3, 4 OF INTEGER;
    sum, i, j : INTEGER;
BEGIN
    FOR i := 0 TO 2 DO
        FOR j := 0 TO 3 DO
            x[i,j] := sum;
            INC(sum);
        END
    END
    FOR i := 0 TO 2 DO
        FOR j := 0 TO 3 DO
            Out.Int(i, 0) Out.Char(',');  Out.Int(j, 0) Out.Char('='); 
            Out.Int(x[i,j], 0) Out.Char(' ');
        END
        Out.Ln;
    END
END array1.