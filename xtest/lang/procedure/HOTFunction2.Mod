MODULE HOTFunction2;

IMPORT Out;
  
PROCEDURE Identity (n: INTEGER): INTEGER;
  BEGIN
    RETURN n
  END Identity;

PROCEDURE Test* ();
  BEGIN
    Out.Int(Identity(1), 0); Out.Ln;
    Out.Int(Identity(2), 0); Out.Ln;
  END Test;

BEGIN
  Test
END HOTFunction2.
