- This project implements two different threaded Quicksort algorithms in order to efficiently sort values. 
- threadedQuickSort.c is the simpler of the two programs which will sort a file of double values and output it into a specified destination file. 
- threadedCSVQuickSort.c is the more complicated of the two programs and sorts a csv file based on the following parameters. 
	1. the program takes 4 command line parameters. The name of the csv file to be sorted, the name of the output file, and a comma seperated list of indexes of columns(0-indexed).
	2. the list of indexes specify which column values of the csv file the rows should be sorted by. 
	3. if during the sorting process, the program encounters two identical values, then the program examines the values in the next column specified by the user to break the tie and determine the order in which to sort the rows. The program will continue to recursively break ties using all of the provided columns. 
	4. example running of the program from the command line would be: 
		./<executable> numbers.csv numbersSorted.csv "1,2" 4 

-Note on Compiling Programs: Be sure to add the -pthread argument to the gcc command when compiling the programs to ensure that threads are supported in the executable. 
