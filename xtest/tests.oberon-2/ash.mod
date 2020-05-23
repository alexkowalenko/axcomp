MODULE ash; (* ASH function*)
IMPORT Out;
VAR x : INTEGER;

BEGIN
       FOR i := 0 TO 7 DO
              x := ASH(1, i);
              Out.Int(x, 0); Out.Ln;
       END
END ash.