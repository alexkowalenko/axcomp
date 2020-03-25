MODULE e03; (* test ELSIF branch *)
VAR x : INTEGER;
BEGIN
    x := 3;
    IF x = 1 THEN
        x := 10;
    ELSIF x = 3 THEN
        x := 30;
    ELSE
        x := 90;
    END;
    RETURN x; 
END e03.