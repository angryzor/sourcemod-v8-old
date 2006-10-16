#include <malloc.h>
#include <assert.h>
#include "sc.h"
#include "sctracker.h"

memuse_list_t *heapusage = NULL;
memuse_list_t *stackusage = NULL;

/**
 * Creates a new mem usage tracker entry
 */
void _push_memlist(memuse_list_t **head)
{
  memuse_list_t *newlist = (memuse_list_t *)malloc(sizeof(memuse_list_t));
  if (*head != NULL)
  {
    newlist->list_id = (*head)->list_id + 1;
  } else {
    newlist->list_id = 0;
  }
  newlist->prev = *head;
  newlist->head = NULL;
  *head = newlist;
}

/**
 * Pops a heap list but does not free it.
 */
memuse_list_t *_pop_save_memlist(memuse_list_t **head)
{
  memuse_list_t *oldlist = *head;
  *head = (*head)->prev;
  return oldlist;
}

/**
 * Marks a memory usage on a memory list
 */
int _mark_memlist(memuse_list_t *head, int type, int size)
{
  memuse_t *use;
  if (type==MEMUSE_STATIC && size==0)
  {
    return 0;
  }
  use=head->head;
  if (use && (type==MEMUSE_STATIC) 
      && (use->type == type))
  {
    use->size += size;
  } else {
    use=(memuse_t *)malloc(sizeof(memuse_t));
    use->type=type;
    use->size=size;
    use->prev=head->head;
    head->head=use;
  }
  return size;
}

void _reset_memlist(memuse_list_t **head)
{
  memuse_list_t *curlist = *head;
  memuse_list_t *tmplist;
  while (curlist) {
    memuse_t *curuse = curlist->head;
    memuse_t *tmpuse;
    while (curuse) {
      tmpuse = curuse->prev;
      free(curuse);
      curuse = tmpuse;
    }
    tmplist = curlist->prev;
    free(curlist);
    curlist = tmplist;
  }
  *head = NULL;
}


/**
 * Wrapper for pushing the heap list
 */
void pushheaplist()
{
  _push_memlist(&heapusage);
}

/**
 * Wrapper for popping and saving the heap list
 */
memuse_list_t *popsaveheaplist()
{
  return _pop_save_memlist(&heapusage);
}

/**
 * Wrapper for marking the heap
 */
int markheap(int type, int size)
{
  return _mark_memlist(heapusage, type, size);
}

/**
 * Wrapper for pushing the stack list
 */
void pushstacklist()
{
  _push_memlist(&stackusage);
}

/**
 * Wrapper for marking the stack
 */
int markstack(int type, int size)
{
  return _mark_memlist(stackusage, type, size);
}

/**
 * Generates code to free all heap allocations on a tracker
 */
void _heap_freeusage(memuse_list_t *heap, int dofree)
{
  memuse_t *cur=heap->head;
  memuse_t *tmp;
  while (cur)
  {
    if (cur->type == MEMUSE_STATIC)
	{
      modheap((-1)*cur->size*sizeof(cell));
    } else {
      modheap_i();
    }
    if (dofree)
    {
      tmp=cur->prev;
      free(cur);
      cur=tmp;
    } else {
      cur=cur->prev;
    }
  }
  if (dofree)
  {
    heap->head=NULL;
  }
}

void _stack_genusage(memuse_list_t *stack, int dofree)
{
  memuse_t *cur = stack->head;
  memuse_t *tmp;
  while (cur)
  {
    if (cur->type == MEMUSE_DYNAMIC)
	{
      /* no idea yet */
      assert(0);
	} else {
      modstk(cur->size * sizeof(cell));
	}
	if (dofree)
	{
      tmp = cur->prev;
      free(cur);
      cur = tmp;
	} else {
      cur = cur->prev;
	}
  }
  if (dofree)
  {
    stack->head = NULL;
  }
}

/**
 * Pops a heap list and frees it.
 */
void popheaplist()
{
  memuse_list_t *oldlist;
  assert(heapusage!=NULL);

  _heap_freeusage(heapusage, 1);
  assert(heapusage->head==NULL);

  oldlist=heapusage->prev;
  free(heapusage);
  heapusage=oldlist;
}

void genstackfree(int stop_id)
{
  memuse_list_t *curlist = stackusage;
  while (curlist && curlist->list_id > stop_id)
  {
    _stack_genusage(curlist, 0);
    curlist = curlist->prev;
  }
}

void genheapfree(int stop_id)
{
  memuse_list_t *curlist = heapusage;
  while (curlist && curlist->list_id > stop_id)
  {
    _heap_freeusage(curlist, 0);
    curlist = curlist->prev;
  }
}

void popstacklist()
{
  memuse_list_t *oldlist;
  assert(stackusage != NULL);

  _stack_genusage(stackusage, 1);
  assert(stackusage->head==NULL);

  oldlist = stackusage->prev;
  free(stackusage);
  stackusage = oldlist;
}

void resetstacklist()
{
  _reset_memlist(&stackusage);
}

void resetheaplist()
{
  _reset_memlist(&heapusage);
}
