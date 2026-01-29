(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_real_1;

IMPORT Out;

PROCEDURE f1 (x: REAL) : REAL;
BEGIN
    RETURN x ;
END f1;

BEGIN
    Out.Real(f1(23.4), 3); WriteLn;
END param_real_1.

(*
CHECK: 23.4
*)
