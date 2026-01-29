(*
RUN:  %comp %s | filecheck %s
*)

MODULE while_2;
VAR 
    x : INTEGER;
BEGIN
    x := 0;
    WHILE x > 0 DO
        WriteInt(x); WriteLn;
        x := x -1;
    END;
    WriteBoolean(TRUE); WriteLn;
END while_2.

(*
CHECK: 1
*)
