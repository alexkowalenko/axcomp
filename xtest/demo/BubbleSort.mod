MODULE BubbleSort;

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

PROCEDURE bInitarr();
	VAR i, temp: LONGINT;
BEGIN
		Initrand();
		biggest := 0; littlest := 0; i := 1;
		WHILE i <= srtelements DO
			temp := Rand();
			sortlist[i] := temp - (temp DIV 100000)*100000 - 50000;
			IF sortlist[i] > biggest THEN biggest := sortlist[i]
			ELSIF sortlist[i] < littlest THEN littlest := sortlist[i]
			END ;
			INC(i)
		END
END bInitarr;

BEGIN
	bInitarr();
	top:=srtelements;
	WHILE top>1 DO
		i:=1;
		WHILE i<top DO
			IF sortlist[i] > sortlist[i+1] THEN
				j := sortlist[i];
				sortlist[i] := sortlist[i+1];
				sortlist[i+1] := j;
			END ;
			i:=i+1;
		END;
		top:=top-1;
	END;
	IF (sortlist[1] # littlest) OR (sortlist[srtelements] # biggest) THEN
	    Out.String("Error3 in Bubble.$"); Out.Ln;
	ELSE
	    Out.String("BubbleSort OK"); Out.Ln;
	END;
END BubbleSort.

(*
RUN: %comp %s | filecheck %s
CHECK: BubbleSort OK
*)
