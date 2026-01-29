(*
RUN: %comp %s | filecheck %s
*)

<* MAIN + *>

MODULE for_inc_1;
VAR
    i : INTEGER;
BEGIN
    FOR i := 1 TO 10 BY 3 DO
        WriteInt(i); WriteLn;
    END;
    WriteBoolean(i = 13); WriteLn;
END for_inc_1.

(*
CHECK: 1
CHECK: 4
CHECK: 7
CHECK: 10
CHECK: 1
*)
