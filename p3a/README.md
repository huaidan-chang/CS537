# About p2a
Using the Linux pthreads to implement parallel sorting. The input file will consist of records; within each record is a key. The key is the first four bytes of the record. The records are fixed-size, and are each 100 bytes (which includes the key). Child threads would sort a part of input file using merge sort. The parent thread would wait for all child threads complete sorting then merge the sorted data from children and write the result to the output file.

## Usage
```
prompt> ./psort input output
```
