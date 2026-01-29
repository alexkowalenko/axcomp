(*
RUN: %comp %s | filecheck %s
*)

module binary_sub;
begin
    return 1 - 2 ;
end binary_sub.

(* -1
CHECK: 255
*)
