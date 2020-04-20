MODULE halt; (* HALT function*)
IMPORT Out;
VAR x : INTEGER;

BEGIN
       FOR i := 1 TO 3 DO
              HALT(3);
       END
       RETURN 0;
END halt.