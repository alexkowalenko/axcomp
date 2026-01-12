MODULE f01; (* Builtin functions *)
VAR x : INTEGER;
BEGIN
    x := 12;
    WriteInt(x); WriteLn();
    RETURN 0
END f01.


(*
RUN: %comp %s | filecheck %s
CHECK: 12
CHECK-NEXT: 0
*)
