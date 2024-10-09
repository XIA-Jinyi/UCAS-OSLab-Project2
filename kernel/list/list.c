#include <os/list.h>

void init_list_head(list_head *ptr) {
  ptr->next = ptr;
  ptr->prev = ptr;
}
 
void list_add(list_head *new, list_head *head) {
  new->next = head->next;
  new->prev = head;
  head->next->prev = new;
  head->next = new;
}
 
void list_add_tail(list_head *new, list_head *head) {
  new->next = head;
  new->prev = head->prev;
  head->prev->next = new;
  head->prev = new;
}
 
void list_del(list_head *entry) {
  entry->prev->next = entry->next;
  entry->next->prev = entry->prev;
}
 
void list_del_init(list_head *entry) {
  list_del(entry);
  init_list_head(entry);
}
 
void list_replace(list_head *old, list_head *new) {
  new->next = old->next;
  new->prev = old->prev;
  new->next->prev = new;
  new->prev->next = new;
}
 
void list_replace_init(list_head *old, list_head *new) {
  list_replace(old, new);
  init_list_head(old);
}
 
void list_move(list_head *list, list_head *head) {
  list_del(list);
  list_add(list, head);
}
 
void list_move_tail(list_head *list, list_head *head) {
  list_del(list);
  list_add_tail(list, head);
}