(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE proc_2;
VAR
    x, y : INTEGER;
    z : BOOLEAN;

PROCEDURE f;
BEGIN
    WriteInt(1); WriteLn;
END f;

PROCEDURE g;
BEGIN
    WriteInt(x); WriteLn;
    f;
END g;

PROCEDURE h;
BEGIN
    WriteBoolean(TRUE); WriteLn;
    g;
END h;

BEGIN
    h;
    g;
    f;
END proc_2.

(*
CHECK: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 1
*)
