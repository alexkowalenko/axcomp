(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_real_1;

import Out;

begin
    Out.Real( 1.2 - 3.1415927, 6); Out.Ln;
end binary_real_1.

(*
CHECK: -1.94159
*)
