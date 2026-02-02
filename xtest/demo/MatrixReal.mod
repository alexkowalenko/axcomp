MODULE MatrixReal;

(* Multiplies two integer matrices. *)

IMPORT Out;

CONST
    rowsize = 40;

TYPE
  realmatrix = ARRAY rowsize+1,rowsize+1 OF REAL;

VAR i, j: LONGINT;
    seed: INTEGER;
    rma, rmb, rmr: realmatrix;

PROCEDURE Initrand ();
BEGIN seed := 74755
END Initrand;

PROCEDURE Rand (): LONGINT;
BEGIN
    seed := (seed * 1309 + 13849) MOD 65535;
    RETURN (seed)
END Rand;

PROCEDURE rInitmatrix (VAR m: realmatrix);
VAR temp, i, j: LONGINT;
BEGIN i := 1;
		WHILE i <= rowsize DO j := 1;
			WHILE j <= rowsize DO
				temp := Rand();
				m[i, j] := FLT( (temp - (temp DIV 120)*120 - 60) DIV 3 );
				INC(j)
			END ;
			INC(i)
		END
END rInitmatrix;

PROCEDURE rInnerproduct(VAR result: REAL; VAR a,b: realmatrix; row,column: LONGINT);
(* computes the inner product of A[row,*] and B[*,column] *)
VAR i: LONGINT;
BEGIN
		result := 0.0; i := 1;
		WHILE i<=rowsize DO result := result+a[row,i]*b[i,column]; INC(i) END
END rInnerproduct;

BEGIN
	Initrand();
	rInitmatrix (rma);
	rInitmatrix (rmb);
	i := 1;
	WHILE i <= rowsize DO j := 1;
		WHILE j <= rowsize DO rInnerproduct(rmr[i,j],rma,rmb,i,j); INC(j) END ;
		INC(i)
	END
	Out.String("Finished!"); Out.Ln;
END MatrixReal.

(*
RUN: %comp %s | filecheck %s
CHECK: Finished!
*)