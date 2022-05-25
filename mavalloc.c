//
// Shane Purdy
// 1001789955
//
//
// The MIT License (MIT)
// 
// Copyright (c) 2022 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "mavalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

enum TYPE
{
    FREE = 0,
    USED
};

struct Node {
  size_t size;
  enum TYPE type;
  void * arena;
  struct Node * next;
  struct Node * prev;
};

struct Node *alloc_list;
struct Node *previous_node;


void * arena;

enum ALGORITHM allo_algorithm = FIRST_FIT;

int mavalloc_init(size_t size, enum ALGORITHM algorithm)
{
	// Initialize all of the values
	arena = malloc(ALIGN4(size));
  
	allo_algorithm = algorithm;

	alloc_list = (struct Node *)malloc(sizeof(struct Node));

	alloc_list->arena = arena;
	alloc_list->size  = ALIGN4(size);
	alloc_list->type  = FREE;
	alloc_list->next  = NULL;
	alloc_list->prev  = NULL;

	previous_node  = alloc_list;

	return 0;
}

void mavalloc_destroy( )
{
	free(arena);
  
	// iterate over the linked list and free the nodes
	struct Node * node; // Initialize a temporary node to hold them
	while(alloc_list)
	{
		node = alloc_list;
		alloc_list = alloc_list->next;
		free(node);
	}
	return;
}

void * mavalloc_alloc( size_t size )
{
	// Initialize needed nodes
	struct Node * node;
	struct Node * nextstart;
	struct Node * nodestart = alloc_list;
	if(allo_algorithm != NEXT_FIT) // Checks if it is next fit or not
	{ 
		// If isn't next fit, set the node at the beginning
		node = alloc_list;
	}
	else if (allo_algorithm == NEXT_FIT)
	{
		// If is next fit, set the node at the previous node, and set a holder for the previous node
		node = previous_node;
		nextstart = previous_node;
	}
	else
	{
		printf("ERROR: Unknown allocation algorithm!\n");
		exit(0);
	}

	size_t aligned_size = ALIGN4(size);
	// Implement First Fit
    if(allo_algorithm == FIRST_FIT)
    {
        while(node) // Loops through the node starting from the first node
        {
            if(node->size >= aligned_size  && node->type == FREE) // Allocate node if it's the right size and is free
            {
                int leftover_size = 0;
  
				node->type  = USED;
				leftover_size = node->size - aligned_size;
				node->size =  aligned_size;
  
				if(leftover_size > 0)
				{
					struct Node * previous_next = node->next;
					struct Node * leftover_node = (struct Node *)malloc(sizeof(struct Node));
  
					leftover_node->arena = node->arena + size;
					leftover_node->type  = FREE;
					leftover_node->size  = leftover_size;
					leftover_node->next  = previous_next;
  
					node->next = leftover_node;
				}
				previous_node = node;
				return (void *) node->arena;
			}
			// Set the node to be the next node
			node = node->next;
		}
	}
	// Implement Next Fit
	else if(allo_algorithm == NEXT_FIT)
	{
		
        do{ // Loops through the node starting from the previous node
            if(node->size >= aligned_size  && node->type == FREE) // Allocate node if it's the right size and is free
            {
                int leftover_size = 0;
  
				node->type  = USED;
				leftover_size = node->size - aligned_size;
				node->size =  aligned_size;
  
				if(leftover_size > 0)
				{
					struct Node * previous_next = node->next;
					struct Node * leftover_node = (struct Node *)malloc(sizeof(struct Node));
  
					leftover_node->arena = node->arena + size;
					leftover_node->type  = FREE;
					leftover_node->size  = leftover_size;
					leftover_node->next  = previous_next;
  
					node->next = leftover_node;
				}
				previous_node = node;
				return (void *) node->arena;
			}
			if(node->next == NULL) // Checks if there is a next node
			{
				node = nodestart; // If there isn't one, set the node to be the beginning of the nodes
			}
			else
			{
				node = node->next; // If there is, set the node to be the next node
			}
		}while(node != nextstart); //Stop when next node equals the start(first previous) node
	}
	// Implement Worst Fit
	else if(allo_algorithm == WORST_FIT)
	{
		// Initialize the worst node
		struct Node * worst = NULL;
		while(node) // Loops through the node until it reaches the end
		{ 
			// Checks if the node is free, the right size, and is the worst sized
			if((node->type == FREE) && (node->size >= aligned_size) && (worst == NULL || node->size < worst->size))
			{
				// If it is, then sets the worst node as this node
				worst = node;
			}
			// Moves the node to the next node
			node = node->next;
		}
		return (void *) worst;
	}
	// Implement Best Fit
	else if(allo_algorithm == BEST_FIT)
	{
		// Initialize the best node
		struct Node * best = NULL;
		while(node)
		{
			//Checks if the node is free, the right size, and is the best sized
			if((node->type == FREE) && (node->size >= aligned_size) && (best == NULL || node->size < best->size))
			{
				// If it is, then sets the best node as this node
				best = node;
			}
			// Moves the node to the next node
			node = node->next;
		}
		return (void *) best;
	}
	return NULL;
}

void mavalloc_free(void * ptr)
{
	struct Node * node = alloc_list;
	while(node) // Loop through the node
	{
		if(node->arena == ptr)
		{
			if(node->type == FREE) // Check if the node is already freed
			{
				printf("Warning: More than one free detected\n");
			}
			// Free the node
			node->type = FREE;
			//break;
		}
		node = node->next;
	}
	return;
}

int mavalloc_size( )
{
	int number_of_nodes = 0;
	struct Node * ptr = alloc_list;

	while(ptr)
	{
		number_of_nodes++;
		ptr = ptr->next; 
	}
	return number_of_nodes;
}
