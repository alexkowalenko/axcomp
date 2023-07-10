MODULE cap; (* CAP function*)
IMPORT Out;
VAR i: INTEGER;

BEGIN
       FOR i := 061H TO 07aH DO
              Out.Char(CHR(i)) Out.Char(' '); Out.Char(CAP(CHR(i)));   Out.Ln;
       END
END cap.