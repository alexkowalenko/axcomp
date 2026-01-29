(*
RUN:  %comp %s | filecheck %s
*)

MODULE if_6;
VAR 
    x : INTEGER;
BEGIN
    x := 1;
    IF x < 1 THEN
        WriteInt(x); WriteLn;
    ELSE
        WriteBoolean(FALSE); WriteLn;
    END;
    WriteBoolean(TRUE); WriteLn;
END if_6.

(*
CHECK: 0
CHECK-NEXT: 1
*)
