MODULE stdlib02; (* Out.String() *)
IMPORT Out;
CONST a = "Constant!";
VAR x: STRING;

BEGIN
   x := 'Hello';
   Out.String(x); Out.Char(' '); Out.String("World!"); Out.Ln;
   Out.String(a); Out.Ln;
   RETURN 0;
END stdlib02.