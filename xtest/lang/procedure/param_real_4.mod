(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_real_4;

IMPORT Out;

PROCEDURE f (a: INTEGER; x: REAL; b: INTEGER) ;
BEGIN
    WriteInt(a); WriteLn;
    Out.Real(x, 6); WriteLn;
    WriteInt(b); WriteLn;
END f;

BEGIN
    f(8, 3.14159, 17);
END param_real_4.

(*
CHECK: 8
CHECK: 3.14159
CHECK: 17
*)
