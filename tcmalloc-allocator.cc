#include "tcmalloc-test.h"

#include <malloc.h>

#include <string>
#include <sstream>

static inline void* Memalign(size_t align, size_t size) {
  // http://linux.die.net/man/3/memalign
  return memalign(align, size);
}
static inline int PosixMemalign(void** ptr, size_t align, size_t size) {
  // http://linux.die.net/man/3/posix_memalign
  return posix_memalign(ptr, align, size);
}

int TCMallocAllocator::Uniform(int n) {
   if (n == 0) {
      return rand() * 0;
   } else {
      return rand() % n;
   }
}

// Allocate memory.  Randomly choose between malloc() or posix_memalign().
void* TCMallocAllocator::alloc(size_t size) {
   if(Uniform(100) < memalign_fraction_ * 100) {
      // Try a few times to find a reasonable alignment, or fall back on malloc.
      for (int i = 0; i < 5; i++) {
         size_t alignment = 1 << Uniform(FLAGS_lg_max_memalign);
         if (alignment >= sizeof(intptr_t) &&
             (size < sizeof(intptr_t) ||
              alignment < FLAGS_memalign_max_alignment_ratio * size)) {
           void *result = reinterpret_cast<void*>(static_cast<intptr_t>(0x1234));
           int err = PosixMemalign(&result, alignment, size);
           if (err == ENOMEM) {
             printf("No memory available!");
           }
           return err == 0 ? result : NULL;
         }
      }
   }
   return malloc(size);
}

char* TCMallocAllocator::getStats() {
   output_buffer.str("");
   output_buffer.clear();
   size_t current_allocated_bytes, heap_size, pageheap_free_bytes, pageheap_unmapped_bytes,
          central_cache_free_bytes, transfer_cache_free_bytes, aggressive_memory_decommit;
   extension->GetNumericProperty("generic.current_allocated_bytes", &current_allocated_bytes);
   extension->GetNumericProperty("generic.heap_size", &heap_size);
   extension->GetNumericProperty("tcmalloc.pageheap_free_bytes", &pageheap_free_bytes);
   extension->GetNumericProperty("tcmalloc.pageheap_unmapped_bytes", &pageheap_unmapped_bytes);
   extension->GetNumericProperty("tcmalloc.central_cache_free_bytes", &central_cache_free_bytes);
   extension->GetNumericProperty("tcmalloc.transfer_cache_free_bytes", &transfer_cache_free_bytes);
   extension->GetNumericProperty("tcmalloc.aggressive_memory_decommit", &aggressive_memory_decommit);

   output_buffer << "generic.heap_size: " << heap_size << "\r\n";
   output_buffer << "generic.current_allocated_bytes: " << current_allocated_bytes << "\r\n";
   output_buffer << "tcmalloc.pageheap_free_bytes: " << pageheap_free_bytes << "\r\n";
   output_buffer << "tcmalloc.pageheap_unmapped_bytes: " << pageheap_unmapped_bytes << "\r\n";
   output_buffer << "tcmalloc.central_cache_free_bytes: " << central_cache_free_bytes << "\r\n";
   output_buffer << "tcmalloc.transfer_cache_free_bytes: " << transfer_cache_free_bytes << "\r\n";
   output_buffer << "tcmalloc.aggressive_memory_decommit: " << aggressive_memory_decommit << "\r\n";

   return const_cast<char*>(output_buffer.str().c_str());
}

char* TCMallocAllocator::getVersion() {
   std::ostringstream text;
   int major;
   int minor;
   const char* patch;
   const char* human_version = tc_version(&major, &minor, &patch);
   text << tc_version(&major, &minor, &patch) << '\n';
   //printf("Version: %d.%d%s\n", major, minor, patch);
   return const_cast<char*>(text.str().c_str());
}
