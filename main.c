#include <stdlib.h>
#include <stdio.h>

struct queue_element {
    unsigned long node;
    unsigned long cost;
};

struct stack_node {
    unsigned long graph_id;
    struct stack_node *next;
};

struct tree_node {
    unsigned long length;
    struct tree_node *left;
    struct tree_node *right;
    struct stack_node *stack;
};

struct {
    unsigned long id;
    unsigned long graph_size;
    unsigned long *adj_matrix;
    unsigned long queue_size;
    struct queue_element **queue;
    unsigned long *queue_position;
    unsigned long *result;
    unsigned long length;
} dijkstra = {.id = 0,
              .length = 0};

struct {
    unsigned long max;
    unsigned long size;
    unsigned long size_curr;
    struct tree_node *ranking;
    unsigned long printed;
} ranking = {.max = 0,
             .size_curr = 0,
             .ranking = NULL};

// -----------   QUEUE METHODS   -----------

unsigned long queue_parent(unsigned long index) {
    return (index - 1) / 2;
}

unsigned long queue_left(unsigned long index) {
    return (2 * index + 1);
}

unsigned long queue_right(unsigned long index) {
    return (2 * index + 2);
}

struct queue_element * new_queue_element(unsigned long node, unsigned long cost) {
    struct queue_element *temp = (struct queue_element *) malloc(sizeof(struct queue_element));
    temp->node = node;
    temp->cost = cost;
    return temp;
}

void queue_swap(unsigned long a, unsigned long b) {
    struct queue_element *temp = dijkstra.queue[a];
    dijkstra.queue[a] = dijkstra.queue[b];
    dijkstra.queue[b] = temp;
    dijkstra.queue_position[dijkstra.queue[a]->node] = a;
    dijkstra.queue_position[dijkstra.queue[b]->node] = b;
}

void queue_decrease(unsigned long index, unsigned long new_cost) {
    dijkstra.queue[index]->cost = new_cost;
    while (index != 0 && dijkstra.queue[queue_parent(index)]->cost > dijkstra.queue[index]->cost) {
        queue_swap(queue_parent(index), index);
        index = queue_parent(index);
    }
}

void queue_insert(struct queue_element *queue_element) {
    if (dijkstra.queue_size != 0 && (dijkstra.queue_position[queue_element->node] != 0 || dijkstra.queue[0]->node == queue_element->node)) {
        if (queue_element->cost < dijkstra.queue[dijkstra.queue_position[queue_element->node]]->cost)
            queue_decrease(dijkstra.queue_position[queue_element->node], queue_element->cost);
        return;
    }

    dijkstra.queue_size++;
    unsigned long index = dijkstra.queue_size - 1;
    dijkstra.queue[index] = queue_element;
    dijkstra.queue_position[queue_element->node] = index;

    while (index != 0 && dijkstra.queue[index]->cost < dijkstra.queue[queue_parent(index)]->cost) {
        queue_swap(index, queue_parent(index));
        index = queue_parent(index);
    }
}

void queue_heapify(unsigned long index) {
    unsigned long left = queue_left(index);
    unsigned long right = queue_right(index);
    unsigned long temp = index;
    if (left < dijkstra.queue_size && dijkstra.queue[left]->cost < dijkstra.queue[index]->cost)
        temp = left;
    if (right < dijkstra.queue_size && dijkstra.queue[right]->cost < dijkstra.queue[temp]->cost)
        temp = right;
    if (temp != index) {
        queue_swap(index, temp);
        queue_heapify(temp);
    }
}

struct queue_element * queue_extract_root() {
    if (dijkstra.queue_size == 0) {
        printf("ERROR: QUEUE IS EMPTY");
        return NULL;
    }

    struct queue_element *temp = dijkstra.queue[0];

    if (dijkstra.queue_size == 1) {
        dijkstra.queue_size--;
        return temp;
    }

    dijkstra.queue[0] = dijkstra.queue[dijkstra.queue_size - 1];
    dijkstra.queue_position[dijkstra.queue[dijkstra.queue_size - 1]->node] = 0;
    dijkstra.queue_size--;
    queue_heapify(0);
    return temp;
}

void queue_scan_row(unsigned long row) {
    for (int i = 1; i < dijkstra.graph_size; i++)
        if (*(dijkstra.adj_matrix + row * dijkstra.graph_size + i) != 0 && i != row)
            queue_insert(new_queue_element(i, *(dijkstra.adj_matrix + row * dijkstra.graph_size + i) + dijkstra.result[row]));
}

// -----------   STACK   -----------

void push (struct stack_node **stack, unsigned long graph_id) {
    struct stack_node *temp = (struct stack_node *) malloc(sizeof(struct stack_node));
    temp->graph_id = graph_id;
    temp->next = *stack;
    *stack = temp;
}

void pop(struct stack_node **stack) {
    if (*stack == NULL)
        return;

    struct stack_node *temp = *stack;
    *stack = (*stack)->next;
    free(temp);
}

// -----------   TREE   -----------

struct tree_node * new_tree_node(unsigned long id, unsigned long length) {
    struct tree_node *temp = (struct tree_node *) malloc(sizeof(struct tree_node));
    temp->length = length;
    temp->left = NULL;
    temp->right = NULL;
    temp->stack = NULL;
    push(&temp->stack, id);
    return temp;
}

struct tree_node * search(struct tree_node **tree, unsigned long length) {
    if ((*tree) == NULL)
        return NULL;
    if ((*tree)->length == length)
        return *tree;
    return (*tree)->length > length ? search(&(*tree)->left, length) : search(&(*tree)->right, length);
}

unsigned long max(struct tree_node **tree) {
    if (*tree == NULL)
        return 0;
    if ((*tree)->right == NULL)
        return (*tree)->length;
    else return max(&(*tree)->right);
}

void remove_max(struct tree_node **tree) {
    if (*tree == NULL)
        return;
    if ((*tree)->right == NULL) {
        pop(&(*tree)->stack);
        if ((*tree)->stack == NULL)
            *tree = (*tree)->left;
    } else remove_max(&(*tree)->right);
}

unsigned long get_max(struct tree_node *tree) {
    if (tree == NULL)
        return 0;
    if (tree->right == NULL)
        return tree->length;
    else return get_max(tree->right);
}

void insert(struct tree_node **tree, unsigned long id, unsigned long length) {
    if (*tree == NULL) {
        *tree = new_tree_node(id, length);
        return;
    }

    if ((*tree)->length > length)
        insert(&(*tree)->left, id, length);
    else if ((*tree)->length < length)
        insert(&(*tree)->right, id, length);
    else push(&(*tree)->stack, id);
}

// -----------   PRINTERS   -----------

void queue_print() {
    for (unsigned long i = 0; i < dijkstra.queue_size; i++)
        printf("node: %lu cost: %lu\n", dijkstra.queue[i]->node, dijkstra.queue[i]->cost);
    for (unsigned long i = 1; i <= dijkstra.queue_size; i++)
        printf("%lu: %lu\n", i, dijkstra.queue_position[i]);
    printf("\n");
}

void print_matrix() {
    for (int i = 0; i < dijkstra.graph_size; i++) {
        for (int j = 0; j < dijkstra.graph_size; j++) {
            printf("%lu ", *(dijkstra.adj_matrix + dijkstra.graph_size * i + j));
        }
        printf("\n");
    }
}

void print_result() {
    setbuf(stdout, 0);
    printf("%lu: ", dijkstra.id);
    for (unsigned long i = 1; i < dijkstra.graph_size; i++)
        printf("%lu:%lu ", i, dijkstra.result[i]);
}

void print_stack(struct stack_node *stack) {
    if (stack != NULL) {
        printf("%lu", stack->graph_id);
        ranking.printed++;
        if (ranking.printed < ranking.size_curr)
            printf(" ");
        print_stack(stack->next);
    }
}

void print_ranking(struct tree_node *tree) {
    if (tree != NULL) {
        print_stack(tree->stack);
        print_ranking(tree->left);
        print_ranking(tree->right);
    }
}

// -----------   OTHERS   -----------

void reset() {
    dijkstra.length = 0;
    dijkstra.id++;
}

void setup() {
    dijkstra.adj_matrix = (unsigned long *) malloc(dijkstra.graph_size * dijkstra.graph_size * sizeof(unsigned long));
    dijkstra.queue = (struct queue_element **) malloc(dijkstra.graph_size * sizeof(struct queue_element *));
    dijkstra.queue_position = (unsigned long *) calloc(dijkstra.graph_size, sizeof(unsigned long));
    dijkstra.result = (unsigned long *) malloc(dijkstra.graph_size * sizeof(unsigned long));
}

void parse_dimensions() {
    char input[22];
    char *pointer;
    if (fgets(input, 22, stdin) == NULL)
        printf("error");
    dijkstra.graph_size = strtoul(input, &pointer, 10);
    ranking.size = strtoul(pointer, NULL, 10);
    setup();
}

void parse_matrix() {
    char input[20000];
    char *pointer;
    unsigned long *cell;
    for (int i = 0; i < dijkstra.graph_size; i++) {
        if (fgets(input, 20000, stdin) == NULL)
            printf("error");
        pointer = input;
        for (int j = 0; j < dijkstra.graph_size; j++) {
            cell = dijkstra.adj_matrix + dijkstra.graph_size * i + j;
            *cell = strtoul(pointer, &pointer, 10);
            pointer++;
        }
    }
}

int string_compare(char *a, char *b) {
    while(*a == *b) {
        if (*a == 0 && *b == 0)
            return 1;
        a++;
        b++;
    }
    return 0;
}

void add_result() {
    if (ranking.size_curr < ranking.size) {
        insert(&ranking.ranking, dijkstra.id, dijkstra.length);
        ranking.size_curr++;
        if (dijkstra.length > ranking.max)
            ranking.max = dijkstra.length;
    } else if (dijkstra.length < ranking.max) {
        remove_max(&ranking.ranking);
        insert(&ranking.ranking, dijkstra.id, dijkstra.length);
        ranking.max = get_max(ranking.ranking);
    }
}

void compute_dijkstra() {
    dijkstra.queue_size = 0;
    dijkstra.result[0] = 0;
    queue_insert(new_queue_element(0, 0));
    for (int i = 1; i < dijkstra.graph_size; i++) {
        dijkstra.result[i] = 4294967295;
        queue_insert(new_queue_element(i, dijkstra.result[i]));
    }

    while (dijkstra.queue_size != 0) {
        struct queue_element *root = queue_extract_root();
        for (int i = 1; i < dijkstra.graph_size; i++) {
            unsigned long distance = *(dijkstra.adj_matrix + root->node * dijkstra.graph_size + i);
            if (distance != 0) {
                unsigned long temp = dijkstra.result[root->node] + distance;
                if (temp < dijkstra.result[root->node] || temp < distance)
                    temp = 4294967295;
                if (temp < dijkstra.result[i]) {
                    dijkstra.result[i] = temp;
                    queue_decrease(dijkstra.queue_position[i], temp);
                }
            }
        }
        free(root);
    }
    for (int i = 1; i < dijkstra.graph_size; i++) {
        if (dijkstra.result[i] != 4294967295)
            dijkstra.length += dijkstra.result[i];
    }
}

int parse_command() {
    char input[15];
    if (fgets(input, 15, stdin) == NULL) {
        return 0;
    }
    if (string_compare(input, "AggiungiGrafo\n")) {
        parse_matrix();
        compute_dijkstra();
        add_result();
        reset();
    } else if (string_compare(input, "TopK\n")) {
        ranking.printed = 0;
        print_ranking(ranking.ranking);
        printf("\n");
    }
    return 1;
}

int main() {
    parse_dimensions();

    while (parse_command()) {}
}
