MODULE Case9;

PROCEDURE P (i: INTEGER);
  BEGIN
    CASE i OF
    | 1:
    | 0..2:
    END;
  END P;

BEGIN
  P(2)
END Case9.
