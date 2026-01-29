(*
RUN: %comp %s | filecheck %s
*)

MODULE const_1;
CONST
    a = 37;
VAR 
    x : INTEGER;
BEGIN
    x := a;
    WriteInt(a); WriteLn;
END const_1.

(*
CHECK: 37
*)
