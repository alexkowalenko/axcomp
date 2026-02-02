MODULE Perm;

IMPORT Out;

CONST
    permrange = 10;

VAR i: LONGINT;
    permarray : ARRAY permrange OF LONGINT;
    pctr: LONGINT;


PROCEDURE Swap (VAR a,b: LONGINT);
VAR t: LONGINT;
BEGIN t := a;  a := b;  b := t;
END Swap;

PROCEDURE Initialize ();
VAR i: LONGINT;
BEGIN i := 1;
	WHILE i <= 7 DO
		permarray[i] := i-1;
		INC(i)
	END
END Initialize;

PROCEDURE Permute (n: LONGINT);
VAR k: LONGINT;
BEGIN
	pctr := pctr + 1;
	IF ( n#1 ) THEN
	    Permute(n-1);
  	    k := n-1;
		WHILE k >= 1 DO
		   	Swap(permarray[n], permarray[k]);
			Permute(n-1);
			Swap(permarray[n], permarray[k]);
			DEC(k)
		END
    END
END Permute;

BEGIN
	pctr := 0; i := 1;
	WHILE i <= 5 DO
		Initialize();
		Permute(7);
		INC(i)
	END ;
	IF ( pctr # 43300 ) THEN
	    Out.String("Error in Perm."); Out.Ln;
	ELSE
	    Out.String("Perm OK"); Out.Ln;
	END
END Perm.

(*
RUN: %comp %s | filecheck %s
CHECK: Perm OK
)
