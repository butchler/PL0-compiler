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
struct vector *vector_concat(struct vector *toVector, struct vector *fromVector);
void vector_free(struct vector *vector);

// Don't use the above functions, use the macros below so that you don't have
// to reference/dereference anything when using vector_get and
// vector_set/vector_push. They also have slightly nicer names, and let you
// pass a type to makeVector, instead of an int to vector_init.
#define makeVector(type) vector_init(sizeof (type))
#define push(vector, item) vector_push(vector, (void*)(&item))
#define set(vector, index, item) vector_set(vector, index, (void*)(&item))
#define get(type, vector, index) (*(type*)vector_get(vector, index))
#define freeVector(vector) vector_free(vector)

#endif
