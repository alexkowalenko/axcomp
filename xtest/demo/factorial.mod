MODULE factorial;

PROCEDURE factor(x : INTEGER) : INTEGER;
BEGIN
     WriteInt(x); WriteLn;
    IF x <= 1 THEN
        RETURN 1
    END
    RETURN x * factor (x -1)
END factor;

BEGIN
    RETURN factor(10)
END factorial.