(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE locals_3;


PROCEDURE f1 (x: INTEGER) : INTEGER;
VAR
    y : INTEGER;
    z : INTEGER;
BEGIN
    y := 1;
    z := y + 4;
    RETURN x + y + z;
END f1;

BEGIN
    WriteInt(f1(2)); WriteLn;
END locals_3.

(*
CHECK: 8
*)
