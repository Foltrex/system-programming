/************************************************************ *************
LAB 1

Edit this file ONLY!

************************************************************ *************/



#include "dns.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/// <summary>
/// The hash table is implemented using an array of lists.
/// To overcome collisions when hashing several elements
/// in one place, they are placed in a list
/// </summary>
typedef struct _list
{
	char* domen;
	IPADDRESS ip;
	struct _list *next;
} list;

// The size of the hash table, as well as the number of "active" elements at best 
const int SIZE = 12837;

/// <summary>
/// Returns a unique identifier, which is a pointer
/// (which, in fact, is the identifier of the element in memory)
///  cast to the DNSHandle type.
/// 
/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// This allows us to create multiple hash tables.
/// </summary>
/// <returns></returns>
DNSHandle InitDNS()
{
	DNSHandle hDNS = (uint32_t)(list*)calloc(SIZE, sizeof(list));
	if ((list*)hDNS != NULL)
		return hDNS;

	return INVALID_DNS_HANDLE;
}


// Hash function implemented through modular hashing using Horner's method
uint32_t hashFunction(char* s, int table_size, const int key) {
	int hash = 0;
	int count = strlen(s);
	for (int i = 0; i < count; ++i) {
		hash = (hash * key + s[i]) % SIZE;
	}

	return hash;
}

void AddDomen(list* ptr, char* domen, IPADDRESS ip);


// Adding an item to a hash table
void AddToHashTable(DNSHandle hDNS, char* domen, IPADDRESS ip) {
	list* dictionary = (list*)hDNS;

	uint32_t hash = hashFunction(domen, SIZE, SIZE - 1);

	if (dictionary[hash].domen)
	{
		list* it = (list*)malloc(sizeof(list));
		AddDomen(it, domen, ip);
		it->next = dictionary[hash].next;
		dictionary[hash].next = it;
	}
	else
		AddDomen(dictionary + hash, domen, ip);
}

// Adding an item at index hash to a list 
void AddDomen(list* ptr, char* domen, IPADDRESS ip) {
	uint32_t domenSize = strlen(domen);

	ptr->domen = (char*)malloc(domenSize + 1);
	strcpy(ptr->domen, domen);
	ptr->ip = ip;
	ptr->next = NULL;
}

// Initializing a hash table by reading from a file
void LoadHostsFile(DNSHandle hDNS, const char* hostsFilePath)
{
	FILE* fInput = NULL;
	if (!(fInput = fopen(hostsFilePath, "r")))
		return;

	uint32_t ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
	char* str = (char*)malloc(201);
	while (fscanf_s(fInput, "%d.%d.%d.%d %s", &ip1, &ip2, &ip3, &ip4, str, 200) != EOF)
	{
		// since each of the 4 numbers in ip is representable using 8 bits, then 4 such numbers can be packed in uint
		IPADDRESS ip = (ip1 & 0xFF) << 24 |
						(ip2 & 0xFF) << 16 |
						(ip3 & 0xFF) << 8 |
						(ip4 & 0xFF);
		AddToHashTable(hDNS, str, ip);
	}

	fclose(fInput);
}

/// <summary>
/// Freeing resources seized for hashtable storage
/// </summary>
/// <param name="hDNS"> - hash table unique identifier</param>
void ShutdownDNS(DNSHandle hDNS)
{
	list* arr = (list*)hDNS;
	for (int i = 0; i < SIZE; ++i) {
		list* next = arr[i].next;
		while (next) {
			list* cur = next;
			next = next->next;
			free(cur);
		}
	}

	free(arr);
}



/// <summary>
/// Search for an address by domain
/// </summary>
/// <param name="hDNS"></param>
/// <param name="hostName"></param>
/// <returns> IP-address </returns>
IPADDRESS DnsLookUp(DNSHandle hDNS, const char* hostName)
{
	list* dictionary = (list*)hDNS;
	uint32_t hash = hashFunction(hostName, SIZE, SIZE - 1);
	list* it = dictionary + hash;
	for (; it && strcmp(it->domen, hostName); it = it->next);
	
	return ( it ? it->ip : INVALID_IP_ADDRESS);
}