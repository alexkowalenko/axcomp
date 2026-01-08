(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
BEGIN
  IF a THEN
    x := 1
  ELSIF b THEN
    x := 2
  END
END M.
