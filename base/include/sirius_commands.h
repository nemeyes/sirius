#ifndef _SIRIUS_COMMANDS_H_
#define _SIRIUS_COMMANDS_H_
#include <vector>

#include <stdint.h>

#define CMD_CONNECT_CLIENT_REQ							2101
#define CMD_CONNECT_CLIENT_RES							2102
#define CMD_ATTENDANT_INFO_IND							2104
#define CMD_DISCONNECT_CLIENT_REQ						2105
#define CMD_DISCONNECT_CLIENT_RES						2106

#define CMD_CONNECT_ATTENDANT_REQ						3101
#define CMD_CONNECT_ATTENDANT_RES						3102
#define CMD_DISCONNECT_ATTENDANT_REQ					3103
#define CMD_DISCONNECT_ATTENDANT_RES					3104
#define CMD_START_ATTENDANT_REQ							3105
#define CMD_START_ATTENDANT_RES							3106
#define CMD_STOP_ATTENDANT_REQ							3107
#define CMD_STOP_ATTENDANT_RES							3108

#define CMD_IFRAME_REQ_IND								2201
#define CMD_END2END_DATA_IND							2202

#define CMD_PLAYBACK_TOTALTIME_IND						3201
#define CMD_PLAYBACK_CURRENTTIME_IND					3202
#define CMD_PLAYBACK_END_IND							3203
#define CMD_PLAYBACK_CURRENTRATE_IND					3204


#define CMD_PLAY_REQ									4001 
#define CMD_PLAY_RES									4002
#define CMD_VIDEO_INDEXED_STREAM_DATA					4003
#define CMD_VIDEO_COORDINATES_STREAM_DATA				4004

#define CMD_KEY_DOWN_IND								2301
#define CMD_KEY_UP_IND									2302
#define CMD_MOUSE_LBD_IND								2303
#define CMD_MOUSE_LBU_IND								2304
#define CMD_MOUSE_RBD_IND								2305
#define CMD_MOUSE_RBU_IND								2306
#define CMD_MOUSE_MOVE_IND								2307
#define CMD_MOUSE_LB_DCLICK_IND							2308
#define CMD_MOUSE_RB_DCLICK_IND							2309
#define CMD_MOUSE_WHEEL_IND								2310

#define CMD_ERROR_IND									7000

#endif