#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SYM_LEN 40
#define BITS (sizeof(unsigned long)*8)
#define NODES_LEN_INIT 10

typedef struct NFA NFA;
typedef struct Node Node;
typedef struct Path Path;
typedef struct PathList PathList;
typedef struct NFA_State NFA_State;

typedef struct NFA{
  Node *start;
  PathList *ends;
}NFA;

typedef struct Path{
  Node *dest;
  char sym[SYM_LEN];
}Path;

typedef struct Node{
  unsigned long id;
  Path p1;
  Path p2;
}Node;

typedef struct PathList{
  Path *p;
  PathList *next;
}PathList;

typedef struct NFA_State{
  NFA nfa;
  Node **nodes;
  size_t nodes_len;
  int total_nodes;
  unsigned long *flags;
}NFA_State;

void NFA_appendNode(NFA *nfa, Node *n);
NFA NFA_matchSym(char sym[]);
NFA NFA_repetition(NFA nfa);
NFA NFA_addition(NFA nfa);
NFA NFA_option(NFA nfa);
NFA NFA_concatenation(NFA nfa, NFA concat);
NFA NFA_alternation(NFA nfa, NFA alt);
void concatPathList(PathList *pl1, PathList *pl2);
void freeNFA(NFA nfa);
void freeNode(Node *n);
NFA_State NFA_finish(NFA nfa);
void explore(NFA_State *s, Node *n);
void setFlag(unsigned long *flags, unsigned long i);
unsigned long getFlag(unsigned long *flags, unsigned long i);
void closure(NFA_State *s);
void close(NFA_State *s, Node *n);
void step(NFA_State *s, char c);
int accept(NFA_State *s);
int dead(NFA_State *s);
int matchPath(Path *p, char c);

void NFA_appendNode(NFA *nfa, Node *n){
  PathList *pl = nfa->ends;
  while(pl){
    pl->p->dest = n;
    PathList *temp = pl->next;
    free(pl);
    pl = temp;
  }
}

NFA NFA_matchSym(char sym[]){
  int symlen = strlen(sym);
  Node *start = calloc(sizeof(Node), 1);
  memcpy(start->p1.sym, sym, symlen);
  NFA nfa;
  nfa.ends = calloc(sizeof(PathList),1);
  nfa.ends->p = &(start->p1);
  nfa.start = start;
  return nfa;
}

NFA NFA_repetition(NFA nfa){
  Node *start = calloc(sizeof(Node), 1);
  NFA_appendNode(&nfa, start);
  start->p2.dest = nfa.start;
  nfa.start = start;
  nfa.ends = calloc(sizeof(PathList),1);
  nfa.ends->p = &(start->p1);
  return nfa;
}

NFA NFA_addition(NFA nfa){
  Node *mid = calloc(sizeof(Node), 1);
  NFA_appendNode(&nfa, mid);
  mid->p2.dest = nfa.start;
  nfa.ends = calloc(sizeof(PathList),1);
  nfa.ends->p = &(mid->p1);
  return nfa;
}

NFA NFA_option(NFA nfa){
  Node *start = calloc(sizeof(Node),1);
  PathList *plst = calloc(sizeof(PathList),1);
  plst->p = &(start->p1);
  concatPathList(nfa.ends, plst);
  start->p2.dest = nfa.start;
  return nfa;
}

NFA NFA_concatenation(NFA nfa, NFA concat){
  NFA_appendNode(&nfa, concat.start);
  nfa.ends = concat.ends;
  return nfa;
}

NFA NFA_alternation(NFA nfa, NFA alt){
  Node *start = calloc(sizeof(Node),1);
  start->p1.dest = nfa.start;
  start->p2.dest = alt.start;
  concatPathList(nfa.ends, alt.ends);
  nfa.start = start;
  return nfa;
}

void concatPathList(PathList *pl1, PathList *pl2){
  PathList **pl = &pl1;
  while(!(*pl)){
    pl = &((*pl)->next);
  }
  *pl = pl2;
}

void freeNFA(NFA nfa){
  freeNode(nfa.start);
  PathList *plst = nfa.ends;
  while(plst){
    PathList *temp = plst->next;
    free(plst);
    plst=temp;
  }
}

void freeNode(Node *n){
  if(n->p1.dest){
    freeNode(n->p1.dest);
  } if(n->p2.dest){
    freeNode(n->p2.dest);
  }
  free(n);
}

NFA_State NFA_finish(NFA nfa){
  NFA_State s;
  s.nfa = nfa;
  s.nodes_len = NODES_LEN_INIT;
  s.nodes = calloc(sizeof(Node*), s.nodes_len);
  s.total_nodes = 0;
  explore(&s, s.nfa.start);
  Node *accept = calloc(sizeof(Node),1);
  NFA_appendNode(&s.nfa, accept);
  accept->id = 0;
  s.nodes[0] = accept;
  s.total_nodes++;

  s.flags = calloc(sizeof(unsigned long), (s.total_nodes/BITS)+1);
  setFlag(s.flags, s.nfa.start->id);
  closure(&s);
  return s;
}

void explore(NFA_State *s, Node *n){
  if(n->id <= 0){
    n->id = ++s->total_nodes;
    if(n->id > s->nodes_len){
      s->nodes_len += 2;
      s->nodes = realloc(s->nodes, s->nodes_len*sizeof(Node*));
    }
    s->nodes[n->id] = n;
    if(n->p1.dest){
      explore(s, n->p1.dest);
    }if(n->p2.dest){
      explore(s, n->p2.dest);
    }
  }
}

void setFlag(unsigned long *flags, unsigned long i){
  unsigned long index = i/BITS;
  unsigned long mask = 1<<(i%BITS);
  flags[index] |= mask;
}

unsigned long getFlag(unsigned long *flags, unsigned long i){
  unsigned long index = i/BITS;
  unsigned long mask = 1<<(i%BITS);
  return flags[index] &mask;
}

void closure(NFA_State *s){
  unsigned long index;
  for(index=0; index< (s->total_nodes/BITS)+1; index++){
    if(s->flags[index]){
      int bit;
      for(bit=0; bit < BITS; bit++){
        unsigned long mask = 1<<bit;
        if(s->flags[index] &mask){
          Node *n = s->nodes[index*BITS+bit];
          close(s,n);
        }
      }
    }
  }
}

void close(NFA_State *s, Node *n){
  setFlag(s->flags, n->id);
  Node *next;
  if(!(n->p1.sym[0]) && (next=n->p1.dest) && !getFlag(s->flags, next->id)){
    close(s,next);
  }if(!(n->p2.sym[0]) && (next=n->p2.dest) && !getFlag(s->flags, next->id)){
    close(s,next);
  }
}

void step(NFA_State *s, char c){
  unsigned long *nextflags = calloc(sizeof(long), (s->total_nodes/BITS)+1);
  unsigned long n_id;
  for(n_id=0; n_id < s->total_nodes; n_id++){
    if(getFlag(s->flags, n_id)){
      Node *n = s->nodes[n_id];
      if(n->p1.dest && matchPath(&(n->p1), c)){
        setFlag(nextflags, n->p1.dest->id);
      } if(n->p2.dest && matchPath(&n->p2, c)){
        setFlag(nextflags, n->p2.dest->id);
      }
    }
  }
  free(s->flags);
  s->flags = nextflags;
  closure(s);
}

int accept(NFA_State *s){
  return getFlag(s->flags, 0);
}

int dead(NFA_State *s){
  int i;
  for(i=0; i < (s->total_nodes/BITS)+1; i++){
    if(s->flags != 0){
      return 0;
    }
  }
  return 1;
}

int matchPath(Path *p, char c){
  char start,test;
  int i;
  for(i=0; (test=p->sym[i]) != '\0'; i++){
    if(test=='-'){
      test=p->sym[++i];
      if(test != '\0' && c>=start && c<=test){
        return 1;
      }
      continue;
    }
    start = test;
    if(c == test){
      return 1;
    }
  }
  return 0;
}
