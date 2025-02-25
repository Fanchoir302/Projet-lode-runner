#include "int_list.h"

int distance(int, int, int, int);
bonus closest_bonus(bonus_list, int, int);
int** create_and_initialize_int_matrix(int, int, int);
int* create_and_initialize_int_array(int, int);
int_list*** create_and_initialize_int_list_matrix(int, int);
void free_int_matrix(int**, int);
bool is_possible_action(levelinfo, action, int, int, int**, int**);
bool is_plot_cable(levelinfo, int, int);
bool is_plot(levelinfo, int, int);
int define_plots(levelinfo, int**);
int identify_ladders(levelinfo, int**, int**);
void identify_ladders_x_position(levelinfo, int*, int**);
void identify_plots_y_position(levelinfo, int*, int**);
void build_plots_accessibility_graph(levelinfo, int_list***, int, int**, int**, int*);
int_list* is_accessible_from(int_list***, int, int);
int weight(int_list***, int*, int, int);
void initialize_best_x_path_and_practical_weight(int*, int*, int_list***, int**, int*, int, int, int, int);
int find_practical_weight(int_list***, int**, int*, int, int, int, int);
int find_best_x_path(int_list***, int**, int*, int, int, int, int);
int find_vertex_minimum_distance(int_list*, int*);
int addition_int_max(int, int);
int find_middle_of_plot(levelinfo, int**, int*, int);
int find_x_position_first_ladder(levelinfo, int_list***, int**, int*, int, int, int, int, int);
void find_exit(levelinfo, int*, int*);
int find_plot_under_exit(int**, int);
int get_identifier_from_position(levelinfo, int, int);
int get_x_position_from_identifier(levelinfo, int);
int get_y_position_from_identifier(levelinfo, int);
void update_plots_accessibility_graph_with_enemies(levelinfo, int_list*, int_list***, int**, int**);
int find_closest_enemy(levelinfo, int_list*, int, int);
void identify_enemies(levelinfo, int**, int_list*);
void identify_bombs(int**, bomb_list);
int count_threats_to_runner(levelinfo, int_list**, int**, int, int);
void print_action(action);