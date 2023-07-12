
(* Test simple constant folding for integers. *)

MODULE Fold1;

BEGIN
    (* constant *)
    WriteInt(123); WriteLn;

    (* integer unary + and - operator *)
    
    WriteInt(+123); WriteLn;
    WriteInt(-123); WriteLn;

    (* integer arithmetic *)
    WriteInt(1 + 2); WriteLn;
    WriteInt(-1 + 1); WriteLn;
    WriteInt(1 - 1); WriteLn;
    WriteInt(1 + 2 * 3); WriteLn;
    WriteInt(2 * 3 + 4 * 5); WriteLn;
    WriteInt(10 DIV 3); WriteLn;
    WriteInt(10 MOD 3); WriteLn;

END Fold1.
