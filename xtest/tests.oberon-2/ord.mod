MODULE ord; (* ORD function*)
IMPORT Out;
VAR i: INTEGER;

BEGIN
       FOR i := 65 TO 90 DO
              Out.Char(CHR(i)) Out.Char(' '); Out.Int(ORD(CHR(i)), 0);   Out.Ln;
       END
END ord.