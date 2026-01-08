(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
BEGIN
  CASE x OF
  | 10: y := 1
  END
END M.
