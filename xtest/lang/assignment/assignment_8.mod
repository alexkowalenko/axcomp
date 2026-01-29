(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_8;
VAR
    x, y, z : INTEGER;
BEGIN
    x := 3;
    y := 30;
    z := y * x;
    WriteInt(z); WriteLn;
    RETURN 0;
END assignment_8.

(*
CHECK: 90
*)
