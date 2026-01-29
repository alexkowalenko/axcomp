(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_5;
VAR
    x, y : INTEGER;
BEGIN
    x := 3;
    WriteInt(x + 1); WriteLn;
    RETURN 0;
END assignment_5.

(*
CHECK: 4
*)
