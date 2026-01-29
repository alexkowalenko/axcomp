(*
RUN:  %comp %s | filecheck %s
*)

MODULE elseif_1;
VAR 
    x : INTEGER;
BEGIN
    x := 1;
    IF x < 1 THEN
        WriteInt(x); WriteLn;
    ELSIF x < 2 THEN
        WriteInt(10); WriteLn;
    ELSE
        WriteBoolean(FALSE); WriteLn;
    END;
    WriteBoolean(TRUE); WriteLn;
END elseif_1.

(*
CHECK: 10
CHECK-NEXT: 1
*)
