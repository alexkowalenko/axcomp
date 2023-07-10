MODULE Real2;

IMPORT Out;

PROCEDURE Print(x: REAL);
  BEGIN
    Out.Real(x, 3); Out.Ln;
  END Print;

PROCEDURE Print2(x: REAL);
  BEGIN
    Print(x);
  END Print2;

PROCEDURE Test*();
  BEGIN
    Print2(1.0);
    Print2(2.0);
    Print2(-0.025);
  END Test;

BEGIN
  Test
END Real2.
