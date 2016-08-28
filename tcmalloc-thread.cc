#include "tcmalloc-test.h"

#include <string.h>
#include <time.h>
#include <unistd.h>

#include <vector>

static char alphanum[] = "0123456789"
       "!@#$%^&*"
       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
       "abcdefghijklmnopqrstuvwxyz";

void TCMallocTestThread::runOperation(int type, int numberOfObjects) {
    ThreadOperation op;
    op.type = type;   
    op.number = numberOfObjects;   
    threadOperations.push(op);
}

void TCMallocTestThread::run() {
     while(true) {
          if(threadOperations.empty()) {
              usleep(500000);
              continue;
          }
          ThreadOperation op = threadOperations.front();
          threadOperations.pop();
          if(op.type == OP_SHUTDOWN) {
              break;
          }
          switch(op.type) {
              case OP_ALLOCATE:
                    for(int i = op.number; --i > 0; ) {
                         AllocateObject();
                    };
                    break;
              case OP_UPDATE_RANDOM:
                    for(int i = op.number; --i > 0; ) {
                         UpdateObject();
                    };
                    break;
              case OP_DELETE_RANDOM:
                    for(int i = op.number; --i > 0; ) {
                         FreeObject();
                    };
                    break;
          }
     }
     DeleteHeap();
}

char TCMallocTestThread::generateString(int length) {
    int charsLength = sizeof(alphanum) - 1;
    return alphanum[rand() % charsLength];
}

void TCMallocTestThread::printStats() {
  printf("thread.object_heap_size: %lu\n", heap_.size());
}

// Allocate a new object
void TCMallocTestThread::AllocateObject() {
    Object object;
    object.size = 1042;
    void* p = tc_allocator->alloc(object.size);
    object.ptr = static_cast<char*>(p);
    assert(object.ptr != NULL);
    FillContents(&object);
    heap_.push_back(object);
    heap_size_ += object.size;
}

// Mutate a random object
void TCMallocTestThread::UpdateObject() {
    if (heap_.empty()) return;
    const int index = rand() % heap_.size();
    FillContents(&heap_[index]);
}

// Free a random object
void TCMallocTestThread::FreeObject() {
    if (heap_.empty()) return;
    const int index = rand() % heap_.size();
    Object object = heap_[index];
    free(object.ptr);
    heap_size_ -= object.size;
    heap_[index] = heap_[heap_.size()-1];
    heap_.pop_back();
}

// Delete all objects in the heap
void TCMallocTestThread::DeleteHeap() {
    while (!heap_.empty()) {
      FreeObject();
    }
}

// Free objects until our heap is small enough
void TCMallocTestThread::ShrinkHeap() {
    while (heap_size_ > FLAGS_threadmb << 20) {
      assert(!heap_.empty());
      FreeObject();
    }
}

// Fill object contents according
void TCMallocTestThread::FillContents(Object* object) {
    const char c = static_cast<char>(TCMallocTestThread::generateString(100));
    memset(object->ptr, c, object->size);
}
