MODULE array_const2;
IMPORT Out;

CONST
    x = 3;
    y = 4;

VAR
    a: ARRAY x, y + 1 OF INTEGER;
    i: INTEGER;
    j: INTEGER;

BEGIN
    i := 0;
    WHILE i < x DO
        j := 0;
        WHILE j < y + 1 DO
            a[i, j] := i * 10 + j;
            INC(j)
        END;
        INC(i)
    END;
    Out.Int(a[2, 4], 0); Out.Ln
END array_const2.

(*
RUN: %comp %s | filecheck %s
CHECK: 24
*)
