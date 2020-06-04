MODULE odd; (* ODD function*)
IMPORT Out;
VAR i: INTEGER;

BEGIN
       FOR i := 0 TO 7 DO
              Out.Bool(ODD(i)); Out.Ln;
       END
END odd.