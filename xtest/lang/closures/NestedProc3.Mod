MODULE NestedProc3;

IMPORT Out;
  
PROCEDURE Test* ();
  VAR
    x: INTEGER;
    
  PROCEDURE Inner (x: INTEGER);
    BEGIN
      x := 123;
    END Inner;
  
  BEGIN
    x := -1;
    Inner (x);
    Out.Int (x, 0); Out.Ln;
  END Test;

BEGIN
  Test
END NestedProc3.
