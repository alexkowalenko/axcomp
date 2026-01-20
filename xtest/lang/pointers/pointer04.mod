MODULE pointer04; (* pointers *)
IMPORT Out;
VAR x : POINTER TO INTEGER;
BEGIN
    Out.String("Equals NIL "); Out.Bool(x = NIL); Out.Ln;
    Out.String("Not Equals NIL "); Out.Bool(x # NIL); Out.Ln;
    RETURN x = NIL;
END pointer04.

(*
RUN: %comp %s | filecheck %s
CHECK: Equals NIL 1
CHECK-NEXT: Not Equals NIL 0
CHECK-NEXT: 1
*)