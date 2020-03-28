ADD R64B_1, R64B_1, 0; // Loading base address A
ADD R64B_2, R64B_2, 524288; // Loading base address B
ADD R64B_3, R64B_3, 1048576; // Loading base address C

// First segment
LDOFF R2048L_1, R64B_1, 0; // Offset 0
LDOFF R2048L_2, R64B_2, 0; // Offset 0
COD vecAdd_2048L R2048L_3, R2048L_1, R2048L_2;
STOFF R2048L_3, R64B_3, 0; // Offset 0
// Second segment
LDOFF R2048L_1, R64B_1, 131072; // Offset 1
LDOFF R2048L_2, R64B_2, 131072; // Offset 1
COD vecAdd_2048L R2048L_3, R2048L_1, R2048L_2;
STOFF R2048L_3, R64B_3, 131072; // Offset 1
// Third segment
LDOFF R2048L_1, R64B_1, 262144; // Offset 2
LDOFF R2048L_2, R64B_2, 262144; // Offset 2
COD vecAdd_2048L R2048L_3, R2048L_1, R2048L_2;
STOFF R2048L_3, R64B_3, 262144; // Offset 2
// Fourth segment
LDOFF R2048L_1, R64B_1, 393216; // Offset 3
LDOFF R2048L_2, R64B_2, 393216; // Offset 3
COD vecAdd_2048L R2048L_3, R2048L_1, R2048L_2;
STOFF R2048L_3, R64B_3, 393216; // Offset 3
COMMIT