MODULE openarray1; (* open arrays *)
IMPORT Out;
VAR x : ARRAY OF INTEGER;
BEGIN
    NEW(x, 10);

    x[1] := 3;
    RETURN 0;
END openarray1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
