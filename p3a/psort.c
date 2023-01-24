#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>

typedef unsigned int uint;
char error_message[30] = "An error has occurred\n";
typedef struct {
    int key;       // 4 bytes
    char *rpointer;
} rec_t;

// the part for each thread
typedef struct {
    int low;
	int high;
    rec_t* map;
} part_t;

// merge two parts
void merge(rec_t* map, int low, int mid, int high) {
	int nleft = mid - low + 1;
	int nright = high - mid;

	rec_t* left = (rec_t *)malloc(nleft * sizeof(rec_t));
	rec_t* right = (rec_t *)malloc(nright * sizeof(rec_t));

	int i, j;
	// initialize left part
	for (i = 0; i < nleft; i++) {
		left[i] = map[i + low];
	}

	// initialize right part
	for (i = 0; i < nright; i++) {
		right[i] = map[i + mid + 1];
	}

	int k = low;
	i = j = 0;

	while (i < nleft && j < nright) {
		if (left[i].key <= right[j].key){
			map[k++] = left[i++];
		} else {
			map[k++] = right[j++];
		}
	}

	while (i < nleft){
		map[k++] = left[i++];
	}

	while (j < nright){
		map[k++] = right[j++];
	}

	free(left);
	free(right);
}

// merge sort
void merge_sort(rec_t* map, int low, int high) {
	int mid = low + (high - low) / 2;

	if (low < high) {
		merge_sort(map, low, mid);
		merge_sort(map, mid + 1, high);
		merge(map, low, mid, high);
	}
}

//merge sort function for threads
void* merge_sort_thread(void* arg) {
	part_t* part = (part_t*)arg;
	int low = part->low;
	int high = part->high;
	int mid = low + (high - low) / 2;

	if (low < high) {
		merge_sort(part->map, low, mid);
		merge_sort(part->map, mid + 1, high);
		merge(part->map, low, mid, high);
	}
}


int main(int argc, char** argv) {
    struct {
        int fd;
        char *map;
        char *filename;
        uint sz;
    } records, sort;

    records.filename = argv[1];
	sort.filename = argv[2];


    // open file
    if ((records.fd = open(records.filename, O_RDONLY)) == -1) {
        fprintf(stderr, "%s", error_message);
        exit(0);
    }
    
    // retrieve information about the file
    struct stat st;
    if (stat(records.filename, &st) == -1) {
		fprintf(stderr, "%s", error_message);
        exit(0);
    } else {
        records.sz = st.st_size;
    }

    //map file into memory
    if ((records.map = mmap(0, records.sz, PROT_READ, MAP_SHARED, records.fd, 0)) ==
          MAP_FAILED){
		fprintf(stderr, "%s", error_message);
        exit(0);
    }

    // one record is 100 byte
    uint recNum = records.sz/100;
    
    // malloc for records
    rec_t* key_map = (rec_t *)malloc(recNum * sizeof(rec_t));
	rec_t* c = key_map;

    for (char *r = records.map; r < records.map + recNum * 100; r += 100) {
        c->key = *(int *)r;
        c->rpointer = r;
        c++;
    }

    // the number of processors currently available in the system
    int threadNum = get_nprocs() + 2;
    pthread_t threads[threadNum];

    // list of all parts of threads
    part_t* partList = (part_t *)malloc(threadNum * sizeof(part_t));
    
    int low = 0;
    // length of each part for a thread
    int len = recNum/threadNum;
    part_t* part;
    for (int i = 0; i < threadNum; i++, low += len) {
		part = &partList[i];
		part->low = low;
		part->high = low + len - 1;
		if (i == (threadNum - 1)){
			part->high = recNum - 1;
        }
	}

    // create threads
	for (int i = 0; i < threadNum; i++) {
		part = &partList[i];
		part->map = key_map;
		pthread_create(&threads[i], 0, merge_sort_thread, part);
	}

  	for (int i = 0; i < threadNum; i++) {
        pthread_join(threads[i], NULL);
    }

	// merge all parts
	for (int i = 1; i < threadNum; i++) {
		part_t* part = &partList[i];
		merge(key_map, 0, part->low - 1, part->high);
	}

	// write output
	sort.fd = open(sort.filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (sort.fd < 0) {
        fprintf(stderr, "%s", error_message);
        exit(EXIT_FAILURE);
    }

    for (int k = 0; k < recNum; k++) {
        int rc = write(sort.fd, key_map[k].rpointer, 100 * sizeof(char));
        assert(rc == 100 * sizeof(char));
    }
	
    close(sort.fd);
	close(records.fd);
	exit(0);
}