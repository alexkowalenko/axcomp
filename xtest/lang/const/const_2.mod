(*
RUN: %comp %s | filecheck %s
*)

MODULE const_2;
CONST
    a = TRUE;
VAR 
    x : BOOLEAN;
BEGIN
    x := a;
    WriteBoolean(a); WriteLn;
END const_2.

(*
CHECK: 1
*)
