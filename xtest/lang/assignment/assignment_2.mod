(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_2;
VAR
    x : BOOLEAN;
BEGIN
    x := TRUE;
    WriteBoolean(x); WriteLn;
    RETURN 0;
END assignment_2.

(*
CHECK: 1
*)
