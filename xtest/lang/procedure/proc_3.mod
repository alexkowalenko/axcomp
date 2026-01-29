(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE proc_3;
VAR
    x, y : INTEGER;
    z : BOOLEAN;

PROCEDURE f1;
BEGIN
    WriteInt(1); WriteLn;
END f1;

PROCEDURE f2;
BEGIN
    WriteInt(x); WriteLn;
    f1;
END f2;

PROCEDURE f3;
BEGIN
    WriteBoolean(TRUE); WriteLn;
    f2;
END f3;

PROCEDURE f4;
BEGIN
    WriteBoolean(FALSE); WriteLn;
    f3;
END f4;

BEGIN
    f4;
    f3;
    f2;
    f1;
END proc_3.

(*
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
*)
