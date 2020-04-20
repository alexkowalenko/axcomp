MODULE chr; (* CHR function*)
IMPORT Out;
VAR x : INTEGER;

BEGIN
       FOR i := 65 TO 90 DO
              Out.Char(CHR(i)); 
       END
       Out.Ln;
        FOR i := 061H TO 07aH DO
              Out.Char(CHR(i)); 
       END
       Out.Ln;
END chr.