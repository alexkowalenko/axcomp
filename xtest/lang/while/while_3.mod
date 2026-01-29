(*
RUN: %comp %s | filecheck %s
*)

MODULE while_3;
VAR 
    x : INTEGER;
BEGIN
    x := 10;
    WHILE x > 0 DO
        WriteInt(x); WriteLn;
        x := x - 1;
        if x < 5 then
            exit;
        end
    END;
    WriteBoolean(TRUE); WriteLn;
END while_3.

(*
CHECK: 10
CHECK-NEXT: 9
CHECK-NEXT: 8
CHECK-NEXT: 7
CHECK-NEXT: 6
CHECK-NEXT: 5
CHECK-NEXT: 1
*)
