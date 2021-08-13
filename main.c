#include <stdlib.h>
#include <stdio.h>

// -----------   STRUCTS   -----------

struct queue_node {
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
    struct tree_node *parent;
    struct stack_node *stack;
    char color;
};

struct {
    unsigned long id;
    unsigned long graph_size;
    unsigned long *adj_matrix;
    unsigned long queue_size;
    struct queue_node **queue;
    unsigned long *queue_position;
    unsigned long *result;
    unsigned long length;
} dijkstra = {.id = 0,
              .length = 0};

struct {
    unsigned long max;
    unsigned long size;
    unsigned long size_curr;
    struct tree_node *root;
    struct tree_node *nil;
    struct tree_node *max_position;
    unsigned long printed;
} leaderboard = {.max = 0,
                 .size_curr = 0};

// -----------   QUEUE   -----------

unsigned long queue_parent(unsigned long index) {
    return (index - 1) / 2;
}

unsigned long queue_left(unsigned long index) {
    return (2 * index + 1);
}

unsigned long queue_right(unsigned long index) {
    return (2 * index + 2);
}

struct queue_node * new_queue_node(unsigned long node, unsigned long cost) {
    struct queue_node *temp = (struct queue_node *) malloc(sizeof(struct queue_node));
    temp->node = node;
    temp->cost = cost;
    return temp;
}

void queue_swap(unsigned long a, unsigned long b) {
    struct queue_node *temp = dijkstra.queue[a];
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

void queue_insert(struct queue_node *queue_node) {
    if (dijkstra.queue_size != 0 && (dijkstra.queue_position[queue_node->node] != 0 || dijkstra.queue[0]->node == queue_node->node)) {
        if (queue_node->cost < dijkstra.queue[dijkstra.queue_position[queue_node->node]]->cost)
            queue_decrease(dijkstra.queue_position[queue_node->node], queue_node->cost);
        return;
    }

    dijkstra.queue_size++;
    unsigned long index = dijkstra.queue_size - 1;
    dijkstra.queue[index] = queue_node;
    dijkstra.queue_position[queue_node->node] = index;

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

struct queue_node * queue_extract_root() {
    if (dijkstra.queue_size == 0) {
        printf("ERROR: QUEUE IS EMPTY");
        return NULL;
    }

    struct queue_node *temp = dijkstra.queue[0];

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

// -----------   STACK   -----------

void push (struct stack_node **stack, unsigned long id) {
    struct stack_node *temp = (struct stack_node *) malloc(sizeof(struct stack_node));
    temp->graph_id = id;
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

struct tree_node * new_tree_node(unsigned long id, unsigned long length, struct tree_node *parent) {
    struct tree_node *temp = (struct tree_node *) malloc(sizeof(struct tree_node));
    temp->length = length;
    temp->left = leaderboard.nil;
    temp->right = leaderboard.nil;
    temp->parent = parent;
    temp->stack = NULL;
    temp->color = 'R';
    push(&temp->stack, id);
    return temp;
}

struct tree_node * tree_max(struct tree_node **tree) {
    if (*tree == leaderboard.nil)
        return leaderboard.nil;
    if ((*tree)->right == leaderboard.nil) {
        return *tree;
    }
    else return tree_max(&(*tree)->right);
}

void left_rotate(struct tree_node *node) {
    struct tree_node *right = node->right;
    node->right = right->left;
    if (right->left != leaderboard.nil)
        right->left->parent = node;
    right->parent = node->parent;
    if (node->parent == leaderboard.nil)
        leaderboard.root = right;
    else if (node == node->parent->left)
        node->parent->left = right;
    else node->parent->right = right;
    right->left = node;
    node->parent = right;
}

void right_rotate(struct tree_node *node) {
    struct tree_node* left = node->left;
    node->left = left->right;
    if (left->right != leaderboard.nil)
        left->right->parent = node;
    left->parent = node->parent;
    if (node->parent == leaderboard.nil)
        leaderboard.root = left;
    else if (node == node->parent->left)
        node->parent->left = left;
    else node->parent->right = left;
    left->right = node;
    node->parent = left;
}

void fix_insertion(struct tree_node *node) {
    struct tree_node *uncle;
    while (node->parent->color == 'R') {
        if (node->parent == node->parent->parent->left) {
            uncle = node->parent->parent->right;
            if (uncle->color == 'R') {
                node->parent->color = 'B';
                uncle->color = 'B';
                node->parent->parent->color = 'R';
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    left_rotate(node);
                }
                node->parent->color = 'B';
                node->parent->parent->color = 'R';
                right_rotate(node->parent->parent);
            }
        } else {
            uncle = node->parent->parent->left;
            if (uncle->color == 'R') {
                node->parent->color = 'B';
                uncle->color = 'B';
                node->parent->parent->color = 'R';
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    right_rotate(node);
                }
                node->parent->color = 'B';
                node->parent->parent->color = 'R';
                left_rotate(node->parent->parent);
            }
        }
    }
    leaderboard.root->color = 'B';
}

void fix_deletion(struct tree_node *node) {
    struct tree_node *sibling;
    while (node != leaderboard.root && node->color == 'B') {
        if (node == node->parent->left) {
            sibling = node->parent->right;
            if (sibling->color == 'R') {
                sibling->color = 'B';
                node->parent->color = 'R';
                left_rotate(node->parent);
                sibling = node->parent->right;
            }
            if (sibling->left->color == 'B' && sibling->right->color == 'B') {
                sibling->color = 'R';
                node = node->parent;
            } else {
                if (sibling->right->color == 'B') {
                    sibling->left->color = 'B';
                    sibling->color = 'R';
                    right_rotate(sibling);
                    sibling = node->parent->right;
                }
                sibling->color = node->parent->color;
                node->parent->color = 'B';
                sibling->right->color = 'B';
                left_rotate(node->parent);
                node = leaderboard.root;
            }
        } else {
            sibling = node->parent->left;
            if (sibling->color == 'R') {
                sibling->color = 'B';
                node->parent->color = 'R';
                right_rotate(node->parent);
                sibling = node->parent->left;
            }
            if (sibling->left->color == 'B' && sibling->right->color == 'B') {
                sibling->color = 'R';
                node = node->parent;
            } else {
                if (sibling->left->color == 'B') {
                    sibling->right->color = 'B';
                    sibling->color = 'R';
                    left_rotate(sibling);
                    sibling = node->parent->left;
                }
                sibling->color = node->parent->color;
                node->parent->color = 'B';
                sibling->left->color = 'B';
                right_rotate(node->parent);
                node = leaderboard.root;
            }
        }
    }
    node->color = 'B';
}

void remove_max(struct tree_node *max) {
    pop(&max->stack);
    if (max->stack == NULL) {
        struct tree_node *temp = max;
        struct tree_node *parent = max->parent;
        struct tree_node *left_max = tree_max(&max->left);
        if (parent->length > left_max->length) {
            leaderboard.max = parent->length;
            leaderboard.max_position = parent;
        } else {
            leaderboard.max = left_max->length;
            leaderboard.max_position = left_max;
        }
        if (max->parent == leaderboard.nil) {
            if (max->left != leaderboard.nil) {
                max->left->color = 'B';
                max->left->parent = leaderboard.nil;
            }
            leaderboard.root = max->left;
        } else {
            max->parent->right = max->left;
            max->left->parent = max->parent;
            if (max->color == 'B')
                fix_deletion(max->left);
        }
        free(temp);
    }
}

void insert(struct tree_node **tree, unsigned long id, unsigned long length, struct tree_node *parent) {
    if (*tree == leaderboard.nil) {
        *tree = new_tree_node(id, length, parent);
        if (length > leaderboard.max) {
            leaderboard.max = length;
            leaderboard.max_position = *tree;
        }
        fix_insertion(*tree);
        return;
    }

    if ((*tree)->length > length)
        insert(&(*tree)->left, id, length, *tree);
    else if ((*tree)->length < length)
        insert(&(*tree)->right, id, length, *tree);
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
        if (leaderboard.printed < leaderboard.size_curr) {
            printf("%lu", stack->graph_id);
            leaderboard.printed++;
        }
        if (leaderboard.printed < leaderboard.size_curr)
            printf(" ");
        print_stack(stack->next);
    }
}

void print_top(struct tree_node *tree) {
    if (tree != leaderboard.nil) {
        print_top(tree->left);
        print_stack(tree->stack);
        print_top(tree->right);
    }
}

// -----------   OTHERS   -----------

void reset() {
    dijkstra.length = 0;
    dijkstra.id++;
}

void setup() {
    dijkstra.adj_matrix = (unsigned long *) malloc(dijkstra.graph_size * dijkstra.graph_size * sizeof(unsigned long));
    dijkstra.queue = (struct queue_node **) malloc(dijkstra.graph_size * sizeof(struct queue_node *));
    dijkstra.queue_position = (unsigned long *) calloc(dijkstra.graph_size, sizeof(unsigned long));
    dijkstra.result = (unsigned long *) malloc(dijkstra.graph_size * sizeof(unsigned long));
    leaderboard.nil = (struct tree_node *) malloc(sizeof(struct tree_node));
    leaderboard.nil->color = 'B';
    leaderboard.nil->parent = NULL;
    leaderboard.nil->left = NULL;
    leaderboard.nil->right = NULL;
    leaderboard.nil->length = 0;
    leaderboard.root = leaderboard.nil;
}

void parse_dimensions() {
    char input[22];
    char *pointer;
    if (fgets(input, 22, stdin) == NULL)
        printf("error");
    dijkstra.graph_size = strtoul(input, &pointer, 10);
    leaderboard.size = strtoul(pointer, NULL, 10);
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
    if (leaderboard.size_curr < leaderboard.size) {
        insert(&leaderboard.root, dijkstra.id, dijkstra.length, leaderboard.nil);
        leaderboard.size_curr++;
    } else if (dijkstra.length < leaderboard.max) {
        remove_max(leaderboard.max_position);
        insert(&leaderboard.root, dijkstra.id, dijkstra.length, leaderboard.nil);
    }
}

void compute_dijkstra() {
    dijkstra.queue_size = 0;
    dijkstra.result[0] = 0;
    queue_insert(new_queue_node(0, 0));
    for (int i = 1; i < dijkstra.graph_size; i++) {
        dijkstra.result[i] = 4294967295;
        queue_insert(new_queue_node(i, dijkstra.result[i]));
    }

    while (dijkstra.queue_size != 0) {
        struct queue_node *root = queue_extract_root();
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
        leaderboard.printed = 0;
        print_top(leaderboard.root);
        printf("\n");
    }
    return 1;
}

int main() {
    parse_dimensions();

    while (parse_command()) {}
}
