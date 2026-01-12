MODULE b13;
BEGIN
    RETURN TRUE & FALSE;
END b13.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
