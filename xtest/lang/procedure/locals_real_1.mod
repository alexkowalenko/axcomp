(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE locals_real_1;

IMPORT Out;

PROCEDURE f () : REAL;
VAR 
    x: REAL;
BEGIN
    x := 3.14159;
    RETURN x;
END f;

BEGIN
    Out.Real(f(), 6); WriteLn;
END locals_real_1.

(*
CHECK: 3.14159
*)
