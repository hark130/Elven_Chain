#indnef __HARKLEDICT_H__
#define __HARKLEDICT_H__

/*
 *	USAGE:
 *		Start - 	
 */

struct HarkleDict 
{
    struct HarkleDict* next;	// Next node
    char* name;					// Human readable name
    unsigned int hash; 			// Lookup hash
    int value;					// Lookup value
};

#define HASHSIZE 101

#ifndef TRUE
#define TRUE ((int)1)
#endif // TRUE

#ifndef FALSE
#define FALSE ((int)0)
#endif // FALSE

// Purpose: Hash an input string
// Input:   Hash input
// Output:  Hash as unsigned int
unsigned int hash(char* input);

// Purpose: Find the node associated with a given name
// Input:   
//          headNode - Pointer to the head node of the linked list
//          needle - String to find in the linked list
// Output:  Pointer to the struct in question
struct HarkleDict* lookup_name(struct HarkleDict* headNode, char* needle);

// Purpose: Find the node associated with a given hash
// Input:   
//          headNode - Pointer to the head node of the linked list
//          needle - Hash to find in the linked list
// Output:  Pointer to the struct in question
struct HarkleDict* lookup_hash(struct HarkleDict* headNode, unsigned int needle);

// Purpose: Find the node associated with a given hash
// Input:   
//          headNode - Pointer to the head node of the linked list
//          needle - Hash to find in the linked list
// Output:  Pointer to the struct in question
struct HarkleDict* lookup_value(struct HarkleDict* headNode, int needle);

// Purpose: Add a hashed (or not) entry into the given linked list
// Input:
//          node - A pointer to *any* node in the linked list
//          name - Name to associate with the node
//          value - Value to associate with the node if hashIt is false
// Output:  Head node (which may have just been created)
// Note:    
//          Insert at the end
//          If this is the first node in the linked list, headNode can be blank
struct HarkleDict* add_entry(struct HarkleDict* headNode, char* name, int value);

// Purpose: Construct one node from start to finish
// Input:   
//          name - Name to dynamically allocate memory for
//          value - Value to assign
// Note:    
//          Allocates memory, assigns values
//          Always hashes name
//          Structs 'name' member is dynamically allocated
struct HarkleDict* build_a_node(char* name, int value);

// Purpose: Zeroizes and deallocates entire linked list of HarkleDict nodes
// Input:   Pointer to the head node pointer
// Output:  Number of nodes destroyed
// Note:    Calls the recursive function destroy_a_node() which operates recursively
int destroy_a_list(struct HarkleDict** headNode);

// Purpose: Tear down a linked list of HarkleDict nodes
// Input:   Pointer to a node pointer
// Output:  Number of nodes zeroized and free()'d
// Note:    This function is recursive because this function is recursive
int destroy_a_node(struct HarkleDict** node_ptr);

// Purpose: Find the tail node in the linked list
// Input:   Any node in the linked list
// Output:  Pointer to the tail node
struct HarkleDict* find_last_node(struct HarkleDict* node);

#endif // __HARKLEHASH_H__