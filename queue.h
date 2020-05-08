/* 
 * Queue class
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#define SIZE 1000

struct Queue {
    int items[SIZE];
    int front;
    int rear;
};


struct Queue* createQueue();
void enqueue(struct Queue* q, int);
int dequeue(struct Queue* q);
void display(struct Queue* q);
int isEmpty(struct Queue* q);
void printQueue(struct Queue* q);
int isVInQueue(struct Queue *q, int v);
void assignLocalAndRemoteVertices(struct Queue *local, struct Queue *remote, struct Queue *q, int rank, int owner[]);


#endif