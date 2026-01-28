(*
RUN: %comp %s | filecheck %s
*)

module zero;
begin
    return 0;
end zero.


(*
CHECK: 0
*)