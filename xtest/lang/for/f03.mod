MODULE f03; (* Builtin function WriteBoolean *)
VAR i: INTEGER;
BEGIN
    FOR i := 1 TO 12 DO
        WriteBoolean(i MOD 2 = 0); WriteLn()
    END;
    RETURN 0
END f03.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
*)
