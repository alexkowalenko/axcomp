(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_1;
VAR
    x : INTEGER;
BEGIN
    x := 3;
    WriteInt(x); WriteLn;
    RETURN 0;
END assignment_1.

(*
CHECK: 3
CHECK-NEXT: 0
*)
