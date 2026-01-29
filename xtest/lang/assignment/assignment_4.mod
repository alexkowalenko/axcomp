(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_4;
VAR
    x : INTEGER;
BEGIN
    x := 8 + 2;
    WriteInt(x); WriteLn;
    RETURN 0;
END assignment_4.

(*
CHECK: 10
*)
