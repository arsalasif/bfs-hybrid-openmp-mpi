/* 
 * Simple filing
 */

#include <stdio.h>

void load(int **graph, char* file_name)
{       
       FILE *load;
       load = fopen(file_name, "r");
       if (load == NULL) 
            printf("\nFile cannot be opened.\n");
       else { 
           // get name of graph
           char name[100];
		   fgets(name,100,load);
           //printf("Name: %s\n", name);
           char vertices[10];
		   fgets(vertices,10,load);
          // While program doesn't reach end of file
          while(!feof(load)){ 
           // Taking input from file in new record
           // Have to make the \n character null because fgets gets \n from file whereas gets ignores \n when taking input from user
           char input_line[1000];
		   fgets(input_line,1000,load);
           char *edge;
           /* get the first token */
           edge = strtok(input_line, " ");
           int u = atoi(edge);
           edge = strtok(NULL, " ");
            /* walk through other tokens */
           while( edge != NULL ) {
                int v = atoi(edge);
                graph[u-1][v-1] = 1;
                graph[v-1][u-1] = 1;
                edge = strtok(NULL, " ");
           } 
         }
      }
      fclose(load);
}


int get_n(char* file_name)
{       
       int n;
       FILE *load;
       load = fopen(file_name, "r");
       if (load == NULL) 
            printf("\nFile cannot be opened.\n");
       else { 
           // get name of graph
           char name[100];
		   fgets(name,100,load);
           char vertices[10];
		   fgets(vertices,10,load);
           // get n = number of vertices of graph
           n = atoi(vertices);
      }
      fclose(load);
      return n;
}

