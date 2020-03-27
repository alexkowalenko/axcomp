MODULE e07; (* FOR statement *)

VAR x : INTEGER;

PROCEDURE f : INTEGER;
BEGIN
    RETURN 9
END f;

BEGIN
    x := 0;
    FOR i := 0 TO f() BY 2 DO
        x := x + i
    END;
    RETURN x
END e07.