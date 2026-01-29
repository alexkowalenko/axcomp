(*
RUN: %comp %s | filecheck %s
*)

MODULE const_real_0;
IMPORT Out;
CONST
    pi = 3.141592;
BEGIN
    Out.Real(pi, 6); WriteLn;
END const_real_0.

(*
CHECK: 3.14159
*)
