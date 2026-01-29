(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE locals_4;


PROCEDURE f1 (x: INTEGER) : INTEGER;
VAR
    y : INTEGER;
    z : BOOLEAN;
    a : INTEGER;
BEGIN
    y := 1;
    z := FALSE;
    a := 5;
    WriteInt(a); WriteLn;
    WriteBoolean(z); WriteLn;
    WriteInt(y); WriteLn;
    RETURN x + y;
END f1;

BEGIN
    WriteInt(f1(2)); WriteLn;
END locals_4.

(*
CHECK: 5
CHECK: 0
CHECK: 1
CHECK: 3
*)
