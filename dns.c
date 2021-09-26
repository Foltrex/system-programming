/************************************************************ *************
LAB 1

Edit this file ONLY!

************************************************************ *************/



#include "dns.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/// <summary>
/// Хеш-таблица реализована с помощью массива списков.
/// Для преодоления коллизий при хешировании нескольких
/// элементов в одну яйчейку они помещаются в список
/// </summary>
typedef struct _list
{
	char* domen;
	IPADDRESS ip;
	struct _list *next;
} list;

// Размер хеш таблицы, а также количество "активных" элементов в лучшем случае 
const int SIZE = 12837;

/// <summary>
/// Возвращает уникальный идентификатор, который является указателем(который по сути и есть 
/// индентификатор элемента в памяти) приведенным к типу DNSHandle.
/// 
/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// Это позволяет нам создать несколько таких хеш таблиц
/// </summary>
/// <returns></returns>
DNSHandle InitDNS()
{
	DNSHandle hDNS = (uint32_t)(list*)calloc(SIZE, sizeof(list));
	if ((list*)hDNS != NULL)
		return hDNS;
	return INVALID_DNS_HANDLE;
}


// Хеш-фукнция, реализованная через модульное хеширование с использованием метода Горнера
uint32_t hashFunction(char* s, int table_size, const int key) {
	int hash = 0;
	int count = strlen(s);
	for (int i = 0; i < count; ++i) {
		hash = (hash * key + s[i]) % SIZE;
	}
	return hash;
}

void AddDomen(list* ptr, char* domen, IPADDRESS ip);


// Добавление элемента в хеш-таблицу
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

// Добавление элемента по индексу hash в список 
void AddDomen(list* ptr, char* domen, IPADDRESS ip) {
	uint32_t domenSize = strlen(domen);

	ptr->domen = (char*)malloc(domenSize + 1);
	strcpy(ptr->domen, domen);
	ptr->ip = ip;
	ptr->next = NULL;
}

// Инициализация хеш-таблицы чтением из файла
void LoadHostsFile(DNSHandle hDNS, const char* hostsFilePath)
{
	FILE* fInput = NULL;

	fInput = fopen(hostsFilePath, "r");

	uint32_t ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
	char* str = (char*)malloc(201);

	if (NULL == fInput)
		return;
	while (fscanf_s(fInput, "%d.%d.%d.%d %s", &ip1, &ip2, &ip3, &ip4, str, 200) != EOF)
	{
		// так как каждое из 4 чисел в ip представимо с использованием 8 бит, то в uint можно упаковать 4 таких числа
		IPADDRESS ip = (ip1 & 0xFF) << 24 |
			(ip2 & 0xFF) << 16 |
			(ip3 & 0xFF) << 8 |
			(ip4 & 0xFF);
		AddToHashTable(hDNS, str, ip);
	}
	free(str);
	fclose(fInput);
}

/// <summary>
///  освобождение ресурсов, захваченных для хранение таблицы
/// </summary>
/// <param name="hDNS"> - уникальный идентификатор хеш-таблицы</param>
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
/// Поиск адреса по домену
/// </summary>
/// <param name="hDNS"></param>
/// <param name="hostName"></param>
/// <returns> IP-адрес сайта </returns>
IPADDRESS DnsLookUp(DNSHandle hDNS, const char* hostName)
{
	list* dictionary = (list*)hDNS;
	uint32_t hash = hashFunction(hostName, SIZE, SIZE - 1);
	list* it = dictionary + hash;
	for (; it && strcmp(it->domen, hostName); it = it->next);
	
	return ( it ? it->ip : INVALID_IP_ADDRESS);
}