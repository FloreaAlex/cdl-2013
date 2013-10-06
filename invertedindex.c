#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "invertedindex.h"

#define NUM_BUCKETS 999983
#define SIZE_ARRAY 1

void free_map(Map_t *map)
{
	int i;
	for(i = 0; i < map->size; i++)
	{
		while(map->buckets[i] != NULL)
		{
			Node_t *node = (Node_t*)map->buckets[i]->next;
			free(map->buckets[i]->data.documents.v);
			free(map->buckets[i]);
			map->buckets[i] = node;
		}

	}
}


int contains_docID(int *array, int size_array, int docID)
{
	int i;
	for(i = 0; i < size_array; i++)
	{
		if(docID == array[i])
			return 1;
	}
	return 0;
} 


Node_t *create_node(char *key, int docID)
{
	//creaza un array ce va reprezenta valoarea cheii curente
	Array_t *array = malloc(sizeof(Array_t));
	array->n = 1;
	array->cap = SIZE_ARRAY;
	array->v = malloc(array->cap * sizeof(int));
	array->v[0] = docID;

	//create o intare noua in map
	Entry_t *entry = malloc(sizeof(Entry_t));
	entry->word = malloc(strlen(key));
	entry->word = strcpy(entry->word, key);
	entry->documents = *array;

	//creaza un nod nou 
	Node_t *node = malloc(sizeof(Node_t));
	node->data = *entry;
	node->next = NULL;
	
	return node;
}
	

unsigned long hash(unsigned char *str)
{
	unsigned long hash = 5381;
	int c;

	while(c = *str++) 
	{
 		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}


void put_doc(Map_t *map, char *key, int docID)
{
	int hash_key = hash(key) % NUM_BUCKETS;
	
	//creaza o lista in buckets daca acesta este gol
	if(map->buckets[hash_key] == NULL)
	{
		map->buckets[hash_key] = create_node(key, docID);
	}
	//dacă se întâlnește o intare având cheia key, se adaugă în array-ul 
   //din intrare
	else
	{
		Node_t* pointer_to_node = map->buckets[hash_key];
		while(1)
		{
			if(strcmp(key, pointer_to_node->data.word) == 0)
			{
				if(!contains_docID(pointer_to_node->data.documents.v,
					pointer_to_node->data.documents.n, docID))
				{
					pointer_to_node->data.documents.n++;
					if(pointer_to_node->data.documents.n > 
						pointer_to_node->data.documents.cap)
					{
						pointer_to_node->data.documents.cap *= 2;
						pointer_to_node->data.documents.v = 
							realloc(pointer_to_node->data.documents.v, pointer_to_node->data.documents.cap * sizeof(int));
					}	
					pointer_to_node->data.documents.v[pointer_to_node->data.documents.n - 1 ] = docID;
				}
				return;
			}
			if(pointer_to_node->next == NULL)
				break;
			pointer_to_node = (Node_t*)pointer_to_node->next;
		}
		//daca cheia key nu se afla in nicio intrare se creaza o intrare cu 
		//cheia key si se adauga docID in array-ul din intrare
		pointer_to_node->next = (struct Node_t*)create_node(key, docID);
	}	 
}
	

Array_t get_docs(Map_t *map, char *key)
{
	int hash_key = hash(key) % NUM_BUCKETS;
	while(map->buckets[hash_key] != NULL)
	{
		if(strcmp(key, map->buckets[hash_key]->data.word) == 0)
		{
			return map->buckets[hash_key]->data.documents;
		}
		map->buckets[hash_key] = (Node_t*)map->buckets[hash_key]->next;
	}
	
	Array_t empty_array;
	empty_array.n = 0;
	
	return empty_array;
}
		

Array_t intersection(const Array_t files1, const Array_t files2)
{
	int i = 0, j = 0;
	Array_t inter;
	inter.cap = 1;
	inter.v = malloc(inter.cap * sizeof(int));
	inter.n = 0;
	for(i = 0; i < files1.n; i++)
	{
		for(j = 0; j < files2.n; j++)
		{
			if(files1.v[i] == files2.v[j])
			{
				inter.n++;
				inter.v[inter.n - 1] = files1.v[i];
				inter.cap++;
				inter.v = realloc(inter.v, inter.cap * sizeof(int));
				break;
			}
		}
	}
	return inter;
}


Array_t reunion(const Array_t files1, const Array_t files2)
{	
	int i = 0, j = 0;
	Array_t reunion;
	reunion.cap = files1.cap;
	reunion.v = malloc(files1.n * sizeof(int));
	reunion.n = files1.n;
	for(i = 0; i < files1.n; i++)
	{
		reunion.v[i] = files1.v[i];
	}
	for(j = 0; j < files2.n; j++)
	{
		if(!contains_docID(reunion.v, reunion.n, files2.v[j]))
		{
			reunion.n++;
			if(reunion.n > reunion.cap)
			{	
				reunion.cap *= 2;
				reunion.v = realloc(reunion.v, reunion.cap * sizeof(int));	
			}
			reunion.v[reunion.n - 1] = files2.v[j];
		}
	}
	return reunion;
}

void solve()
{
	FILE *input = fopen("input.in", "r");
	if(input == NULL)
		printf("ERROR: couldn't open file input.in");

	Map_t *map = malloc(sizeof(Map_t));
	map->size = NUM_BUCKETS;
	map->buckets = malloc(map->size * sizeof(Node_t*));
	
	char *line_file = malloc(2000 * sizeof(char));	
	int no_files, nr_queries, i, j;
	char s[20];
	fscanf(input, "%d", &no_files);
	
	//vector ce contine numele documentelor
	char array_name_files[no_files][20];
	
	//adauga numele fisierelor in vectorul de nume de fisiere
	for(i = 0; i < no_files; i++)
	{
		fscanf(input, "%s", array_name_files[i]);
	}
	
	
	FILE *f;
	for(i = 0; i < no_files; i++)
	{
		f = fopen(array_name_files[i], "r");
		if(f == NULL)
			printf("Error: couldn't open file %s", array_name_files[i]);
		//citeste linie cu linie
		while(fgets(line_file, 2000, f) != NULL)
		{	
			int length = strlen(line_file) - 1;
			line_file[length] = '\0';

			//se imparte linia in cuvinte si se pune fiecare cuvant in intarea 
			//corespunzatoare
			char *str = strtok(line_file, " ,.?!()[]:;'{}|\"");
			while(str != NULL)
			{
				put_doc(map, str, i);
				str = strtok(NULL, " ,.?!()[]:;'{}|\"");
			}
		}
		fclose(f);
	}

	fscanf(input, "%s", s);
	nr_queries = atoi(s);
	fscanf(input, "\n");
	char *line_query = malloc(2000);	
	Array_t docs;
	
	FILE* output = fopen("output.out", "w");

	for(i = 0; i < nr_queries; i++)
	{
		//se citeste cate un query, pe rand, din fisierul de input
		fgets(line_query, 2000, input);
		
		int length1 = strlen(line_query) - 2;
		if(line_query[length1] == '\r')
			line_query[length1] = '\0';

		char* cpy_line_query = strdup(line_query);
		
		//se imparte fiecare query in cuvinte
		char *split_str = strtok(line_query, " ");
		
		//ID-urile documentelor aferente primului cuvant se pun in array-ul docs
		docs = get_docs(map, split_str);
		split_str = strtok(NULL, " ");
		while(split_str)
		{
			if(strcmp(split_str, "|") == 0)
			{
				split_str = strtok(NULL, " ");
				//se face reuniune daca token-ul este "|"
				docs = reunion(docs, get_docs(map, split_str));
			}
			else if(strcmp(split_str, "&") == 0)
			{
				split_str = strtok(NULL, " ");
				//se face intersectie daca token-ul este "&"
				docs = intersection(docs, get_docs(map, split_str));
			}
			split_str = strtok(NULL, " ");
		}
		fprintf(output, "%s: ", cpy_line_query);
		for(j = 0; j < docs.n; j++)
			fprintf(output, "%d ", docs.v[j]);
		fprintf(output, "\n");
	}
	fclose(input);
	fclose(output);
	
	free_map(map);
}


int main() {
	solve();

return 0;
}

