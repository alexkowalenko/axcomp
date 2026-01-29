(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_12;
VAR
    x, y  : INTEGER;
BEGIN
    y := 18;
    x := y;
    WriteInt(x); WriteLn;
    WriteInt(y); WriteLn;
END assignment_12.

(*
CHECK: 18
CHECK-NEXT: 18
*)
