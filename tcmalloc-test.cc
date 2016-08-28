#include "tcmalloc-test.h"

#include <math.h>

#include <string>
#include <iostream>

static const int MB = 1048576;

void TCMallocTest::showMenu() {
   int option;
   menu->printStats(tc_allocator->getStats());
   while((option = menu->printMenu()) != TCMallocTestMenu::MENU_QUIT) {
       switch(option) {
           case TCMallocTestMenu::MENU_LOAD_OBJECTS:
                executeOperationOnAllThreads(TCMallocTestThread::OP_ALLOCATE, 50);
                break;
           case TCMallocTestMenu::MENU_UPDATE_OBJECTS:
                executeOperationOnAllThreads(TCMallocTestThread::OP_UPDATE_RANDOM, 50);
                break;
           case TCMallocTestMenu::MENU_DELETE_OBJECTS:
                executeOperationOnAllThreads(TCMallocTestThread::OP_DELETE_RANDOM, 50);
                break;
       }
       menu->refreshScreen();
       menu->printStats(tc_allocator->getStats());
   }
}

void TCMallocTest::startAllThreads() {
  // acquiring a lock to prevent running threads from another instance
  mtx_lock.lock();

  for(int t = 0; t < FLAGS_numthreads; t++) {
        TCMallocTestThread* testThread = new TCMallocTestThread(t);
        threadObjects.push_back(testThread);
        threads[t] = std::thread([testThread]() {
              testThread->run();
        });
  }

  for(int t = 0; t < FLAGS_numthreads; t++) {
    //pthread_join(&threads[i], NULL); 
    threads[t].detach();
  }

  mtx_lock.unlock();
}

void TCMallocTest::executeOperationOnAllThreads(int type, int number) {
  if(number > 0) {
     int numberPerThread = (int) ceil((double) number / (double) threadObjects.size());
     for(auto& testThread : threadObjects) {
         testThread->runOperation(type, numberPerThread);
     }
  }
}

void TCMallocTest::stopAllThreads() {
  executeOperationOnAllThreads(TCMallocTestThread::OP_SHUTDOWN, 1);
  for (int t = 0; t < FLAGS_numthreads; t++) { 
    delete &threads[t];  // Cleanup
    //pthread_exit(NULL);
  }
}

int main(int argc, char** argv) {
  TCMallocTest *test = new TCMallocTest();
  test->startAllThreads();
  test->showMenu();
  test->stopAllThreads();
}
