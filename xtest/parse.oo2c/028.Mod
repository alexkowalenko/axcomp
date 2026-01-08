(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
BEGIN
  x := a
END M.
