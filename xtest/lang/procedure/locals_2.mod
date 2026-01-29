(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE locals_2;

CONST
    a = 1;
VAR z : INTEGER;

PROCEDURE f1 (x: INTEGER) : INTEGER;
CONST
    b = 3;
VAR
    y : INTEGER;
BEGIN
    y := b * 2;
    RETURN x + a + b + y;
END f1;

BEGIN
    WriteInt(f1(2)); WriteLn;
END locals_2.

(*
CHECK: 12
*)
