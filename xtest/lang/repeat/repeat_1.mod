(*
RUN:   %comp %s | filecheck %s
*)

MODULE repeat_1;
VAR 
    x : INTEGER;
BEGIN
    x := 0;
    REPEAT
        WriteInt(x); WriteLn;
        x := x + 1;
    UNTIL x > 9 ;
    WriteBoolean(TRUE); WriteLn;
END repeat_1.

(*
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 4
CHECK-NEXT: 5
CHECK-NEXT: 6
CHECK-NEXT: 7
CHECK-NEXT: 8
CHECK-NEXT: 9
CHECK-NEXT: 1
*)
