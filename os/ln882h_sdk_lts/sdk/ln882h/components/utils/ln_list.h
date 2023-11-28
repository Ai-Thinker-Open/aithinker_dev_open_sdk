#ifndef __LN_LIST_H__
#define __LN_LIST_H__

#include "ln_types.h"
#include "ln_compiler.h"

typedef struct ln_list_s {
    struct ln_list_s *next;
    struct ln_list_s *prev;
} ln_list_t;

#define LN_LIST_ENTRY(node, type, field)  \
        ((type *)((uint8_t *)(node) - (uint32_t)(&(((type *)0)->field))))

#define LN_LIST_FOR_EACH_ENTRY(entry, type, field, list) \
    for (entry = LN_LIST_ENTRY((list)->next, type, field); \
        &entry->field != (list); \
        entry = LN_LIST_ENTRY(entry->field.next, type, field))

#define LN_LIST_FOR_EACH_ENTRY_REVERSE(entry, type, field, list) \
    for (entry = LN_LIST_ENTRY((list)->prev, type, field); \
            &entry->field != (list); \
            entry = LN_LIST_ENTRY(entry->field.prev, type, field))

#define LN_LIST_FOR_EACH_ENTRY_SAFE(entry, tmp, type, field, list) \
    for (entry = LN_LIST_ENTRY((list)->next, type, field), \
            tmp = LN_LIST_ENTRY(entry->field.next, type, field); \
            &entry->field != (list); \
            entry = tmp, tmp = LN_LIST_ENTRY(entry->field.next, type, field))

#define LN_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(entry, tmp, type, field, list) \
    for (entry = LN_LIST_ENTRY((list)->prev, type, field), \
            tmp = LN_LIST_ENTRY(entry->field.prev, type, field); \
            &entry->field != (list); \
            entry = tmp, tmp = LN_LIST_ENTRY(entry->field.prev, type, field))

#define ln_list_entry(node, type, member) ((type *)((uint8_t *)(node) - (size_t)(&((type *)0)->member)))

__STATIC_INLINE__ void ln_list_init(ln_list_t *list_head)
{
    list_head->next = list_head;
    list_head->prev = list_head;
}

__STATIC_INLINE__ int ln_is_list_empty(ln_list_t *list)
{
    return (list->next == list);
}

__STATIC_INLINE__ void ln_list_insert(ln_list_t *head, ln_list_t *element)
{
    element->prev = head->prev;
    element->next = head;

    head->prev->next = element;
    head->prev       = element;
}

__STATIC_INLINE__ void ln_list_add(ln_list_t *head, ln_list_t *element)
{
    element->prev = head;
    element->next = head->next;

    head->next->prev = element;
    head->next = element;
}

__STATIC_INLINE__ void ln_list_rm(ln_list_t *element)
{
    element->prev->next = element->next;
    element->next->prev = element->prev;
}

__STATIC_INLINE__ void ln_list_rm_init(ln_list_t *element)
{
    element->prev->next = element->next;
    element->next->prev = element->prev;

    element->next = element->prev = element;
}

#endif /* __LN_LIST_H__ */

