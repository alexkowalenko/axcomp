(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN+ *>

module caser_5;
var x: INTEGER;
begin
    x := 3;
    case x of
      1: WriteInt(1); WriteLn;
      | 3..7+1:  WriteInt(4); WriteLn;
      | 10 : WriteInt(10); WriteLn;
    else
      WriteInt(999); WriteLn;
    end;
end caser_5.

(*
CHECK: 4
*)

