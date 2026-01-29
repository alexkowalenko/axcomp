(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE proc_1;
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

BEGIN
    g;
    f;
END proc_1.

(*
CHECK: 0
CHECK: 1
CHECK: 1
*)
