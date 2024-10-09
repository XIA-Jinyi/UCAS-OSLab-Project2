/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Copyright (C) 2018 Institute of Computing
 * Technology, CAS Author : Han Shukai (email :
 * hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Changelog: 2019-8 Reimplement queue.h.
 * Provide Linux-style doube-linked list instead of original
 * unextendable Queue implementation. Luming
 * Wang(wangluming@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * */

#ifndef INCLUDE_LIST_H_
#define INCLUDE_LIST_H_

#include <type.h>

// double-linked list
typedef struct list_node
{
    struct list_node *next, *prev;
} list_node_t;

typedef list_node_t list_head;

// LIST_HEAD is used to define the head of a list.
#define LIST_HEAD(name) struct list_node name = {&(name), &(name)}

/* TODO: [p2-task1] implement your own list API */

#define offsetof(t, d) __builtin_offsetof(t, d)

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member)                                          \
  ((type *)((char *)(ptr)-offsetof(type, member)))

/**
 * init_list_head - initialize a new list head
 * @ptr: the list head to be added.
 */
void init_list_head(list_head *ptr);

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 */
void list_add(list_head *new, list_head *head);

/**
 * list_add_tail - add a new entry at the end
 * @new: new entry to be added
 * @head: list head to add it before
 */
void list_add_tail(list_head *new, list_head *head);

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 */
void list_del(list_head *entry);

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
void list_del_init(list_head *entry);

/**
 * list_replace - replace old entry by new one
 * @old:  the element to be replaced
 * @new:  the new element to insert
 */
void list_replace(list_head *old, list_head *new);

/**
 * list_replace_init - replace old entry by new one and initialize it
 * @old:  the element to be replaced
 * @new:  the new element to insert
 */
void list_replace_init(list_head *old, list_head *new);

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
void list_move(list_head *list, list_head *head);

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
void list_move_tail(list_head *list, list_head *head);

/**
 * list_empty - tests whether a list is empty
 * @list: the list to test.
 */
#define list_empty(list) ((list)->next == (list))

/**
 * list_is_singular - tests whether a list has just one entry.
 * @list: the list to test.
 */
#define list_is_singular(list)                                                 \
  ((list)->next != (list) && (list)->next == (list)->prev)

/**
 * list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_head within the struct.
 */
#define list_first_entry(ptr, type, member)                                    \
  list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_head within the struct.
 */
#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)

/**
 * list_next_entry - get the next element in list
 * @pos:    the type * to cursor
 * @member: the name of the list_head within the struct.
 */
#define list_next_entry(pos, member)                                           \
  list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * list_prev_entry - get the prev element in list
 * @pos:    the type * to cursor
 * @member: the name of the list_head within the struct.
 */
#define list_prev_entry(pos, member)                                           \
  list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_for_each - iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each(pos, head)                                               \
  for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_safe(pos, n, head)                                       \
  for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * list_for_each_prev - iterate over a list backwards
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each_prev(pos, head)                                          \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head)                                  \
  for (pos = (head)->prev, n = pos->prev; pos != (head); pos = n, n = pos->prev)

#endif
