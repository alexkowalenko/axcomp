MODULE a;
VAR
    x : BOOLEAN;
BEGIN
    x := ~x;
    WriteBoolean(x); WriteLn;
    RETURN 0;
END a.