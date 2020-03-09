MODULE e04; (* test ELSIF branch *)
VAR x : INTEGER;
BEGIN
    x := 2;
    IF x = 1 THEN
        x := 10;
    ELSIF x = 2 THEN
        x := 20;
    ELSIF x = 3 THEN
        x := 30;
    ELSE
        x := 90;
    END;
    RETURN x; 
END e04.