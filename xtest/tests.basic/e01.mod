MODULE e01; (* IF statements *)
VAR x : INTEGER;
BEGIN
    IF x = 1 THEN
        x := 0
    ELSE
        x := 2
    END;
    RETURN x
END e01.