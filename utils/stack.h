#ifndef STACK_H
#define STACK_H

#include "type_alias.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef i16 Stack_Type;

typedef enum {
    STACK_NO_ERROR,
    STACK_ERROR_INVALID_SELF,
    STACK_ERROR_OVERFLOW,
    STACK_ERROR_UNDERFLOW,
} Stack_Error;

typedef struct {
    Stack_Type* data;
    size_t size;
    Stack_Type* stack_ptr;
    bool verbose;
    i32 error;
    bool valid;
} Stack;

static Stack
Stack_construct(u32 size, bool verbose_error)
{
    Stack stack = {};

    stack.data = malloc(size * sizeof(Stack_Type));

    if(!stack.data)
    {
        stack.valid = false;
        return stack;
    }

    stack.size = size;
    stack.stack_ptr = &stack.data[stack.size];

    stack.verbose = verbose_error;
    stack.valid = true;
    return stack;
}

static Stack_Type
Stack_pop(Stack* self)
{
    if(!self || !self->valid)
    {
        self->error = STACK_ERROR_INVALID_SELF;
        if(self->verbose) fprintf(stderr, "Error: Stack: Invalid self\n");
        return 0;
    }

    if((self->stack_ptr + 1 - self->data) > self->size)
    {
        self->error = STACK_ERROR_UNDERFLOW;
        if(self->verbose) fprintf(stderr, "Error: Stack: stack underflow\n");
        return 0;
    }

    self->error = STACK_NO_ERROR;
    return *self->stack_ptr++;
}

static void
Stack_push(Stack* self, Stack_Type elm)
{
    if(!self || !self->valid)
    {
        self->error = STACK_ERROR_INVALID_SELF;
        if(self->verbose) fprintf(stderr, "Error: Stack: Invalid self\n");
        return;
    }

    if((self->stack_ptr - 1) < self->data)
    {
        self->error = STACK_ERROR_OVERFLOW;
        if(self->verbose) fprintf(stderr, "Error: Stack: stack overflow\n");
        return;
    }

    self->stack_ptr--;
    *self->stack_ptr = elm;

    self->error = STACK_NO_ERROR;
}

static bool
Stack_is_empty(Stack* self)
{
    if(!self || !self->valid)
    {
        self->error = STACK_ERROR_INVALID_SELF;
        if(self->verbose) fprintf(stderr, "Error: Stack: Invalid self\n");
        return false;
    }

    return self->stack_ptr == &self->data[self->size - 1];
}

static bool
Stack_is_full(Stack* self)
{
    if(!self || !self->valid)
    {
        self->error = STACK_ERROR_INVALID_SELF;
        if(self->verbose) fprintf(stderr, "Error: Stack: Invalid self\n");
        return false;
    }

    return self->stack_ptr == self->data;
}

static void
Stack_deconstruct(Stack* self)
{
    if(!self)
    {
        return;
    }

    self->valid = false;
    free(self->data);
}

#endif // STACK_H