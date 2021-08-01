#include <stdlib.h>
#include <stdio.h>

struct stack {
    int graph_index;
    struct stack *next;
};

struct stack * push(struct stack *stack, int index) {
    struct stack *temp = (struct stack *)malloc(sizeof(struct stack));
    temp->graph_index = index;
    temp->next = stack;
    stack = temp;
    return stack;
}

void pop(struct stack *stack) {
    if (stack == NULL)
        return;
    struct stack *temp = stack;
    stack = stack->next;
    free(temp);
}

struct ranking {
    int size;
    int min;
    int max;
    //ranking data structure
};

//int main() {
//    printf("Hello World");
//}