(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN + *>

MODULE for_1;
VAR
    i : INTEGER;
BEGIN
    FOR i := 1 TO 10 DO
        WriteInt(i); WriteLn;
    END;
    WriteBoolean(i = 11); WriteLn;
END for_1.

(*
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 4
CHECK-NEXT: 5
CHECK-NEXT: 6
CHECK-NEXT: 7
CHECK-NEXT: 8
CHECK-NEXT: 9
CHECK-NEXT: 10
CHECK-NEXT: 1
*)
