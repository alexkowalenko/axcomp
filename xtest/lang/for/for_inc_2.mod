(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN + *>

MODULE for_inc_2;
VAR
    i : INTEGER;
BEGIN
    FOR i := 1 TO 10 BY 2 DO
        WriteInt(i); WriteLn;
        IF i > 4 THEN
            EXIT;
        END
    END;
    WriteBoolean(TRUE); WriteLn;
END for_inc_2.

(*
CHECK: 1
CHECK-NEXT: 3
CHECK-NEXT: 5
CHECK-NEXT: 1
*)
