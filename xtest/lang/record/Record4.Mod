MODULE Record4;

IMPORT Out;

TYPE
  R0 = RECORD
    x: INTEGER;
  END;
  R = RECORD
    (R0)
    y: INTEGER;
  END;
  
VAR
  r: R;
  
PROCEDURE Set(a, b: INTEGER);
  BEGIN
    r. x := a;
    r. y := b;
  END Set;

PROCEDURE Test* ();
  BEGIN
    Set (1, 2);
    Out.Int (r. x, 2); Out.Int (r. y, 2); Out.Ln;
    Set (3, 4);
    Out.Int (r. x, 2); Out.Int (r. y, 2); Out.Ln;
  END Test;

BEGIN
  Test
END Record4.
