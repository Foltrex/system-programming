/************************************************************ *************
LAB 1

Edit this file ONLY!

************************************************************ *************/



#include "dns.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct _list
{
	char* domen;
	IPADDRESS ip;
	struct _list *next;
} list;

const int SIZE = 12837;

DNSHandle InitDNS()
{
	DNSHandle hDNS = (uint32_t)(list*)calloc(SIZE, sizeof(list));
	if ((list*)hDNS != NULL)
		return hDNS;
	return INVALID_DNS_HANDLE;
}


uint32_t hashFunction(char* s, int table_size, const int key) {
	int hash = 0;
	int count = strlen(s);
	for (int i = 0; i < count; ++i) {
		hash = (hash * key + s[i]) % SIZE;
	}
	return hash;
}

void AddDomen(list* ptr, char* domen, IPADDRESS ip) {
	uint32_t domenSize = strlen(domen);

	ptr->domen = (char*)malloc(domenSize + 1);
	strcpy(ptr->domen, domen);
	ptr->ip = ip;
	ptr->next = NULL;
}
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


void LoadHostsFile(DNSHandle hDNS, const char* hostsFilePath)//открытие файла заполнение сервера
{
	FILE* fInput = NULL;
	if (!(fInput = fopen(hostsFilePath, "r")))
		return;

	char* str = (char*)malloc(201);
	uint32_t ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
	while (fscanf_s(fInput, "%d.%d.%d.%d %s", &ip1, &ip2, &ip3, &ip4, str, 200) != EOF)
	{
		IPADDRESS ip = (ip1 & 0xFF) << 24 |
			(ip2 & 0xFF) << 16 |
			(ip3 & 0xFF) << 8 |
			(ip4 & 0xFF);
		AddToHashTable(hDNS, str, ip);
	}
	fclose(fInput);
}



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




IPADDRESS DnsLookUp(DNSHandle hDNS, const char* hostName)
{
	list* dictionary = (list*)hDNS;
	uint32_t hash = hashFunction(hostName, SIZE, SIZE - 1);
	list* it = dictionary + hash;
	for (; it && strcmp(it->domen, hostName); it = it->next);
	
	return ( it ? it->ip : INVALID_IP_ADDRESS);
}