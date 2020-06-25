MODULE T2;

IMPORT Out;

PROCEDURE f*(a, b: LONGINT): LONGINT;
  BEGIN
    RETURN a-b;
  END f;

BEGIN
  Out.Int(f(7, 4), 0); Out.Ln;
END T2.
