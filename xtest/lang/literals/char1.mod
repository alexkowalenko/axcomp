(*
RUN: %comp %s | filecheck %s
CHECK: 99
*)

MODULE char1;
BEGIN
    RETURN 'c';
END char1.