(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
BEGIN
  LOOP
    y := 1
  END
END M.
