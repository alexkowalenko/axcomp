MODULE T3;

IMPORT Out;

PROCEDURE f*(select: BOOLEAN; a, b: LONGINT): LONGINT;
  BEGIN
    IF select THEN
      RETURN a+b;
    ELSE
      RETURN a-b;
    END;
  END f;

BEGIN
  Out.Int(f(TRUE, 3, 17), 0); Out.Ln;
END T3.
