(*
RUN:  %comp %s | filecheck %s
*)

MODULE if_2;
VAR 
    x : BOOLEAN;
BEGIN
    x := FALSE;
    IF x THEN 
       WriteBoolean(TRUE); WriteLn; 
    END;
    WriteBoolean(FALSE); WriteLn;
END if_2.

(*
CHECK: 0
*)
