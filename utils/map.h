#ifndef MAP_H
#define MAP_H

#include "type_alias.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
    MapI32_NO_ERROR,
    MapI32_ERROR_CANNOT_ALLOC_MEM,
    MapI32_ERROR_KEY_NOTFOUND,
    MapI32_ERROR_FULL_MAP,
} MapI32_Error;

typedef struct {
    i32 key;
    u32 value;
} MapI32_Node;

typedef struct {
    MapI32_Node* nodes;
    u32 len;
    bool valid;
    MapI32_Error error;
} MapI32;

static MapI32
MapI32_construct(u32 len)
{
    MapI32 map = {};
    map.nodes = calloc(len, sizeof(MapI32_Node));
    if(!map.nodes)
    {
        map.valid = false;
        map.error = MapI32_ERROR_CANNOT_ALLOC_MEM;
        return map;
    }

    for(int iii = 0; iii < len; ++iii)
    {
        map.nodes[iii].key = -1;
    }

    map.valid = true;
    map.len = len;

    return map;
}

static bool
MapI32_insert(MapI32* self, i32 key, u32 value)
{
    if(!self)
    {
        fputs("Empty MapI32", stderr);
        return false;
    }

    u32 hash = key % self->len;
    u32 orig_hash = hash;

    //move in array until an empty or deleted cell
    while(self->nodes[hash].key != -1) {
        //go to next cell
        ++hash;
            
        //wrap around the table
        hash %= self->len;

        if(hash == orig_hash)
        {
            self->error = MapI32_ERROR_FULL_MAP;
            return false;
        }
    }

    self->nodes[hash].key = key;
    self->nodes[hash].value = value;

    self->error = MapI32_NO_ERROR;
    return true;
}

static const void*
MapI32_search(MapI32* self, i32 key)
{
    if(!self)
    {
        fputs("Empty MapI32", stderr);
        return NULL;
    }

    //get the hash
    i32 hash = key % self->len;
    u32 counter = 0;
        
    //move in array until an empty
    while(self->nodes[hash].key != -1)
    {
        if(counter == self->len)
        {
            break;
        }

        if(self->nodes[hash].key == key)
        {
            self->error = MapI32_NO_ERROR;
            return &self->nodes[hash].value;
        }
                
        //go to next cell
        ++hash;
            
        //wrap around the table
        hash %= self->len;

        counter++;
    }

    self->error = MapI32_ERROR_KEY_NOTFOUND;
    return NULL;
}

static void
MapI32_deconstruct(MapI32* self)
{
    if(!self)
    {
        fputs("Empty MapI32", stderr);
        return;
    }

    if(!self->valid)
    {
        fputs("Invalid MapI32", stderr);
        return;
    }

    free(self->nodes);

    self->valid = false;
}

#endif // MAP_H