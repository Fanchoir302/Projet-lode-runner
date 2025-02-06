#include "int_list.h"


//Fonctions associées aux listes :
int_list* create_and_initialize_int_list(int e){
  //Créé une liste et y place l'entier e
  int_list* l = (int_list*)malloc(sizeof(int_list));
  l->data = e;
  l->next = NULL;
  return l;
}
 

bool is_in_int_list(int_list* l, int e){
    //Renvoie true si e est dans la liste d'entiers l, false sinon
    int_list* current_position = l;
    bool found = false;
    while(current_position != NULL && !found){
        if(current_position->data == e){
            found = true;
        }
        current_position = current_position->next;
    }
    return found;
}

void add_if_new_int_list(int_list** l, int e){
  //Ajoute e en tête de l s'il n'est pas déjà dedans
  if(!is_in_int_list(*l, e)){
    int_list* temp = *l;
    *l = create_and_initialize_int_list(e);
    (*l)->next = temp;
  }
}

int_list* copy_int_list(int_list* l){
  //Renvoie une liste identique à la liste l, sans doublons
  //Fonctionne récursivement
  if(l == NULL){
    return NULL;
  }
  else if(l->next == NULL){
    return create_and_initialize_int_list(l->data);
  }
  else{
    int_list* new_element = copy_int_list(l->next);
    add_if_new_int_list(&new_element, l->data);
    return new_element;
  }
}

void remove_first_element_int_list(int_list** l){
  //Supprime le premier élément de la liste d'entiers l sous réserve d'existence
  if(l != NULL){
    int_list* first = *l;
    *l = (*l)->next;
    free(first);
  }
}

void free_int_list(int_list* l){
    //Libère la liste d'entiers l
    while(l != NULL){
      remove_first_element_int_list(&l);
    }
    free(l);
}

void remove_element_int_list(int_list** l, int e){
  //Supprime toutes les occurrences potentielles de e dans l
  if(*l != NULL){
    if((*l)->next == NULL){
      //Cas où il y a un unique élément dans l
      if((*l)->data == e){
        remove_first_element_int_list(l);
      }
    }
    else{
      //Cas où il y a au moins 2 éléments dans l
      if((*l)->data == e){
        remove_first_element_int_list(l);
        remove_element_int_list(l, e);
      }
      else{
        remove_element_int_list(&(*l)->next, e);
      }
    }
  }
}


int_list*** create_and_initialize_int_list_matrix(int n, int m){
  //Initialise une matrice de taille n * m et initialise toutes ses cases à une liste vide de type int_list
  int_list*** matrix = (int_list***)malloc(sizeof(int_list**)*n);
  for (int i = 0; i < n; i++){
    matrix[i] = (int_list**)malloc(sizeof(int_list*)*m);
    for (int j = 0; j < m; j++){
      matrix[i][j] = NULL;
    }
  }
  return matrix;
}