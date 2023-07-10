MODULE fibonacci;

IMPORT Out;


PROCEDURE fibonacci(x: INTEGER): INTEGER;
BEGIN
    IF x < 2 THEN
        RETURN x;
    ELSE
        RETURN fibonacci(x -1) + fibonacci(x -2);
    END
END fibonacci;

BEGIN
    Out.Int(fibonacci(35), 0); Out.Ln();
END fibonacci.