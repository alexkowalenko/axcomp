MODULE e12;
VAR x : INTEGER;
BEGIN
    x := 0;
    WHILE x < 10 DO
        x := x + 1;
        IF x = 5 THEN
            RETURN 5;
        END;
    END;
    RETURN x; 
END e12.