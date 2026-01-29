(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE locals_1;

CONST
    a = 1;
VAR z : INTEGER;

PROCEDURE f1 (x: INTEGER) : INTEGER;
CONST
    b = 3;
BEGIN
    RETURN x + a + b;
END f1;

BEGIN
    WriteInt(f1(2)); WriteLn;
END locals_1.

(*
CHECK: 6
*)
