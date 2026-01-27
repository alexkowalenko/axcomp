MODULE pointer05; (* pointer equality regression *)
IMPORT Out;
VAR x : POINTER TO INTEGER;
BEGIN
    Out.String("Uninitialised equals NIL "); Out.Bool(x = NIL); Out.Ln;
    Out.String("Uninitialised not equals NIL "); Out.Bool(x # NIL); Out.Ln;

    NEW(x);
    x^ := 3;
    Out.String("Allocated equals NIL "); Out.Bool(x = NIL); Out.Ln;
    Out.String("Allocated not equals NIL "); Out.Bool(x # NIL); Out.Ln;

    RETURN 0;
END pointer05.

(*
RUN: %comp %s | filecheck %s
CHECK: Uninitialised equals NIL 1
CHECK-NEXT: Uninitialised not equals NIL 0
CHECK-NEXT: Allocated equals NIL 0
CHECK-NEXT: Allocated not equals NIL 1
CHECK-NEXT: 0
*)
