(*
RUN: %comp %s | filecheck %s
*)

module binary_unary_op;
begin
    return -(1 - 2) ;
end binary_unary_op.

(*
CHECK: 1
*)
