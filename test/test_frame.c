#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "frame.h"

int
main(int argc, char *argv[])
{
	struct frame *f;
	char buffer[32];
	memset(buffer, 0, 32);

	// Неверные параметры
	assert(frame_parse(NULL, 10) == NULL);
	assert(frame_parse(buffer, 0) == NULL);

	// Неправильная длина сообщения
	buffer[1] = 64;  
	assert(frame_parse(buffer, 32) == NULL);

	// Не установлен бит окончания сообщения
	buffer[1] = 30;
	assert(frame_parse(buffer, 32) == NULL);

	// Правильный вызов  
	buffer[0] = 0x80;  
	assert(frame_parse(buffer, 32) != NULL);

	buffer[0] = 0x81;
	buffer[1] = 0x84;
	buffer[2] = 0xe3;
	buffer[3] = 0xa8;
	buffer[4] = 0x8f;
	buffer[5] = 0x28;
	buffer[6] = 0x97;
	buffer[7] = 0xcd;
	buffer[8] = 0xfc;
	buffer[9] = 0x5c;
	f = frame_parse(buffer, 10);
	assert(f != NULL);
	assert(strcmp(f->payload, "test") == 0);
	assert(f->opcode == OPCODE_TEXT);

	// Неправильные параметры
	assert(frame_create(NULL, 32, OPCODE_TEXT) == NULL);

	// Правильный вызов
	assert(frame_create(NULL, 0, OPCODE_PING) != NULL);
}