(*
RUN:  %comp %s | filecheck %s
*)

MODULE while_1;
VAR 
    x : INTEGER;
BEGIN
    x := 4;
    WHILE x > 0 DO
        WriteInt(x); WriteLn;
        x := x -1;
    END;
    WriteBoolean(TRUE); WriteLn;
END while_1.

(*
CHECK: 4
CHECK-NEXT: 3
CHECK-NEXT: 2
CHECK-NEXT: 1
CHECK-NEXT: 1
*)
