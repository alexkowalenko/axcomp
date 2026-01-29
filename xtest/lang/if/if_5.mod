(*
RUN:  %comp %s | filecheck %s
*)

MODULE if_5;
VAR 
    x : INTEGER;
BEGIN
    x := 1;
    IF x < 1 THEN
        WriteInt(x); WriteLn;
    END;
    WriteBoolean(TRUE); WriteLn;
END if_5.

(*
CHECK: 1
*)
