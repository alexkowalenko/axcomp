MODULE OpExprCompat12;

TYPE
  Small = REAL;
  Large = LONGREAL;

PROCEDURE Test*;
VAR 
  s : Small;
  l : Large;
BEGIN
  s := s + l;
END Test;

BEGIN
  Test
END OpExprCompat12.
