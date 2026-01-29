(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_10;
VAR
    x, y, z : INTEGER;
BEGIN
    x := 3;
    y := 30;
    z := y MOD x;
    WriteInt(z); WriteLn;
    RETURN 0;
END assignment_10.

(*
CHECK: 0
*)
