#ifndef VECTOR_H
#define VECTOR_H

// Dealing with passing around lengths along with every array is *******
// painful in C, so I'm using a simple vector implementation. In this case, a
// vector is just an automatically growing array.
struct vector
{
   void *items;
   int itemSize;   // Item size in bytes.
   int length;     // Maximum index used so far.
   int capacity;   // Number of spaces allocated so far.
};

// Number of spaces to initialize when calling vector_init.
#define INITIAL_CAPACITY 20
// Number of spaces to add when capacity is exceeded.
#define CAPACITY_STEP 20

struct vector* vector_init(int itemSize);
struct vector* vector_copy(struct vector *vector);
void vector_push(struct vector *vector, void *item);
void *vector_get(struct vector *vector, int index);
void vector_set(struct vector *vector, int index, void *item);
void vector_resize(struct vector *vector, int newCapacity);
void vector_free(struct vector *vector);

#endif
