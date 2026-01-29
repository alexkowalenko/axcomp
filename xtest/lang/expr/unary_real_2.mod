(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module unary_real_2;

import Out;

begin
    Out.Real( + 3.1415927, 6); WriteLn;
end unary_real_2.

(*
CHECK: 3.14159
*)
