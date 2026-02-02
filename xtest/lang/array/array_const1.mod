MODULE array_const1;
IMPORT Out;

CONST
    n = 5;

VAR
    a: ARRAY n + 1 OF INTEGER;
    i: INTEGER;
    sum: INTEGER;

BEGIN
    i := 0; sum := 0;
    WHILE i < n + 1 DO
        a[i] := i;
        sum := sum + a[i];
        INC(i)
    END;
    Out.Int(sum, 0); Out.Ln
END array_const1.

(*
RUN: %comp %s | filecheck %s
CHECK: 15
*)
