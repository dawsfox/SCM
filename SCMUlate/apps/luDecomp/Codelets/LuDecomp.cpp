#include "LuDecomp.hpp"
#include <iomanip>

// BENCH is externed so now we can access it directly instead of using a base register

// sizes of codelets? big questions here
// col -- read/write
// diag -- read
IMPLEMENT_CODELET(fwd_2048L,
  float *col = this->getParams().getParamValueAs<float*>(1);
  float *diag = this->getParams().getParamValueAs<float*>(2);

  int i;
  int j;
  int k;
  for (j=0; j<bots_arg_size_1; j++)
    for (k=0; k<bots_arg_size_1; k++) 
        for (i=k+1; i<bots_arg_size_1; i++)
          col[i*bots_arg_size_1+j] = col[i*bots_arg_size_1+j] - diag[i*bots_arg_size_1+k]*col[k*bots_arg_size_1+j];
);

IMPLEMENT_CODELET(bdiv_2048L,
  float *row = this->getParams().getParamValueAs<float*>(1);
  float *diag = this->getParams().getParamValueAs<float*>(2);

  int i; 
  int j;
  int k;
  for (i=0; i<bots_arg_size_1; i++)
    for (k=0; k<bots_arg_size_1; k++)
    {
      row[i*bots_arg_size_1+k] = row[i*bots_arg_size_1+k] / diag[k*bots_arg_size_1+k];
      for (j=k+1; j<bots_arg_size_1; j++)
        row[i*bots_arg_size_1+j] = row[i*bots_arg_size_1+j] - row[i*bots_arg_size_1+k]*diag[k*bots_arg_size_1+j];
    }
);

IMPLEMENT_CODELET(bmod_2048L,
  float *inner = this->getParams().getParamValueAs<float*>(1);
  float *row = this->getParams().getParamValueAs<float*>(2);
  float *col = this->getParams().getParamValueAs<float*>(3);

  int i; 
  int j;
  int k;
  for (i=0; i<bots_arg_size_1; i++)
    for (j=0; j<bots_arg_size_1; j++)
      for (k=0; k<bots_arg_size_1; k++)
        inner[i*bots_arg_size_1+j] = inner[i*bots_arg_size_1+j] - row[i*bots_arg_size_1+k]*col[k*bots_arg_size_1+j];
);

IMPLEMENT_CODELET(lu0_2048L,
  float *diag = this->getParams().getParamValueAs<float*>(1);
  //printf("diag is %p\n", diag);
  //printf("printing first 4: %lx %lx %lx %lx\n", diag[0], diag[1], diag[2], diag[3]);
  //printf("printing last 4: %lx %lx %lx %lx\n", diag[bots_arg_size_1*bots_arg_size_1-4], diag[bots_arg_size_1*bots_arg_size_1-3], diag[bots_arg_size_1*bots_arg_size_1-2], diag[bots_arg_size_1*bots_arg_size_1-1]);
  int i; 
  int j;
  int k;
  for (k=0; k<bots_arg_size_1; k++)
    for (i=k+1; i<bots_arg_size_1; i++)
    {
      diag[i*bots_arg_size_1+k] = diag[i*bots_arg_size_1+k] / diag[k*bots_arg_size_1+k];
      for (j=k+1; j<bots_arg_size_1; j++)
      diag[i*bots_arg_size_1+j] = diag[i*bots_arg_size_1+j] - diag[i*bots_arg_size_1+k] * diag[k*bots_arg_size_1+j];
    }
  
  //printf("printing first 4: %lx %lx %lx %lx\n", diag[0], diag[1], diag[2], diag[3]);
);

IMPLEMENT_CODELET(zero_2048L,
  float *toZero = this->getParams().getParamValueAs<float*>(1);
  for (int i=0; i<bots_arg_size_1; i++) {
    for (int j=0; j<bots_arg_size_1; j++) {
      toZero[i*bots_arg_size_1+j] = 0.0;
    }
  }
);

// takes an offset kk which is used to locate the submatrix
// should be used before the lu0 codelet to load the data it needs
MEMRANGE_CODELET(loadSubMat_2048L,
  uint64_t row_offset = *(this->getParams().getParamValueAs<uint64_t*>(2)); // kk in the original code
  uint64_t col_offset = *(this->getParams().getParamValueAs<uint64_t*>(3));
  uint64_t actual_offset = (row_offset * bots_arg_size + col_offset); // original contains bots_arg_size which is 50; define at top
  // Add range that will be touched (range of sub matrix)
  uint64_t submat = (uint64_t) BENCH[actual_offset];
  // BENCH should be the first thing allocated in the SCM memory space, so the offset to the submatrix is submat pointer - BENCH
  uint64_t mem_offset = submat - (uint64_t)BENCH;
  this->addReadMemRange(mem_offset, bots_arg_size_1*bots_arg_size_1*sizeof(float)); // accounts for range in submatrix
  this->addReadMemRange(actual_offset*sizeof(float*), sizeof(float*)); // accounts for pointer being read in overall matrix
);

IMPLEMENT_CODELET(loadSubMat_2048L,
  float * destReg = this->getParams().getParamValueAs<float *>(1); 
  //float * addressStart = reinterpret_cast<float *>(getAddress(memoryRanges->reads.begin()->memoryAddress));
  float * addressStart = reinterpret_cast<float *>(getAddress(memoryRanges->reads.rbegin()->memoryAddress));
  // if this memory address is 0, that means the pointer we read from BENCH was NULL and this is a critical error
  uint64_t bench_idx = (uint64_t) memoryRanges->reads.begin()->memoryAddress; // get index of bench we're reading submat pointer from for error checking
  //printf("addressStart offset: 0x%lx, bench_idx offset: 0x%lx\n", memoryRanges->reads.rbegin()->memoryAddress, bench_idx);
  //printf("addressStart offset: 0x%lx\n", memoryRanges->reads.rbegin()->memoryAddress);
  SCMULATE_ERROR_IF(0, *((float**)(((unsigned char *)BENCH)+bench_idx)) == nullptr, "Error: loading submatrix from invalid pointer");
  std::memcpy(destReg, addressStart, memoryRanges->reads.rbegin()->size);
);

MEMRANGE_CODELET(storeSubMat_2048L,
  uint64_t row_offset = *(this->getParams().getParamValueAs<uint64_t*>(2)); // kk in the original code
  uint64_t col_offset = *(this->getParams().getParamValueAs<uint64_t*>(3));
  float ** bench_addr = BENCH;
  uint64_t actual_offset = (row_offset * bots_arg_size + col_offset); // original contains bots_arg_size which is 50; define at top
  // Add range that will be touched (range of sub matrix)
  uint64_t submat = (uint64_t) BENCH[actual_offset];
  // BENCH should be the first thing allocated in the SCM memory space, so the offset to the submatrix is submat pointer - BENCH
  uint64_t mem_offset = submat - (uint64_t)BENCH;
  this->addWriteMemRange(mem_offset, bots_arg_size_1*bots_arg_size_1*sizeof(float)); // account for writing range in the submatrix
  this->addReadMemRange(actual_offset*sizeof(float*), sizeof(float*)); // account for reading the pointer in the overarching matrix
);

IMPLEMENT_CODELET(storeSubMat_2048L,
  float * sourceReg = this->getParams().getParamValueAs<float *>(1); 
  printf("sourceReg @ %p\n", sourceReg);
  fflush(stdout);
  float * addressStart = reinterpret_cast<float *>(getAddress(memoryRanges->writes.begin()->memoryAddress));
  uint64_t bench_idx = (uint64_t) memoryRanges->reads.begin()->memoryAddress; // get index of bench we're reading submat pointer from for error checking
  printf("sourceReg @ %p; submatrix in memory @ %p; bench offset is 0x%lx\n", sourceReg, addressStart, bench_idx);
  SCMULATE_ERROR_IF(0, *((float**)(((unsigned char *)BENCH)+bench_idx)) == nullptr, "Error: storing submatrix to invalid pointer");
  std::memcpy(addressStart, sourceReg, memoryRanges->writes.begin()->size);
  //printf("printing first 4: %lx %lx %lx %lx\n", addressStart[0], addressStart[1], addressStart[2], addressStart[3]);
);

// yes, this is a memory codelet that only exists to load the address of BENCH into a register
// yes, this is hacky
// yes, I'm sure there IS a better way to do this
// no, I'm not the one that will be doing it right now
MEMRANGE_CODELET(loadBenchAddr_64B,
  //this->addReadMemRange((uint64_t)&BENCH, sizeof(float**));
  // BENCH is located at the beginning of SCM memory, hence an offset of 0
  this->addReadMemRange((uint64_t)0, sizeof(float**));
);

IMPLEMENT_CODELET(loadBenchAddr_64B,
  float *** destReg = this->getParams().getParamValueAs<float ***>(1);
  //printf("bench: %p\n", BENCH);
  std::memcpy(destReg, &BENCH, sizeof(float **));
);

/*
MEMRANGE_CODELET(LoadSqTile_2048L, 
  // Obtaining the parameters
  uint8_t* address_reg = this->getParams().getParamValueAs<uint8_t*>(2);
  uint64_t ldistance = this->getParams().getParamValueAs<uint64_t>(3);

  uint64_t address = address_reg[0];
  ldistance *= sizeof(double);
  for (int i = 1; i < 8; i++) {
    address <<= 8;
    address += address_reg[i];
  }
  // Add the ranges
  for (uint64_t i = 0; i < TILE_DIM; i++) {
    this->addReadMemRange(address+ldistance*i, TILE_DIM*sizeof(double));
  }
);
IMPLEMENT_CODELET(LoadSqTile_2048L,
  double *destReg = this->getParams().getParamValueAs<double*>(1);
  int i = 0;
  for (auto it = memoryRanges->reads.begin(); it != memoryRanges->reads.end(); it++) {
    double *addressStart = reinterpret_cast<double *> (getAddress(it->memoryAddress)); // Address L2 memory to a pointer of the runtime
    std::memcpy(destReg+TILE_DIM*i++, addressStart, it->size);
  }
);

IMPLEMENT_CODELET(MatMult_2048L,
  double *A = this->getParams().getParamValueAs<double*>(2);
  double *B = this->getParams().getParamValueAs<double*>(3);
  double *C = this->getParams().getParamValueAs<double*>(1);

  // for (int i = 0; i < TILE_DIM; i++)
  //   for (int j = 0; j < TILE_DIM; j++)
  //     for (int k = 0; k < TILE_DIM; k++)
        // C[i + j*TILE_DIM] = A[i + k*TILE_DIM]*B[j*TILE_DIM + k]; 
#ifndef NOBLAS
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, TILE_DIM, TILE_DIM, TILE_DIM, 
              1, A, TILE_DIM, B, TILE_DIM, 1, C, TILE_DIM);
#else
  for (int i=0; i<TILE_DIM; i=i+1){
    for (int j=0; j<TILE_DIM; j=j+1){
        for (int k=0; k<TILE_DIM; k=k+1) {
          C[i*TILE_DIM + j]+=((A[i*TILE_DIM + k])*(B[k*TILE_DIM + j]));
        }
    }
   }
#endif
);

MEMRANGE_CODELET(StoreSqTile_2048L, 
  // Obtaining the parameters
  uint8_t* address_reg = this->getParams().getParamValueAs<uint8_t*>(2);
  uint64_t ldistance = this->getParams().getParamValueAs<uint64_t>(3);

  uint64_t address = address_reg[0];
  ldistance *= sizeof(double);
  for (int i = 1; i < 8; i++) {
    address <<= 8;
    address += address_reg[i];
  }
  for (uint64_t i = 0; i < TILE_DIM; i++) {
    this->addWriteMemRange(address+ldistance*i, TILE_DIM*sizeof(double));
  }
);

IMPLEMENT_CODELET(StoreSqTile_2048L,
  // Obtaining the parameters
  double *sourceReg = this->getParams().getParamValueAs<double*>(1);

  int i = 0;
  for (auto it = memoryRanges->writes.begin(); it != memoryRanges->writes.end(); it++) {
    double *addressStart = reinterpret_cast<double *> (getAddress(it->memoryAddress)); // Address L2 memory to a pointer of the runtime
    std::memcpy(addressStart, sourceReg+TILE_DIM*i++, it->size);
  }
);
*/
