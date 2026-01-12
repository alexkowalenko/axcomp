MODULE b14;
BEGIN
    RETURN ~ TRUE
END b14.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
