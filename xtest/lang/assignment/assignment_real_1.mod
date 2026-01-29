(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_real_1;
IMPORT Out;
VAR
    x, y  : REAL;
BEGIN
    y := 3.1415926;
    x := y;
    Out.Real(x, 6); Out.Ln;
END assignment_real_1.

(*
CHECK: 3.14159
*)
