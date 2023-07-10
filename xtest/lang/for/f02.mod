MODULE f02; (* Builtin functions *)
VAR i: INTEGER;
BEGIN
    FOR i := 1 TO 12 DO
        WriteInt(i); WriteLn()
    END;
    RETURN 0
END f02.