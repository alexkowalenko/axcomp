MODULE MatrixInt;

(* Multiplies two integer matrices. *)

IMPORT Out;

CONST
    rowsize = 40;

TYPE
    intmatrix = ARRAY rowsize + 1 OF ARRAY rowsize + 1 OF LONGINT;

VAR i, j: LONGINT;
    seed: INTEGER;
    ima, imb, imr: intmatrix;

PROCEDURE Initrand ();
BEGIN seed := 74755
END Initrand;

PROCEDURE Rand (): LONGINT;
BEGIN
    seed := (seed * 1309 + 13849) MOD 65535;
    RETURN (seed)
END Rand;

PROCEDURE Initmatrix (VAR m: intmatrix);
VAR temp, i, j: LONGINT;
BEGIN i := 1;
   WHILE i <= rowsize DO
        j := 1;
		WHILE j <= rowsize DO
			temp := Rand();
			m[i][j] := temp - (temp DIV 120)*120 - 60;
			INC(j)
		END ;
		INC(i)
	END
END Initmatrix;

PROCEDURE Innerproduct(VAR result: LONGINT; VAR a,b: intmatrix; row,column: LONGINT);
VAR i: LONGINT;
  (* computes the inner product of A[row,*] and B[*,column] *)
BEGIN
	result := 0; i := 1;
	WHILE i <= rowsize DO
	    result := result+a[row][i]*b[i][column];
	    INC(i)
	END
END Innerproduct;

BEGIN
	Initrand();
	Initmatrix (ima);
	Initmatrix (imb);
	i := 1;
	WHILE i <= rowsize DO j := 1;
		WHILE j <= rowsize DO Innerproduct(imr[i][j],ima,imb,i,j); INC(j) END ;
		INC(i)
	END
	Out.String("Finished!"); Out.Ln;
END MatrixInt.

(*
RUN: %comp %s | filecheck %s
CHECK: Finished!
*)