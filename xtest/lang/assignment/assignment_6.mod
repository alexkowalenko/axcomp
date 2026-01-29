(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_6;
VAR
    x, y : INTEGER;
BEGIN
    x := 3;
    y := 30;
    WriteInt(x + y); WriteLn;
    RETURN 0;
END assignment_6.

(*
CHECK: 33
*)
