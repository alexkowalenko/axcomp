MODULE e06; (* FOR statement *)
VAR x : INTEGER;
BEGIN
    x := 0;
    FOR i := 0 TO 9 DO
        x := x + i
    END;
    RETURN x
END e06.