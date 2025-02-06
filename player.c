#include <stdbool.h> // bool, true, false
#include <stdlib.h>  // rand
#include <stdio.h>   // printf
#include <math.h>
#include <limits.h>   //INT_MAXint_list* create_and_initialize_int_list(int);
#include "lode_runner.h" // types and prototypes used to code the game
#include "player.h"

// global declarations used by the game engine
extern const char BOMB;  // ascii used for the bomb
extern const char BONUS;  // ascii used for the bonuses
extern const char CABLE;  // ascii used for the cable
extern const char ENEMY;  // ascii used for the ennemies
extern const char EXIT;   // ascii used for the exit
extern const char FLOOR;  // ascii used for the floor
extern const char LADDER; // ascii used for the ladder
extern const char PATH;   // ascii used for the pathes
extern const char RUNNER; // ascii used for runner
extern const char WALL;   // ascii used for the walls

extern const int BOMB_TTL; // time to live for bombs

extern bool DEBUG; // true if and only if the game runs in debug mode

const char *students = "Clem JOURDIN"; // replace Random with the student names here


int distance(int x, int y, int i, int j){
  //Renvoie la distance "à vol d'oiseau" entre les cases (x, y) et (i, j)
  return abs(x - i) + abs(y - j);
}

bonus closest_bonus(bonus_list bonuses, int x, int y){
  //Renvoie le bonus le plus proche de la position (x,y), parmi ceux présents dans la liste bonuses
  if (bonuses == NULL){
    printf("ERREUR : La liste passée en entrée ne peut pas être vide\n");
    exit(EXIT_FAILURE);
  }
  bonus best_bonus = bonuses->b;                                            //Bonus le plus proche dans ceux traités pour l'instant
  int best_distance = distance(best_bonus.x, best_bonus.y, x, y);    //Distance entre (x, y) et le bonus courant
  bonus_list remaining_bonuses = bonuses->next;
  while (remaining_bonuses != NULL){
    bonus current_bonus = remaining_bonuses->b;
    int current_distance = distance(current_bonus.x, current_bonus.y, x, y);
    if (current_distance < best_distance){
      best_bonus = current_bonus;
      best_distance = current_distance;
    }
    remaining_bonuses = remaining_bonuses->next;
  }
  return best_bonus;
}

int** create_and_initialize_int_matrix(int n, int m, int e){
  //Initialise une matrice de taille n * m et initialise toutes ses cases à l'entier e
  int** matrix = (int**)malloc(sizeof(int*)*n);
  for (int i = 0; i < n; i++){
    matrix[i] = (int*)malloc(sizeof(int)*m);
    for (int j = 0; j < m; j++){
      matrix[i][j] = e;
    }
  }
  return matrix;
}

int* create_and_initialize_int_array(int n, int e){
  //Initialise un tableau de taille n et initialise toutes ses cases à l'entier e
  int* a = (int*)malloc(sizeof(int)*n);
  for(int i = 0; i < n; i++){
    a[i] = e;
  }
  return a;
}

void free_int_matrix(int** matrix, int n){
  //Libère une matrice d'entiers à n lignes
  for (int i = 0; i < n; i++){
    free(matrix[i]);
  }
  free(matrix);
}

void free_int_list_matrix(int_list*** matrix, int n, int m){
  //Libère la matrice de liste d'entiers matrix de taille n * m
  for(int i = 0; i < n; i++){
    for(int j = 0; j < m; j++){
      free_int_list(matrix[i][j]);
    }
    free(matrix[i]);
  }
  free(matrix);
}

bool is_possible_action(levelinfo level, action a, int x, int y, int** enemies, int** bombs){
  //Renvoie true si l'action a est possible à partir de la position (x, y) dans level et false sinon
  //On considère comme impossible le fait de foncer sur un ennemi
  bool ok = false;  //ok sera passée à true si le déplacement s'avère possible
  if (a == NONE){
    ok = true;
  }
  if (a == UP && ((level.map[y][x] == LADDER || level.map[y][x] == EXIT) && enemies[y-1][x] == -1)){
    ok = true;
  }
  if (a == DOWN && ((level.map[y + 1][x] == LADDER || level.map[y + 1][x] == EXIT || level.map[y + 1][x] == PATH || level.map[y + 1][x] == CABLE) && enemies[y+1][x] == -1)){
    ok = true;
  }
  if (a == LEFT && (((level.map[y][x - 1] != WALL && level.map[y][x - 1] != FLOOR) && enemies[y][x-1] == -1) && bombs[y+1][x-1] == -1)){
    ok = true;
  }
  if (a == RIGHT && (((level.map[y][x + 1] != WALL && level.map[y][x + 1] != FLOOR) && enemies[y][x+1] == -1) && bombs[y+1][x+1] == -1)){
    ok = true;
  }
  if (a == BOMB_LEFT && (level.map[y + 1][x - 1] == FLOOR && level.map[y][x - 1] == PATH)){
    ok = true;
  }
  if (a == BOMB_RIGHT && (level.map[y + 1][x + 1] == FLOOR && level.map[y][x + 1] == PATH)){
    ok = true;
  }
  return ok;
}

bool is_plot_cable(levelinfo level, int x, int y){
  //Renvoie true si le bloc en position (x, y) est un plot_cable, false sinon
  if(x <= 0 || x >= level.xsize || y <= 0 || y >= level.ysize){
    //Les bordures ne peuvent pas être des plot_cables et on évite les dépassements
    return false;
  }
  else if((level.map[y][x] != FLOOR) && (level.map[y][x] != WALL)   //Les sols et les murs ne peuvent pas être des plot_cables car le runner ne peut pas être sur ces blocs
    && (level.map[y - 1][x] == CABLE || level.map[y - 1][x - 1] == CABLE || level.map[y - 1][x + 1] == CABLE)){
    //bloc directement sous un câble || bloc en diagonale bas droite d'un câble || bloc en diagonale bas gauche d'un câble
    //Pas de dépassements grâce au cas précédent
    return true;
  }
  else{
    return false;
  }
}

bool is_plot(levelinfo level, int x, int y){
  //Renvoie true si le bloc (x, y) est une parcelle, false sinon
  if(x == 0 || x == level.xsize || y == 0 || y == level.ysize){
    //Les bordures de la carte ne peuvent pas être des parcelles
    //Cas traité à part pour éviter les dépassements dans les cas suivants
    return false;
  }
  else if ((is_plot_cable(level, x, y) || level.map[y][x] == LADDER || level.map[y][x] == EXIT) 
      && ((is_plot_cable(level, x - 1, y) || level.map[y][x - 1] == LADDER || level.map[y][x - 1] == EXIT) || (is_plot_cable(level, x + 1, y) || level.map[y][x + 1] == LADDER || level.map[y][x + 1] == EXIT))){
      //Cas 1 : (Câble ou Échelle) adjacent (gauche droite uniquement) à un autre (Câble ou Échelle)
    return true;
  }
  else if (((level.map[y][x] == PATH || level.map[y][x] == BONUS || level.map[y][x] == LADDER || level.map[y][x] == EXIT) && (level.map[y + 1][x] == FLOOR || level.map[y + 1][x] == LADDER || level.map[y + 1][x] == EXIT)) 
    && ((((level.map[y][x - 1] == PATH || level.map[y][x - 1] == BONUS || level.map[y][x - 1] == LADDER || level.map[y][x - 1] == EXIT) && (level.map[y + 1][x - 1] == FLOOR || level.map[y + 1][x - 1] == LADDER || level.map[y + 1][x - 1] == EXIT)) 
    || ((level.map[y][x + 1] == PATH || level.map[y][x + 1] == BONUS || level.map[y][x + 1] == LADDER || level.map[y][x + 1] == EXIT) && (level.map[y + 1][x + 1] == FLOOR || level.map[y + 1][x + 1] == LADDER || level.map[y + 1][x + 1] == EXIT))))){
    //Cas 2 : (Vide ou Bonus ou Échelle) au dessus de (Sol ou Échelle) adjacent (gauche droite uniquement) à un autre bloc suivant les mêmes conditions
    return true;
  }
  else{
    return false;
  }
}

int define_plots(levelinfo level, int** plots){
  //Remplit la matrice plots (initialement rempli de -1) selon les règles décrites dans la représentation des parcelles
  //plots est de taille level.ysize * level.xsize
  //Renvoie le nombre de parcelles trouvées
  int nb_plots = 0;
  for (int y = 1; y < level.ysize - 1; y++){
    //On va de 1 à level.ysize - 2 car les bords du plateau sont inatteignables donc forcément à -1 (pas des parcerelles)
    for (int x = 1; x < level.xsize - 1; x++){
      if(is_plot(level, x, y)){
        if(!is_plot(level, x - 1, y)){
          //Cas 1 : Premier bloc de la parcelle
          plots[y][x] = nb_plots;
          nb_plots += 1;
        }
        else{
          //Cas 2 : On était déjà sur une parcelle ie le bloc à la gauche du bloc courant appartient à la même parcelle que le bloc courant (et il est déjà identifié en tant que tel)
          plots[y][x] = plots[y][x - 1];
        }
      }
    }
  }
  return nb_plots;
}

int identify_ladders(levelinfo level, int** ladders, int** plots){
  //Remplit la matrice ladders (initialement rempli de -1) selon les règles décrites dans la représentation des échelles (Objet)
  //Prend la matrice de parcelles plots en entrée
  //ladders est de taille level.ysize * level.xsize
  //Renvoie le nombre d'échelles (Objet) trouvées
  int nb_ladders = 0;
  for (int y = 1; y < level.ysize - 1; y++){
    //On va de 1 à level.ysize - 2 car les bords du plateau ne seront jamais des Échelles (Objet)
    for (int x = 1; x < level.xsize - 1; x++){
      if((level.map[y][x] == LADDER || level.map[y][x] == EXIT) && ladders[y][x] == -1){
        //Échelle pas encore traitée donc le bloc (x, y) est le plus haut de l'Échelle (Objet) qui est une échelle (L)
        ladders[y][x] = nb_ladders;
        if(plots[y - 1][x] != -1){
          //Cas où le bloc au dessus de l'échelle (L) rencontrée est une parcelle
          ladders[y - 1][x] = nb_ladders;
        }
        int i = 1;
        while(level.map[y + i][x] == LADDER || level.map[y + i][x] == EXIT){
          ladders[y + i][x] = nb_ladders;
          i += 1;
        }
        nb_ladders += 1;
      }
    }
  }
  return nb_ladders;
}

void identify_ladders_x_position(levelinfo level, int* ladders_x_position, int** ladders){
  //Remplit le tableau ladders_x_position tel que ladders_x_position[i] contienne la position x de l'Échelle (Objet) i
  //ladders_x_position est initialement rempli de -1
  for(int y = 1; y < level.ysize - 1; y++){
    //Inutile de parcourir les bordures car on sait que ce ne sont pas des échelles (L), d'où le choix de bornes
    for(int x = 1; x < level.xsize - 1; x++){
      //Idem pour les bornes
      if(ladders[y][x] != -1){
        //Cas où une Échelle (Objet) est trouvée
        if(ladders_x_position[ladders[y][x]] == -1){
          //Cas où l'Échelle trouvée n'a pas encore été traitée
          ladders_x_position[ladders[y][x]] = x;
        }
      }
    }
  }
}

void identify_plots_y_position(levelinfo level, int* plots_y_position, int** plots){
  //Remplit le tableau plots_y_position tel que plots_y_position[i] contienne la position y de la parcelle i
  //ladders_x_position est initialement rempli de -1
  for(int y = 1; y < level.ysize - 1; y++){
    //Inutile de parcourir les bordures car on sait que ce ne sont pas des parcelles, d'où le choix de bornes
    for(int x = 1; x < level.xsize - 1; x++){
      //Idem pour les bornes
      if(plots[y][x] != -1){
        //Cas où une parcelle est trouvée
        if(plots_y_position[plots[y][x]] == -1){
          //Cas où la parcelle trouvée n'a pas encore été traitée
          plots_y_position[plots[y][x]] = y;
        }
      }
    }
  }
}

void build_plots_accessibility_graph(levelinfo level, int_list*** plots_accessibility_graph, int nb_ladders, int** plots, int** ladders, int* ladders_x_position){
  //Remplit la matrice d'adjacence plots_accessibility_graph, représentant le graphe d'accéssibilité des parcelles, de taille nb_plots * nb_plots
  //m[i][j] != -1 si et seulement si j est accessible depuis i
  //Si m[i][j] != -1, alors m[i][j] = position x de l'Échelle (Objet) ou de la chute qui permet de rejoindre j à partir de i
  
  //Adjacences liées aux Échelles (Objet) (symétrique) :
  for(int current_ladder = 0; current_ladder < nb_ladders; current_ladder++){
    for(int y = 1; y < level.ysize - 1; y++){
      //Inutile de parcourir les bordures car on sait que ce ne sont pas des parcelles, d'où le choix de bornes
      if(ladders[y][ladders_x_position[current_ladder]] == current_ladder){
        //Cas où on a atteint l'Échelle (Objet) current_ladder dans notre descente du plateau sur la colonne où elle se trouve
        if(plots[y][ladders_x_position[current_ladder]] != -1){
          //Cas où le bloc courant est l'intersection entre current_ladder et une parcelle
          //Cette parcelle est donc accessible depuis toutes les autres parcelles traversées par current_ladder
          int plot1 = plots[y][ladders_x_position[current_ladder]];
          for(int y2 = y + 1; y2 < level.ysize - 1; y2++){
            if((plots[y2][ladders_x_position[current_ladder]] != -1) && (ladders[y2][ladders_x_position[current_ladder]] == current_ladder)){
              //Cas où on rencontre une deuxième parcelle et on est toujours sur l'Échelle (Objet)
              int plot2 = plots[y2][ladders_x_position[current_ladder]];
              //Symétrie de la relation d'accessibilité lorsque le lien est une Échelle (Objet)
              add_if_new_int_list(&plots_accessibility_graph[plot1][plot2], ladders_x_position[current_ladder]);
              add_if_new_int_list(&plots_accessibility_graph[plot2][plot1], ladders_x_position[current_ladder]);
            }
          }
        }
      }
    }
  }

  //Adjacences liées aux chutes (non symétrique) :
  for(int y = 1; y < level.ysize - 1; y++){
    for(int x = 1; x < level.xsize - 1; x++){
      //Chutes par tombée de câble
      if(level.map[y][x] == CABLE){
        int higher_plot = plots[y + 1][x];
        if(level.map[y + 2][x] != FLOOR){
          int y_next_plot = y + 2;
          while(plots[y_next_plot][x] == -1 && y_next_plot < level.ysize - 1){
            //On descend sous le cable (parcelle haute) jusqu'à tomber sur une autre parcelle (parcelle basse) ou jusqu'à atteindre la fin du plateau
            y_next_plot += 1;
          }
          if(plots[y_next_plot][x] != -1){
            //Cas où on est sortis du while parce qu'une parcelle a été atteinte
            int lower_plot = plots[y_next_plot][x];
            add_if_new_int_list(&plots_accessibility_graph[higher_plot][lower_plot], x);
          }
          }
      }
    }
    for(int x = 2; x < level.xsize - 2; x++){
      //Chutes pas sortie de parcelle (déplacement à gauche ou à droite lorsque positionné sur un dernier bloc de parcelle)
      //x ne va que de 2 à level.xsize - 3 car on ne peut pas tomber d'une parcelle si celle-ci est directement accolée à un mur
      if(plots[y][x - 1] == -1 && plots[y][x] != -1 && level.map[y][x - 1] != FLOOR){
        //Cas où on peut tomber à gauche de la parcelle en (y, x)
        int higher_plot = plots[y][x];
        int y_next_plot = y;
        while(plots[y_next_plot][x - 1] == -1 && y_next_plot < level.ysize - 1){
          //On descend la colonne à gauche de la parcelle haute jusqu'à tomber sur une autre parcelle (parcelle basse) ou jusqu'à atteindre la fin du plateau
          y_next_plot += 1;
        }
        if(plots[y_next_plot][x - 1] != -1){
          //Cas où on est sortis du while parce qu'une parcelle a été atteinte
          int lower_plot = plots[y_next_plot][x - 1];
          add_if_new_int_list(&plots_accessibility_graph[higher_plot][lower_plot], x - 1);
        }
      }
      if(plots[y][x + 1] == -1 && plots[y][x] != -1 && level.map[y][x + 1] != FLOOR){
        //Cas où on peut tomber à droite de la parcelle en (y, x)
        int higher_plot = plots[y][x];
        int y_next_plot = y;
        while(plots[y_next_plot][x + 1] == -1 && y_next_plot < level.ysize - 1){
          //On descend la colonne à droite de la parcelle haute jusqu'à tomber sur une autre parcelle (parcelle basse) ou jusqu'à atteindre la fin du plateau
          y_next_plot += 1;
        }
        if(plots[y_next_plot][x + 1] != -1){
          //Cas où on est sortis du while parce qu'une parcelle a été atteinte
          int lower_plot = plots[y_next_plot][x + 1];
          add_if_new_int_list(&plots_accessibility_graph[higher_plot][lower_plot], x + 1);
        }
      }
    }
  }
}

int_list* is_accessible_from(int_list*** m, int n, int v){
  //Renvoie la liste des sommets accessibles depuis le sommet v dans le graphe représenté par la matrice d'adjacence m de taille n*?
  int_list* vertex = NULL;
  for(int i = 0; i < n; i++){
    if(m[v][i] != NULL){
      add_if_new_int_list(&vertex, i);
    }
  }
  return vertex;
}

int weight(int_list*** plots_accessibility_graph, int* plots_y_position, int plot1, int plot2){
  //Renvoie la pondération de l'arc allant du plot1 au sommet plot2 de plots_accessibility_graph
  if(plots_accessibility_graph[plot1][plot2] == NULL){
    //Cas où le sommet plot2 n'est pas accessible directement depuis le sommet plot1
    return INT_MAX;
  }
  else{
    //Cas où le sommet plot2 est accessible depuis le sommet plot1
    return abs(plots_y_position[plot1] - plots_y_position[plot2]);
  }
}

void initialize_best_x_path_and_practical_weight(int* best_path, int* practical_weight, int_list*** plots_accessibility_graph, int** plots, int* plots_y_position, int x1, int y1, int x2, int y2){
  //Initialise les valeurs de best_path et practical_weight correspondant au chemin de position1 à position2
  //Si position2 non accessible depuis position1 : best_path = -1 et practical_weight = INT_MAX
  int plot1 = plots[y1][x1];
  int plot2 = plots[y2][x2];
  if(weight(plots_accessibility_graph, plots_y_position, plot1, plot2) == INT_MAX){
    //Cas où plot2 n'est pas accessible depuis plot1
    *best_path = -1;
    *practical_weight = INT_MAX;
  }
  else{
    //Cas où plots_accessibility_graph[plot1][plot2] n'est pas la liste vide
    int_list* paths = copy_int_list(plots_accessibility_graph[plot1][plot2]);  //Liste des accessibilités de plot1 à plot2 que l'on peut manipuler librement sans détruire plots_accessibility_graph[plot1][plot2]
    int current_path = paths->data;
    int current_best_path = current_path;
    int current_best_distance;  //distance lattérale minimum à parcourir pour aller en position1 à position2 en passant par current_path
    if((current_path >= x1 && current_path <= x2) || (current_path <= x1 && current_path >= x2)){
        //Cas 1 : le chemin courant est lattéralement entre la position1 et la position2 (alignement lattérale (position1 | (current_path | position2) OU (position2 | current_path | position1))
        current_best_distance = abs(x1 - x2);
      }
    else if((current_path <= x1 && x1 <= x2) || (current_path >= x1 && x1 >= x2)){
      //Cas 2 : alignement lattérale (current_path | position1 | position2) OU (position2 | position1 | current_path)
      current_best_distance = 2*abs(current_best_path - x1) + abs(x1 - x2);
    }
    else{
      //Cas 3 : alignement lattérale (current_path | position2 | position1) OU (position1 | position2 | current_path)
      current_best_distance = 2*abs(current_best_path - x2) + abs(x1 - x2);
    }

    int current_distance;
    while(paths != NULL){
      if((current_path >= x1 && current_path <= x2) || (current_path <= x1 && current_path >= x2)){
        //Cas 1
        current_distance = abs(x1 - x2);
      }
      else if((current_path <= x1 && x1 <= x2) || (current_path >= x1 && x1 >= x2)){
        //Cas 2
        current_distance = 2*abs(current_path - x1) + abs(x1 - x2);
      }
      else{
        //Cas 3
        current_distance = 2*abs(current_path - x2) + abs(x1 - x2);
      }
      if(current_distance < current_best_distance){
        current_best_distance = current_distance;
        current_best_path = current_path;
      }
      current_path = paths->data;
      remove_first_element_int_list(&paths);
    }
    *best_path = current_best_path;
    *practical_weight = current_best_distance + weight(plots_accessibility_graph, plots_y_position, plot1, plot2);
    free_int_list(paths);
  }
}

int find_practical_weight(int_list*** plots_accessibility_graph, int** plots, int* plots_y_position, int x1, int y1, int x2, int y2){
  //Renvoie le poids utile calculé par initialize_best_x_path_and_practical_weight
  int practical_weight;
  int best_path;    //Inutile ici
  initialize_best_x_path_and_practical_weight(&best_path, &practical_weight, plots_accessibility_graph, plots, plots_y_position, x1, y1, x2, y2);
  return practical_weight;
}

int find_best_x_path(int_list*** plots_accessibility_graph, int** plots, int* plots_y_position, int x1, int y1, int x2, int y2){
  //Renvoie la meilleur Échelle (Objet) trouvée par initialize_best_x_path_and_practical_weight
  int practical_weight;    //Inutile ici
  int best_path;
  initialize_best_x_path_and_practical_weight(&best_path, &practical_weight, plots_accessibility_graph, plots, plots_y_position, x1, y1, x2, y2);
  return best_path;
}

int find_vertex_minimum_distance(int_list* vertex, int* priorities){
  //Renvoie le sommet dans la liste de sommets vertex de priorité associée minimum
  //On suppose vertex non vide
  int_list* current_position = copy_int_list(vertex);
  int current_best_vertex = vertex->data;
  remove_first_element_int_list(&current_position);
  while(current_position != NULL){
    if(priorities[current_position->data] < priorities[current_best_vertex]){
      current_best_vertex = current_position->data;
    }
    remove_first_element_int_list(&current_position);
  }
  free_int_list(current_position);
  return current_best_vertex;
}

int addition_int_max(int a, int b){
  //Renvoie a + b si les 2 sont différents de INT_MAX et INT_MAX si au moins un des 2 est égal à INT_MAX
  if(a == INT_MAX || b == INT_MAX){
    return INT_MAX;
  }
  else{
    return a + b;
  }
}

int find_middle_of_plot(levelinfo level, int** plots, int* plots_y_position, int plot){
  //Renvoie la coordonnée centrale x de plot
  int first_block = 0;    //Premier bloc de la parcelle
  int last_block = level.ysize;     //Dernier bloc de la parcelle
  int y = plots_y_position[plot];
  for(int x = 1; x < level.xsize - 1; x++){
    if(plots[y][x - 1] != plot && plots[y][x] == plot){
      first_block = x;
    }
    if(plots[y][x] == plot && plots[y][x + 1] != plot){
      last_block = x;
    }
  }
  return (first_block + last_block)/2;
}

int find_x_position_first_ladder(levelinfo level, int_list*** plots_accessibility_graph, int** plots, int* plots_y_position, int nb_plots, int x1, int y1, int x2, int y2){
  //Renvoie la coordonnée y de la première Échelle (Objet) à utiliser pour s'approcher de (x2, y2) à partir de (x1, y1)
  //Présuppose que les positions 1 et 2 ne sont pas sur la même parcelle
  //Dans le cas où plot2 n'est momentanément pas accessible depuis plot1 (possible avec les suppressions d'arcs dues aux ennemis), renvoie -1
  int plot1 = plots[y1][x1];
  int plot2 = plots[y2][x2];
  int best_x_path = find_best_x_path(plots_accessibility_graph, plots, plots_y_position, x1, y1, x2, y2);
  if(best_x_path != -1){
    //Cas où position2 est accessible à partir de position1 en passant par une unique Échelle (Objet)
    return best_x_path;
  }
  else{
    //Application de l'algorithme de Dijkstra au graphe d'accessibilités pondéré par les poids utiles
    int* distances = create_and_initialize_int_array(nb_plots, INT_MAX);
    distances[plot1] = 0;
    int* predecessors = create_and_initialize_int_array(nb_plots, -1);
    int_list* queue = NULL;
    for(int vertex = 0; vertex < nb_plots; vertex++){
      add_if_new_int_list(&queue, vertex);
    }
    while(queue != NULL){
      int vertex = find_vertex_minimum_distance(queue, distances);
      remove_element_int_list(&queue, vertex);
      int_list* neighbors = is_accessible_from(plots_accessibility_graph, nb_plots, vertex);
      while(neighbors != NULL){
        int neighbor = neighbors->data;
        if(is_in_int_list(queue, neighbor)){
          int new_distance = addition_int_max(distances[vertex], weight(plots_accessibility_graph, plots_y_position, vertex, neighbor));
          if(new_distance < distances[neighbor]){
            distances[neighbor] = new_distance;
            predecessors[neighbor] = vertex;
          }
        }
        remove_first_element_int_list(&neighbors);
      }
      free_int_list(neighbors);
    }
    free_int_list(queue);
    //Reconstruction du chemin à partir des tableaux distances et predecessors fournis par dijkstra :
    int first_plot_in_path = predecessors[plot2];   //Première parcelle traversée sur le chemin de plot1 à plot2
    if(first_plot_in_path == -1){
      //Cas où plot2 n'est momentanément pas accessible depuis plot1
      best_x_path = -1;
    }
    else{
      int current_plot = predecessors[plot2];
      while(current_plot != plot1){
        current_plot = predecessors[current_plot];
        if(current_plot != plot1){
          first_plot_in_path = current_plot;
        }
      }
      int x_first_plot_in_path = find_middle_of_plot(level, plots, plots_y_position, first_plot_in_path);
      int y_first_plot_in_path = plots_y_position[first_plot_in_path];
      best_x_path = find_best_x_path(plots_accessibility_graph, plots, plots_y_position, x1, y1, x_first_plot_in_path, y_first_plot_in_path);
    }
    free(distances);
    free(predecessors);
    return best_x_path;
  }
}

void find_exit(levelinfo level, int* x_exit, int* y_exit){
  //Enregistre les coordonnées x et y de la sortie respectivement dans *x et *y
  for(int y = 0; y < level.ysize; y++){
    for(int x = 0; x < level.xsize; x++){
      if(level.map[y][x] == EXIT){
        *x_exit = x;
        *y_exit = y;
      }
    }
  }
}

int find_plot_under_exit(int** plots, int x_exit){
  //Renvoie l'identifiant de la première parcelle accessible directement depuis l'Échelle (Objet) rattachée à la sortie
  //On considère que la sortie est forcément sur le bord supérieur du plateau
  int y = 0;
  while(plots[y][x_exit] == -1){
    y++;
  }
  return plots[y][x_exit];
}

int get_identifier_from_position(levelinfo level, int x, int y){
  //Renvoie l'identifiant correspondant à la case (x,y) de level.map
  //Identifiants comptés de gauche à droite puis de haut en bas
  return x + y*(level.xsize);
}

int get_x_position_from_identifier(levelinfo level, int identifier){
  return identifier%(level.xsize);
}

int get_y_position_from_identifier(levelinfo level, int identifier){
  return identifier/(level.xsize);
}

void update_plots_accessibility_graph_with_enemies(levelinfo level, int_list* enemies_list, int_list*** plots_accessibility_graph, int** plots, int** ladders){
  //Si un ennemi se trouve sur une Échelle (Objet), cette fonction supprime tous les arcs dûs à cette Échelle (Objet) dans plots_accessibility_graph
  int_list* current_enemy = enemies_list;
  while(current_enemy != NULL){
    int enemy = current_enemy->data;
    int enemy_x_position = get_x_position_from_identifier(level, enemy);
    int enemy_y_position = get_y_position_from_identifier(level, enemy);
    int ladder_enemy = ladders[enemy_y_position][enemy_x_position];   //Échelle (Objet) sur laquelle se trouve l'ennemi courant
    if(ladder_enemy != -1){
      //L'ennemi courant est sur une Échelle (Objet)
      int_list* plots_crossed_by_ladder_enemy = NULL;
      for(int y = 0; y < level.ysize; y++){
        //Parcours de la ligne verticale sur laquelle se trouve ladder_enemy (ligne de coordonnée x = enemy_x_position)
        if(ladders[y][enemy_x_position] == ladder_enemy){
          //La case courante est sur l'Échelle (Objet) de l'ennemi
          if(plots[y][enemy_x_position] != -1){
            //La case courante est sur une parcelle
            add_if_new_int_list(&plots_crossed_by_ladder_enemy, plots[y][enemy_x_position]);
          }
        }
      }
      //Suppression des y dûs à ladder_enemy dans plots_accessibility_graph :
      int_list* current_plot_1 = plots_crossed_by_ladder_enemy;
      while(current_plot_1 != NULL){
        int_list* current_plot_2 = plots_crossed_by_ladder_enemy;
        while(current_plot_2 != NULL){
          remove_element_int_list(&plots_accessibility_graph[current_plot_1->data][current_plot_2->data], enemy_x_position);
          current_plot_2 = current_plot_2->next;
        }
        current_plot_1 = current_plot_1->next;
      }
      free_int_list(plots_crossed_by_ladder_enemy);
    }
    current_enemy = current_enemy->next;
  }
}

int find_closest_enemy(levelinfo level, int_list* enemies_list, int x, int y){
  //Renvoie l'identifiant de la case sur laquelle se trouve l'ennemi le plus proche de la case (x, y) parmi la liste non vide d'ennemi enemies
  if(enemies_list == NULL){
    printf("ERREUR : La liste d'ennemis passée en entrée n'est pas censée être vide\n");
    return -1;
  }
  else{
    int closest_enemy = enemies_list->data;
    int_list* current_position = enemies_list->next;
    while(current_position != NULL){
      if(distance(x, y, get_x_position_from_identifier(level, current_position->data), get_y_position_from_identifier(level, current_position->data)) < distance(x, y, get_x_position_from_identifier(level, closest_enemy), get_y_position_from_identifier(level, closest_enemy))){
        //Cas où l'ennemi courant est plus proche que l'ancien ennemi le plus proche sauvegardé
        closest_enemy = current_position->data;
      }
      current_position = current_position->next;
    }
    return closest_enemy;
  }
}

void identify_enemies(levelinfo level, int** enemies, int_list* enemies_list){
  //Place un 0 dans enemies aux endroits de la map où il y a un ennemi
  int_list* current_position = enemies_list;
  while(current_position != NULL){
    int enemy = current_position->data;
    int enemy_x_position = get_x_position_from_identifier(level, enemy);
    int enemy_y_position = get_y_position_from_identifier(level, enemy);
    enemies[enemy_y_position][enemy_x_position] = 0;
    current_position = current_position->next;
  }
}

void identify_bombs(int** bombs, bomb_list bombl){
  //Place son délai restant dans bombs pour chaque bombe présente dans bombl
  bomb_list current_position = bombl;
  while(current_position != NULL){
    bombs[current_position->y][current_position->x] = current_position->delay;
    current_position = current_position->next;
  }
}

int count_threats_to_runner(levelinfo level, int_list** threats, int** enemies, int x_runner, int y_runner){
  //Renvoie le nombre d'ennemis à une distance inférieure à 2 du runner et enregistre leur identifiant dans la liste *threats
  int nb_threats = 0;
  for(int y = 0; y < level.ysize; y++){
    for(int x = 0; x < level.xsize; x++){
      if(enemies[y][x] != -1 && distance(x, y, x_runner, y_runner) <= 2){
        nb_threats += 1;
        add_if_new_int_list(threats, get_identifier_from_position(level, x, y));
      }
    }
  }
  return nb_threats;
}


/* 
  function to code: it may use as many modules (functions and procedures) as needed
  Input (see lode_runner.h for the type descriptions): 
    - level provides the information for the game level
    - characterl is the linked list of all the characters (runner and enemies)
    - bonusl is the linked list of all the bonuses that have not been collected yet
    - bombl is the linked list of all the bombs that are still active
  Output
    - the action to perform
*/
action lode_runner(
  levelinfo level,
  character_list characterl,
  bonus_list bonusl,
  bomb_list bombl
  )
{
  action a; // action to choose and then return
  
  int x; // runner's x position
  int y; // runner's y position

  int_list* enemies_list = NULL;  //Liste des ennemis, repérés par leur identifiants

  character_list pchar=characterl; // iterator on the character list

  // looking for the runner ; we know s.he is in the list
  do
  { 
    if(pchar->c.item==RUNNER) // runner found
    {
      x=pchar->c.x; 
      y=pchar->c.y;
      pchar=pchar->next;
    }
    else // otherwise move on next character and add the current one to the list of enemies by identifier
    {
      add_if_new_int_list(&enemies_list, get_identifier_from_position(level, pchar->c.x, pchar->c.y));
      pchar=pchar->next;
    }
  } while(pchar != NULL);

  //Création de plots (matrice identifiant les parcelles)
  int** plots = create_and_initialize_int_matrix(level.ysize, level.xsize, -1);
  int nb_plots = define_plots(level, plots);

  //Identification des Échelles (Objet)
  int** ladders = create_and_initialize_int_matrix(level.ysize, level.xsize, -1);
  int nb_ladders = identify_ladders(level, ladders, plots);

  //Identification des positions x des Échelles (Objet)
  int* ladders_x_position = create_and_initialize_int_array(nb_ladders, -1);
  identify_ladders_x_position(level, ladders_x_position, ladders);

  //Identification des positions y des parcelles
  int* plots_y_position = create_and_initialize_int_array(nb_plots, -1);
  identify_plots_y_position(level, plots_y_position, plots);

  //Construction du graphe d'accessibilités (représenté par matrice d'adjacence)
  int_list*** plots_accessibility_graph = create_and_initialize_int_list_matrix(nb_plots, nb_plots);
  build_plots_accessibility_graph(level, plots_accessibility_graph, nb_ladders, plots, ladders, ladders_x_position);

  //Ennemies :
  int** enemies = create_and_initialize_int_matrix(level.ysize, level.xsize, -1);
  identify_enemies(level, enemies, enemies_list);

  //Bombes :
  int** bombs = create_and_initialize_int_matrix(level.ysize, level.xsize, -1);
  identify_bombs(bombs, bombl);

  //Mise à jour du graphe d'accessibilités
  update_plots_accessibility_graph_with_enemies(level, enemies_list, plots_accessibility_graph, plots, ladders);


  //Coordonnées de la case qu'on essaye d'atteindre (bonus le plus proche ou sortie dans le cas où tous les bonus ont été trouvés)
  bool ok = false;
  int x_goal = 0;
  int y_goal = 0;
  if(bonusl != NULL){
    //Il reste des bonus à récupérer
    bonus best_bonus = closest_bonus(bonusl, x, y);   //bonus le plus proche, qui sera notre première cible
    x_goal = best_bonus.x;
    y_goal = best_bonus.y;
  }
  else{
    int x_exit = 0;
    int y_exit = 0;
    find_exit(level, &x_exit, &y_exit);
    int plot_under_exit = find_plot_under_exit(plots, x_exit);
    x_goal = find_middle_of_plot(level, plots, plots_y_position, plot_under_exit);
    y_goal = plots_y_position[plot_under_exit];
    if(y == y_goal && plots[y][x] == plots[y_goal][x_goal]){
      ok = true;
      //Le runner est sur la bonne plateforme pour atteindre la sortie
      if(x == x_goal){
        //Le runner est sur l'Échelle (Objet) qui mène à la sortie
        if(y < y_goal){
          //Le runner est au dessus de la sortie
          if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
            a = DOWN;
          }
          else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
            a = RIGHT;
          }
          else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
            a = LEFT;
          }
          else{
            a = NONE;
          }
        }
        else{
          //Le runner est en dessous de la sortie
          if(is_possible_action(level, UP, x, y, enemies, bombs)){
            a = UP;
          }
          else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
            a = RIGHT;
          }
          else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
            a = LEFT;
          }
          else{
            a = NONE;
          }
        }
      }
      else{
        //Le runner n'est pas au niveau de l'Échelle (Objet) qui mène à la sortie
        if(x < x_goal){
          //Le runner est à gauche de l'Échelle (Objet) qui mène à la sortie
          if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
            a = RIGHT;
          }
          else if(is_possible_action(level, UP, x, y, enemies, bombs)){
            a = UP;
          }
          else if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
            a = DOWN;
          }
          else{
            a = NONE;
          }
        }
        else{
          //Le runner est à droite de l'Échelle (Objet) qui mène à la sortie
          if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
            a = LEFT;
          }
          else if(is_possible_action(level, UP, x, y, enemies, bombs)){
            a = UP;
          }
          else if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
            a = DOWN;
          }
          else{
            a = NONE;
          }
        }
      }
    }
    else if(y_exit < y && y < plots_y_position[plot_under_exit] && plots[plots_y_position[plot_under_exit]][x] == plot_under_exit){
      //Le runner est entre la parcelle sous la sortie et la sortie
      //Le runner est a priori forcément sur une Échelle (Objet)
      ok = true;
      if(is_possible_action(level, UP, x, y, enemies, bombs)){
        a = UP;
      }
      else{
        printf("Cas théoriquement impossible si on part du postulat que la sortie est nécessairement sur le bord supérieur du plateau\n");
        ok = false;
      }
    }
  }
  
  if(!ok){
    if(plots[y][x] != -1){
      //Le runner se trouve sur une parcelle
      if(plots[y][x] == plots[y_goal][x_goal]){
        //Le runner se trouve sur la même parcelle que le bonus visé
        if(x < x_goal){
          //Le runner est à gauche du bonus visé
          a = RIGHT;
        }
        else{
          //Le runner est à droite du bonus visé
          a = LEFT;
        }
      }
      else{
        //Le runner se trouve sur une parcelle différente du bonus visé
        int x_path = find_x_position_first_ladder(level, plots_accessibility_graph, plots, plots_y_position, nb_plots, x, y, x_goal, y_goal);
        if(x_path == -1){
          //Pas d'Échelle (Objet) trouvée : plot2 momentanément non accessible depuis plot1 à cause de ennemis
          int closest_enemy = find_closest_enemy(level, enemies_list, x, y);
          int x_position_closest_enemy = get_x_position_from_identifier(level, closest_enemy);
          if(x < x_position_closest_enemy){
            //Le runner est à gauche de l'ennemi le plus proche, donc on le redirige vers la droite
            x_path = x + 1;
          }
          else{
            //Le runner est à droite de l'ennemi le plus proche, donc on le redirige vers la gauche
            x_path = x - 1;
          }
        }
        if(x < x_path){
          //Le runner est à gauche du chemin visé
          a = RIGHT;
        }
        else if(x > x_path){
          //Le runner est à droite du chemin visé
          a = LEFT;
        }
        else{
          //Le runner est sur le chemin visé
          int move = rand() % 2;
          if(move == 0){
            //DOWN choisi au hasard
            if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
              //Le runner peut aller vers le bas
              a = DOWN;
            }
            else{
              a = UP;
            }
          }
          else{
            //UP choisi au hasard
            if(is_possible_action(level, UP, x, y, enemies, bombs)){
              //Le runner peut aller vers le haut
              a = UP;
            }
            else{
              a = DOWN;
            }
          }
        }
      }
    }
    else if(ladders[y][x] != -1){
      //Le runner se trouver sur une Échelle (Objet)
      if(y > y_goal){
        //Le runner est en dessous du bonus visé
        if(is_possible_action(level, UP, x, y, enemies, bombs)){
          //Le runner peut encore monter sur l'Échelle (Objet) sur laquelle il se trouve
          a = UP;
        }
        else{
          //Le runner est en haut d'une Échelle (Objet)
          a = DOWN;
        }
      }
      else{
        //Le runner est au dessus du bonus visé
        if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          //Le runner peut encore descendre sur l'Échelle (Objet) sur laquelle il se trouve
          a = DOWN;
        }
        else{
          //Le runner est en bas d'une Échelle (Objet)
          a = UP;
        }
      }
    }
    else{
      //Le runner n'est ni sur une parcelle ni sur une Échelle (Objet)
      if(y < y_goal){
        //Le runner est au dessus du bonus visé
        if(is_possible_action(level, DOWN, y, x, enemies, bombs)){
          //Le runner peut descendre
          a = DOWN;
        }
        else if(x < x_goal){
          //Le runner est à gauche du bonus visé
          if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
            //Le runner peut aller vers la droite
            a = RIGHT;
          }
          else{
            a = LEFT;
          }
        }
        else{
          //Le runner est à droite du bonus visé
          if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
            //Le runner peut aller vers la gauche
            a = LEFT;
          }
          else{
            a = RIGHT;
          }
        }
      }
      else{
        //Le runner est en dessous ou au même niveau que le bonus visé
        if(x < x_goal){
          //Le runner est à gauche du bonus visé
          if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
            //Le runner peut aller vers la droite
            a = RIGHT;
          }
          else{
            a = LEFT;
          }
        }
        else{
          //Le runner est à droite ou au même niveau que le bonus visé
          if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
            //Le runner peut aller vers la gauche
            a = LEFT;
          }
          else{
            a = RIGHT;
          }
        }
      }
    }

    //Prise en considération des ennemis :
    int_list* threats = NULL;
    int nb_threats = count_threats_to_runner(level, &threats, enemies, x, y);
    if(nb_threats > 0){
      //Ennemis directement à côté du runner :
      if(enemies[y][x-1] != -1){
        //Ennemi à la gauche du runner
        if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else{
          a = NONE;
        }
      }
      else if(enemies[y][x+1] != -1){
        //Ennemi à la droite du runner
        if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else{
          a = NONE;
        }
      }
      else if(enemies[y-1][x] != -1){
        //Ennemi au dessus du runner
        if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else{
          a = NONE;
        }
      }
      else if(enemies[y+1][x] != -1){
        //Ennemi en dessous
        if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else{
          a = NONE;
        }
      }

      //Ennemis à une distance de 2 du runner :
      if(x > 1 && enemies[y][x-2] != -1){
        //Ennemi 2 cases à gauche du runner
        //Pas de dépassements dans enemies par paresse de l'opérateur &&
        if(is_possible_action(level, BOMB_LEFT, x, y, enemies, bombs)){
          a = BOMB_LEFT;
        }
        else if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else{
          a = NONE;
        }
      }
      else if(x < level.xsize - 2 && enemies[y][x+2] != -1){
        //Ennemi 2 cases à droite du runner
        //Pas de dépassements dans enemies par paresse de l'opérateur &&
        if(is_possible_action(level, BOMB_RIGHT, x, y, enemies, bombs)){
          a = BOMB_RIGHT;
        }
        else if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else{
          a = NONE;
        }
      }
      else if(y > 1 && enemies[y-2][x] != -1){
        //Ennemi 2 cases en haut du runner
        //Pas de dépassements dans enemies par paresse de l'opérateur &&
        if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else{
          a = NONE;
        }
      }
      else if(y < level.ysize - 2 && enemies[y+2][x] != -1){
        //Ennemi 2 cases en bas du runner
        //Pas de dépassements dans enemies par paresse de l'opérateur &&
        if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else{
          a = NONE;
        }
      }
      else if(enemies[y-1][x-1] != -1){
        //Ennemi en haut à gauche du runner
        if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else{
          a = NONE;
        }
      }
      else if(enemies[y-1][x+1] != -1){
        //Ennemi en haut à droite du runner
        if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else if(is_possible_action(level, DOWN, x, y, enemies, bombs)){
          a = DOWN;
        }
        else{
          a = NONE;
        }
      }
      else if(enemies[y+1][x-1] != -1){
        //Ennemi en bas à gauche du runner
        if(is_possible_action(level, RIGHT, x, y, enemies, bombs)){
          a = RIGHT;
        }
        else if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else{
          a = NONE;
        }
      }
      else if(enemies[y+1][x+1] != -1){
        //Ennemi en bas à droite du runner
        if(is_possible_action(level, LEFT, x, y, enemies, bombs)){
          a = LEFT;
        }
        else if(is_possible_action(level, UP, x, y, enemies, bombs)){
          a = UP;
        }
        else{
          a = NONE;
        }
      }
    }
    free_int_list(threats);
  }

  ok= true;


  free_int_matrix(plots, level.ysize);
  free_int_matrix(ladders, level.ysize);
  free(ladders_x_position);
  free(plots_y_position);
  free_int_list_matrix(plots_accessibility_graph, nb_plots, nb_plots);
  free_int_list(enemies_list);
  free_int_matrix(enemies, level.ysize);
  free_int_matrix(bombs, level.ysize);

  do
  {
    if(DEBUG) // only when the game is in debug mode
    {
      printf("[Player] Candidate action ");
      print_action(a);
      if(ok) 
        printf(" is valid"); 
      else 
        printf(" not valid");
      printf(".\n");
    }
  } while (!ok);

  return a; // action to perform
}

/*
  Procedure that print the action name based on its enum type value
  Input:
    - the action a to print
*/
void print_action(action a)
{
  switch (a)
  {
  case NONE:
    printf("NONE");
    break;
  case UP:
    printf("UP");
    break;
  case DOWN:
    printf("DOWN");
    break;
  case LEFT:
    printf("LEFT");
    break;
  case RIGHT:
    printf("RIGHT");
    break;
  case BOMB_LEFT:
    printf("BOMB_LEFT");
    break;
  case BOMB_RIGHT:
    printf("BOMB_RIGHT");
    break;
  }
}