#include "queue.h"


struct Queue* createQueue() {
    struct Queue* q = malloc(sizeof(struct Queue));
    q->front = -1;
    q->rear = -1;
    return q;
}


int isEmpty(struct Queue* q) {
    if(q->rear == -1) 
        return 1;
    return 0;
}

void enqueue(struct Queue* q, int value){
    if(q->rear == SIZE-1)
        return;
    else {
        if(q->front == -1)
            q->front = 0;
        q->rear++;
        q->items[q->rear] = value;
    }
}

int dequeue(struct Queue* q){
    int item;
    if(isEmpty(q)){
        item = -1;
    }
    else{
        item = q->items[q->front];
        q->front++;
        if(q->front > q->rear){
            q->front = q->rear = -1;
        }
    }
    return item;
}

void printQueue(struct Queue *q) {
    int i = q->front;

    if(isEmpty(q)) {
    } else {
        for(i = q->front; i < q->rear + 1; i++) {
                printf("%d ", q->items[i]);
        }
    }   
    printf("\n"); 
}


int isVInQueue(struct Queue *q, int v) {
    int i = q->front;

    if(isEmpty(q)) {
    } else {
        for(i = q->front; i < q->rear + 1; i++) {
                if(v == q->items[i])
                    return 1;
        }
    }    
    return 0;
}


void assignLocalAndRemoteVertices(struct Queue *local, struct Queue *remote, struct Queue *q, int rank, int owner[]) {
    int i = q->front;

    if(isEmpty(q)) {
    } else {
        for(i = q->front; i < q->rear + 1; i++) {
                int v = q->items[i];
                if(owner[v] == rank)
                    enqueue(local, v);
                else enqueue(remote, v);
        }
    }    
}
