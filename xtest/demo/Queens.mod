MODULE Queens;

IMPORT Out;

VAR i: INTEGER;

(* The eight queens problem, solved 50 times. *)
(*
  type
      doubleboard =   2..16;
      doublenorm  =   -7..7;
      boardrange  =   1..8;
      aarray      =   array [boardrange] of boolean;
      barray      =   array [doubleboard] of boolean;
      carray      =   array [doublenorm] of boolean;
      xarray      =   array [boardrange] of boardrange;
*)

PROCEDURE Try(i: LONGINT;
              VAR q: BOOLEAN;
              VAR a: ARRAY 17 OF BOOLEAN;
              VAR b: ARRAY 9 OF BOOLEAN;
              VAR c: ARRAY 15 OF BOOLEAN;
              VAR x: ARRAY 9 OF LONGINT);
VAR j: LONGINT;
BEGIN
    j := 0;
    q := FALSE;
	WHILE (~q) & (j # 8) DO
		j := j + 1;
		q := FALSE;
		IF b[j] & a[i+j] & c[i-j+7] THEN
			x[i] := j;
			b[j] := FALSE;
			a[i+j] := FALSE;
			c[i-j+7] := FALSE;
			IF i < 8 THEN
				Try(i+1,q,a,b,c,x);
				IF ~q THEN
					b[j] := TRUE;
					a[i+j] := TRUE;
					c[i-j+7] := TRUE
				END
       	    ELSE q := TRUE
		    END
	    END
	END
END Try;

PROCEDURE Doit ();
VAR i: LONGINT; q: BOOLEAN;
	a: ARRAY 9 OF BOOLEAN;
	b: ARRAY 17 OF BOOLEAN;
	c: ARRAY 15 OF BOOLEAN;
	x: ARRAY 9 OF LONGINT;
BEGIN
	i := 0 - 7;
	WHILE i <= 16 DO
	    IF (i >= 1) & (i <= 8) THEN a[i] := TRUE END ;
	    IF i >= 2 THEN b[i] := TRUE END ;
		IF i <= 7 THEN c[i+7] := TRUE END ;
		i := i + 1;
	END ;
	Try(1, q, b, a, c, x);
	IF ( ~ q ) THEN
	    Out.String(" Error in Queens.$"); Out.Ln;
	END
END Doit;

BEGIN
    i := 1;
	WHILE i <= 50 DO
	    Doit();
	    INC(i)
	END

	Out.String("Queens done"); Out.Ln;
END Queens.

(*
RUN: %comp %s | filecheck %s
CHECK: Queens done
*)

