MODULE HOTFunction1;

IMPORT Out;
  
PROCEDURE OneTwoThree (): INTEGER;
  BEGIN
    RETURN 123
  END OneTwoThree;

PROCEDURE Test* ();
  BEGIN
    Out.Int(123, 0); Out.Ln;
    Out.Int(OneTwoThree(), 0); Out.Ln;
  END Test;

BEGIN
  Test
END HOTFunction1.
