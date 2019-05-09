#include "NFA.h"

#include <assert.h>
#include <stdio.h>

void test_basic(){
  NFA_State s = FIN(MATCH("a-z"));

  assert(!accept(&s));
  assert(!dead(&s));

  step(&s, 'a');
  assert(accept(&s));
  assert(!dead(&s));

  reset(&s);
  assert(!accept(&s));
  assert(!dead(&s));

  step(&s, 'b');
  assert(accept(&s));
  assert(!dead(&s));

  step(&s, 'c');
  assert(!accept(&s));
  assert(dead(&s));

  reset(&s);

  step(&s, '0');
  assert(!accept(&s));
  assert(dead(&s));

  freeNFA_State(&s);
}

char test_match(){

  char c;
  for(c='a'; c<='z'; c++){
    NFA_State s = FIN(MATCH("a-z"));

    assert(!accept(&s));
    assert(!dead(&s));
    step(&s, c);
    assert(accept(&s));
    assert(!dead(&s));
    step(&s, c);
    assert(!accept(&s));
    assert(dead(&s));
  }
}

int main(int argc, char *argv[]){
  test_basic();
  printf("Passed all tests!\n");
}