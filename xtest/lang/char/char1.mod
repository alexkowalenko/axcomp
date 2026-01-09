(*
RUN: %comp %s | filecheck %s
CHECK: 97
*)

MODULE char1;
CONST a = 'a';
BEGIN
    RETURN a
END char1.