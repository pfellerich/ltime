#include "ltime.h"

int open_datetime(lua_State *L);
int datetime_new(lua_State *L);
int datetime_mktime(lua_State *L);
int open_epoch(lua_State *L);
int epoch_new(lua_State *L);

/*
 *  version = Ltime.VERSION()
 * /
static int ltime_version(lua_State *L) {
	
	lua_pushstring(L, LTIME_VERSION);
	return 1;
}
*/
int luaopen_ltime(lua_State *L) {
	
    static const luaL_Reg ltime_functions[] = {
		{"Time", datetime_new},
		{"mktime", datetime_mktime},
		{"Epoch", epoch_new},
	//	{"VERSION", ltime_version},
		{NULL, NULL}
	};

	open_datetime(L);
	open_epoch(L);
    luaL_newlib(L, ltime_functions);

	lua_pushstring(L, "VERSION");
	lua_pushstring(L, LTIME_VERSION);
	lua_rawset(L, -3);

    return 1;
}
