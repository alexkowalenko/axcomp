MODULE enum3;
TYPE color_type = (red, green, blue);
     quantity_type = (up, down, charm, strange, top, bottom);
     quark = RECORD
         color: color_type;
         quantity: quantity_type;
     END;
BEGIN
END enum3.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
