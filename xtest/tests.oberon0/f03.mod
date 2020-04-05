MODULE f03; (* Builtin function WriteBoolean *)
BEGIN
    FOR i := 1 TO 12 DO
        WriteBoolean(i MOD 2 = 0); WriteLn()
    END;
    RETURN 0
END f03.