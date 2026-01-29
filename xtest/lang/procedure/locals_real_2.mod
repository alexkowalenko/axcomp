(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE locals_real_2;

IMPORT Out;

PROCEDURE f (x: REAL) : REAL;
VAR
    y : REAL;
BEGIN
    y := 4.0;
    RETURN x * y * y;
END f;

BEGIN
    Out.Real(f(3.14159), 6); WriteLn;
END locals_real_2.

(*
CHECK: 50.2654
*)
