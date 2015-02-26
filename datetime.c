#include "ltime.h"
#include <sys/time.h>
/*
 * 2014-01-28	__eq/le/lt and parameterToVMS fixed.... big time...
 */

#define isDatetime(L, i) luaL_checkudata(L, i, LTIME_MT_DATETIME)

/*
 * Create a new datetime object
 * - leave the new object on top of the stack
 * - return pointer to datetime object
 */
t_datetime *newDatetime(lua_State *L) {
	t_datetime *self = (t_datetime *)lua_newuserdata(L, sizeof(t_datetime));
	luaL_setmetatable(L, LTIME_MT_DATETIME);
	return self;
}

/*
 *  Extract unsigned value from Lua table
 */
static unsigned tableFieldToUnsigned(lua_State *L, int index, const char *key) {
	
	lua_pushstring(L, key);
	lua_rawget(L, index);
	unsigned value = (unsigned)lua_tointeger(L, -1);	// _tounsiged() in 5.2
	lua_pop(L, 1);
	return value;
}

/*
 *  Gregorian calendar Y-M-D to Modified Julian Day
 *  M from 01 to 12 and D from 01 to 31
 *  Return -1 if input date is prior to 1858-11-17
 *  Algorithm from http://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
 */
int toMJD(unsigned Y, unsigned M, unsigned D) {
	
	if (M < 1 || M > 12 || D < 1 || 
		(M==1 && D > 366) || (M > 1 && D > 31) ||	// special case: month 1 is allowed to have up to 366 days
		Y < 1858 || (Y == 1858 && (M < 11 || (M == 11 && D < 17))))
		return -1;
	if (M < 3) {
		Y = Y - 1;
		M = M + 12;
	}	
	int a = Y / 100;
	int b = a / 4;
	int c = 2 - a + b;
	int e = 365.25 * ( Y + 4716);
	int f = 30.6001 * (M + 1);
	return c + D + e + f - 2401525;
}

/*
 *  Gregorian calendar Y-M-D h:m:s.us to VMS timestamp
 *  M from 01 to 12, D from 01 to 31, h from 00 to 23, m from 00 to 59, s from 00 to 59
 *  Return -1 if input datetime is prior to 1858-11-17 00:00:00.0000000
 */
static long long toVMS(unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	long long MJD = (long long)toMJD(Y, M, D);
	if (MJD == -1 || h > 23 || m > 59 || s > 59 || us > 999999)
		return -1;
	return ((((MJD * 24 + h) * 60 + m) * 60 + s) * (long long)1e6 + us) * 10;
}

/*
 *  Lua parameter at index to VMS timestamp
 */
static long long parameterToVMS(lua_State *L, int index) {
	int	ltype = index==0 ? LUA_TNIL : lua_type(L, index);
	/* nil is considered "now" */
	if (ltype == LUA_TNIL) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return VMS_1970 + 10000000LL * (long long)tv.tv_sec + 10LL * (long long)tv.tv_usec;
	}
	/* Parameter is a number, considered number of seconds since 1970-01-01 00:00:00 */
	else if (ltype == LUA_TNUMBER) {
		return VMS_1970 + (long long)(10000000LL * lua_tonumber(L, index));
	}
	/* Parameter is a string, considered strict ISO 8601 string */
	else if (ltype == LUA_TSTRING) {
		// Relaxed version.... more like MySQL. And hopefully fast.
		unsigned Y = 0, M = 0, D = 0, h = 0, m = 0, s = 0, us=0;
		register int n;
		char *p = (char *)lua_tostring(L, index);
		while (*p && isspace(*p)) p++;
		// <Y> <M> <D>
		n = 4;
		while (*p && isdigit(*p) && n--) Y=Y*10+(*p++ -'0');
		if (*p && !isdigit(*p)) p++;
		n = 2;
		while (*p && isdigit(*p) && n--) M=M*10+(*p++ -'0');
		if (*p && !isdigit(*p)) p++;
		n = 2;
		while (*p && isdigit(*p) && n--) D=D*10+(*p++ -'0');
		while (*p && !isdigit(*p)) p++;

		// optional time part [T] <h> <m> [<s> [ . <fs>]]
		if (*p) {
			//if (*p && (*p=='T' || *p==':')) p++;
			n = 2;
			while (*p && isdigit(*p) && n--) h=h*10+(*p++ -'0');
			if (*p && !isdigit(*p)) p++;
			n = 2;
			while (*p && isdigit(*p) && n--) m=m*10+(*p++ -'0');
			if (*p && !isdigit(*p)) p++;
			n = 2;
			while (*p && isdigit(*p) && n--) s=s*10+(*p++ -'0');
			if (*p && *p=='.') p++;
			if (*p) {
				n=6;
				while (*p && isdigit(*p) && n--) us=us*10+(*p++ -'0');
				while (n-- > 0) us=us*10; // right pad
			}
		}
		long long t = toVMS(Y, M, D, h, m, s, us);
		if (t == -1)
			luaL_error(L, LTIME_ERR_DATETIME_CONSTRUCTOR);
		return t;

		//*/
	}
	/* Parameter is a table */
	else if (ltype == LUA_TTABLE) {
		unsigned Y = tableFieldToUnsigned(L, index, LTIME_KEY_YEAR);
		unsigned M = tableFieldToUnsigned(L, index, LTIME_KEY_MONTH);
		unsigned D = tableFieldToUnsigned(L, index, LTIME_KEY_DAY);
		unsigned h = tableFieldToUnsigned(L, index, LTIME_KEY_HOUR);
		unsigned m = tableFieldToUnsigned(L, index, LTIME_KEY_MIN);
		unsigned s = tableFieldToUnsigned(L, index, LTIME_KEY_SEC);
		unsigned us = tableFieldToUnsigned(L, index, LTIME_KEY_USEC);
		long long t = toVMS(Y, M, D, h, m, s, us);
		if (t == -1)
			luaL_error(L, LTIME_ERR_DATETIME_CONSTRUCTOR);
		return t;
	}
	else {
		t_datetime *param = luaL_checkudata(L, index, LTIME_MT_DATETIME);
		return param->t;
	}
	return 0;
}

/*
 *  Modified Julian Day to Gregorian Calendar Y-M-D
 *  Algorithm from http://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
 */
static void fromMJD(int MJD, unsigned *Y, unsigned *M, unsigned *D) {

	int z = MJD + 2400001;
	int w = (z - 1867216.25) / 36524.25;
	int x = w / 4;
	int a = z + 1 + w - x;
	int b = a + 1524;
	int c = (b - 122.1) / 365.25;
	int d = 365.25 * c;
	int e = (b - d) / 30.6001;
	int f = 30.6001 * e;
	int day = b - d - f;
	int month = (e - 1) <= 12 ? (e - 1) : (e - 13);
	int year = month <= 2 ? (c - 4715) : (c - 4716);
	if (Y)
		*Y = year;
	if (M)
		*M = month;
	if (D)
		*D = day;
}

/*
 *  VMS timestamp to Gregorian calendar Y-M-D h:m:s.us
 */
void fromVMS(long long t, unsigned *Y, unsigned *M, unsigned *D, unsigned *h, unsigned *m, unsigned *s, unsigned *us) {
	
	if (us)
		*us = t / 10 % (long long)1e6;
	if (s)
		*s = t / (long long)1e7 % 60;
	if (m)
		*m = t / (long long)6e8 % 60;
	if (h)
		*h = t / (long long)3.6e10 % 24;
	fromMJD(t / (long long)8.64e11, Y, M, D);
}

/*
 *  Time = Ltime.Time(parameter)
 */
int datetime_new(lua_State *L) {
	int top = lua_gettop(L);
	t_datetime *self = newDatetime(L);
	self->t = parameterToVMS(L, top>0 ? 1 : 0 );
	return 1;
}

/*
 *  Time = Ltime.mktime(Y, M, D, h, m, s, us)
 */
int datetime_mktime(lua_State *L) {
	if (lua_gettop(L) == 0)
		luaL_error(L, LTIME_ERR_DATETIME_CONSTRUCTOR);

	long long t = toVMS(
			luaL_checkinteger(L, 1),
			luaL_optinteger(L, 2, 1),
			luaL_optinteger(L, 3, 1),
			luaL_optinteger(L, 4, 0),
			luaL_optinteger(L, 5, 0),
			luaL_optinteger(L, 6, 0),
			luaL_optinteger(L, 7, 0)
		);
	if (t == -1)
		luaL_error(L, LTIME_ERR_DATETIME_CONSTRUCTOR);
	t_datetime *self = newDatetime(L);
	self->t = t;
	return 1;
}

/*
 * 	Return a clone of the Time object
 *  Time2 = Time:clone()
 */ 
static int datetime_clone(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	t_datetime *clone = newDatetime(L);
	clone->t = self->t;
	return 1;
}

/*
 *  Set/get date without affecting time of day
 *  Setter: Time = Time:date(parameter)
 *  Getter: Time2 = Time:date()
 */
static int datetime_date(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	unsigned Y, M, D, h, m, s, us;
	fromVMS(self->t, &Y, &M, &D, &h, &m, &s, &us);
	if (lua_gettop(L) == 1) {
		t_datetime *result = newDatetime(L);
		result->t = toVMS(Y, M, D, 0, 0, 0, 0);
	} else {
		long long t = parameterToVMS(L, 2);
		fromVMS(t, &Y, &M, &D, 0, 0, 0, 0);
		self->t = toVMS(Y, M, D, h, m, s, us);
		lua_settop(L, 1);
	}
	return 1;
}

/*
 *  Set/get time of day without affecting date
 *  Setter: Time = Time:time(parameter)
 *  Getter: Epoch = Time:time()
 */
static int datetime_time(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	unsigned Y, M, D, h, m, s, us;
	fromVMS(self->t, &Y, &M, &D, &h, &m, &s, &us);
	if (lua_gettop(L) == 1) { // getter
		t_epoch *result = newEpoch(L);
		result->t = ((((long long)h * 60 + m) * 60 + s) * (long long)1e6 + us) * 10;
	} else { // setter
		long long t = parameterToTicks(L, 2);
		if (t < 0 || t >= 864000000000)
			luaL_error(L, LTIME_ERR_TIME_OUT_OF_RANGE);
		fromTicks(t, 0, &h, &m, &s, &us);
		self->t = toVMS(Y, M, D, h, m, s, us);
		lua_settop(L, 1);
	}
	return 1;
}

/*
 * 	Add onto the same object
 *  Time = Time:add(parameter)
 */
static int datetime_self_add(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	self->t = self->t + parameterToTicks(L, 2);
	if (self->t < 0)
		luaL_error(L, LTIME_ERR_DATETIME_OUT_OF_RANGE);
	lua_settop(L, 1);
	return 1;
}

/*
 * 	Substract from the same object
 *  Time = Time:sub(parameter)
 */
static int datetime_self_sub(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	self->t = self->t - parameterToTicks(L, 2);
	if (self->t < 0)
		luaL_error(L, LTIME_ERR_DATETIME_OUT_OF_RANGE);
	lua_settop(L, 1);
	return 1;
}

/*
 * 	Round down the same object to the next "stop"
 *  Time = Time:floor(parameter)
 */
static int datetime_floor(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	long long t = parameterToTicks(L, 2);
	if (t == 0)
		luaL_error(L, LTIME_ERR_MOD_ZERO_UNDEFINED);
	self->t -= self->t % t;
	lua_pushvalue(L, 1); // return self
	return 1;
}

/*
 * 	Round up the object to the next "stop"
 *  Time2 = Time:ceil(parameter)
 */
static int datetime_ceil(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	long long t = parameterToTicks(L, 2);
	if (t == 0)
		luaL_error(L, LTIME_ERR_MOD_ZERO_UNDEFINED);
	//self->t = self->t + (self->t + t + 1) % t;
	long long x = self->t % t;
	if (x>0) self->t += t-x; 
	lua_pushvalue(L, 1); // return self
	return 1;
}

/*
 * 	Return true if leap year, false otherwise
 *  boolean = Time:leap()
 */
static int datetime_leap(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	unsigned Y;
	fromVMS(self->t, &Y, 0, 0, 0, 0, 0, 0);
	lua_pushboolean(L, Y % 4 == 0 && (Y % 100 != 0 || Y % 400 == 0));
	return 1;
}

/*
 *  Set/get yearday without affecting time of day
 * 	(1 = January 1st, ...)
 *  Setter: Time = Time:doy(yearday)
 *  Getter: yearday = Time:doy()
 */
static int datetime_yearday(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	if (lua_gettop(L) > 1 && lua_isnumber(L, 2)) {
		int yearday = lua_tointeger(L, 2);
		if (yearday >= 1) {
			unsigned Y, M, D, h, m, s, us;
			fromVMS(self->t, &Y, &M, &D, &h, &m, &s, &us);
			self->t = ((((((long long)toMJD(Y, 01, 01) + yearday - 1) * 24 + h) * 60 + m) * 60 + s) * 1e6 + us) * 10;
		}
		lua_settop(L, 1);
	} else {
		unsigned Y, M, D;
		fromVMS(self->t, &Y, &M, &D, 0, 0, 0, 0);
		lua_pushnumber(L, toMJD(Y, M, D) - toMJD(Y, 1, 1) + 1);
	}
	return 1;
}

/*
 * 	Get weekday (1 = Monday, ..., 7 = Sunday)
 *  weekday = Time:weekday()
 */
static int datetime_weekday(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	lua_pushnumber(L, (self->t / (long long)1e7 / 86400 + 2) % 7 + 1);
	return 1;
}

/*
 * 	Set/get date via MJD without affecting time of day
 *  Setter: Time = Time:mjd(mjd)
 *  Getter: mjd = Time:mjd()
 */
static int datetime_mjd(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	if (lua_gettop(L) > 1 && lua_isnumber(L, 2)) {
		self->t = (long long)lua_tonumber(L, 2) * 86400 * 1e7;
		lua_settop(L, 1);
	} else {
		lua_pushnumber(L, self->t / (long long)1e7 / 86400);
	}
	return 1;
}

/*
 * 	Set/get VMS timestamp
 *  Setter: Time = Time:vms(vmsTimestamp)
 *  Getter: vmsTimestamp = Time:vms()
 */
static int datetime_vms(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	if (lua_gettop(L) > 1) {
		int ltype = lua_type(L,2);
		if (ltype == LUA_TSTRING) {
			size_t len=0;
			const char *str = lua_tolstring(L, 2, &len);
			if (len==2 && str[0]=='*') {
				// Getter Mode!
				char buf[20];
				if (str[1]=='x') {	// as 16 hex nibbles
					snprintf(buf, 17, "%016llX", self->t);
					lua_pushstring(L, buf);
				}
				else if (str[1]=='h') {	// as proper hex string
					snprintf(buf, 19, "0x%llX", self->t);
					lua_pushstring(L, buf);
				}
				else if (str[1]=='b') { // as binary object (8 bytes)
					long long val = self->t;
					for (int i=7; i>=0; i--) {
						buf[i] = (unsigned char)(val & 0xFF);
						val = val >> 8;
						lua_pushlstring(L, buf, 8);
					}
				}
				else
					lua_pushnil(L);
				return 1;
			}
			else // Setter mode!
				self->t = strtoll(str, NULL, 0);
		}
		else { // Setter mode!
			self->t = luaL_checkinteger(L, 2); // 5.2: luaL_checknumber
			if (self->t < 0) luaL_error(L, LTIME_ERR_DATETIME_OUT_OF_RANGE);
		}
		lua_pushvalue(L, 1);
	} else { // Getter Mode!
		lua_pushinteger(L, self->t); // 5.2: lua_pushnumber
	}
	return 1;
}

/*
 * 	Set/get unix timestamp (seconds since 1970) either as 
 *  a number representing floating point seconds, or as sec+usec
 *  Setter: Time = Time:unix(second_microsecond)
 * 			Time = Time:unix(seconds, microseconds)
 *  Getter: unixTimestamp = Time:unix()
 */
static int datetime_unix(lua_State *L) {
	
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	int n = lua_gettop(L) ;
	// set fractional seconds case
	if (n == 2) {
		self->t = VMS_1970 + 10000000LL * luaL_checknumber(L, 2);
		lua_settop(L, 1);
	}
	// set seconds and microseconds case
	else if (n == 3) {
		long usec = (long)luaL_checkinteger(L, 3);	// 5.2: luaL_checklong
		if (usec>999999) usec = 999999;
		else if (usec<0) usec = 0;

		self->t = VMS_1970 + 10000000LL * luaL_checkinteger(L, 2) + 10LL * usec; // 5.2: luaL_checklong
		lua_settop(L, 1);
	} 
	// return seconds/microseconds case
	else {
		long long ts = (self->t - VMS_1970) / 10LL;
		long long sec = ts / 1000000LL;
		lua_pushnumber(L, (lua_Number)sec);
		lua_pushinteger(L, (ts % 1000000LL));
		return 2;
	}
	return 1;
}

/*
 *  Time:__unm()
 */
static int datetime_unm(lua_State *L) {

	luaL_error(L, LTIME_ERR_DATETIME_UNM_UNDEFINED);
	return 0;
}

/*
 *  Time.__add(a, b)
 */
static int datetime_add(lua_State *L) {
	long long t1, t2;
	t_datetime *self = luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	t1 = self->t;
	t2 = parameterToTicks(L, 2);
	t_datetime *result = newDatetime(L);
	result->t = t1 + t2;
	if (result->t < 0)
		luaL_error(L, LTIME_ERR_DATETIME_OUT_OF_RANGE);
	return 1;
}

/*
 *  Time.__sub(a, b)
 */
static int datetime_sub(lua_State *L) {

	t_datetime *a = luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	t_datetime *b = luaL_testudata(L, 2, LTIME_MT_DATETIME);
	if (b) {
		t_epoch *result = newEpoch(L);
		result->t = a->t - b->t;
	} else {
		long long t = parameterToTicks(L, 2);
		t_datetime *result = newDatetime(L);
		result->t = a->t - t;
		if (result->t < 0)
			luaL_error(L, LTIME_ERR_DATETIME_OUT_OF_RANGE);
	}
	return 1;
}

/*
 *  Time.__mul(a, b)
 */
static int datetime_mul(lua_State *L) {
	
	luaL_error(L, LTIME_ERR_DATETIME_MUL_UNDEFINED);
	return 0;
}

/*
 *  Time.__div(a, b)
 */
static int datetime_div(lua_State *L) {
	
	luaL_error(L, LTIME_ERR_DATETIME_DIV_UNDEFINED);
	return 0;
}

/*
 *  Time.__mod(a, b)
 */
static int datetime_mod(lua_State *L) {

	t_datetime *self = luaL_checkudata(L, 1, LTIME_MT_DATETIME);

	/* Second operand is a number, considered number of seconds, return seconds */
	if (lua_type(L, 2) == LUA_TNUMBER) {
		long long t = lua_tonumber(L, 2) * 1e7;
		if (t == 0)
			luaL_error(L, LTIME_ERR_MOD_ZERO_UNDEFINED);
		lua_pushnumber(L, self->t % t / 1e7);
	}
	/* Else create Epoch value with second parameter, return Epoch */
	else {
		long long t = parameterToTicks(L, 2);
		if (t == 0)
			luaL_error(L, LTIME_ERR_MOD_ZERO_UNDEFINED);
		t_epoch *result = newEpoch(L);
		result->t = self->t % t;
	}
	return 1;
}

/*
 *  Time.__eq(a, b)
 */
static int datetime_eq(lua_State *L) {
	lua_pushboolean(L, parameterToVMS(L, 1) == parameterToVMS(L, 2));
	return 1;
}

/*
 *  Time.__lt(a, b)
 */
static int datetime_lt(lua_State *L) {
	lua_pushboolean(L, parameterToVMS(L, 1) < parameterToVMS(L, 2));
	return 1;
}

/*
 *  Time.__le(a, b)
 */
static int datetime_le(lua_State *L) {
	lua_pushboolean(L, parameterToVMS(L, 1) <= parameterToVMS(L, 2));
	return 1;
}

/*
 *  Time:__tostring()
 * 	Format the value for use in MySQL
 */
static int datetime_tostring(lua_State *L) {
	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	unsigned Y, M, D, h, m, s, us;
	char buffer[50];
	fromVMS(self->t, &Y, &M, &D, &h, &m, &s, &us);
/*	if (us && us % 1000)
		sprintf(buffer, "%04u-%02u-%02u %02u:%02u:%02u.%06u", Y, M, D, h, m, s, us);
	else if (us)
		sprintf(buffer, "%04u-%02u-%02u %02u:%02u:%02u.%03u", Y, M, D, h, m, s, us / 1000);
	else //if (h || m || s)
		sprintf(buffer, "%04u-%02u-%02u %02u:%02u:%02u", Y, M, D, h, m, s);
//	else
//		sprintf(buffer, "%04u-%02u-%02u", Y, M, D);
*/

	if (us)
		sprintf(buffer, "%04u-%02u-%02u %02u:%02u:%02u.%06u", Y, M, D, h, m, s, us);
	else
		sprintf(buffer, "%04u-%02u-%02u %02u:%02u:%02u", Y, M, D, h, m, s);

	lua_pushstring(L, buffer);
	return 1;
}

/*
 *  Time:__gc()
 * /
static int datetime_gc(lua_State *L) {
	
	lua_pushboolean(L, 1);
    return 1;  
}*/

int open_datetime(lua_State *L) {
		
    static const luaL_Reg datetime_methods[] = {
		{"format", datetime_format},
		{"clone", datetime_clone},
		{"date", datetime_date},
		{"time", datetime_time},
		{"add", datetime_self_add},
		{"sub", datetime_self_sub},
		{"floor", datetime_floor},
		{"ceil", datetime_ceil},
		{"leap", datetime_leap},
		{"yearday", datetime_yearday},
		{"doy", datetime_yearday},
		{"weekday", datetime_weekday},
		{"mjd", datetime_mjd},
		{"vms", datetime_vms},
		{"unix", datetime_unix},
		{NULL, NULL}
    };

    static const luaL_Reg datetime_meta_methods[] = {
		{"__unm", datetime_unm},
		{"__add", datetime_add},
		{"__sub", datetime_sub},
		{"__mul", datetime_mul},
		{"__div", datetime_div},
		{"__mod", datetime_mod},
		{"__eq",  datetime_eq},
		{"__lt",  datetime_lt},
		{"__le",  datetime_le},
		{"__tostring", datetime_tostring},
//		{"__gc", datetime_gc},
		{NULL, NULL}
    };

	// create the metatable first
	luaL_newmetatable(L, LTIME_MT_DATETIME);
	// and set all metamethods except __index
	luaL_setfuncs(L, datetime_meta_methods, 0);

	// create the library table
	luaL_newlib(L, datetime_methods);
	// and set the __index metamethod
	lua_setfield(L, -2, "__index");
	
    return 1;  
}
