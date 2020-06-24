MODULE Const6;

IMPORT Out;

PROCEDURE P (b: BOOLEAN);
  BEGIN
    LOOP
      CASE 1 OF
      | 0:
        Out.String ("foo");
        EXIT;
      ELSE
        IF b THEN
          Out.String ("bar");
          EXIT;
        END;
      END;
    END;
  END P;

PROCEDURE Test* ();
  BEGIN
    P(TRUE);
    Out.Ln;
  END Test;

BEGIN
  Test
END Const6.
