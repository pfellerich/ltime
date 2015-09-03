#include "ltime.h"
/*
 * 2014-01-28	__eq/le/lt and parameterToEpoch fixed.... big time...
 */

#define isEpoch(L, i) luaL_checkudata(L, i, LTIME_MT_EPOCH)

/*
 * Create a new epoch object
 * - leave the new object on top of the stack
 * - return pointer to datetime object
 */
t_epoch *newEpoch(lua_State *L) {
	t_epoch *self = (t_epoch *)lua_newuserdata(L, sizeof(t_epoch));
	luaL_setmetatable(L, LTIME_MT_EPOCH);
	return self;
}



/*
 *  Lua parameter at index to 100 nanosecond ticks
 */
long long parameterToTicks(lua_State *L, int index) {
	int	ltype = index==0 ? LUA_TNIL : lua_type(L, index);
	/* No parameter or nil, considered 0 second */
	if (ltype == LUA_TNIL) {
		return 0;
	}
	/* Parameter is a number, considered number of seconds */
	else if (ltype == LUA_TNUMBER) {
		// Must do this below to avoid pathological truncation errors....
		lua_Number x = lua_tonumber(L, index) * 1e7;
		x += x>0 ? .5 : -.5;
		return x;
	}
	/* Parameter is a string, considered deltatime string */
	else if (ltype == LUA_TSTRING) {
		// Relaxed version.... more like MySQL. And hopefully fast.
		unsigned int D = 0, h = 0, m = 0, s = 0, us = 0;
		int sign = 1;
		register int n;
		char *p = (char *)lua_tostring(L, index);

		while (*p && isspace(*p)) p++;		// skip leading spaces
		if (!*p) goto fail;
		if (*p=='+') p++;					// optional sign
		else if (*p=='-') { sign = -1; p++; };

		n = 9;								// up to 9 digits allowed for days (below 2^32)
		while (*p && isdigit(*p) && n--) D=D*10+(*p++ -'0');
		if (!*p) goto fail;

		if (isspace(*p)) {					// space separator: D was really D and not h
			p++;
			// need to do hours now
			n = 9;							// up to 9 digits allowed for hours (below 2^32)
			while (*p && isdigit(*p) && n--) h=h*10+(*p++ -'0');
			if (!*p) goto fail;
		}
		else {								// other separator, number was probably hours
			h = D; D = 0;
		}

		// now looking at the colon preceding the mandatory minutes
		if (*p!=':') goto fail;
		p++; 
		n = 2; // minutes have 1 or 2 digits
		while (*p && isdigit(*p) && n--) m=m*10+(*p++ -'0');
		//if (n==2) goto fail;
		if (*p) {							// if anything follows, must be a colon
			if (*p!=':') goto fail;
			p++;
			n = 2; 							// seconds
			while (*p && isdigit(*p) && n--) s=s*10+(*p++ -'0');
			// now look at optional fractional seconds
			if (*p) {
				if (*p!='.') goto fail;
				p++;
				n=6;
				while (*p && isdigit(*p) && n--) us=us*10+(*p++ -'0');
				while (n-- > 0) us=us*10; // right pad
			}
		}

		return sign * (((((long long)D * 24 + h) * 60 + m) * 60 + s) * (long long)1e6 + us) * 10;

fail:
		luaL_error(L, LTIME_ERR_EPOCH_CONSTRUCTOR);
		return 0;

		//*/
	}
	else {
		t_epoch *param = luaL_checkudata(L, index, LTIME_MT_EPOCH);
		return param->t;
	}

	return 0;
}

/*
 *  100 nanosecond ticks to +/- D h:m:s.us
 */
int fromTicks(long long t, unsigned *D, unsigned *h, unsigned *m, unsigned *s, unsigned *us) {
	
	char sign = (t >= 0 ? 1 : -1);
	t = (t >= 0 ? t : -t);
	if (us)
		*us = t / 10 % (unsigned long long)1e6;
	if (s)
		*s = t / (unsigned long long)1e7 % 60;
	if (m)
		*m = t / (unsigned long long)6e8 % 60;
	if (h)
		*h = t / (unsigned long long)3.6e10 % 24;
	if (D)
		*D = t / (unsigned long long)8.64e11;
	return sign;
}

/*
 *  Epoch = Ltime.Epoch(parameter)
 */
int epoch_new(lua_State *L) {
	int top = lua_gettop(L);
	t_epoch *self = newEpoch(L);
	self->t = parameterToTicks(L, top>0 ? 1 : 0 );
	return 1;
}

/*
 *  Epoch2 = Epoch:clone()
 */
static int epoch_clone(lua_State *L) {

	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	t_epoch *clone = newEpoch(L);
	clone->t = self->t;
	return 1;
}

/*
 * 	Get Epoch value as microseconds
 *  useconds = Epoch:useconds()
 */
static int epoch_useconds(lua_State *L) {

	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	lua_pushnumber(L, (double)self->t / 10);
	return 1;
}

/*
 * 	Get Epoch value as milliseconds
 *  mseconds = Epoch:mseconds()
 */
static int epoch_mseconds(lua_State *L) {

	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	lua_pushnumber(L, (double)self->t / 10000);
	return 1;
}

/*
 * 	Get Epoch value as seconds
 *  seconds = Epoch:seconds()
 */
static int epoch_seconds(lua_State *L) {

	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	lua_pushnumber(L, (double)self->t / 10000000);
	return 1;
}

/*
 * 	Get Epoch value as minutes
 *  minutes = Epoch:minutes()
 */
static int epoch_minutes(lua_State *L) {

	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	lua_pushnumber(L, (double)self->t / 600000000);
	return 1;
}

/*
 * 	Get Epoch value as hours
 *  hours = Epoch:hours()
 */
static int epoch_hours(lua_State *L) {

	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	lua_pushnumber(L, (double)self->t / 36000000000);
	return 1;
}

/*
 * 	Get Epoch value as days
 *  days = Epoch:days()
 */
static int epoch_days(lua_State *L) {

	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	lua_pushnumber(L, (double)self->t / 864000000000);
	return 1;
}

/*
 *  Epoch:__unm()
 */
static int epoch_unm(lua_State *L) {
	
	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	self->t = - self->t;
	return 1;
}

/*
 *  Epoch.__add(a, b)
 */
static int epoch_add(lua_State *L) {
	long long a, b;
	t_datetime *d;

	a = parameterToTicks(L, 1);
	if (NULL!=(d = luaL_testudata(L, 2, LTIME_MT_DATETIME))) {
		t_datetime *result = newDatetime(L);
		result->t = a + d->t;
	}
	else {
		b = parameterToTicks(L, 2);
		t_epoch *self = newEpoch(L);
		self->t = a + b;
	}
	return 1;
}

/*
 *  Epoch.__sub(a, b)
 */
static int epoch_sub(lua_State *L) {
	
	long long a, b;
	a = parameterToTicks(L, 1);
	b = parameterToTicks(L, 2);
	t_epoch *self = newEpoch(L);
	self->t = a - b;
	return 1;
}

/*
 *  Epoch.__mul(a, b)
 */
static int epoch_mul(lua_State *L) {
	
	long long t;
	double x;
	if (lua_isnumber(L, 1)) {
		x = lua_tonumber(L, 1);
		t = parameterToTicks(L, 2);
		t_epoch *self = newEpoch(L);
		self->t = t * x;
	} else if (lua_isnumber(L, 2)) {
		t = parameterToTicks(L, 1);
		x = lua_tonumber(L, 2);
		lua_pushnumber(L, x*t/1e7);
	} else {
		luaL_error(L, LTIME_ERR_EPOCH_MUL_NO_NUMBER);
		return 0;
	}
	return 1;
}

/*
 *  Epoch.__div(a, b)
 */
static int epoch_div(lua_State *L) {

	if (lua_isnumber(L, 1)) {
		luaL_error(L, LTIME_ERR_EPOCH_DIV_ARGERR);
	} else if (lua_isnumber(L, 2)) {
		long long t = parameterToTicks(L, 1);
		double x = lua_tonumber(L, 2);
		if (x == 0)
			luaL_error(L, LTIME_ERR_DIV_ZERO_UNDEFINED);
		t_epoch *self = newEpoch(L);
		self->t = t / x;
		return 1;
	} else {
		luaL_error(L, LTIME_ERR_EPOCH_DIV_NO_NUMBER);
	}
	return 0;
}

/*
 *  Epoch.__mod(a, b)
 */
static int epoch_mod(lua_State *L) {
/*
	long long a, b;
	a = parameterToTicks(L, 1);
	b = parameterToTicks(L, 2);
	t_epoch *self = newEpoch(L);
	self->t = a % b;
	return 1;
*/
	long long a, b;
	a = parameterToTicks(L, 1);
	b = parameterToTicks(L, 2);
	if (b == 0)	luaL_error(L, LTIME_ERR_MOD_ZERO_UNDEFINED);
	if (lua_isnumber(L, 2)) {
		lua_pushnumber(L, (a%b)/1e7);
	}
	else {
		t_epoch *self = newEpoch(L);
		self->t = a % b;
	}
	return 1;
}


/*
 *  Epoch.__concat(a, b)
 */
static int epoch_concat(lua_State *L) {
	luaL_tolstring(L,1,NULL);
	luaL_tolstring(L,2,NULL);
	lua_concat(L, 2);
	return 1;
}


/*
 *  Epoch.__eq(a, b)
 */
static int epoch_eq(lua_State *L) {
	lua_pushboolean(L, parameterToTicks(L, 1) == parameterToTicks(L, 2));
	return 1;
}

/*
 *  Epoch.__lt(a, b)
 */
static int epoch_lt(lua_State *L) {
	lua_pushboolean(L, parameterToTicks(L, 1) < parameterToTicks(L, 2));
	return 1;
}

/*
 *  Epoch.__le(a, b)
 */
static int epoch_le(lua_State *L) {
	lua_pushboolean(L, parameterToTicks(L, 1) <= parameterToTicks(L, 2));
	return 1;
}

/*
 *  Epoch:__tostring() 
 */
static int epoch_tostring(lua_State *L) {
	
	t_epoch *self = (t_epoch *)luaL_checkudata(L, 1, LTIME_MT_EPOCH);
	unsigned D, h, m, s, us;
	char buffer[50] = "";
	
	char *sign = fromTicks(self->t, &D, &h, &m, &s, &us) < 0 ? "-" : "";
	
	if (D && us)
		sprintf(buffer, "%s%u %02u:%02u:%02u.%06u", sign, D, h,m,s, us);
	else if (us)
		sprintf(buffer, "%s%02u:%02u:%02u.%06u", sign, h,m,s, us);
	else if (D)
		sprintf(buffer, "%s%u %02u:%02u:%02u", sign, D, h,m,s);
	else
		sprintf(buffer, "%s%02u:%02u:%02u", sign, h,m,s);

/*
	if (fromTicks(self->t, &D, &h, &m, &s, &us) == -1)
		strcat(buffer, "-");
	if (D)
		sprintf(&buffer[strlen(buffer)], "%u ", D);
	sprintf(&buffer[strlen(buffer)], "%02u:%02u:%02u", h, m, s);
	if (us && us % 1000)
		sprintf(&buffer[strlen(buffer)], ".%06u", us);
	else if (us)
		sprintf(&buffer[strlen(buffer)], ".%03u", us / 1000);
*/
	
	lua_pushstring(L, buffer);
	return 1;
}

/*
 *  Epoch:__gc()
 * /
static int epoch_gc(lua_State *L) {
	
	lua_pushboolean(L, 1);
	return 1;
}*/

int open_epoch(lua_State *L) {
	
    static const luaL_Reg epoch_methods[] = {
		{"clone", epoch_clone},
		{"useconds", epoch_useconds},
		{"mseconds", epoch_mseconds},
		{"seconds", epoch_seconds},
		{"tonumber", epoch_seconds},
		{"minutes", epoch_minutes},
		{"hours", epoch_hours},
		{"days", epoch_days},
		{NULL, NULL}
    };

    static const luaL_Reg epoch_meta_methods[] = {
		{"__unm", epoch_unm},
		{"__add", epoch_add},
		{"__sub", epoch_sub},
		{"__mul", epoch_mul},
		{"__div", epoch_div},
		{"__mod", epoch_mod},
		{"__concat", epoch_concat},
		{"__eq",  epoch_eq},
		{"__lt",  epoch_lt},
		{"__le",  epoch_le},
		{"__tostring", epoch_tostring},
//		{"__gc", epoch_gc},
		{NULL, NULL}
    };

	// create the metatable first
	luaL_newmetatable(L, LTIME_MT_EPOCH);
	// and set all metamethods except __index
	luaL_setfuncs(L, epoch_meta_methods, 0);

	// create the library table
	luaL_newlib(L, epoch_methods);
	// and set the __index metamethod
	lua_setfield(L, -2, "__index");

    return 1;    
}
