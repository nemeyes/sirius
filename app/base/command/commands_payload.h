#ifndef _COMMANDS_PAYLOAD_H_
#define _COMMANDS_PAYLOAD_H_

#include <cstdint>
#include <sirius_commands.h>

#pragma pack(1)
typedef struct _CMD_KEY_UP_IND_T
{
	int8_t input_type;
	int32_t	key_code;
} CMD_KEY_UP_IND_T;

typedef struct _CMD_KEYDOWN_IND_T
{
	int8_t input_type;
	int32_t	key_code;
} CMD_KEY_DOWN_IND_T;

typedef struct _CMD_MOUSE_LBD_IND_T
{
	int32_t x;
	int32_t y;
} CMD_MOUSE_LBD_IND_T;

typedef struct _CMD_MOUSE_LBU_IND_T
{
	int32_t x;
	int32_t y;
} CMD_MOUSE_LBU_IND_T;

typedef struct _CMD_MOUSE_RBD_IND_T
{
	int32_t x;
	int32_t y;
} CMD_MOUSE_RBD_IND_T;

typedef struct _CMD_MOUSE_RBU_IND_T
{
	int32_t x;
	int32_t y;
} CMD_MOUSE_RBU_IND_T;

typedef struct _CMD_MOUSE_MOVE_IND_T
{
	int32_t x_translation;
	int32_t y_translation;
} CMD_MOUSE_MOVE_IND_T;

typedef struct _CMD_MOUSE_LBD_MOVE_IND_T
{
	int32_t x_translation;
	int32_t y_translation;
} CMD_MOUSE_LBD_MOVE_IND_T;

typedef struct _CMD_MOUSE_RBD_MOVE_IND_T
{
	int32_t x_translation;
	int32_t y_translation;
} CMD_MOUSE_RBD_MOVE_IND_T;

typedef struct _CMD_MOUSE_LB_DCLICK_IND_T
{
	int32_t x;
	int32_t y;
} CMD_MOUSE_LB_DCLICK_IND_T;

typedef struct _CMD_MOUSE_RB_DCLICK_IND_T
{
	int32_t x;
	int32_t y;
} CMD_MOUSE_RB_DCLICK_IND_T;

typedef struct _CMD_MOUSE_WHEEL_IND_T
{
	int32_t z_delta;
	int32_t x;
	int32_t y;
} CMD_MOUSE_WHEEL_IND_T;

typedef struct _CMD_SEEK_KEY_DOWN_T
{
	int32_t diff;
}CMD_SEEK_KEY_DOWN_T;

typedef struct _CMD_SEEK_POS_T
{
	int32_t second;
}CMD_SEEK_POS_T;
typedef struct _CMD_SA_PLAYER_STATUS_T
{
	int32_t cmd_type;
	char*	IND_msg;
	int32_t IND_size;
}CMD_SA_PLAYER_STATUS_T;

typedef union _uifunion 
{
	unsigned int ui;
	float f;
} uifunion;

typedef struct _CMD_GYRO_IND_T
{
	uifunion x;
	uifunion y;
	uifunion z;
} CMD_GYRO_IND_T;

typedef struct _CMD_PINCH_ZOOM_IND_T
{
	uifunion delta;
} CMD_PINCH_ZOOM_IND_T;

typedef struct _CMD_GYRO_ROT_IND_T
{
	uifunion x;
	uifunion y;
	uifunion z;
	uifunion w;
} CMD_GYRO_ROT_IND_T;

typedef struct _CMD_MAT4X4_IND_T
{
	uifunion m[4][4];
} CMD_MAT4X4_IND_T;

#pragma pack()
#endif
