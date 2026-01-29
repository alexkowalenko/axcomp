(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE locals_5;

VAR y : INTEGER;

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
    y := 7;
    y := f1(2) + y;
    WriteInt(y); WriteLn;
END locals_5.

(*
CHECK: 5
CHECK: 0
CHECK: 1
CHECK: 10
*)
