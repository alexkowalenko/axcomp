MODULE index04; (* open arrays *)
IMPORT Out;
VAR x : ARRAY OF INTEGER;
    sum, i: INTEGER;
BEGIN
    NEW(x, 10);
    x[2] := 2;
    FOR i := 0 TO 9 DO
        x[i] := i*i + x[i];
    END
    FOR i := 0 TO 9 DO
        Out.Int(x[i], 2) Out.Char(' ');
    END
    Out.Ln;
END index04.

(*
RUN: %comp %s | filecheck %s
CHECK: 1 6 9 16 25 36 49 64 81
CHECK: 0
*)