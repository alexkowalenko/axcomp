MODULE beta;

CONST a* = 3;
VAR b* : INTEGER;
VAR d* : BOOLEAN;

PROCEDURE c*(x : INTEGER);
BEGIN
    WriteInt(x); WriteLn();
    x := 10;
    WriteInt(x); WriteLn()
END c;

BEGIN
    RETURN 0
END beta.