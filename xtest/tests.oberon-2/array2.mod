MODULE array2; (* Multi-dimension arrays *)
IMPORT Out;
VAR x : ARRAY 3, 4, 5 OF INTEGER;
    sum : INTEGER;
BEGIN
    FOR i := 0 TO 2 DO
        FOR j := 0 TO 3 DO
            FOR k := 0 TO 4 DO
                x[i,j, k] := sum;
                INC(sum);
            END
        END
    END
    FOR i := 0 TO 2 DO
        FOR j := 0 TO 3 DO
            FOR k := 0 TO 4 DO
                Out.Int(i, 0) Out.Char(','); Out.Int(j, 0) Out.Char(','); Out.Int(k, 0) Out.Char('='); 
                Out.Int(x[i,j,k], 0) Out.Char(' ');
            END
            Out.Ln;
        END
        Out.Ln;
    END
END array2.