#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>

// Number of spaces to initialize when calling vector_init.
#define INITIAL_CAPACITY 20
// Number of spaces to add when capacity is exceeded.
#define CAPACITY_STEP 20

// declareVector is used to declare the struct and functions for new given
// vector type. You should put it in a header file.
#define declareVector(vectorTypeName, type)\
\
struct vectorTypeName {\
    type *items;\
    int length;\
    int capacity;\
};\
\
struct vectorTypeName *vectorTypeName##_init();\
void vectorTypeName##_resize(struct vectorTypeName *vector, int newCapacity);\
void vectorTypeName##_set(struct vectorTypeName *vector, int index, type item);\
void vectorTypeName##_push(struct vectorTypeName *vector, type item);\
type vectorTypeName##_get(struct vectorTypeName *vector, int index);\
void vectorTypeName##_free(struct vectorTypeName *vector);

// defineVector is used to define the implementation of the functions declared
// by declareVector. You should put it in the .c file that corresponds to the
// header file you put declareVector in.
#define defineVector(vectorTypeName, type)\
\
struct vectorTypeName *vectorTypeName##_init() {\
   struct vectorTypeName *vector = (struct vectorTypeName*)malloc(sizeof (struct vectorTypeName));\
\
   vector->length = 0;\
   vector->capacity = INITIAL_CAPACITY;\
\
   vector->items = (type*)malloc(sizeof (type) * vector->capacity);\
\
   return vector;\
}\
\
void vectorTypeName##_resize(struct vectorTypeName *vector, int newCapacity) {\
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
   /* Make sure that length is never longer than capacity, in case the capacity
    * was reduced.  Otherwise, you could cause a segfault if you did a
    * "for (i = 0; i < vector->length; i++) { ... }" loop. */\
   if (vector->length > vector->capacity)\
      vector->length = vector->capacity;\
}\
\
void vectorTypeName##_set(struct vectorTypeName *vector, int index, type item) {\
   if (index >= vector->length)\
      vector->length = index + 1;\
\
   if (vector->length > vector->capacity)\
      vectorTypeName##_resize(vector, vector->capacity + CAPACITY_STEP);\
\
    vector->items[index] = item;\
}\
\
void vectorTypeName##_push(struct vectorTypeName *vector, type item) {\
   vectorTypeName##_set(vector, vector->length, item);\
}\
\
type vectorTypeName##_get(struct vectorTypeName *vector, int index) {\
    return vector->items[index];\
}\
\
void vectorTypeName##_free(struct vectorTypeName *vector) {\
   free(vector->items);\
   free(vector);\
}\
\
struct vectorTypeName *vectorTypeName##_concat(struct vectorTypeName *vector, struct vectorTypeName *other) {\
    int i;\
    for (i = 0; i < other->length; i++) {\
        vectorTypeName##_push(vector, vectorTypeName##_get(other, i));\
    }\
\
    return vector;\
}

#endif
