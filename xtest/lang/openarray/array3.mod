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
CHECK: 0 1 4 9 16 25 36 49 64 81
CHECK-NEXT: 0
*)
