//
// Created by Zixiong Liu on 12/20/16.
//

#include "../lib/printf.h"
#include "../thread.h"
#include <vector>


#define SYN_MALLOC_MAX 100
int syn_malloc () {
  //thread_current = nullptr;
  printf("starting...\n");

  std::vector<int> test_vec;
  for (int i = 0; i < SYN_MALLOC_MAX; i++) {
    test_vec.push_back(i);
  }
  for (int i = 0; i < SYN_MALLOC_MAX; i++) {
   printf("NUM = %d\n", test_vec[i]);
  }
  test_vec.clear();

/*
  int test[1000];
  for (int i = 0; i < 1000; i++)
    test[i] = i;
  for (int i = 0; i < 1000; i++) {
    printf("NUM = %d\n", test[i]);
  }
  //delete[] test;
  */
  printf("ended...\n");
}

void do_multi_syn_malloc (int n) {

  for (int i = 0; i < n; i++) {
    thread_current->thread_sleep(1000);
    thread* t = new thread((void*)&syn_malloc);
    //thread_current->thread_sleep(100);
  }
  while (true) {
    asm ("hlt");
  }
}