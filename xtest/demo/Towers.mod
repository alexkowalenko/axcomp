MODULE Towers;
(*  Program to Solve the Towers of Hanoi *)

IMPORT Out;

CONST
    maxcells = 18;
    stackrange = 3;

TYPE
    element = RECORD
		discsize: LONGINT;
		next: LONGINT;
	END ;

VAR
    stack: ARRAY stackrange+1 OF LONGINT;
	cellspace: ARRAY maxcells+1 OF element;
	freelist: LONGINT;
	movesdone: LONGINT;
	i: LONGINT;

PROCEDURE Makenull (s: LONGINT);
BEGIN stack[s] := 0
END Makenull;

PROCEDURE Getelement (): LONGINT;
VAR temp: LONGINT;
BEGIN
	IF ( freelist>0 ) THEN
		temp := freelist;
		freelist := cellspace[freelist].next;
	ELSE
		Out.String("out of space   $"); Out.Ln;
	END ;
	RETURN temp
END Getelement;

PROCEDURE Push(i,s: LONGINT);
VAR localel: LONGINT; errorfound: BOOLEAN;
BEGIN
	errorfound := FALSE;
	IF ( stack[s] > 0 ) THEN
		IF ( cellspace[stack[s]].discsize<=i ) THEN
			errorfound := TRUE;
			Out.String("disc size error$"); Out.Ln;
		END
	END ;
	IF ( ~ errorfound ) THEN
		localel := Getelement();
		cellspace[localel].next := stack[s];
		stack[s] := localel;
		cellspace[localel].discsize := i
	END
END Push;

PROCEDURE Init (s,n: LONGINT);
	VAR discctr: LONGINT;
BEGIN
	Makenull(s); discctr := n;
	WHILE discctr >= 1 DO
		Push(discctr,s);
		DEC(discctr)
	END
END Init;

PROCEDURE Pop (s: LONGINT): LONGINT;
VAR temp, temp1: LONGINT;
BEGIN
	IF ( stack[s] > 0 ) THEN
		temp1 := cellspace[stack[s]].discsize;
		temp := cellspace[stack[s]].next;
		cellspace[stack[s]].next := freelist;
		freelist := stack[s];
		stack[s] := temp;
	ELSE
		Out.String("nothing to pop $"); Out.Ln;
		temp1 := 0;
	END
	RETURN (temp1)
END Pop;

PROCEDURE Move (s1,s2: LONGINT);
BEGIN
	Push(Pop(s1),s2);
	movesdone := movesdone+1;
END Move;

PROCEDURE tower(i,j,k: LONGINT);
	VAR other: LONGINT;
BEGIN
	IF ( k=1 ) THEN
		Move(i,j);
	ELSE
		other := 6-i-j;
		tower(i,other,k-1);
		Move(i,j);
		tower(other,j,k-1)
	END
END tower;

BEGIN
    i := 1;
	WHILE i <= maxcells DO
	    cellspace[i].next := i-1;
	    INC(i)
	END;
	freelist := maxcells;
	Init(1,14);
	Makenull(2);
	Makenull(3);
	movesdone := 0;
	tower(1,2,14);
	IF ( movesdone # 16383 ) THEN
	    Out.String("Error in Towers."); Out.Ln;
	ELSE
	    Out.String("Success!"); Out.Ln;
	END;
END Towers.

(*
RUN: %comp %s | filecheck %s
CHECK: Success!
*)

