(* 
* AX compiler
*
* Copyright © 2020 Alex Kowalenko
*
* Runtime library: Standard Output
*)

MODULE Math;

CONST
	e* = 2.7182818284590452354D0;
	pi* = 3.14159265358979323846D0;
	ln2* = 0.693147180559945309417232121458D0;
	eps = 2.2D-16;

PROCEDURE Equal* (x, y: REAL): BOOLEAN;
BEGIN
	IF x > y THEN
		x := x - y
	ELSE
		x := y - x
	END;
	RETURN x < eps
END Equal;

PROCEDURE sin*(x: REAL): REAL;
END sin;

PROCEDURE cos*(x: REAL): REAL;
END cos;

PROCEDURE arctan*(y:REAL): REAL;
END arctan;

PROCEDURE sqrt*(x: REAL): REAL;
END sqrt;

PROCEDURE ln*(x: REAL): REAL;
END ln;

PROCEDURE exp*(x: REAL): REAL;
END exp;

END Math.
