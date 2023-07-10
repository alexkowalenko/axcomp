MODULE d10; (* Test call args *)

  PROCEDURE add(x : INTEGER; y : INTEGER): INTEGER;
  BEGIN
      RETURN x + y
  END add;

  PROCEDURE mult(x : INTEGER; y : INTEGER): INTEGER;
  BEGIN
      RETURN x * y
  END mult;

BEGIN
    RETURN add(mult(3,3),mult(4,4))
END d10.