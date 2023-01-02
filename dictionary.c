
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct set_node
{
  const char *key;
  size_t key_len;
  void* value;
  struct set_node *next;
};

struct set_table
{
  struct set_node **nodes;
  size_t hashmap_size;
};

size_t djb33x_hash(const char *key, const size_t keylen)
{
  size_t hash = 5381;
  for (size_t i = 0; i < keylen; i++)
  {
    hash = ((hash << 5) + hash) ^ key[i];
  }
  return hash;
}

struct set_table *set_table_new(const size_t hashmap_size)
{
  struct set_table *table = malloc(sizeof(struct set_table));
  if (!table)
  {
    return NULL;
  }
  table->hashmap_size = hashmap_size;
  table->nodes = calloc(table->hashmap_size, sizeof(struct set_node *));
  if (!table->nodes)
  {
    free(table);
    return NULL;
  }
  return table;
}

void rehash(struct set_table *table, size_t new_size)
{
  struct set_node **new_nodes = calloc(new_size, sizeof(struct set_node *));
  if (new_nodes == NULL)
  {
    return;
  }

  for (size_t i = 0; i < table->hashmap_size; i++)
  {
    struct set_node *node = table->nodes[i];
    while (node != NULL)
    {
      size_t hash = djb33x_hash(node->key, node->key_len);
      size_t index = hash % new_size;

      struct set_node *next = node->next;
      node->next = new_nodes[index];
      new_nodes[index] = node;
      node = next;
    }
  }
  free(table->nodes);
  table->nodes = new_nodes;
  table->hashmap_size = new_size;
}

struct set_node *set_insert(struct set_table *table, const char *key, const size_t key_len, void *value)
{
  size_t hash = djb33x_hash(key, key_len);
  size_t index = hash % table->hashmap_size;
  struct set_node *head = table->nodes[index];
  int collision_count = 0;

  if (!head)
  {
    table->nodes[index] = malloc(sizeof(struct set_node));

    if (!table->nodes[index])
    {
      return NULL;
    }

    table->nodes[index]->key = key;
    table->nodes[index]->key_len = key_len;
    table->nodes[index]->value = value;
    table->nodes[index]->next = NULL;

    return table->nodes[index];
  }

  struct set_node *new_item = malloc(sizeof(struct set_node));

  if (!new_item)
  {
    return NULL;
  }

  new_item->key = key;
  new_item->key_len = key_len;
  new_item->value = value;
  new_item->next = NULL;
  struct set_node *tail = head;

  while (head)
  {
    if (head->key_len == key_len && strcmp(head->key, key) == 0)
    {
      head->value = value;
      head->key = key;
      return head;
    }
    tail = head;
    collision_count++;
    head = head->next;
  }
  tail->next = new_item;

  if (collision_count > 0)
  {
    size_t new_size = table->hashmap_size * 8;
    rehash(table, new_size);
  }

  return tail;
}

struct set_node *search(struct set_table *table, const char *key, size_t key_len)
{
  size_t hash = djb33x_hash(key, key_len);
  size_t index = hash % table->hashmap_size;
  struct set_node *head = table->nodes[index];

  while (head)
  {
    if (head->key_len == key_len && strcmp(head->key, key) == 0)
    {

      printf("key %s found\n", head->key);
      printf("with value %s\n", (char *)head->value);
      return head;
    }
    head = head->next;
  }
  printf("key not found\n");
  return NULL;
}

void delete_element(struct set_table *table, const char *key, size_t key_len)
{
  size_t hash = djb33x_hash(key, key_len);
  size_t index = hash % table->hashmap_size;
  struct set_node *head = table->nodes[index];
  if (head && head->key_len == key_len && strncmp(head->key, key, key_len) == 0)
  {
    table->nodes[index] = head->next;
    free(head);
    return;
  }
  while (head)
  {
    if (head->next && head->next->key_len == key_len && strncmp(head->key, key, key_len) == 0)
    {
      struct set_node *to_delete = head->next;
      head->next = to_delete->next;
      free(to_delete);
      return;
    }
    head = head->next;
  }
}

void print_set(struct set_table *table)
{
  for (size_t i = 0; i < table->hashmap_size; i++)
  {
    printf("Index %zu:\n", i);
    struct set_node *node = table->nodes[i];
    while (node != NULL)
    {
      printf("  Key: %s, Value: %s\n", node->key, (char *) node->value);
      node = node->next;
    }
  }
}

int main()
{
  struct set_table *my_set = set_table_new(8);

  set_insert(my_set, "1", 1, "red");
  set_insert(my_set, "2", 1, "blue");
  set_insert(my_set, "3", 1, "white");
  set_insert(my_set, "4", 1, "black");
  set_insert(my_set, "5", 1, "yellow");
  set_insert(my_set, "6", 1, "orange");
  set_insert(my_set, "12", 2, "white");

  print_set(my_set);
  search(my_set, "2", 1);
  delete_element(my_set, "4", 1);
  search(my_set, "4", 1);
  // print_set(my_set);

  return 0;
}