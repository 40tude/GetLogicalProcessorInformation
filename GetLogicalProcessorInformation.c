// http://msdn.microsoft.com/en-us/library/windows/desktop/ms683194(v=vs.85).aspx 
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

// ----------------------------------------------------------------------------
// Helper function to count set bits in the processor mask
int CountSetBits(ULONG_PTR bitMask){
  
  int       LSHIFT = sizeof(ULONG_PTR)*8 - 1;
  int       bitSetCount = 0;
  ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
  
  for (int i = 0; i <= LSHIFT; ++i){
    bitSetCount += ((bitMask & bitTest)?1:0);
    bitTest/=2;
  }
  return bitSetCount;
}

// ----------------------------------------------------------------------------
int main (){
  
  int                                   done = FALSE;
  int                                   returnLength = 0;             // Attention. Must be initialized to 0 (not -1 or other wierd value)
  int                                   logicalProcessorCount = 0;
  int                                   numaNodeCount = 0;
  int                                   processorCoreCount = 0;
  int                                   processorPackageCount = 0;    
  int                                   processorL1CacheCount = 0, processorL2CacheCount = 0, processorL3CacheCount = 0;
  int                                   processorL1CacheSize = -1, processorL2CacheSize = -1, processorL3CacheSize = -1;
  int                                   byteOffset = 0;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
  PCACHE_DESCRIPTOR                     Cache;

  while (!done){
    int rc = GetLogicalProcessorInformation(buffer, &returnLength);
    if (FALSE == rc){
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER){
        if (buffer) 
          free(buffer);

        buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);

        if (NULL == buffer){
          printf("\nError: Allocation failure\n");
          return EXIT_FAILURE;
        }
      }else{
        printf("\nError %d\n"), GetLastError();
        return EXIT_FAILURE;
      }
    }else{
      done = TRUE;
    }
  }

  ptr = buffer;

  while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength){
    switch (ptr->Relationship){
      case RelationNumaNode:
        // Non-NUMA systems report a single record of this type.
        numaNodeCount++;
      break;

      case RelationProcessorCore:
        processorCoreCount++;
        // A hyperthreaded core supplies more than one logical processor.
        logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
      break;

      case RelationCache:
        // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
        Cache = &ptr->Cache;
        if (Cache->Level == 1){
          processorL1CacheCount++;
          processorL1CacheSize =  Cache->Size;  
          printf("L1 Line size = %d (bytes)\n", Cache->LineSize);
        }else if (Cache->Level == 2){
          processorL2CacheCount++;
          processorL2CacheSize =  Cache->Size;     
          printf("L2 Line size = %d (bytes)\n", Cache->LineSize);
        }else if (Cache->Level == 3){
          processorL3CacheCount++;
          processorL3CacheSize =  Cache->Size;
          printf("L3 Line size = %d (bytes)\n", Cache->LineSize);
        }
      break;

      case RelationProcessorPackage:
        // Logical processors share a physical package.
        processorPackageCount++;
      break;

      default:
        printf("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n");
        break;
    }
    byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    ptr++;
  }

  printf("Number of NUMA nodes                  : %d\n",numaNodeCount);           // NUMA : non-uniform memory access. http://msdn.microsoft.com/en-us/library/windows/desktop/aa363804(v=vs.85).aspx
  printf("Number of physical processor packages : %d\n",processorPackageCount);
  printf("Number of processor cores             : %d\n", processorCoreCount);
  printf("Number of logical processors          : %d\n", logicalProcessorCount);
  printf("Number of processor L1/L2/L3 caches   : %d/%d/%d\n", processorL1CacheCount, processorL2CacheCount, processorL3CacheCount);
  printf("Size of L1 cache                      : %d (bytes)\n", processorL1CacheSize);
  printf("Size of L2 cache                      : %d (bytes)\n", processorL2CacheSize);      
  printf("Size of L3 cache                      : %d (bytes)\n", processorL3CacheSize);      
  free(buffer);
  
  printf("\nStrike ENTER to exit : ");
  getchar();
}
