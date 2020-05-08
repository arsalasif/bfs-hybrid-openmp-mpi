/* 
 * Imports
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include "filing.c"
#include "queue.c"

/* Constants */

#define PRINT_VECS 1  // flag so we can turn off printing when N is large
#define MAX_RAND 100  // max value of elements generated for array

/* Prototypes */
void init_vec(int *vec, int len);
void print_vec(const char *label, int *vec, int len);

/* Functions */

// Fills a vector with random integers in the range [0, MAX_RAND)
void init_vec(int *vec, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        vec[i] = rand() % MAX_RAND;
    }    
}

// Prints the given vector to stdout
void print_vec(const char *label, int *vec, int len)
{
#if PRINT_VECS
    printf("%s", label);
    
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%d ", vec[i]);
    }
    printf("\n\n");
#endif
}


void bfs_sequential(int** graph, int source, int n)
{

    int visited[n];
    struct Queue* q = createQueue();
    for(int i = 0; i < n; i++)
        visited[i] = 0;
    
    visited[source] = 1;
    enqueue(q, source);
    
    while(!isEmpty(q)){
       int u = dequeue(q);
    
       for(int v = 0; v < n; v++)
       {
            if(graph[u][v] == 1)
            {
                if(visited[v] == 0){
                    visited[v] = 1;
                    enqueue(q, v);
                }
            }
       }
    }
}


void bfs_sequential_top_down(int** graph, int source, int n)
{
    int parent[n];
    for(int i = 0; i < n; i++)
        parent[i] = -1;
    parent[source] = source;

    struct Queue* frontier = createQueue();
    enqueue(frontier, source);

    struct Queue* next = NULL;
    
    while(frontier != NULL)
    {
        while(!isEmpty(frontier)){
            int u = dequeue(frontier);
            
            for(int v = 0; v < n; v++)
            {
                if(graph[u][v] == 1)
                {
                    if(next == NULL)
                    {
                        next = createQueue();
                    }
                    if(parent[v] == -1)
                    {
                        enqueue(next, v);
                        parent[v] = u;
                    }
                }
            }
        }
        frontier = next; next = NULL;
    }
}




void bfs_sequential_bottom_up(int** graph, int source, int n)
{
    int parent[n];
    for(int i = 0; i < n; i++)
        parent[i] = -1;
    parent[source] = source;

    struct Queue* frontier = createQueue();
    enqueue(frontier, source);

    struct Queue* next = NULL;
    
    while(frontier != NULL)
    {
        for(int u = 0; u < n; u++)
        {
            if(parent[u] == -1)
            {
                for(int v = 0; v < n; v++)
                {
                    if(graph[u][v] == 1 && isVInQueue(frontier, v) == 1)
                    {
                        if(next == NULL)
                        {
                            next = createQueue();
                        }
                        enqueue(next, u);
                        parent[u] = v;
                        break;
                    }
                }
            }

        }
        frontier = next; next = NULL;
    }
}

int main(int argc, char *argv[])
{
    // Declare process-related vars
    // and initialize MPI
    int rank;
    int num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //grab this process's rank
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); //grab the total num of processes

    int** graph;
    
    double start_time;
    double stop_time;
    
    char* file_name = "Sparse500.txt";
    int n = get_n(file_name);
    // init graph nxn matrix
    graph = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
            graph[i] = (int *)malloc(n * sizeof(int));
            for (int j = 0; j < n; j++)
            {
                graph[i][j] = 0;
            }

    } 

    
    load(graph, file_name);

    if(!rank)
    {
        int edges = 0;
        for (int i = 0; i < n; i++)
        {
                for (int j = 0; j < n; j++)
                {
                    if(graph[i][j] == 1)
                        edges++;
                }

        }
        printf("Edges: %d\n", edges);
        printf("Procs: %d\n", num_procs);
        printf("File: %s\n", file_name);
    }
    srand(time(NULL));
    start_time = MPI_Wtime();

    int chunk_size = n / num_procs;
    int owner[n];
    // assign first chunk to proc 0
    for(int i = 0; i < chunk_size; i++)
    {
            owner[i] = 0;
    }
    // assign othe chunks to respective processors
    int curr_owner = 0;
    for(int i = chunk_size; i < n; i++)
    {
            if(i % chunk_size == 0 && curr_owner < num_procs - 1)
                curr_owner++;
            owner[i] = curr_owner;
    }

    int source = 0;
    int visited[n];
    // init visited array for each proc
    for(int i = 0; i < n; i++)
        visited[i] = 0;
    // init queue for each proc
    struct Queue* frontier = createQueue();
    // if proc 0, enqueue root
   if(rank == 0) 
   {
        for(int v = 0; v < n; v++)
        {
            if(graph[source][v] == 1)
            {
                enqueue(frontier, v);
            }
        }
        visited[source] = 1;
    }       

    // init 2D send buffer for each proc
    int **sendBuf = (int **)malloc(num_procs * sizeof(int *));
    for (int i = 0; i < num_procs; i++)
    {
         sendBuf[i] = (int *)malloc(n * sizeof(int));
         for(int j = 0; j < n; j++)
            sendBuf[i][j] = 0;
    }

    // init 1D recv buffer for each proc
    int* recvBuf = (int *)malloc(n * sizeof(int));
    int sendTo[n];
    for(int i = 0; i < n; i++)
        sendTo[i] = 0;

    // BFS
    while(frontier != NULL)
    {
        struct Queue* remote = createQueue();
        struct Queue* local = createQueue();
        // set local and remote vertices
        assignLocalAndRemoteVertices(local, remote, frontier, rank, owner);
        free(frontier);
        frontier = NULL;
        // add vertices from remote to send buf
        while(!isEmpty(remote))
        {
            int v = dequeue(remote);
            sendBuf[owner[v]][v] = 1;
            sendTo[owner[v]] = 1;
        }

        // send respective remote vertices
        MPI_Request request;
        for(int proc = 0; proc < num_procs; proc++)
        {
            if(proc != rank && sendTo[proc] == 1)
            {
                MPI_Isend(sendBuf[proc], n, MPI_INT, proc, 1, MPI_COMM_WORLD, &request);
                sendTo[proc] = 0;
            }
        }

        // process local vertices
        while(!isEmpty(local))
        {
            int v = dequeue(local);
            if(visited[v] == 0)
            {
                int u;
                #pragma omp parallel for private(u)
                for(u = 0; u < n; u++)
                    if(graph[v][u] == 1)
                    {
                        #pragma omp critical
                        {
                            if(frontier == NULL)
                            {
                                frontier = createQueue();
                            }
                            enqueue(frontier, u);
                        }
                    }
                visited[v] = 1;
            }
        } 

        // recv vertices from remote and enqueue neighbours in frontier
        MPI_Status status;
        MPI_Recv(recvBuf, n, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int j;
        #pragma omp parallel for private(j)
        for(j = 0; j < n; j++)
        {
            if(recvBuf[j] == 1)
            {
                if(visited[j] == 0)
                {
                    for(int k = 0; k < n; k++)
                    {
                        if(graph[j][k] == 1)
                        {
                            #pragma omp critical
                            {
                                if(frontier == NULL)
                                {
                                    frontier = createQueue();
                                }
                                enqueue(frontier, k);
                            }
                        }
                    }

                    visited[j] = 1;
                }
                
            }
        }
    } 

    // wait for all procs to finish -- so execution time noted is correct
    MPI_Barrier(MPI_COMM_WORLD);

    if (!rank)
    {  
        stop_time = MPI_Wtime();
        printf("Total time (sec): %f\n", stop_time - start_time);
        start_time = MPI_Wtime();
        bfs_sequential(graph, 0, n);
        stop_time = MPI_Wtime();
        printf("Total time BFS_Sequential (sec): %f\n", stop_time - start_time);
        printf("BFS TOP DOWN\n");
        start_time = MPI_Wtime();
        bfs_sequential_top_down(graph, 0, n);
        stop_time = MPI_Wtime();
        printf("Total time BFS_Sequential Top-Down (sec): %f\n", stop_time - start_time);
        printf("BFS BOTTOM UP\n");
        start_time = MPI_Wtime();
        bfs_sequential_bottom_up(graph, 0, n);
        stop_time = MPI_Wtime();
        printf("Total time BFS_Sequential Bottom-Up (sec): %f\n", stop_time - start_time);
    }


    // Shutdown MPI (important - don't forget!)
    MPI_Finalize();
    
    return EXIT_SUCCESS;;
}
