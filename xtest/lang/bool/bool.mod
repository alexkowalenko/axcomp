MODULE bool;
    IMPORT Out;
BEGIN
    Out.Bool(TRUE); Out.Ln;
    Out.Bool(FALSE); Out.Ln;
END bool.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 0
CHECK-NEXT: 0
*)
