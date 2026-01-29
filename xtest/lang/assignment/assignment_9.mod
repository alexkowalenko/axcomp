(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_9;
VAR
    x, y, z : INTEGER;
BEGIN
    x := 3;
    y := 30;
    z := y DIV x;
    WriteInt(z); WriteLn;
    RETURN 0;
END assignment_9.

(*
CHECK: 10
*)
