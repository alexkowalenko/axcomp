(*
RUN:  %comp %s | filecheck %s
*)

MODULE if_1;
VAR 
    x : BOOLEAN;
BEGIN
    x := TRUE;
    IF x THEN 
       WriteBoolean(TRUE); WriteLn; 
    END;
    WriteBoolean(FALSE); WriteLn;
END if_1.

(*
CHECK: 1
CHECK-NEXT: 0
*)
