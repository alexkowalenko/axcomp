(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE return_real_1;

IMPORT Out;

PROCEDURE f () : REAL;
BEGIN
    RETURN 3.14159 ;
END f;

BEGIN
    Out.Real(f(), 6); WriteLn;
END return_real_1.

(*
CHECK: 3.14159
*)
