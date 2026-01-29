(*
RUN:   %comp %s | filecheck %s
*)

MODULE repeat_2;
VAR 
    x : INTEGER;
BEGIN
    x := 10;
    REPEAT
        WriteInt(x); WriteLn;
        x := x + 1;
    UNTIL x > 9 ;
    WriteBoolean(TRUE); WriteLn;
END repeat_2.

(*
CHECK: 10
CHECK-NEXT: 1
*)
