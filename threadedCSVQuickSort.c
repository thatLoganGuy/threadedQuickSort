#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h> 
#define LMAX 255
    
void* quickSort(void* data);
int partitionFloat( int, int, int);  
int partitionString(int, int, int);
int partitionController(int, int, int); 
char* getfield(char*, int); 
void* subSortControl(int, int, int);

char** input;  
int* sortCols;
int amntLines = 0;  
int numColumns = 0;
int maxThreads = 0;
sem_t thread_semaphore;

struct info {
        int start_index;
        int end_index;  
		int column;
    };

int main(int argc, char **argv){
	int i;  
	
	//Read in max number of threads and establish semaphores.
	maxThreads = atoi(argv[4]);  
	//catch invalid input.
	if(maxThreads <= 2){ 
		printf("Program only accepts a maxthread value of 3 or more.\n"); 
		return 0;
	}
	sem_init(&thread_semaphore, 0, maxThreads);
	
	//open file and setup array to be utilized. 
	char* ln = NULL; 
	size_t n = 0; 
	ssize_t nchr = 0; 
	size_t idx = 0; 
	size_t it = 0; 
	size_t lmax = LMAX;
	FILE* fp = NULL; 

	//open file for reading
	if (!(fp = fopen(argv[1], "r"))){ 
		fprintf(stderr, "error:file open filed '%s'.", argv[1]);
		return 1;
	} 

	//allocate LMAX pointers and set to NULL. Each pointer will point to beginning of each string read from file. 
	if (!(input = calloc(LMAX, sizeof *input))){ 
		fprintf(stderr, "error: memory allocation failed."); 
		return 1;
	} 

	//read in and save lines of the file to the array, allocating more space when it is filled. 
	while((nchr = getline(&ln, &n, fp)) != -1){ 
		while (nchr > 0 && (ln[nchr-1] == '\n' || ln[nchr-1] == '\r')){ 
			ln[--nchr] = 0;
		} 
		input[idx++] = strdup(ln);   
		amntLines += 1;
		if(idx == lmax){ 
			char** tmp = realloc(input, lmax * 2 * sizeof *input); 
			if(!tmp) 
				return -1; 
		 	input = tmp; 
			lmax *= 2;
		}
	} 
	
	//close file and free memory.
	if (fp) fclose(fp); 
	if (ln) free(ln);  
	
	//A loop to determine the totalColumns that the csv file has to ensure that a column index that is passed by the user is out of bounds
	int totalColumns = 0; 
    char* tempLine = malloc(sizeof(input[0])); 
	strcpy(tempLine, input[0]);
	char* tok = strtok(tempLine, ",");   
	while(tok != NULL){ 
		tok = strtok(NULL,","); 
		totalColumns++;
	}
	free(tempLine);

	int k;
	//Establish columns to be sorted by and save them to an array.
	char*  sortingColumns = argv[3]; 
	numColumns = (strlen(sortingColumns) + 1)  / 2; 
	sortCols = malloc(sizeof(int) * numColumns); 
	char* token = strtok(sortingColumns, ",");
	for(i = 0; i < numColumns; i++){
		int length = strlen(token); 
		//for loop catches invalid column indexes.
		for(k = 0; k < length; k++){ 
			if(!isdigit(token[k])){ 
				printf("Non-integer value passed as column index.\n"); 
				return 0;
			}
		}
		sortCols[i] = atoi(token);  
		//if statement catches column indexes that are out of bounds.
		if(sortCols[i] >= totalColumns){ 
			printf("Error in input. Column value passed as argument outside of scope of file.\n"); 
			return 0;
		}
		token = strtok(NULL, ",");
	}		
	int primaryColumn = sortCols[0];
	
	//setup struct to pass to quicksort
	struct info *info = malloc(sizeof(*info));	
	info->start_index=0;
    info->end_index=amntLines-1;   
	info->column = primaryColumn;
	
	//run the quicksort across the primary column first and then across any other columns to break tied values.
	quickSort(info); 
	if(numColumns > 1){ 
		subSortControl(0, amntLines, 0);
	}
	
	//print out the sorted array to the console to verify its correctness.
	for(i = 0; i < amntLines; i++){ 
		printf("%s, ", input[i]);
	} 
	printf("\n");
	
	//Write out sorted data to file.
    FILE* outFile = fopen(argv[2], "w");  
	for(i = 0; i < amntLines; i ++){ 
		fprintf(outFile, "%s\n", input[i]);
	}
	fclose(outFile); 
	free(input); 
	
    return 0;
}

void* quickSort( void* data){
	//Read in passed parameters from struct and intialize other_info to be passed to threaded quicksort. 
	struct info *info = (struct info*)data;
	struct info other_info;
    int j,l,r,c;  
	c = info->column;
    l = info->start_index;
    r = info->end_index;
	
	//setting up thread attributes and name for use in the threaded quicksort.
    pthread_attr_t attr;
    pthread_t thread_id1;
    pthread_attr_init(&attr);

    if( l < r ){ 
		//as long as the left index is smaller than the right index. Call the partition controller and call the quicksort to be run on the current thread and a new thread.
		j = partitionController(l, r, c);
	    
		//assign vaues for the other_info struct to be passed to the threaded quicksort.
		other_info.start_index=j+1;
	    other_info.end_index=r; 
		other_info.column = info->column;
	    if(other_info.end_index<0)other_info.end_index=0;
		   
		 //Wait for an available thread to be open that will run the quicksort function with a pointer to theother_info struct.
		sem_wait(&thread_semaphore);
	    if (pthread_create(&thread_id1, NULL, quickSort, &other_info)) {
		    fprintf(stderr, "Thread unable to be created.\n");
		    return NULL;
	  	}
		
		//update the values in the info struct to be used in the quicksort call made tot he current thread. 
		info->start_index=l;
	    info->end_index=j-1; 
	    if(info->end_index < 0) info->end_index = 0; 

		//quicksort for this thread called and pthread joined after quicksort running in this thread finished. Resulting in parallelization of the left and right quicksort calls. 
		quickSort(info); 
		pthread_join(thread_id1, NULL);	 
		   
       } 
	 //update thread semaphore that a quicksort thread has finished now that the function is over.
    sem_post(&thread_semaphore); 
    return NULL;
}

int partitionController(int l, int r, int c){ 
	//SInce the quicksort deals with columns that may have types of Strings or Floats. This function determines if the column has float or string values and calls the appropriate partition function based on the data type. 
	int j = 0;  
	int len = 0; 
	float ignore = 0.0;
	char* pivot = getfield(input[0], c);  
	int check = sscanf(pivot, "%f %n",  &ignore, &len); 
	//this string can be represented as a float and the partitionFloat function will be called. 
	if (check == 1 && !pivot[len]){ 
		j = partitionFloat(l, r, c);
	}	  
	//this stiring cannot be represented as a float and the string partittion will be called. 
	else{ 
		j = partitionString(l, r, c); 
	} 
	return j;
}

int partitionFloat(int l, int r, int c) {
	//establish pivot value and cast as a float for comparisons.
	int i, j; 
    float pivot;
	pivot = atof(getfield(input[l],c));  
	char* t;
    i = l; j = r + 1;
	//the loop organizes and swaps the position of values based on if they are smaller or larger than the pivot value.   
    while( 1){
        do{
			++i;  
			if(getfield(input[i],c) == NULL){ 
				break;
			}
		}
		while( atof(getfield(input[i],c)) <= pivot && i <= r );
	    do{ 
		     --j; 
		}
		while( atof(getfield(input[j],c)) > pivot );
		if( i >= j ) break;
		t = input[i];
		input[i] = input[j];
		input[j] = t;
	}  
    t = input[l];
	input[l] = input[j];
	input[j] = t;
	
	return j;
}  

int partitionString(int l, int r, int c) {
	//establish pivot value for comparisons.
	int i, j;  
    char* pivot = getfield(input[l],c); 
	char* t;
    i = l; j = r + 1;
	
	//the loop organizes and swaps the position of values based on if they are smaller or larger than the pivot value. 
    while(1){
        do{
			++i;   
			if(i == amntLines){ 
				break;
			}
		} 
		while( i <= r && strcmp(getfield(input[i],c), pivot) <= 0 ); 
	    do{ 
		     --j; 
			 if(getfield(input[j],c) == NULL){ 
				break;
			}
		}
		while( strcmp(getfield(input[j],c), pivot) > 0) ;
		if( i >= j ) break;
		t = input[i];
		input[i] = input[j];
		input[j] = t;
	}  
    t = input[l];
	input[l] = input[j];
	input[j] = t;
	return j;
} 

char* getfield(char* line, int num){ 
	//this function is important to the program as it allows for specific column values to be extracted from a line for the purposes of comparisons done in sorting.
    char* tempLine = malloc(sizeof(line)); 
	//strcpy is used to preserve the original string representing the entire row.
	strcpy(tempLine, line);
	char* tok = strtok(tempLine, ","); 
	int i = 0;   
	//loop through all of the tokens until the one at the correct index is found. 
	while(tok != NULL && i <= num){ 
		if(i == num){ 
			return tok;
		} 
		tok = strtok(NULL,","); 
		i++;
	}
	free(tempLine);
    return NULL;
} 

void* subSortControl(int start, int end, int colIndex){ 
	//this function handles calling the quicksort algorithm for secondary, tertiary, etc columns to sort tied values in previous columns.
	struct info *info = malloc(sizeof(*info));
	int first = 0; 
	int last = 0;   
	int c = sortCols[colIndex]; 
	//this loop finds the start and end index of where tied values exist in the chunk of the array that subsortcontrol is observing. 
	char* currentValue = "";
	for(int i = start; i < end; i++){ 
		if(strcmp(currentValue, getfield(input[i], c)) != 0){
			if(last - first > 0){
				info->start_index = first; 
				info->end_index = last ;  
				info->column = sortCols[colIndex+1]; 
				//quickSort the rows that have tied values based on the next specified array.
				quickSort(info);      
				//if more columns can be sorted by to settle ties, call subsortcontrol again.
				if(colIndex + 2 < numColumns){
					subSortControl(first, last + 1, colIndex + 1); 
				}
			}
			first = i; 
			last = i; 
			currentValue = getfield(input[i], c);
		}  
		else{ 
			last = i; 
		}
	 }  
	 free(info);
	 return NULL;
}