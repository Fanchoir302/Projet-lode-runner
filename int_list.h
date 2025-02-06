#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct s_list int_list;
struct s_list
{
  int_list *next;   //pointeur sur le reste de la liste
  int data;         //donnée
};

bool is_in_int_list(int_list*, int);
void add_if_new_int_list(int_list**, int);
int_list* copy_int_list(int_list*);
void remove_first_element_int_list(int_list**);
void free_int_list(int_list*);
void remove_element_int_list(int_list**, int);
void free_int_list_matrix(int_list***, int, int);