(*
RUN: %comp %s | filecheck %s
*)

module binary_div_neg;
begin
    return (-12) DIV 5;
end binary_div_neg.

(* -2
CHECK: 254
*)
