(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_relation_mixed_1;

begin
     WriteBoolean(  1 = 2.0); WriteLn;
     WriteBoolean(  1 # 2.0); WriteLn;
     WriteBoolean(  1 > 2.0); WriteLn;
     WriteBoolean(  1 >= 2.0); WriteLn;
     WriteBoolean(  1 < 2.0); WriteLn;
     WriteBoolean(  1 <= 2.0); WriteLn;
end binary_relation_mixed_1.

(*
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 1
*)
