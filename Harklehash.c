#include "Harklehash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Purpose: Hash an input string
// Input:   Hash input
// Output:  Hash as unsigned int
unsigned int hash(char* input)
{
    unsigned int retVal = 0;
    for (; *input != '\0'; input++)
    {
        retVal = *input + 31 * retVal;
    }
    return retVal % HASHSIZE;
}


// Purpose: Find the node associated with a given name
// Input:   
//          headNode - Pointer to the head node of the linked list
//          needle - String to find in the linked list
// Output:  Pointer to the struct in question
struct HarkleDict* lookup_name(struct HarkleDict* headNode, char* needle)
{
    struct HarkleDict* retVal = headNode;

    if (retVal)
    {
        do
        {
            if (strcmp(needle, retVal->name) == 0)
            {
                    return retVal;  // Found it
            }

            retVal = retVal->next;
        } while(retVal->next);
    }
    
    return NULL;  // Didn't find it
}


// Purpose: Find the node associated with a given hash
// Input:   
//          headNode - Pointer to the head node of the linked list
//          needle - Hash to find in the linked list
// Output:  Pointer to the struct in question
struct HarkleDict* lookup_hash(struct HarkleDict* headNode, unsigned int needle)
{
    struct HarkleDict* retVal = headNode;

    if (retVal)
    {
        do
        {
            if (needle == retVal->hash)
            {
                    return retVal;  // Found it
            }

            retVal = retVal->next;
        } while(retVal->next);
    }

    return NULL;  // Didn't find it
}


// Purpose: Find the node associated with a given hash
// Input:   
//          headNode - Pointer to the head node of the linked list
//          needle - Hash to find in the linked list
// Output:  Pointer to the struct in question
struct HarkleDict* lookup_value(struct HarkleDict* headNode, int needle)
{
    struct HarkleDict* retVal = headNode;

    if (retVal)
    {
        do
        {
            // fprintf(stdout, "lookup_value()... retVal->value:\t%d\n", retVal->value);  // DEBUGGING
            if (needle == retVal->value)
            {
                    return retVal;  // Found it
            }

            retVal = retVal->next;
        } while(retVal);
    }

    return NULL;  // Didn't find it
}


// Purpose: Add a hashed (or not) entry into the given linked list
// Input:
//          node - A pointer to *any* node in the linked list
//          name - Name to associate with the node
//          value - Value to associate with the node if hashIt is false
// Output:  Head node (which may have just been created)
// Note:    
//          Insert at the end
//          If this is the first node in the linked list, headNode can be blank
struct HarkleDict* add_entry(struct HarkleDict* headNode, char* name, int value)
{
    struct HarkleDict* retVal = NULL;

    /* Input Validation */
    if (name)
    {
        /* Allocate Memory */
        retVal = build_a_node(name, value);

        /* Link the Node */
        if (retVal)
        {
            if (headNode)
            {
                find_last_node(headNode)->next = retVal;
                retVal = headNode;
            }
        }
    }

    return retVal;
}


// Purpose: Construct one node from start to finish
// Input:   
//          name - Name to dynamically allocate memory for
//          value - Value to assign
// Note:    
//          Allocates memory, assigns values
//          Always hashes name
//          Structs 'name' member is dynamically allocated
struct HarkleDict* build_a_node(char* name, int value)
{
    struct HarkleDict* retVal = NULL;
    size_t stringLength = 0;

    if (name)
    {
        stringLength = strlen(name);
        // Allocate struct memory
        retVal = (struct HarkleDict*)calloc(1, sizeof(struct HarkleDict));

        if (retVal)
        {
            // Allocate struct name memory
            retVal->name = (char*)calloc(stringLength + 1, sizeof(char));

            if (retVal->name)
            {
                // Copy the name into the struct
                if (strncpy(retVal->name, name, stringLength) != retVal->name)
                {
                    puts("Bad Copy!");  // DEBUGGING
                    /* Roll Back */
                    if (retVal->name)
                    {
                        free(retVal->name);
                        retVal->name = NULL;
                    }
                    if (retVal)
                    {
                        free(retVal);
                        retVal = NULL;
                    }
                }
                else
                {
                    // Fill in the rest
                    retVal->hash = hash(name);
                    retVal->value = value;
                    retVal->next = NULL;  // Just in case
                }
            }
        }
    }
    
    return retVal;
}


// Purpose: Zeroizes and deallocates entire linked list of HarkleDict nodes
// Input:   Pointer to the head node pointer
// Output:  Number of nodes destroyed
// Note:    Calls the recursive function destroy_a_node() which operates recursively
int destroy_a_list(struct HarkleDict** headNode)
{
    int retVal = destroy_a_node(headNode);

    return retVal;
}


// Purpose: Tear down a linked list of HarkleDict nodes
// Input:   Pointer to a node pointer
// Output:  Number of nodes zeroized and free()'d
// Note:    This function is recursive because this function is recursive
int destroy_a_node(struct HarkleDict** node_ptr)
{
    int retVal = 0;
    size_t stringLength = 0;

    if (node_ptr)
    {
        if (*node_ptr)
        {
            if ((*node_ptr)->next)
            {
                /* Destroy Last Node First */
                retVal += destroy_a_node(&((*node_ptr)->next));
            }
            else
            {
                /* Clear Name */
                if ((*node_ptr)->name)
                {
                    // Determine string length
                    stringLength = strlen((*node_ptr)->name);
                    // Zeroize string
                    memset((*node_ptr)->name, 0, stringLength);
                    // Free memory
                    free((*node_ptr)->name);
                    // Set string pointer to NULL
                    (*node_ptr)->name = NULL;
                }

                /* Clear Hash */
                /* NO LONGER IMPLEMENTED AS A CHAR POINTER */
                // if ((*node_ptr)->hash)
                // {
                //     // Determine string length
                //     stringLength = strlen((*node_ptr)->hash);
                //     // Zeroize string
                //     memset((*node_ptr)->hash, 0, stringLength);
                //     // Free memory
                //     free((*node_ptr)->hash);
                //     // Set string pointer to NULL
                //     (*node_ptr)->hash = NULL;
                // }
                (*node_ptr)->hash = 0;

                /* Clear Value */
                (*node_ptr)->value = 0;

                /* Free Struct Memory */
                free(*node_ptr);

                /* Zeroize Pointer */
                *node_ptr = NULL;

                /* Increment The Count */
                retVal++;
            }
        }
    }

    return retVal;
}


// Purpose: Find the tail node in the linked list
// Input:   Any node in the linked list
// Output:  Pointer to the tail node
struct HarkleDict* find_last_node(struct HarkleDict* node)
{
    struct HarkleDict* retVal = node;

    if (retVal)
    {
        while (retVal->next)
        {
            retVal = retVal->next;
        }
    }

    return retVal;
}
