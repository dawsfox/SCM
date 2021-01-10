// Registers: R<size>_<number> where size = 64B, 1L, 8L, 16L, 256L, 512L, 1024L, 2048L
  Label1:
    JMPLBL Label2;
    JMPLBL Label2; // IGNORED
    JMPLBL Label2; // IGNORED
  Label2:
    JMPPC 2;
    JMPPC 100; // Ignored
    BREQ R64B_1, R64B_2, 2;
    BREQ R64B_1, R64B_2, 100; // Ignored
    BGT R64B_1, R64B_2, 2; 
    BGT R64B_1, R64B_2, 1; 
    BGET R64B_1, R64B_2, 2;
    BGET R64B_1, R64B_2, 1; // Ignored
    BGET R64B_1, R64B_2, Future_label;
    BLET R64B_1, R64B_2, 1; // Ignored
    BLET R64B_1, R64B_2, 1; // Ignored
    BLET R64B_1, R64B_2, 1; // Ignored
  Future_label:
    BLT R64B_1, R64B_2, 2;
    BLT R64B_1, R64B_2, 1;
    BLET R64B_1, R64B_2, 2;
    BLET R64B_1, R64B_2, 1; // Ignored
    BLET R64B_1, R64B_2, Future_label2; // Ignored
    BLET R64B_1, R64B_2, 1; // Ignored
    BLET R64B_1, R64B_2, 1; // Ignored
  Future_label2:
    ADD R64B_1, R64B_2, 100; 
    COD print R64B_1, 8; // PC = 23
    ADD R64B_1, R64B_1, 100; 
    ADD R64B_1, R64B_1, 100; 
    ADD R64B_2, R64B_1, R64B_1; 
    SUB R64B_1, R64B_2, R64B_1; 
    SUB R64B_1, R64B_2, 100; 
    //SHFL R64B_1, R64B_2;
    //SHFL R64B_1, -100;
    //SHFR R64B_1, R64B_2;
    //SHFR R64B_1, -100;
    LDIMM R64B_2, 0;
    LDIMM R64B_3, 0;
    LDADR R64B_1, R64B_2;
    LDADR R64B_1, 02020;
    LDOFF R64B_1, R64B_2, R64B_3; 
    LDOFF R64B_1, R64B_2, 02020; 
    LDOFF R64B_1, 1010, 10;
    STADR R64B_1, R64B_2;
    STADR R64B_1, 02020;
    STOFF R64B_1, R64B_2, R64B_3; 
    STOFF R64B_1, R64B_2, 10; 
    STOFF R64B_1, 1010, 10; 
    COMMIT;