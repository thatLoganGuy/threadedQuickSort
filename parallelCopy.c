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
#include <sys/stat.h>
#include <sys/mman.h> 
#include <fcntl.h>
    
#define LMAX 255
    sem_t thread_semaphore;

    void* quickSort( void* data);
    int partition( char** a, int, int);


    struct info {
        int start_index;
        char** data_set;
        int end_index; 
	int size;
    };


    int main(int argc, char **argv){
	int max_threads = 3; 
	sem_init(&thread_semaphore, 0, max_threads);
	
	//open file and setup array to be utilized. 
	char** input = NULL; 
	char* ln = NULL; 
	size_t n = 0; 
	ssize_t nchr = 0; 
	size_t idx = 0; 
	size_t it = 0; 
	size_t lmax = LMAX;
	FILE* fp = NULL; 
	int amntNums = 0; 

	//open file for reading
	if (!(fp = fopen(argv[1], "r"))){ 
		fprintf(stderr, "error:file open filed '%s'.", argv[1]);
		return 1;
	} 

printf("FIle opened.\n");
	//allocate LMAX pointers and set to NULL. Each pointer will point to beginning of each string read from file. 
	if (!(input = calloc(LMAX, sizeof *input))){ 
		fprintf(stderr, "error: memory allocation failed."); 
		return 1;
	} 

	while((nchr = getline(&ln, &n, fp)) != -1){ 
		while (nchr > 0 && (ln[nchr-1] == '\n' || ln[nchr-1] == '\r')){ 
			ln[--nchr] = 0;
		} 
		input[idx++] = strdup(ln); 
		amntNums += 1;
		if(idx == lmax){ 
			char** tmp = realloc(input, lmax * 2 * sizeof *input); 
			if(!tmp) 
				return -1; 
		 	input = tmp; 
			lmax *= 2;
		}
	} 

	if (fp) fclose(fp); 
	if (ln) free(ln);  
	printf("Read in array: ");
	for(int i = 0; i < amntNums; i++){ 
		printf("%s, ", input[i]);
	} 
	printf("\n");
	printf("Input created successfully.\n");	
	//setup struct to pass to initial pthread quicksort
	//float a[] = { 7, 12, 1, -2, 8, 2, 0, 96, 78, 7};
	pthread_t thread_id;
	struct info *info = malloc(sizeof(*info));
    	info->data_set = malloc(amntNums * sizeof(char*));
    	memcpy(info->data_set, input, amntNums * sizeof(char*));
	free(input);
	printf("Array in Struct: ");
	for(int i =0; i < amntNums; i++){ 
		printf("%s, ", info->data_set[i]);
	} 
	printf("\n");
	info->start_index=0;
    	info->end_index=amntNums-1; 
	info->size = amntNums;
	sem_wait(&thread_semaphore);
   	if(pthread_create(&thread_id, NULL, quickSort, info)){  
		return 1;
	} 

	pthread_join(thread_id, NULL);

	sem_destroy(&thread_semaphore);
	printf("Successful sort completion.\n");

	

    	printf("Sorted array is:  ");
    	int i;
      	for(i = 0; i < amntNums; ++i)
           printf(" %s ", info->data_set[i]);
	printf("\n");  
	//Write out sorted data to file.
	FILE* outFile = fopen(argv[2], "w"); 
	fwrite(info->data_set, sizeof(char), sizeof(info->data_set), outFile);
	fclose(outFile); 
	free(info);
    	return 0;
    }

    void* quickSort( void *data){
	struct info *info = (struct info*)data;
	struct info other_info; 
	//other_info = malloc(sizeof(*other_info)); 
	//other_info->data_set = malloc(info->size * sizeof(char*));
        int j,l,r;
        l = info->start_index;
        r = info->end_index;

        pthread_attr_t attr;
        pthread_t thread_id1;
        pthread_attr_init(&attr);

       if( l < r ){
	       j = partition( info->data_set, l, r);
	       printf("returned from partition\n");
	       other_info.start_index=j+1;
	       other_info.end_index=r;
	       other_info.size = info->size;
	       other_info.data_set = info->data_set; 
	       printf("Other_info array: ");
	       for(int i = 0; i < other_info.size; i++){ 
	       		printf("%s, ", other_info.data_set[i]);
	       } 
	       printf("\n");
	       if(other_info.end_index<0)other_info.end_index=0;
	       sem_wait(&thread_semaphore);
	       if (pthread_create(&thread_id1, NULL, quickSort, &other_info)) {
		       fprintf(stderr, "No threads for you.\n");
		       return NULL;
	  	}
	       info->start_index=l;
	       info->end_index=j-1;
	       if(info->end_index < 0) info->end_index = 0;
	       quickSort(info);  /* don't care about the return value */
		pthread_join(thread_id1, NULL);	       
       }
  
     sem_post(&thread_semaphore);
     printf("Successful quicksort thread.\n");
    return NULL;

    }


    int partition( char** a, int l, int r) {
       printf("Partition entered.\n");
       int i, j; 
       char* t = malloc(sizeof(char) * 20);
       float pivot;
       pivot = atof(a[l]); 
       printf("Pivot value: %f\n", pivot); 
       i = l; j = r+1;
	printf("right before partition loop.\n");
       while( 1)
       {
        do ++i; while( atof(a[i]) <= pivot && i <= r );
	printf("First dowhile completed.\n");
        do --j; while( atof(a[j]) > pivot );
	printf("Second dowhile completed.\n");
        if( i >= j ) break;
	printf("right before strcopy.\n");
	strcpy(t, a[i]);
	printf("strcopy1 successful.\n");
	strcpy(a[i], a[j]);  
	printf("strcopy2 successful\n");
	strcpy(a[j], t);
	printf("strcpy3 successful.\n");
       }  
       printf("last strcpy.\n");
       strcpy(t, a[l]);
       printf("last strcopy1\n");
       strcpy(a[l], a[j]);
       printf("last strcopy2\n");
       strcpy(a[j], t);
       printf("last strcopy3\n"); 
       free(t);
       return j;
    }
