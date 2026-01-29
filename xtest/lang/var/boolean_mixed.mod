(*
RUN: %comp %s | filecheck %s
*)

MODULE boolean_mixed;
VAR
    x : INTEGER;
    z : BOOLEAN;
    y : INTEGER;
BEGIN
    x := 3;
    z := true;
    y := 30;
    WriteInt(x); WriteLn;
    WriteBoolean(z); WriteLn;
    WriteInt(y); WriteLn;
    RETURN 0;
END boolean_mixed.

(*
CHECK: 3
CHECK-NEXT: 1
CHECK-NEXT: 30
*)
