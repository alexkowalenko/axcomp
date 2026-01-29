(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_real_3;

IMPORT Out;

PROCEDURE f (a: INTEGER; x: REAL) ;
BEGIN
    WriteInt(a); WriteLn;
    Out.Real(x, 6); WriteLn;
END f;

BEGIN
    f(8, 3.14159);
END param_real_3.

(*
CHECK: 8
CHECK: 3.14159
*)
