(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
BEGIN
  x := a;
  y := b;
END M.
