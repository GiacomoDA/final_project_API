#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct queue_element {
    unsigned long node;
    unsigned long previous;
    unsigned long cost;
};

struct result_element {
    unsigned long previous;
    unsigned long long cost;
};

struct {
    int id;
    unsigned long graph_size;
    unsigned long *adj_matrix;
    unsigned long long length;
    struct queue_element **queue;
    unsigned long queue_size;
    unsigned long *queue_position;
    struct result_element **result;
} dijkstra = {.id = 0,
              .adj_matrix = NULL,
              .length = 0};

struct {
    unsigned long long min;
    unsigned long long max;
    unsigned long size;
} ranking = {.size = 0};

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
    struct queue_element *temp = (struct queue_element *)malloc(sizeof(struct queue_element));
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

void queue_add(struct queue_element *queue_element) {
    dijkstra.queue[dijkstra.queue_size] = queue_element;
    dijkstra.queue_position[queue_element->node] = dijkstra.queue_size;
    dijkstra.queue_size++;
}

void queue_insert(struct queue_element *queue_element) {
    if (dijkstra.queue_size == dijkstra.graph_size) {
        printf("ERROR: QUEUE FULL");
        return;
    }

    dijkstra.queue_size++;
    unsigned long index = dijkstra.queue_size - 1;
    dijkstra.queue[index] = queue_element;

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

void queue_make_heap() {
    unsigned long start = dijkstra.queue_size / 2;
    while (1) {
        queue_heapify(start);
        if (start == 0)
            break;
        start--;
    }
}

void queue_decrease(unsigned long index, unsigned long new_value) {
    dijkstra.queue[index]->cost = new_value;
    while (index != 0 && dijkstra.queue[queue_parent(index)]->cost > dijkstra.queue[index]->cost) {
        queue_swap(index, queue_parent(index));
        index = queue_parent(index);
    }
}

struct queue_element * queue_root() {
    return dijkstra.queue[0];
}

void queue_delete_root() {
    if (dijkstra.queue_size == 0) {
        printf("ERROR: QUEUE IS EMPTY");
        return;
    }

    struct queue_element *temp = dijkstra.queue[0];
    free(temp);

    if (dijkstra.queue_size == 1) {
        dijkstra.queue_size--;
        return;
    }

    dijkstra.queue[0] = dijkstra.queue[dijkstra.queue_size - 1];
    dijkstra.queue_size--;
    queue_heapify(0);
}

void queue_scan_row(unsigned long row) {
    for (int i = 1; i < dijkstra.graph_size; i++)
        if (*(dijkstra.adj_matrix + sizeof(unsigned long) * i) != 0 && i != row)
            queue_insert(new_queue_element(i, row, *(dijkstra.adj_matrix + sizeof(unsigned long) * i) + dijkstra.result[i]->cost));
}

// -----------   PRINTERS   -----------

void queue_print() {
    for (unsigned long i = 0; i < dijkstra.queue_size; i++)
        printf("node: %lu previous: %lu cost: %lu\n", dijkstra.queue[i]->node, dijkstra.queue[i]->previous, dijkstra.queue[i]->cost);
    for (unsigned long i = 1; i <= dijkstra.queue_size; i++)
        printf("%lu: %lu\n", i, dijkstra.queue_position[i]);
}

void print_matrix() {
    for (int i = 0; i < dijkstra.graph_size; i++) {
        for (int j = 0; j < dijkstra.graph_size; j++) {
            printf("%lu ", *(dijkstra.adj_matrix + dijkstra.graph_size * sizeof(unsigned long) * i + sizeof(unsigned long) * j));
        }
        printf("\n");
    }
}

// -----------   OTHERS   -----------

void result_reset() {
    for (int i = 0; i < dijkstra.graph_size; i++) {
        dijkstra.result[i]->cost = 0;
        dijkstra.result[i]->previous = 0;
    }
}

void setup() {
    dijkstra.adj_matrix = (unsigned long *) malloc(dijkstra.graph_size * dijkstra.graph_size * sizeof(unsigned long));
    dijkstra.queue = (struct queue_element **) malloc(dijkstra.graph_size * sizeof(struct queue_element *));
    dijkstra.queue_position = (unsigned long *) malloc(sizeof(unsigned long) * dijkstra.graph_size);
    dijkstra.result = (struct result_element **) calloc(sizeof(struct result_element *) * dijkstra.graph_size, 0);
    for (int i = 0; i < dijkstra.graph_size; i++) {
        dijkstra.result[i] = (struct result_element *) malloc(sizeof(struct result_element));
        dijkstra.result[i]->cost = 0;
        dijkstra.result[i]->previous = 0;
    }
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
            fscanf(stdin, "%lu,", dijkstra.adj_matrix + dijkstra.graph_size * sizeof(unsigned long) * i + sizeof(unsigned long) * j);
        }
        fgets(NULL, 1, stdin);
    }
}

void compute_dijkstra() {
    dijkstra.queue_size = 0;
    for (int i = 1; i < dijkstra.graph_size; i++)
        if (*(dijkstra.adj_matrix + sizeof(unsigned long) * i) != 0)
            queue_add(new_queue_element(i, 0, *(dijkstra.adj_matrix + sizeof(unsigned long) * i)));
    queue_make_heap();

    while (dijkstra.queue_size != 0) {
        struct queue_element *root = queue_root();
        if (root->cost < dijkstra.result[root->node]->cost || dijkstra.result[root->node]->cost == 0) {
            dijkstra.result[root->node]->cost = root->cost;
            dijkstra.result[root->node]->previous = root->previous;
            queue_delete_root();
            queue_scan_row(root->node);
        }
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
        queue_print();
        //add to ranking
        result_reset();
    } else if (!strcmp(input, "TopK")) {
        print_matrix();
        //print ranking
    }
}

int main() {
    parse_dimensions();

    while (1) {
        parse_command();
    }
}
