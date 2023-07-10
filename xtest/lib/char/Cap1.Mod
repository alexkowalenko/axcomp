MODULE Cap1;

IMPORT Out;

PROCEDURE Cap (ch, cap: CHAR);
  BEGIN
    Out.Char (ch);
    Out.Char (cap);
    Out.Char (CAP (ch));
    Out.Char (CAP (cap));
    Out.Ln;
  END Cap;

PROCEDURE Test* ();
  BEGIN
    Cap ("a", CAP ("a"));
    Cap ("z", CAP (7AX));
    Cap ("0", CAP ("0"));
  END Test;

BEGIN
  Test
END Cap1.
