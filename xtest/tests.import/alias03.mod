MODULE alias03;
    
IMPORT  B:= beta;

BEGIN
    B.d := TRUE;
    WriteBoolean(B.d); WriteLn;
    RETURN 0; 
END alias03.