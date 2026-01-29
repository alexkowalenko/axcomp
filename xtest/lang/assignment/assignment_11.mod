(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_11;
VAR
    x, y, z : INTEGER;
BEGIN
    x := 3;
    y := 30;
    z := y * x + x;
    WriteInt(z); WriteLn;
    RETURN 0;
END assignment_11.

(*
CHECK: 93
*)
