(*
RUN: %comp %s | filecheck %s
*)

MODULE assignment_3;
VAR
    x : BOOLEAN;
BEGIN
    x := ~TRUE;
    WriteBoolean(x); WriteLn;
    RETURN 0;
END assignment_3.

(*
CHECK: 0
*)
