(*
RUN: %comp %s | filecheck %s
*)

module binary_sub_neg;
begin
    return 2 - (-1) ;
end binary_sub_neg.

(*
CHECK: 3
*)
