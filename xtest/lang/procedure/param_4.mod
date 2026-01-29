(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE param_4;

PROCEDURE f1 (x: INTEGER) : INTEGER;
BEGIN
    RETURN x;
END f1;

PROCEDURE f2 (x, y: INTEGER) : INTEGER;
BEGIN
    RETURN x + f1(y);
END f2;

PROCEDURE f3 (x, y, z: INTEGER) : INTEGER;
BEGIN
    RETURN f1(x) + f2(y, z);
END f3;


BEGIN
    WriteInt(f1(23) + f2(2, 5) + f3(3, 5, 6)); WriteLn;
END param_4.

(*
CHECK: 44
*)
