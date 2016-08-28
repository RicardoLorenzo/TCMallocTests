#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <ncurses.h>

#include <thread>
#include <queue>
#include <mutex>
#include <sstream>
#include <fstream>

#include "gperftools/malloc_hook.h"
#include "gperftools/malloc_extension.h"
#include "gperftools/tcmalloc.h"

#if defined(__linux)
  static bool kOSSupportsMemalign = true;
#else
  static bool kOSSupportsMemalign = false;
#endif

static const int FLAGS_numthreads = 2;
static const int FLAGS_threadmb = 4;
static const int FLAGS_lg_max_memalign = 18; // lg of max alignment for memalign
static const double FLAGS_memalign_min_fraction = 0;    // min expected%
static const double FLAGS_memalign_max_fraction = 0.4;  // max expected%
static const double FLAGS_memalign_max_alignment_ratio = 6;

static std::ofstream log_file(
        "/tmp/debug.log", std::ios_base::out | std::ios_base::app);

class TCMallocTestMenu {
   private:
       WINDOW *win_stats;
       WINDOW *win_menu;

       void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string); 

   public:
       static const int MENU_LOAD_OBJECTS = 0;
       static const int MENU_UPDATE_OBJECTS = 1;
       static const int MENU_DELETE_OBJECTS = 2;
       static const int MENU_QUIT = 3;

       TCMallocTestMenu() {
           initscr();
           cbreak();
           noecho();
           curs_set(0);
       }

       ~TCMallocTestMenu() {
           endwin();
       }

       void printStats(char* message);
       int printMenu();
       void refreshScreen();
};


class TCMallocAllocator {
 private:
    MallocExtension* extension;
    double memalign_fraction_;
    std::ostringstream output_buffer;

    int Uniform(int n);
 public:
   explicit TCMallocAllocator() : memalign_fraction_(0) {
     extension = MallocExtension::instance();
     if (kOSSupportsMemalign) {
       double delta = FLAGS_memalign_max_fraction - FLAGS_memalign_min_fraction;
       memalign_fraction_ = (Uniform(10000)/10000.0 * delta +
                          FLAGS_memalign_min_fraction);
       //printf("memalign fraction: %f\n", memalign_fraction_);
     }
   }

   virtual ~TCMallocAllocator() {}

   void* alloc(size_t size);
   char* getStats();
   char* getVersion();
};

class TCMallocTestThread {
 private:
   struct Object {
      char*       ptr;                 // Allocated pointer
      int         size;                // Allocated size
   };
   struct ThreadOperation {
      int         type;                // Allocated pointer
      int       number;                // Allocated size
   };
   int id_;
   TCMallocAllocator* tc_allocator;
   std::queue<ThreadOperation> threadOperations;
   std::vector<Object> heap_;
   size_t heap_size_;

   void AllocateObject();
   void UpdateObject();
   void FreeObject();
   void DeleteHeap();
   void ShrinkHeap();
   void FillContents(Object* object);

 public:
   static const int OP_ALLOCATE = 1;
   static const int OP_UPDATE_RANDOM = 2;
   static const int OP_DELETE_RANDOM = 3;
   static const int OP_SHUTDOWN = 4;
   TCMallocTestThread(int id)
     : id_(id),
       heap_size_(0) {
     // initialize random seed: 
     srand(time(NULL));
     tc_allocator = new TCMallocAllocator;
   }

   virtual ~TCMallocTestThread() {
     printf("Thread %2d ended\n", id_);
   }

   void run();
   void runOperation(int type, int numberOfObjects);
   char generateString(int length);
   void printStats();
};

class TCMallocTest {
 private:
   std::mutex mtx_lock;
   std::vector<TCMallocTestThread*> threadObjects;
   std::thread threads[FLAGS_numthreads];
   TCMallocAllocator* tc_allocator;
   TCMallocTestMenu* menu;

 public:
   TCMallocTest() {
     tc_allocator = new TCMallocAllocator;
     menu = new TCMallocTestMenu;
   }
   ~TCMallocTest() {}

   void showMenu();
   void startAllThreads();
   void stopAllThreads();
   void executeOperationOnAllThreads(int type, int number);
};
