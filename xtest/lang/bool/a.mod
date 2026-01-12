MODULE a;
VAR
    x : BOOLEAN;
BEGIN
    x := ~x;
    WriteBoolean(x); WriteLn;
    RETURN 0;
END a.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 0
*)
