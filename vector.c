#include "vector.h"
#include <stdlib.h>
#include <string.h>

struct vector* vector_init(int itemSize)
{
   struct vector *vector = (struct vector*)malloc(sizeof (struct vector));

   vector->itemSize = itemSize;
   vector->length = 0;
   vector->capacity = INITIAL_CAPACITY;

   vector->items = malloc(itemSize * vector->capacity);

   return vector;
}

struct vector* vector_copy(struct vector *vector)
{
   struct vector *newVector = (struct vector*)malloc(sizeof (struct vector));

   newVector->itemSize = vector->itemSize;
   newVector->length = vector->length;
   newVector->capacity = vector->capacity;
   newVector->items = malloc(newVector->itemSize * newVector->capacity);

   memcpy(newVector->items, vector->items, newVector->itemSize * newVector->capacity);

   return newVector;
}

void vector_push(struct vector *vector, void *item)
{
   vector_set(vector, vector->length, item);
}

void* vector_get(struct vector *vector, int index)
{
   int offset = index * vector->itemSize;
   return (void*)((char*)(vector->items) + offset);
}

void vector_set(struct vector *vector, int index, void *item)
{
   if (index + 1 > vector->length)
      vector->length = index + 1;

   if (vector->length > vector->capacity)
      vector_resize(vector, vector->capacity + CAPACITY_STEP);

   int offset = index * vector->itemSize;
   memcpy((char*)(vector->items) + offset, item, vector->itemSize);
}

void vector_resize(struct vector *vector, int newCapacity)
{
   // Attempt to realloc vector->items, or just malloc it again if that doesn't work.
   void *newItems = realloc(vector->items, vector->itemSize * newCapacity);

   if (newItems == NULL)
   {
      newItems = malloc(vector->itemSize * newCapacity);
      memcpy(newItems, vector->items, vector->itemSize * vector->capacity);
      free(vector->items);
   }

   vector->items = newItems;
   vector->capacity = newCapacity;

   // Make sure that length is never longer than capacity.
   // Otherwise, you could cause a segfault if you did a
   // "for (i = 0; i < vector->length; i++) { ... }" loop.
   if (vector->length > vector->capacity)
      vector->length = vector->capacity;
}

void vector_free(struct vector *vector)
{
   free(vector->items);
   free(vector);
}

