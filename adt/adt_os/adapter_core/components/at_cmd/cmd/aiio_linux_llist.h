#ifndef __AIIO_LINUX_LIST_H__
#define __AIIO_LINUX_LIST_H__
#include<stdio.h>

/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define AI_LIST_POISON1  ((void *) 0x00)
#define AI_LIST_POISON2  ((void *) 0x00)

#define AI_prefetch(x) ((void*)(x))

#define AI_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define AI_container_of(ptr, type, member) ({                  \
        const typeof(((type*)0)->member)* __mptr = (ptr);   \
        (type*)((char*)__mptr - AI_offsetof(type, member)); })
/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct AI_list_head {
    struct AI_list_head *next, *prev;
};

#define AI_LIST_HEAD_INIT(name) { &(name), &(name) }

#define AI_LIST_HEAD(name) \
    struct AI_list_head name = AI_LIST_HEAD_INIT(name)

static inline void AI_INIT_LIST_HEAD(struct AI_list_head *list)
{
    list->next = list;
    list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
#ifndef CONFIG_DEBUG_LIST
static inline void __AI_list_add(struct AI_list_head *new,
                  struct AI_list_head *prev,
                  struct AI_list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}
#else
extern void __AI_list_add(struct AI_list_head *new,
                  struct AI_list_head *prev,
                  struct AI_list_head *next);
#endif

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void AI_list_add(struct AI_list_head *new, struct AI_list_head *head)
{
    __AI_list_add(new, head, head->next);
}


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void AI_list_add_tail(struct AI_list_head *new, struct AI_list_head *head)
{
    __AI_list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __AI_list_del(struct AI_list_head * prev, struct AI_list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
#ifndef CONFIG_DEBUG_LIST
static inline void AI_list_del(struct AI_list_head *entry)
{
    __AI_list_del(entry->prev, entry->next);
    entry->next = AI_LIST_POISON1;
    entry->prev = AI_LIST_POISON2;
}
#else
extern void AI_list_del(struct AI_list_head *entry);
#endif

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void AI_list_replace(struct AI_list_head *old,
                struct AI_list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

static inline void AI_list_replace_init(struct AI_list_head *old,
                    struct AI_list_head *new)
{
    AI_list_replace(old, new);
    AI_INIT_LIST_HEAD(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void AI_list_del_init(struct AI_list_head *entry)
{
    __AI_list_del(entry->prev, entry->next);
    AI_INIT_LIST_HEAD(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void AI_list_move(struct AI_list_head *list, struct AI_list_head *head)
{
    __AI_list_del(list->prev, list->next);
    AI_list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void AI_list_move_tail(struct AI_list_head *list,
                  struct AI_list_head *head)
{
    __AI_list_del(list->prev, list->next);
    AI_list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int AI_list_is_last(const struct AI_list_head *list,
                const struct AI_list_head *head)
{
    return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int AI_list_empty(const struct AI_list_head *head)
{
    return head->next == head;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static inline int AI_list_empty_careful(const struct AI_list_head *head)
{
    struct AI_list_head *next = head->next;
    return (next == head) && (next == head->prev);
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int AI_list_is_singular(const struct AI_list_head *head)
{
    return !AI_list_empty(head) && (head->next == head->prev);
}

static inline void __AI_list_cut_position(struct AI_list_head *list,
        struct AI_list_head *head, struct AI_list_head *entry)
{
    struct AI_list_head *new_first = entry->next;
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry;
    entry->next = list;
    head->next = new_first;
    new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *  and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void AI_list_cut_position(struct AI_list_head *list,
        struct AI_list_head *head, struct AI_list_head *entry)
{
    if (AI_list_empty(head))
        return;
    if (AI_list_is_singular(head) &&
        (head->next != entry && head != entry))
        return;
    if (entry == head)
        AI_INIT_LIST_HEAD(list);
    else
        __AI_list_cut_position(list, head, entry);
}

static inline void __AI_list_splice(const struct AI_list_head *list,
                 struct AI_list_head *prev,
                 struct AI_list_head *next)
{
    struct AI_list_head *first = list->next;
    struct AI_list_head *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void AI_list_splice(const struct AI_list_head *list,
                struct AI_list_head *head)
{
    if (!AI_list_empty(list))
        __AI_list_splice(list, head, head->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void AI_list_splice_tail(struct AI_list_head *list,
                struct AI_list_head *head)
{
    if (!AI_list_empty(list))
        __AI_list_splice(list, head->prev, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void AI_list_splice_init(struct AI_list_head *list,
                    struct AI_list_head *head)
{
    if (!AI_list_empty(list)) {
        __AI_list_splice(list, head, head->next);
        AI_INIT_LIST_HEAD(list);
    }
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void AI_list_splice_tail_init(struct AI_list_head *list,
                     struct AI_list_head *head)
{
    if (!AI_list_empty(list)) {
        __AI_list_splice(list, head->prev, head);
        AI_INIT_LIST_HEAD(list);
    }
}

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define AI_list_entry(ptr, type, member) \
    AI_container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define AI_list_first_entry(ptr, type, member) \
    AI_list_entry((ptr)->next, type, member)

/**
 * list_for_each    -   iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 *
 * AI_prefetch(pos->next)部分单片机不支持该语法，此处注释掉了，不影响功能
 */
#define AI_list_for_each(pos, head) \
    for (pos = (head)->next; /*AI_prefetch(pos->next), */pos != (head); \
            pos = pos->next)

/**
 * __list_for_each  -   iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __AI_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev   -   iterate over a list backwards
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define AI_list_for_each_prev(pos, head) \
    for (pos = (head)->prev; AI_prefetch(pos->prev), pos != (head); \
            pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define AI_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define AI_list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
         AI_prefetch(pos->prev), pos != (head); \
         pos = n, n = pos->prev)

/**
 * list_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define AI_list_for_each_entry(pos, head, member)               \
    for (pos = AI_list_entry((head)->next, typeof(*pos), member);   \
         AI_prefetch(pos->member.next), &pos->member != (head);     \
         pos = AI_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define AI_list_for_each_entry_reverse(pos, head, member)           \
    for (pos = AI_list_entry((head)->prev, typeof(*pos), member);   \
         AI_prefetch(pos->member.prev), &pos->member != (head);     \
         pos = AI_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:    the type * to use as a start point
 * @head:   the head of the list
 * @member: the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define AI_list_prepare_entry(pos, head, member) \
    ((pos) ? : AI_list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define AI_list_for_each_entry_continue(pos, head, member)      \
    for (pos = AI_list_entry(pos->member.next, typeof(*pos), member);   \
         AI_prefetch(pos->member.next), &pos->member != (head); \
         pos = AI_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define AI_list_for_each_entry_continue_reverse(pos, head, member)      \
    for (pos = AI_list_entry(pos->member.prev, typeof(*pos), member);   \
         AI_prefetch(pos->member.prev), &pos->member != (head); \
         pos = AI_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define AI_list_for_each_entry_from(pos, head, member)          \
    for (; AI_prefetch(pos->member.next), &pos->member != (head);   \
         pos = AI_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define AI_list_for_each_entry_safe(pos, n, head, member)           \
    for (pos = AI_list_entry((head)->next, typeof(*pos), member),   \
        n = AI_list_entry(pos->member.next, typeof(*pos), member);  \
         &pos->member != (head);                    \
         pos = n, n = AI_list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define AI_list_for_each_entry_safe_continue(pos, n, head, member)      \
    for (pos = AI_list_entry(pos->member.next, typeof(*pos), member),       \
        n = AI_list_entry(pos->member.next, typeof(*pos), member);      \
         &pos->member != (head);                        \
         pos = n, n = AI_list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_from
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define AI_list_for_each_entry_safe_from(pos, n, head, member)          \
    for (n = AI_list_entry(pos->member.next, typeof(*pos), member);     \
         &pos->member != (head);                        \
         pos = n, n = AI_list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define AI_list_for_each_entry_safe_reverse(pos, n, head, member)       \
    for (pos = AI_list_entry((head)->prev, typeof(*pos), member),   \
        n = AI_list_entry(pos->member.prev, typeof(*pos), member);  \
         &pos->member != (head);                    \
         pos = n, n = AI_list_entry(n->member.prev, typeof(*n), member))

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

struct AI_hlist_head {
    struct AI_hlist_node *first;
};

struct AI_hlist_node {
    struct AI_hlist_node *next, **pprev;
};

#define AI_HLIST_HEAD_INIT { .first = NULL }
#define AI_HLIST_HEAD(name) struct AI_hlist_head name = {  .first = NULL }
#define AI_INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void AI_INIT_HLIST_NODE(struct AI_hlist_node *h)
{
    h->next = NULL;
    h->pprev = NULL;
}

static inline int AI_hlist_unhashed(const struct AI_hlist_node *h)
{
    return !h->pprev;
}

static inline int AI_hlist_empty(const struct AI_hlist_head *h)
{
    return !h->first;
}

static inline void __AI_hlist_del(struct AI_hlist_node *n)
{
    struct AI_hlist_node *next = n->next;
    struct AI_hlist_node **pprev = n->pprev;
    *pprev = next;
    if (next)
        next->pprev = pprev;
}

static inline void AI_hlist_del(struct AI_hlist_node *n)
{
    __AI_hlist_del(n);
    n->next = AI_LIST_POISON1;
    n->pprev = AI_LIST_POISON2;
}

static inline void AI_hlist_del_init(struct AI_hlist_node *n)
{
    if (!AI_hlist_unhashed(n)) {
        __AI_hlist_del(n);
        AI_INIT_HLIST_NODE(n);
    }
}

static inline void AI_hlist_add_head(struct AI_hlist_node *n, struct AI_hlist_head *h)
{
    struct AI_hlist_node *first = h->first;
    n->next = first;
    if (first)
        first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

/* next must be != NULL */
static inline void AI_hlist_add_before(struct AI_hlist_node *n,
                    struct AI_hlist_node *next)
{
    n->pprev = next->pprev;
    n->next = next;
    next->pprev = &n->next;
    *(n->pprev) = n;
}

static inline void AI_hlist_add_after(struct AI_hlist_node *n,
                    struct AI_hlist_node *next)
{
    next->next = n->next;
    n->next = next;
    next->pprev = &n->next;

    if(next->next)
        next->next->pprev  = &next->next;
}

/*
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void AI_hlist_move_list(struct AI_hlist_head *old,
                   struct AI_hlist_head *new)
{
    new->first = old->first;
    if (new->first)
        new->first->pprev = &new->first;
    old->first = NULL;
}

#define AI_hlist_entry(ptr, type, member) AI_container_of(ptr,type,member)

#define AI_hlist_for_each(pos, head) \
    for (pos = (head)->first; pos && ({ AI_prefetch(pos->next); 1; }); \
         pos = pos->next)

#define AI_hlist_for_each_safe(pos, n, head) \
    for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
         pos = n)

/**
 * hlist_for_each_entry - iterate over list of given type
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
 */
#define AI_hlist_for_each_entry(tpos, pos, head, member)             \
    for (pos = (head)->first;                    \
         pos && ({ AI_prefetch(pos->next); 1;}) &&           \
        ({ tpos = AI_hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = pos->next)

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @member: the name of the hlist_node within the struct.
 */
#define AI_hlist_for_each_entry_continue(tpos, pos, member)      \
    for (pos = (pos)->next;                      \
         pos && ({ AI_prefetch(pos->next); 1;}) &&           \
        ({ tpos = AI_hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = pos->next)

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @member: the name of the hlist_node within the struct.
 */
#define AI_hlist_for_each_entry_from(tpos, pos, member)          \
    for (; pos && ({ AI_prefetch(pos->next); 1;}) &&             \
        ({ tpos = AI_hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = pos->next)

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @n:      another &struct hlist_node to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
 */
#define AI_hlist_for_each_entry_safe(tpos, pos, n, head, member)         \
    for (pos = (head)->first;                    \
         pos && ({ n = pos->next; 1; }) &&               \
        ({ tpos = AI_hlist_entry(pos, typeof(*tpos), member); 1;}); \
         pos = n)

#endif  //end of __AI_LINUX_LIST_H__
