MODULE chr2; (* CHR function*)
IMPORT Out;
VAR x : INTEGER;

BEGIN
       FOR i := ORD('👾') TO ORD('👾') + 9 DO
              Out.Char(CHR(i));   Out.Char(' '); 
       END
       Out.Ln;
END chr2.