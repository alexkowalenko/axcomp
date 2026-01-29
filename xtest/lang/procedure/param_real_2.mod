(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_real_2;

IMPORT Out;

PROCEDURE f (x: REAL; a: INTEGER) ;
BEGIN
    Out.Real(x,4); WriteLn;
    WriteInt(a); WriteLn;
END f;

BEGIN
    f(3.456, 8);
END param_real_2.

(*
CHECK: 3.456
CHECK: 8
*)
