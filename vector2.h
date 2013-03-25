#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>

// Number of spaces to initialize when calling vector_init.
#define INITIAL_CAPACITY 20
// Number of spaces to add when capacity is exceeded.
#define CAPACITY_STEP 20

#define define_vector(vector_type_name, type)\
\
struct vector_type_name {\
    type *items;\
    int length;\
    int capacity;\
};\
\
struct vector_type_name *make_##vector_type_name() {\
   struct vector_type_name *vector = (struct vector_type_name*)malloc(sizeof (struct vector_type_name));\
\
   vector->length = 0;\
   vector->capacity = INITIAL_CAPACITY;\
\
   vector->items = (type*)malloc(sizeof (type) * vector->capacity);\
\
   return vector;\
}\
\
void vector_resize(struct vector_type_name *vector, int newCapacity) {\
   /* Attempt to realloc vector->items, or just malloc it again if that doesn't work. */\
   type *newItems = (type*)realloc(vector->items, sizeof (type) * newCapacity);\
\
   if (newItems == NULL)\
   {\
      newItems = malloc(sizeof (type) * newCapacity);\
      memcpy(newItems, vector->items, sizeof (type) * vector->capacity);\
      free(vector->items);\
   }\
\
   vector->items = newItems;\
   vector->capacity = newCapacity;\
\
   /* Make sure that length is never longer than capacity.
    * Otherwise, you could cause a segfault if you did a
    * "for (i = 0; i < vector->length; i++) { ... }" loop. */\
   if (vector->length > vector->capacity)\
      vector->length = vector->capacity;\
}\
\
void vector_set(struct vector_type_name *vector, int index, type item) {\
   if (index >= vector->length)\
      vector->length = index + 1;\
\
   if (vector->length > vector->capacity)\
      vector_resize(vector, vector->capacity + CAPACITY_STEP);\
\
    vector->items[index] = item;\
}\
\
void vector_push(struct vector_type_name *vector, type item) {\
   vector_set(vector, vector->length, item);\
}\
\
type vector_get(struct vector_type_name *vector, int index) {\
    return vector->items[index];\
}\
\
void vector_free(struct vector_type_name *vector) {\
   free(vector->items);\
   free(vector);\
}

#endif
