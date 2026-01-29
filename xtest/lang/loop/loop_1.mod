(*
RUN:  %comp %s | filecheck %s
*)

MODULE loop_1;
VAR 
    x : INTEGER;
BEGIN
    x := 1;
    LOOP
        WriteInt(x); WriteLn;
        x := x + 1;
        if x > 5 then
            EXIT;
        end
    END;
    WriteBoolean(TRUE); WriteLn;
END loop_1.

(*
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 4
CHECK-NEXT: 5
CHECK-NEXT: 1
*)
