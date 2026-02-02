MODULE QuickSort;

 (* Sorts an array using bubblesort *)

IMPORT Out;

CONST
    sortelements = 5000;
    srtelements = 500;

VAR
     sortlist: ARRAY sortelements+1 OF LONGINT;
     biggest, littlest, top: LONGINT;
     i, j: LONGINT;
     seed: LONGINT;

PROCEDURE Initrand ();
BEGIN seed := 74755
END Initrand;

PROCEDURE Rand (): LONGINT;
BEGIN
    seed := (seed * 1309 + 13849) MOD 65535;
    RETURN (seed)
END Rand;

PROCEDURE Initarr();
		VAR i, temp: LONGINT;
BEGIN
		Initrand();
		biggest := 0; littlest := 0; i := 1;
		WHILE i <= sortelements DO
			temp := Rand();
			sortlist[i] := temp - (temp DIV 100000)*100000 - 50000;
			IF sortlist[i] > biggest THEN biggest := sortlist[i]
			ELSIF sortlist[i] < littlest THEN littlest := sortlist[i]
			END ;
			INC(i)
		END
END Initarr;

PROCEDURE Quicksort(VAR a: ARRAY sortelements+1 OF LONGINT; l,r: LONGINT);
  (* quicksort the array A from start to finish *)
VAR i,j,x,w: LONGINT;
BEGIN
		i:=l; j:=r;
		x:=a[(l+r) DIV 2];
		REPEAT
			WHILE a[i]<x DO i := i+1 END;
			WHILE x<a[j] DO j := j-1 END;
			IF i<=j THEN
				w := a[i];
				a[i] := a[j];
				a[j] := w;
				i := i+1;    j := j-1
			END ;
		UNTIL i > j;
		IF l<j THEN Quicksort(a,l,j) END;
		IF i<r THEN Quicksort(a,i,r) END
END Quicksort;

BEGIN
    Initarr();
    Quicksort(sortlist,1,sortelements);
    Out.String("littlest: "); Out.Int(littlest, 3); Out.Char(' '); Out.Int(sortlist[1], 3); Out.Ln;
    Out.String("biggest: "); Out.Int(biggest, 3); Out.Char(' '); Out.Int(sortlist[sortelements], 3); Out.Ln;
	IF (sortlist[1] # littlest) OR (sortlist[sortelements] # biggest) THEN
	    Out.String("Error3 in Quick.$"); Out.Ln;
	ELSE
	    Out.String("QuickSort OK"); Out.Ln;
	END;
END QuickSort.

(*
RUN: %comp %s | filecheck %s
CHECK: QuickSort OK
*)
