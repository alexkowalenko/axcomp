MODULE Case6;

PROCEDURE P (i: CHAR);
  BEGIN
    CASE i OF
    | 100X: (* nothing *)
    END;
  END P;

BEGIN
  P('a')
END Case6.
