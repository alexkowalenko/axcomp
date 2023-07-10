MODULE not;
    IMPORT Out;
    VAR x : BOOLEAN;
BEGIN
    WriteBoolean(~TRUE); WriteLn;
    WriteBoolean(~FALSE); WriteLn;
    WriteBoolean(~~TRUE); WriteLn;
    x := TRUE;
    WriteBoolean(x); WriteLn;
    x := ~x;
    WriteBoolean(x); WriteLn;
END not.