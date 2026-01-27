MODULE string8; (* STRING type *)
IMPORT Out;
VAR   x: STRING;
    y: STRING;

BEGIN
    x := "Hello there!"
    COPY(x, y);
    Out.String(y); Out.Ln;
    Out.String("Original:"); Out.Ln;
    Out.String(x); Out.Ln;
    RETURN 0;
END string8.

(*
RUN: %comp %s | filecheck %s
CHECK:Hello there!
CHECK-NEXT: Original:
CHECK-NEXT: Hello there!
CHECK-NEXT: 0
*)