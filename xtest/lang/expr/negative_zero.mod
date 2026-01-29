(*
RUN: %comp %s | filecheck %s
*)

module negative_zero;
begin
    return -0;
end negative_zero.

(*
CHECK: 0 
*)
