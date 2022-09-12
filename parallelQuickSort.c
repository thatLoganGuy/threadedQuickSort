    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <pthread.h>
    #include <unistd.h>  // sleep()
    #include <stdio.h>
    #include <stdlib.h>  // EXIT_SUCCESS
    #include <string.h>  // strerror()
    #include <errno.h>
    #include <semaphore.h> 
    #include <stdbool.h> 
    #include <sys/types.h> 
    #include <sys/syscall.h> 

    #define SIZE_OF_DATASET 10 
    
    sem_t thread_semaphore;

    void* quickSort( void* data);
    int partition( int* a, int, int);


    struct info {
        int start_index;
        int* data_set;
        int end_index;
    };



    int main(int argc, char **argv)
    {
	int max_threads = 1; 
	sem_init(&thread_semaphore, 0, max_threads);
        int a[] = { 7, 12, 1, -2, 8, 2, 0, 96, 78, 7};
	 struct info *info = malloc(sizeof(struct info));
    	info->data_set=malloc(sizeof(int)*SIZE_OF_DATASET);
    	info->data_set=a;
    	info->start_index=0;
    	info->end_index=SIZE_OF_DATASET-1;

    	quickSort(info); 
	sem_destroy(&thread_semaphore);
    	printf("Sorted array is:  ");
    	int i;
      	for(i = 0; i < SIZE_OF_DATASET; ++i)
           printf(" %d ", info->data_set[i]);
	printf("\n");
    	return 0;
    }

    void* quickSort( void *data)
    {
	struct info *info = data;
	struct info other_info;
        int j,l,r;
        l = info->start_index;
        r = info->end_index;

        pthread_attr_t attr;
        pthread_t thread_id1;
        pthread_attr_init(&attr);

       if( l < r ){
	       j = partition( info->data_set, l, r);
	       other_info.start_index=j+1;
	       other_info.end_index=r;
	       other_info.data_set = info->data_set; 
	       if(other_info.end_index<0)other_info.end_index=0;
	       sem_wait(&thread_semaphore);
	       if (pthread_create(&thread_id1, NULL, quickSort, &other_info)) {
		       fprintf(stderr, "No threads for you.\n");
		       return NULL;
	       }
	       sem_post(&thread_semaphore); 
	       info->start_index=l;
	       info->end_index=j-1;
	       if(info->end_index < 0) info->end_index = 0;
	       quickSort(info);  /* don't care about the return value */
	       pthread_join(thread_id1, NULL);
      } 
      

    return NULL;

    }


    int partition( int* a, int l, int r) {
       int pivot, i, j, t;
       pivot = a[l];
       i = l; j = r+1;

       while( 1)
       {
        do ++i; while( a[i] <= pivot && i <= r );
        do --j; while( a[j] > pivot );
        if( i >= j ) break;
        t = a[i]; a[i] = a[j]; a[j] = t;
       }
       t = a[l]; a[l] = a[j]; a[j] = t;
       return j;
    }
