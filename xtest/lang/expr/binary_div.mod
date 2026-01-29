(*
RUN: %comp %s | filecheck %s
*)

module binary_div;
begin
    return 4 DIV 2;
end binary_div.

(*
CHECK: 2
*)
