#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

int hit;
int miss;
int read;
int write;
int cachesize;
int blocksize;
int numset;
int numline;

typedef struct line{
	int valid;
	long tag;
	int age;
}line;

int find(line** cache, long tagnum, int index);
void calculation(line** cache, char* filename, int prefetch);
int invalid(line** cache, int index);
line** updateage(line** cache, int index);
void lru(line** cache, char* filename, int prefetch);

int main(int argn, char** argv){
	cachesize = atoi(argv[1]);
	blocksize = atoi(argv[5]);
	int prefetch;
	if(argv[3][1] == '0'){
		prefetch = 0;
	}else if(argv[3][1] == '1'){
		prefetch = 1;
	}	
	if(argv[2][0] == 'd'){
		numline = 1;
		numset = cachesize/blocksize;
	}else if(argv[2][5] == ':'){
		sscanf(argv[2],"assoc:%d",&numline);
		numset = (cachesize/blocksize)/numline;
	}else{
		numline = cachesize/blocksize;
		numset = 1;
	}
	int i;
	int j;
	line** cache = (line**)malloc(numset*sizeof(line*));
	for(i = 0; i < numset; i++){
		cache[i] = (line*)malloc((numline)*sizeof(line));
	}
	for(i = 0; i < numset; i++){
		for(j = 0; j < numline; j++){
			cache[i][j].valid = 0;
		}
	}
	if(argv[4][0] == 'f'){
		calculation(cache, argv[6], prefetch);
	}else if(argv[4][0] == 'l'){
		lru(cache, argv[6], prefetch);
	}
	
	printf("Memory reads: %d\n", read);
	printf("Memory writes: %d\n", write);
	printf("Cache hits: %d\n", hit);
	printf("Cache misses: %d\n", miss);
	
	for(i = 0; i < numset; i++){
		line* temp = cache[i];
                free(temp);
	}
	return 0;
}


void calculation(line** cache, char* filename, int prefetch){
	int b = log(blocksize)/log(2);
	int s = log(numset)/log(2);
	int temp = ((1 << s) - 1);
	int i;
	int agecount = 0;	
	char instruction;
	long address;
	FILE* fp;
	fp = fopen(filename, "r");
	if(fp == NULL){
		return;
	}
	while(fscanf(fp, "%*x: %c %lx", &instruction, &address) == 2){
		int index = (address >> b) & temp;
		long tagnum = address >> (b + s);	
		if(instruction == 'R'){
			if(find(cache, tagnum, index) != -1){
				hit++;
               		}else{
				miss++;
				read++;
				int fill = invalid(cache, index);
				if(fill != -1){
					cache[index][fill].valid = 1;
					cache[index][fill].tag = tagnum;
					agecount++;
					cache[index][fill].age = agecount;
				}else{
					int min = 0;
					for(i = 1; i < numline; i ++){
						if(cache[index][i].age < cache[index][min].age){
							min = i;
						}
					}
					cache[index][min].tag = tagnum;
					agecount++;
					cache[index][min].age = agecount;
                		}
				if(prefetch == 1){
					long newaddress = address + blocksize;
					index = (newaddress >> b) & temp;
					tagnum = newaddress >> (b + s);
					if(find(cache, tagnum, index) == -1){
						read++;
						int fill = invalid(cache, index);
						if(fill != -1){
							cache[index][fill].valid = 1;
							cache[index][fill].tag = tagnum;
							agecount++;
							cache[index][fill].age = agecount;
						}else{
							int min = 0;
							for(i = 1; i < numline; i ++){
								if(cache[index][i].age < cache[index][min].age){
									min = i;
								}
							}
							cache[index][min].tag = tagnum;
							agecount++;
							cache[index][min].age = agecount;
						}
					}
				}
			}	
		}else if(instruction == 'W'){
			if(find(cache, tagnum, index) != -1){
				hit++;
				write++;
                        }else{
				miss++;
				read++;
				write++;
				int fill = invalid(cache, index);
				if(fill != -1){
					cache[index][fill].valid = 1;
					cache[index][fill].tag = tagnum;
					agecount++;
					cache[index][fill].age = agecount;
				}else{
					int min = 0;
                                	for(i = 1; i < numline; i ++){
                                        	if(cache[index][i].age < cache[index][min].age){
                                                       	min = i;
                                        	}
                                	}
                                	cache[index][min].tag = tagnum;
                               		agecount++;	
                                	cache[index][min].age = agecount;	
				}
				if(prefetch == 1){
                                        long newaddress = address + blocksize;
                                        index = (newaddress >> b) & temp;
                                        tagnum = newaddress >> (b + s);
                                        if(find(cache, tagnum, index) == -1){
                                                read++;
                                                int fill = invalid(cache, index);
                                                if(fill != -1){
                                                        cache[index][fill].valid = 1;
                                                        cache[index][fill].tag = tagnum;
                                                	agecount++;
                                                        cache[index][fill].age = agecount;
                                                }else{
                                                        int min = 0;
                                                        for(i = 1; i < numline; i ++){
                                                                if(cache[index][i].age < cache[index][min].age){
                                                                        min = i;
                                                                }
                                                        }
                                                        cache[index][min].tag = tagnum;
                                                	agecount++;
                                                        cache[index][min].age = agecount;
                                                }
                                        }
                                }
			}		
		}
	}
	fclose(fp);
	return;
}

int find(line** cache, long tagnum, int index){
	int i;
	for(i = 0; i < numline; i++){
		if(cache[index][i].valid == 1 && cache[index][i].tag == tagnum){
			return i;
		}
	}
	return -1;
}

int invalid(line** cache, int index){
	int i;
	for(i = 0; i < numline; i++){
		if(cache[index][i].valid == 0){
			return i;
		}
	}
	return -1;
}

void lru(line** cache, char* filename, int prefetch){
        int b = log(blocksize)/log(2);
        int s = log(numset)/log(2);
        int temp = ((1 << s) - 1);
        int i;
	int j;
	for(i = 0; i < numset; i++){
		for(j = 0; j < numline; j++){
			cache[i][j].age = j;
		}
	}
        char instruction;
        long address;
        FILE* fp;
        fp = fopen(filename, "r");
        if(fp == NULL){
                return;
        }
        while(fscanf(fp, "%*x: %c %lx", &instruction, &address) == 2){
                int index = (address >> b) & temp;
                long tagnum = address >> (b + s);
                if(instruction == 'R'){
                        if(find(cache, tagnum, index) != -1){
                                hit++;
                        }else{
                                miss++;
                                read++;
				int empty = invalid(cache, index);
				if(empty != -1){
					cache[index][empty].valid = 1;
					cache[index][empty].tag = tagnum;
					cache = updateage(cache, index);
				}else{
					for(i = 0; i < numline; i++){
						if(cache[index][i].age == 0){
							cache[index][i].tag = tagnum;
							cache = updateage(cache, index);
						}
					}
				}
                                if(prefetch == 1){
                                        long newaddress = address + blocksize;
                                        index = (newaddress >> b) & temp;
                                        tagnum = newaddress >> (b + s);
					read++;
					if(find(cache, tagnum, index) == -1){
						int empty = invalid(cache, index);
						if(empty != -1){
							cache[index][empty].valid = 1;
							cache[index][empty].tag = tagnum;
							cache = updateage(cache, index);
						}else{
							for(i = 0; i < numline; i++){
								if(cache[index][i].age == 0){
									cache[index][i].tag = tagnum;
									cache = updateage(cache, index);
								}
							}
						}
					}
                                }
                        }
                }else if(instruction == 'W'){
                        if(find(cache, tagnum, index) != -1){
                                hit++;
                                write++;
                        }else{
                                miss++;
                                read++;
                                write++;
                                int empty = invalid(cache, index);
                                if(empty != -1){
                                        cache[index][empty].valid = 1;
                                        cache[index][empty].tag = tagnum;
                                        cache = updateage(cache, index);
                                }else{
                                        for(i = 0; i < numline; i++){
                                                if(cache[index][i].age == 0){
                                                        cache[index][i].tag = tagnum;
                                                        cache = updateage(cache, index);
                                                }
                                        }
                                }				
                                if(prefetch == 1){
                                        long newaddress = address + blocksize;
                                        index = (newaddress >> b) & temp;
                                        tagnum = newaddress >> (b + s);
                                        if(find(cache, tagnum, index) == -1){
						read++;
                                                int empty = invalid(cache, index);
                                                if(empty != -1){
                                                        cache[index][empty].valid = 1;
                                                        cache[index][empty].tag = tagnum;
                                                        cache = updateage(cache, index);
                                                }else{
                                                        for(i = 0; i < numline; i++){
                                                                if(cache[index][i].age == 0){
                                                                        cache[index][i].tag = tagnum;
                                                                        cache = updateage(cache, index);
                                                                }
                                                        }
                                                }
                                        }                                      
                                }
                        }
                }
        }
        fclose(fp);
        return;
}
line** updateage(line** cache, int index){
	int i;
	for(i = 0; i < numline; i++){
		if(cache[index][i].age == 0){
			cache[index][i].age = 3;
		}else{
			cache[index][i].age--;
		}
	}
	return cache;
}
