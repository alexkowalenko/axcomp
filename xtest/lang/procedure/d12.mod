MODULE d12; (* Test call args *)

  PROCEDURE add(x, y : INTEGER): INTEGER;
  BEGIN
      RETURN x + y
  END add;

  PROCEDURE mult(x, y : INTEGER): INTEGER;
  BEGIN
      RETURN x * y
  END mult;

BEGIN
    RETURN add(mult(3,3),mult(4,4))
END d12.