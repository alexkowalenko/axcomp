MODULE Undef12;

IMPORT Out;

PROCEDURE Fct*(): BOOLEAN;
  BEGIN
    ASSERT(TRUE);
    Out.String("After"); Out.Ln;
  END Fct;

BEGIN
  Fct
END Undef12.
