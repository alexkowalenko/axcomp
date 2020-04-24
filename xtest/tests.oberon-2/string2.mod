MODULE string2; (* STRING type *)
IMPORT Out;
VAR x: STRING;
    y: STRING;

PROCEDURE identity(x: STRING): STRING;
BEGIN
    RETURN x;
END identity;

BEGIN
   x := 'Hello';
   y := identity(x);
   Out.String(y); Out.Ln;
   RETURN 0;
END string2.