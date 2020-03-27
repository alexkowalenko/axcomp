MODULE d09; (* Test call args *)

  PROCEDURE f (x : INTEGER): INTEGER;
  BEGIN
      RETURN x + 2
  END f;

BEGIN
    RETURN 5 + f(2) + (f(3) * f(4))
END d09.