(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
(**This doc comment is attached to the module name.  *)  
END M.
