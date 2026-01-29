(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_2;

PROCEDURE f1 (x, y: INTEGER) : INTEGER;
BEGIN
    RETURN x + y;
END f1;


BEGIN
    WriteInt(f1(23,45)); WriteLn;
END param_2.

(*
CHECK: 68
*)
