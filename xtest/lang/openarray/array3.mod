MODULE array3; (* open arrays *)
IMPORT Out;
VAR x : ARRAY OF INTEGER;
    sum, i: INTEGER;
BEGIN
    NEW(x, 10);
    FOR i := 0 TO 9 DO
        x[i] := i*i + i + 1;
    END
    FOR i := 0 TO 9 DO
        Out.Int(x[i], 2) Out.Char(' ');
    END
    Out.Ln;

END array3.

(*
RUN: %comp %s | filecheck %s
CHECK: 1 3 7 13 21 31 43 57 73 91
CHECK-NEXT: 0
*)
