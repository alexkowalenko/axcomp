MODULE HOTFunction5;

IMPORT Out;
  
PROCEDURE Inc (n: INTEGER): INTEGER;
  BEGIN
    RETURN n+1
  END Inc;

PROCEDURE Test* ();
  BEGIN
    Out.Int(Inc (-1), 0); Out.Ln;
    Out.Int(Inc (0), 0); Out.Ln;
    Out.Int(Inc (1), 0); Out.Ln;
  END Test;

BEGIN
  Test
END HOTFunction5.
