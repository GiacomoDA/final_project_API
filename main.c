#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct queue_element {
    unsigned long node;
    unsigned long previous;
    unsigned long cost;
};

struct {
    int id;
    unsigned long graph_size;
    unsigned long *adj_matrix;
    unsigned long long length;
    struct queue_element **queue;
    unsigned long queue_size;
    unsigned long *queue_position;
    unsigned long long *result;
} dijkstra = {.id = 0,
              .adj_matrix = NULL,
              .length = 0};

struct {
    unsigned long long min;
    unsigned long long max;
    unsigned long size;
} ranking;

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

struct queue_element * new_queue_element(unsigned long node, unsigned long previous, unsigned long cost) {
    struct queue_element *temp = (struct queue_element *) malloc(sizeof(struct queue_element));
    temp->node = node;
    temp->previous = previous;
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

void queue_decrease(unsigned long index, unsigned long new_prev, unsigned long new_cost) {
    dijkstra.queue[index]->previous = new_prev;
    dijkstra.queue[index]->cost = new_cost;
    while (index != 0 && dijkstra.queue[queue_parent(index)]->cost > dijkstra.queue[index]->cost) {
        queue_swap(queue_parent(index), index);
        index = queue_parent(index);
    }
}

void queue_insert(struct queue_element *queue_element) {
    if (dijkstra.queue_size != 0 && (dijkstra.queue_position[queue_element->node] != 0 || dijkstra.queue[0]->node == queue_element->node)) {
        if (queue_element->cost < dijkstra.queue[dijkstra.queue_position[queue_element->node]]->cost)
            queue_decrease(dijkstra.queue_position[queue_element->node], queue_element->previous, queue_element->cost);
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
            queue_insert(new_queue_element(i, row, *(dijkstra.adj_matrix + row * dijkstra.graph_size + i) + dijkstra.result[row]));
}

// -----------   PRINTERS   -----------

void queue_print() {
    for (unsigned long i = 0; i < dijkstra.queue_size; i++)
        printf("node: %lu previous: %lu cost: %lu\n", dijkstra.queue[i]->node, dijkstra.queue[i]->previous, dijkstra.queue[i]->cost);
//    for (unsigned long i = 1; i <= dijkstra.queue_size; i++)
//        printf("%lu: %lu\n", i, dijkstra.queue_position[i]);
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
    for (unsigned long i = 1; i < dijkstra.graph_size; i++)
        printf("%lu:%llu ", i, dijkstra.result[i]);
    printf("\n");
}

// -----------   OTHERS   -----------

void result_reset() {
    for (int i = 1; i < dijkstra.graph_size; i++)
        dijkstra.result[i] = 0;
}

void setup() {
    dijkstra.adj_matrix = (unsigned long *) malloc(dijkstra.graph_size * dijkstra.graph_size * sizeof(unsigned long));
    dijkstra.queue = (struct queue_element **) malloc(dijkstra.graph_size * sizeof(struct queue_element *));
    dijkstra.queue_position = (unsigned long *) calloc(dijkstra.graph_size, sizeof(unsigned long));
    dijkstra.result = (unsigned long long *) calloc(dijkstra.graph_size, sizeof(unsigned long long));
}

// -----------   PARSERS AND MAIN   -----------

void parse_dimensions() {
    fscanf(stdin, "%lu", &dijkstra.graph_size);
    fscanf(stdin, "%lu\n", &ranking.size);
    setup();
}

void parse_matrix() {
    for (int i = 0; i < dijkstra.graph_size; i++) {
        for (int j = 0; j < dijkstra.graph_size; j++) {
            fscanf(stdin, "%lu,", dijkstra.adj_matrix + dijkstra.graph_size * i + j);
        }
        fgets(NULL, 1, stdin);
    }
}

void compute_dijkstra() {
    dijkstra.queue_size = 0;
    queue_scan_row(0);

    while (dijkstra.queue_size != 0) {
        //queue_print();
        struct queue_element *root = queue_extract_root();
        if (dijkstra.result[root->node] == 0 || root->cost < dijkstra.result[root->node]) {
            dijkstra.result[root->node] = root->cost;
            queue_scan_row(root->node);
        }
        free(root);
    }
}

void parse_command() {
    char input[15];
    //memset(input, 0, 14);
    fgets(input, 15, stdin);
    input[strcspn(input, "\n\r")] = 0;
    if (!strcmp(input, "AggiungiGrafo")) {
        parse_matrix();
        compute_dijkstra();
        print_result();
        //add to ranking
        result_reset();
        dijkstra.id++;
    } else if (!strcmp(input, "TopK")) {
        //print ranking
    }
}

int main() {
    parse_dimensions();

    while (1) {
        parse_command();
    }
}
