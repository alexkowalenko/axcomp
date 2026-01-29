(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN + *>

MODULE for_inc_4;
VAR
    i : INTEGER;
BEGIN
    FOR i := 1 TO 10 BY 2+1 DO
         WriteInt(i); WriteLn; 
    END;
    WriteBoolean(TRUE); WriteLn;
END for_inc_4.

(*
CHECK: 1
CHECK: 4
CHECK: 7
CHECK: 10
CHECK: 1
*)
