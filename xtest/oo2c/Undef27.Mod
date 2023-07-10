MODULE Undef27;

IMPORT Out;

PROCEDURE Q();
  BEGIN
    HALT(1);
  END Q;

PROCEDURE P*(): INTEGER;
  BEGIN
    Q();
    RETURN 1;
  END P;

BEGIN
  Out.Int(P, 0); Out.Ln;
END Undef27.
