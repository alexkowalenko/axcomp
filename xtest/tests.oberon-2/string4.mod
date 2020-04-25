MODULE string4; (* STRING type *)
IMPORT Out;
VAR   x: STRING;
      xx: ARRAY 3 OF CHAR;
      c: CHAR;

BEGIN
   x := 'Hello';
   xx[0] := 'A';
   Out.String(x); Out.Ln;
   c := x[0]; 
   Out.Char(c); Out.Ln;
   c := xx[0]; 
   Out.Char(c); Out.Ln;
   RETURN c;
END string4.