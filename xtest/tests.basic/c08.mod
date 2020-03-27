MODULE c08; (* multiple VAR & CONST declarations*)

CONST alpha = 24;
VAR x : INTEGER;
CONST beta = 48;
VAR y : BOOLEAN;

BEGIN
   y := FALSE;
   x := beta;
   x := 3 * alpha;
   RETURN ~ (x = 72)
END c08.