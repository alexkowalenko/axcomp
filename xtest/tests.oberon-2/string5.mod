MODULE string5; (* STRING type *)
IMPORT Out;
VAR   x: STRING;

BEGIN
   x := 'Hello';
   FOR i := 0 TO LEN(x) - 1 DO
      Out.Char(x[i]); Out.Char(' ');
   END
   Out.Ln;
   RETURN 0;
END string5.