(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_7;
VAR
    x, y, z : INTEGER;
BEGIN
    x := 3;
    y := 30;
    z := y - x;
    WriteInt(z); WriteLn;
    RETURN 0;
END assignment_7.

(*
CHECK: 27
*)
