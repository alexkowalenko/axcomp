(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE funct_3;

PROCEDURE f: INTEGER;
BEGIN
    RETURN 7;
END f;

BEGIN
    WriteInt(f() + f() + f()); WriteLn;
END funct_3.

(*
CHECK: 21
*)
