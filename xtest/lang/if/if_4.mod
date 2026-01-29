(*
RUN:  %comp %s | filecheck %s
*)

MODULE if_4;
VAR 
    x : INTEGER;
BEGIN
    x := 1;
    IF x < 1 THEN 
        WriteBoolean(TRUE); WriteLn;
    ELSE
        WriteBoolean(FALSE); WriteLn;
    END;
    WriteBoolean(TRUE); WriteLn;
END if_4.

(*
CHECK: 0
CHECK-NEXT: 1
*)
