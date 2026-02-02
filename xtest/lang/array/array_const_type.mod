MODULE array_const_type;
IMPORT Out;

CONST
    n = 3;

TYPE
    intmatrix = ARRAY n + 2 OF INTEGER;

VAR
    m: intmatrix;

BEGIN
    m[0] := 1;
    m[4] := 5;
    Out.Int(m[0] + m[4], 0); Out.Ln
END array_const_type.

(*
RUN: %comp %s | filecheck %s
CHECK: 6
*)
