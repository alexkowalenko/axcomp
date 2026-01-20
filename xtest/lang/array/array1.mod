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

(*
RUN: %comp %s | filecheck %s
CHECK: 0,0=0 0,1=1 0,2=2 0,3=3
CHECK-NEXT: 1,0=4 1,1=5 1,2=6 1,3=7
CHECK-NEXT: 2,0=8 2,1=9 2,2=10 2,3=11
CHECK-NEXT: 0
*)