(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 1
CHECK-NEXT: 4.71828
CHECK-NEXT: 3
*)

MODULE shortint; (* compatibity *)

IMPORT Out, Math;

TYPE I = INTEGER;

VAR x : SHORTINT;
    xx : I;
    y : LONGINT;
    z : HUGEINT;
    f : LONGREAL;
    
BEGIN
    x := 1;
    xx := 2;
    x := xx + 1;
    y := x - 2;
    z := y * xx * x;

    f := Math.e + xx;

    Out.Int(y, 0); Out.Ln;
    Out.Int(y, 0); Out.Ln;
    Out.Real(f, 0); Out.Ln;

    RETURN x;
END shortint.