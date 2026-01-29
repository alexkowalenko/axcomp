(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_1;

PROCEDURE f1 (x: INTEGER) : INTEGER;
BEGIN
    RETURN x;
END f1;


BEGIN
    WriteInt(f1(223)); WriteLn;
END param_1.

(*
CHECK: 223
CHECK-NEXT: 0
*)
