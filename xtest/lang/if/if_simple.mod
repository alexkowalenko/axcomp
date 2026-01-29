(*
RUN:  %comp %s | filecheck %s
*)

MODULE if_simple;

BEGIN
    IF false THEN
        return 1;
    else
        return 0;
    end
end if_simple.

(*
CHECK: 0
*)
