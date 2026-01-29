(*
RUN:   %comp %s | filecheck %s
*)

MODULE repeat_3;
VAR 
    x : INTEGER;
BEGIN
    x := 1;
    REPEAT
        WriteInt(x); WriteLn;
        x := x + 1;
        if x > 4 then
            exit
        end
    UNTIL x > 9 ;
    WriteBoolean(TRUE); WriteLn;
END repeat_3.

(*
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 4
CHECK-NEXT: 1
*)
