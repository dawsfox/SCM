#include "MatMulOMPoffload.hpp"
#include <iomanip>
#include "mkl.h"
#include "mkl_cblas.h"
#include "mkl_omp_offload.h"

#pragma omp requires unified_shared_memory

#define TILE_DIM 128

IMPLEMENT_CODELET(LoadSqTileGPU_2048L,
  // Obtaining the parameters
  unsigned char ** args = static_cast<unsigned char **>(this->getParams());
  unsigned char *reg1 = args[0]; // Getting register 1
  unsigned char *reg2 = args[1]; // Getting register 2
  unsigned char *reg3 = args[2]; // Getting register 3
  double *destReg = reinterpret_cast<double*>(reg1);
  uint64_t address = reinterpret_cast<uint8_t*>(reg2)[0];
  uint64_t ldistance = reinterpret_cast<uint64_t>(reg3);
  for (int i = 1; i < 8; i++) {
    address <<= 8;
    address += static_cast<uint8_t>(reg2[i]);
  }
  double *addressStart = reinterpret_cast<double *> (getAddress(address)); // Address L2 memory to a pointer of the runtime
  for (uint64_t i = 0; i < TILE_DIM; i++) {
    std::memcpy(destReg+TILE_DIM*i, addressStart+ldistance*i, TILE_DIM*sizeof(double));
  }
);

IMPLEMENT_CODELET(MatMultGPU_2048L,
  // Obtaining the parameters
  unsigned char ** args = static_cast<unsigned char **>(this->getParams());
  unsigned char *reg1 = args[0]; // Getting register 1
  unsigned char *reg2 = args[1]; // Getting register 2
  unsigned char *reg3 = args[2]; // Getting register 3
  double *A = reinterpret_cast<double*>(reg2);
  double *B = reinterpret_cast<double*>(reg3);
  double *C = reinterpret_cast<double*>(reg1);

    
_Pragma("omp target data map(to:A[0:TILE_DIM*TILE_DIM],B[0:TILE_DIM*TILE_DIM]) map(tofrom:C[0:TILE_DIM*TILE_DIM])")
    {

        // run gemm on gpu, use standard oneMKL interface within a variant dispatch construct
_Pragma("omp target variant dispatch use_device_ptr(A, B, C)")
        {
          cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, TILE_DIM, TILE_DIM, TILE_DIM, 1, A, TILE_DIM, B, TILE_DIM, 1, C, TILE_DIM);
        }       
    }

);


IMPLEMENT_CODELET(StoreSqTileGPU_2048L,
  // Obtaining the parameters
  unsigned char ** args = static_cast<unsigned char **>(this->getParams());
  unsigned char *reg1 = args[0]; // Getting register 1
  unsigned char *reg2 = args[1]; // Getting register 2
  unsigned char *reg3 = args[2]; // Getting register 3
  double *sourceReg = reinterpret_cast<double*>(reg1);
  uint64_t address = reinterpret_cast<uint8_t*>(reg2)[0];
  uint64_t ldistance = reinterpret_cast<uint64_t>(reg3);
  for (int i = 1; i < 8; i++) {
    address <<= 8;
    address += static_cast<uint8_t>(reg2[i]);
  }
  double *addressStart = reinterpret_cast<double *> (getAddress(address)); // Address L2 memory to a pointer of the runtime
  for (uint64_t i = 0; i < TILE_DIM; i++) {
    std::memcpy(addressStart+ldistance*i, sourceReg+TILE_DIM*i, TILE_DIM*sizeof(double));
  }
);
