(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_3;

PROCEDURE f1 (x: INTEGER) : INTEGER;
BEGIN
    RETURN x;
END f1;

PROCEDURE f2 (x, y: INTEGER) : INTEGER;
BEGIN
    RETURN x + f1(y);
END f2;

BEGIN
    WriteInt(f1(23) + f2(2, 5)); WriteLn;
END param_3.

(*
CHECK: 30
CHECK-NEXT: 0
*)
