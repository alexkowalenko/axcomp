(*
RUN: %comp %s | filecheck %s
*)

<* MAIN + *>

MODULE for_2;
VAR
    i : INTEGER;
BEGIN
    FOR i := 11 TO 10 DO
        WriteInt(i); WriteLn;
    END;
    WriteBoolean(i = 11); WriteLn;
END for_2.

(*
CHECK: 1
*)
