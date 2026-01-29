(*
RUN:  %comp %s | filecheck %s
*)

MODULE elseif_2;
VAR 
    x : INTEGER;
BEGIN
    x := 2;
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
END elseif_2.

(*
CHECK: 20
CHECK-NEXT: 1
*)
