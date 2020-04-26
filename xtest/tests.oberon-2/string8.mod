MODULE string8; (* STRING type *)
IMPORT Out;
VAR   x: STRING;
    y: STRING;

BEGIN
    x := "Hello there!"
    COPY(x, y);
    Out.String(y); Out.Ln;
    Out.String("Orignal:"); Out.Ln;
    Out.String(x); Out.Ln;
    RETURN 0;
END string8.