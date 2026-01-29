(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_real_mixed;
IMPORT Out;
VAR
    x, y  : REAL;
BEGIN
    y := 3.0;
    x := y;
    Out.Real(x, 3); Out.Ln;
END assignment_real_mixed.

(*
CHECK: 3
*)
