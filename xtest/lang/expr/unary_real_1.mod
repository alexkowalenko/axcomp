(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module unary_real_1;

import Out;

begin
    Out.Real( - 3.1415927, 6); Out.Ln;
end unary_real_1.

(*
CHECK: -3.14159
*)
