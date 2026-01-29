(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE funct_1;

PROCEDURE f: INTEGER;
BEGIN
    RETURN 7;
END f;

BEGIN
    WriteInt(f()); WriteLn;
END funct_1.

(*
CHECK: 7
*)
