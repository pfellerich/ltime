#ifndef LTIME_H_
# define LTIME_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define LTIME_VERSION		"LTime v0.7"

#define LTIME_MT_DATETIME	"LTime_Datetime"
#define LTIME_MT_EPOCH		"LTime_Epoch"

#define LTIME_KEY_YEAR		"year"
#define LTIME_KEY_MONTH		"month"
#define LTIME_KEY_DAY		"day"
#define LTIME_KEY_HOUR		"hour"
#define LTIME_KEY_MIN		"min"
#define LTIME_KEY_SEC		"sec"
#define LTIME_KEY_USEC		"usec"

#define LTIME_ERR_DIV_ZERO_UNDEFINED		"Ltime: Division by zero is undefined.\n"
#define LTIME_ERR_MOD_ZERO_UNDEFINED		"Ltime: Modulo zero is undefined.\n"
#define LTIME_ERR_TIME_OUT_OF_RANGE			"Ltime: Time of day must be comprised between 00:00:00.000000 and 23.59.59.999999.\n"
#define LTIME_ERR_DATETIME_CONSTRUCTOR		"Ltime: Time constructor: unsupported parameter.\n"
#define LTIME_ERR_DATETIME_UNM_UNDEFINED	"Ltime: Unary minus not defined for Time objects.\n"
#define LTIME_ERR_DATETIME_ADD_TWO			"Ltime: Adding two Time values doesn't make sense.\n"
#define LTIME_ERR_DATETIME_SUB_ARG1			"Ltime: Time value as substraction's second operand doesn't make sense.\n"
#define LTIME_ERR_DATETIME_MUL_UNDEFINED	"Ltime: Multiplication not defined for Time objects.\n"
#define LTIME_ERR_DATETIME_DIV_UNDEFINED	"Ltime: Division not defined for Time objects.\n"
#define LTIME_ERR_DATETIME_MOD_ARG1			"Ltime: Time value as modulo's second operand doesn't make sense.\n"
#define LTIME_ERR_DATETIME_OUT_OF_RANGE		"Ltime: Time object: out of range value.\n"
#define LTIME_ERR_DATETIME_STRING_FORMAT	"Ltime: Time string format: YYYY-MM-DD[Thh:mm:ss[.uuuuuu]].\n"
#define LTIME_ERR_DATETIME_MISSING_FORMAT	"Ltime: Missing format string.\n"
#define LTIME_ERR_EPOCH_CONSTRUCTOR			"Ltime: Epoch constructor: unsupported parameter.\n"
#define LTIME_ERR_EPOCH_STRING_FORMAT		"Ltime: Epoch string format: [+/-][D ]hh:mm:ss[.uuuuuu].\n"
#define LTIME_ERR_EPOCH_MUL_NO_NUMBER		"Ltime: Epoch multiplication: one operand must be a number.\n"
#define LTIME_ERR_EPOCH_DIV_NO_NUMBER		"Ltime: Epoch division: second operand must be a number.\n"
#define LTIME_ERR_EPOCH_DIV_ARGERR			"Ltime: Epoch division: first operand must be an Epoch.\n"

#define MJD_1970	40587
#define VMS_1970	((long long)40587 * (long long)86400 * (long long)1e7)

typedef struct s_datetime {
	/* Number of 100 nanoseconds ticks since 1858-11-17 00:00:00 */
	long long t;
} t_datetime;

typedef struct s_epoch {
	/* Number of 100 nanoseconds ticks */
	long long t;
} t_epoch;

int toMJD(unsigned Y, unsigned M, unsigned D);
void fromVMS(long long t, unsigned *Y, unsigned *M, unsigned *D, unsigned *h, unsigned *m, unsigned *s, unsigned *us);
long long parameterToTicks(lua_State *L, int index);
int fromTicks(long long t, unsigned *D, unsigned *h, unsigned *m, unsigned *s, unsigned *us);
int datetime_format(lua_State *L);

t_epoch *newEpoch(lua_State *L);
t_datetime *newDatetime(lua_State *L);

#endif /* LTIME_H_ */
