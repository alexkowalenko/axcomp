(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_real_5;

IMPORT Out;

PROCEDURE f (x: REAL ): REAL ;
BEGIN
    x := 2.0;
    RETURN x;
END f;

BEGIN
    Out.Real(f(3.14159), 2); WriteLn;
END param_real_5.

(*
CHECK: 2
*)
