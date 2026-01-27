MODULE pointer04; (* pointers *)
IMPORT Out;
VAR x : POINTER TO INTEGER;
BEGIN
    NEW(x);
    x^ := 1;
    Out.String("Equals NIL "); Out.Bool(x = NIL); Out.Ln;
    Out.String("Not Equals NIL "); Out.Bool(x # NIL); Out.Ln;
    RETURN 0;
END pointer04.

(*
RUN: %comp %s | filecheck %s
CHECK: Equals NIL 0
CHECK-NEXT: Not Equals NIL 1
CHECK-NEXT: 0
*)