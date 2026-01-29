(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE funct_2;

PROCEDURE f: INTEGER;
BEGIN
    RETURN 7;
END f;

BEGIN
    WriteInt(f() + f()); WriteLn;
END funct_2.

(*
CHECK: 14
*)
