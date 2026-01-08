(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE Simple;
IMPORT AM := AnotherModule;
END Simple.
