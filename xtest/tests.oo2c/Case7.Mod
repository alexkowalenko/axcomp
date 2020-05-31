MODULE Case7;

PROCEDURE P (i: INTEGER);
  BEGIN
    CASE i OF
    | 1(*first*):
    | 1(*second*):
    END;
  END P;

BEGIN
  P(2)
END Case7.
