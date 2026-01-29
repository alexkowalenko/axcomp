(*
RUN:  %comp %s | filecheck %s
*)

MODULE elseif_3;
VAR 
    x : INTEGER;
BEGIN
    x := 4;
    IF x < 1 THEN
        WriteInt(x); WriteLn;
    ELSIF x < 2 THEN
        WriteInt(10); WriteLn;
    ELSIF x < 3 THEN
        WriteInt(20); WriteLn;
    ELSE
        WriteBoolean(FALSE); WriteLn;
    END;
    WriteBoolean(TRUE); WriteLn;
END elseif_3.

(*
CHECK: 0
CHECK-NEXT: 1
*)
