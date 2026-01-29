(*
RUN:  %comp %s | filecheck %s
*)

MODULE if_3;
VAR 
    x : BOOLEAN;
BEGIN
    x := FALSE;
    IF x THEN 
        WriteBoolean(TRUE); WriteLn; 
    ELSE
        WriteBoolean(FALSE); WriteLn;
    END;
    WriteBoolean(FALSE); WriteLn;
END if_3.

(*
CHECK: 0
CHECK-NEXT: 0
*)
