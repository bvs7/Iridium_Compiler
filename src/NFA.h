#ifndef NFA_H
#define NFA_H

#define MATCH(pattern) NFA_matchSym(pattern)
#define REP(nfa) NFA_repetition(nfa)
#define ADD(nfa) NFA_addition(nfa)
#define OPT(nfa) NFA_option(nfa)
#define CON(nfa) NFA_concatenation(nfa)
#define ALT(nfa) NFA_alternation(nfa)
#define FIN(nfa) NFA_finish(nfa)

typedef struct NFA NFA;
typedef struct NFA_State NFA_State;

typedef struct Node Node;
typedef struct Path Path;
typedef struct PathList PathList;

typedef struct NFA{
  Node *start;
  PathList *ends;
}NFA;

typedef struct NFA_State{
  NFA nfa;
  Node **nodes;
  int nodes_len;
  int total_nodes;
  unsigned long *flags;
  int flags_len;
}NFA_State;

NFA NFA_matchSym(char sym[]);
NFA NFA_repetition(NFA nfa);
NFA NFA_addition(NFA nfa);
NFA NFA_option(NFA nfa);
NFA NFA_concatenation(NFA nfa, NFA concat);
NFA NFA_alternation(NFA nfa, NFA alt);
NFA_State NFA_finish(NFA nfa);
void step(NFA_State *s, char c);
int accept(NFA_State *s);
int dead(NFA_State *s);
void reset(NFA_State *s);
void freeNFA_State(NFA_State *s);

#endif