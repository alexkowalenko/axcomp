(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN + *>

MODULE for_inc_3;
VAR
    i : INTEGER;
BEGIN
    (* NEGATIVE STEP *)
    FOR i := 10 TO 1 BY -3 DO
        WriteInt(i); WriteLn;
    END;
    WriteBoolean(TRUE); WriteLn;
END for_inc_3.

(*
CHECK: 10
CHECK-NEXT: 7
CHECK-NEXT: 4
CHECK-NEXT: 1
CHECK-NEXT: 1
*)
