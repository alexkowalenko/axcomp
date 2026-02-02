MODULE T1;

(* Sorts an array using treesort *)

IMPORT Out;

TYPE
   	node = POINTER TO nodeDesc;
   	nodeDesc = RECORD
   		left, right: node;
   		val: INTEGER;
  	END;

VAR
    tree: node;

PROCEDURE CreateNode (VAR t: node; n: LONGINT);
BEGIN
    NEW(t);
	t.left := NIL;
	t.right := NIL;
	t.val := n
END CreateNode;


END T1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
