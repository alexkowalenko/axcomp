MODULE Cse3;

IMPORT Out;
  
PROCEDURE F (x: INTEGER): INTEGER;
  BEGIN
    RETURN (x+x)+(x+x);
  END F;

PROCEDURE Test* ();
  BEGIN
    Out.Int(F(3), 0); Out.Ln;
  END Test;

BEGIN
  Test
END Cse3.
