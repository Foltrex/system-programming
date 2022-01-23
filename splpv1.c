/*
 * SPLPv1.c
 * The file is part of practical task for System programming course.
 * This file contains validation of SPLPv1 protocol.
 */


 /*
   Черненков, Артём, Витальевич
   № 13
 */



 /*
 ---------------------------------------------------------------------------------------------------------------------------
 # |      STATE      |         DESCRIPTION       |           ALLOWED MESSAGES            | NEW STATE | EXAMPLE
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 1 | INIT            | initial state             | A->B     CONNECT                      |     2     |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 2 | CONNECTING      | client is waiting for con-| A<-B     CONNECT_OK                   |     3     |
   |                 | nection approval from srv |                                       |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 3 | CONNECTED       | Connection is established | A->B     GET_VER                      |     4     |
   |                 |                           |        -------------------------------+-----------+----------------------
   |                 |                           |          One of the following:        |     5     |
   |                 |                           |          - GET_DATA                   |           |
   |                 |                           |          - GET_FILE                   |           |
   |                 |                           |          - GET_COMMAND                |           |
   |                 |                           |        -------------------------------+-----------+----------------------
   |                 |                           |          GET_B64                      |     6     |
   |                 |                           |        ------------------------------------------------------------------
   |                 |                           |          DISCONNECT                   |     7     |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 4 | WAITING_VER     | Client is waiting for     | A<-B     VERSION ver                  |     3     | VERSION 2
   |                 | server to provide version |          Where ver is an integer (>0) |           |
   |                 | information               |          value. Only a single space   |           |
   |                 |                           |          is allowed in the message    |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 5 | WAITING_DATA    | Client is waiting for a   | A<-B     CMD data CMD                 |     3     | GET_DATA a GET_DATA
   |                 | response from server      |                                       |           |
   |                 |                           |          CMD - command sent by the    |           |
   |                 |                           |           client in previous message  |           |
   |                 |                           |          data - string which contains |           |
   |                 |                           |           the following allowed cha-  |           |
   |                 |                           |           racters: small latin letter,|           |
   |                 |                           |           digits and '.'              |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 6 | WAITING_B64_DATA| Client is waiting for a   | A<-B     B64: data                    |     3     | B64: SGVsbG8=
   |                 | response from server.     |          where data is a base64 string|           |
   |                 |                           |          only 1 space is allowed      |           |
 --+-----------------+---------------------------+---------------------------------------+-----------+----------------------
 7 | DISCONNECTING   | Client is waiting for     | A<-B     DISCONNECT_OK                |     1     |
   |                 | server to close the       |                                       |           |
   |                 | connection                |                                       |           |
 ---------------------------------------------------------------------------------------------------------------------------

 IN CASE OF INVALID MESSAGE THE STATE SHOULD BE RESET TO 1 (INIT)

 */


#include "splpv1.h"
#include <string.h>
#include <stdlib.h>




 /* FUNCTION:  validate_message
  *
  * PURPOSE:
  *    This function is called for each SPLPv1 message between client
  *    and server
  *
  * PARAMETERS:
  *    msg - pointer to a structure which stores information about
  *    message
  *
  * RETURN VALUE:
  *    MESSAGE_VALID if the message is correct
  *    MESSAGE_INVALID if the message is incorrect or out of protocol
  *    state
  */


int curState = 1;
int command;

const char base64[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 0,	0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
};

const char results[] =
{
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,		0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
};

const char* commands[] = { "GET_DATA", "GET_FILE", "GET_COMMAND" };
const char* state3[] = { "GET_VER", "Null", "GET_B64", "DISCONNECT" };

int my_strcmp(char* message, char* commands[]) {
	int i = 0;
	for (; i < 3 && strcmp(commands[i], message); ++i);
	command = i;
	return (i > 2 ? 1 : 0);
}

int isCorrectState3(char* message, char* state3[]) {
	int state = 0;
	for (int i = 4; !state && i < 8; ++i) {
		if (i == 5) {
			state = !my_strcmp(message, commands);
		}
		else {
			state = !strcmp(message, state3[i - 4]);
		}
		curState = i;
	}
	return state;
}

enum test_status validate_A_TO_B(char* message) {
	if (curState == 1 && !strcmp(message, "CONNECT")) {
		curState = 2;
		return MESSAGE_VALID;
	}
	else if (curState == 3 && isCorrectState3(message, state3)) {
		return MESSAGE_VALID;
	}
	curState = 1;
	return MESSAGE_INVALID;
}
enum test_status validate_B_TO_A(char* message) {
	if (curState == 2 && !strcmp(message, "CONNECT_OK")) {
		curState = 3;
		return MESSAGE_VALID;
	}
	else if (curState == 4 && !strncmp(message, "VERSION ", 8)) {
		message += 8;
		if (*message > 48 && *message < 58) {
			for (++message; *message; message++)
				if (*message < 48 || *message > 57) {
					curState = 1;
					return MESSAGE_INVALID;
				}
			curState = 3;
			return MESSAGE_VALID;
		}
	}
	else if (curState == 5) {
		int l = strlen(commands[command]);
		if (!strncmp(message, commands[command], l))
		{
			message += l;
			if (*message == ' ')
			{
				message++;
				for (; results[*message]; ++message);
				char* space = (*message == ' ') ? message + 1 : NULL;
				if (space && !strcmp(space, commands[command])) {
					curState = 3;
					return MESSAGE_VALID;
				}
			}
		}
	}
	else if (curState == 6 && !strncmp(message, "B64: ", 5)) {
		message += 5;
		char* b = message;
		for (; base64[*message]; ++message);
		char eq = 0;
		for (; (eq < 2) && (message[eq] == '='); ++eq);
		if ((message - b + eq) % 4 == 0 && !message[eq]) {
			curState = 3;
			return MESSAGE_VALID;
		}
	}
	else if (curState == 7 && !strcmp(message, "DISCONNECT_OK")) {
		curState = 1;
		return MESSAGE_VALID;
	}
	curState = 1;
	return MESSAGE_INVALID;
}

enum test_status validate_message(struct Message* msg) {
	return (msg->direction == A_TO_B
		? validate_A_TO_B(msg->text_message)
		: validate_B_TO_A(msg->text_message)
		);
}
