MODULE T6;

IMPORT Out;

PROCEDURE f*(a, b: BOOLEAN): LONGINT;
  BEGIN
    IF a & b THEN
      RETURN 9;
    ELSE
      RETURN -9;
    END;
  END f;

BEGIN
  Out.Int(f(TRUE, TRUE), 0); Out.Ln;
END T6.
