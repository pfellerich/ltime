#include "ltime.h"

typedef struct s_spec {
	char	c;
	int		max_length;
	int		(*func)(char *, long long, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned);
} t_spec;

static char *abreviated_weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static char *weekdays[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
static char *abreviated_months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

/*
 *  %a -> The abreviated weekday in english
 */
static int format_a(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {

	int weekday = (t / 864000000000 + 2) % 7;
	int length = strlen(abreviated_weekdays[weekday]);
	strncpy(buffer, abreviated_weekdays[weekday], length);
	return length;
}

/*
 *  %A -> The full weekday in english
 */
static int format_A(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {

	int weekday = (t / 864000000000 + 2) % 7;
	int length = strlen(weekdays[weekday]);
	strncpy(buffer, weekdays[weekday], length);
	return length;
}

/*
 *  %b -> The abreviated month name in english
 */
static int format_b(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	int length = strlen(abreviated_months[M - 1]);
	strncpy(buffer, abreviated_months[M - 1], length);
	return length;
}

/*
 *  %B -> The full month name in english
 */
static int format_B(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	int length = strlen(months[M - 1]);
	strncpy(buffer, months[M - 1], length);
	return length;
}

/*
 *  %C -> The century number (year/100) as a 2-digit integer
 */
static int format_C(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%02u", Y / 100);
	return 2;
}

/*
 *  %d -> The day of the month as a decimal number (range 01 to 31)
 */
static int format_d(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%02u", D);
	return 2;
}

/*
 *  %D -> Equivalent to %m/%d/%y
 */
static int format_D(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 9, "%02u/%02u/%02u", D, M, Y % 100);
	return 8;
}

/*
 *  %e -> Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space
 */
static int format_e(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {

	snprintf(buffer, 3, "%2u", D);
	return 2;
}

/*
 *  %F -> Equivalent to %Y-%m-%d (the ISO 8601 date format)
 */
static int format_F(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {

	snprintf(buffer, 11, "%04u-%02u-%02u", Y, M, D);
	return 10;
}

/*
 *  %h -> Equivalent to %b
 */
static int format_h(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	int length = strlen(abreviated_months[M - 1]);
	strncpy(buffer, abreviated_months[M - 1], length);
	return length;
}

/*
 *  %H -> The hour as a decimal number using a 24-hour clock (range 00 to 23)
 */
static int format_H(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%02u", h);
	return 2;
}

/*
 *  %I -> The hour as a decimal number using a 12-hour clock (range 01 to 12)
 */
static int format_I(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	if (h > 12)
		h = h - 12;
	if (h == 0)
		h = 12;
	snprintf(buffer, 3, "%02u", h);
	return 2;
}

/*
 *  %j -> The day of the year as a decimal number (range 001 to 366)
 */
static int format_j(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 4, "%03d", toMJD(Y, M, D) - toMJD(Y, 1, 1) + 1);
	return 3;
}

/*
 *  %k -> The hour (24-hour  clock) as a decimal number (range 0 to 23), single digits are preceded by a blank
 */
static int format_k(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%2u", h);
	return 2;
}

/*
 *  %l -> The hour (12-hour clock) as a decimal number (range  1  to  12), single digits are preceded by a blank
 */
static int format_l(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	if (h > 12)
		h = h - 12;
	if (h == 0)
		h = 12;
	snprintf(buffer, 3, "%2u", h);
	return 2;
}

/*
 *  %m -> The month as a decimal number (range 01 to 12)
 */
static int format_m(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%02u", M);
	return 2;
}

/*
 *  %M -> The minute as a decimal number (range 00 to 59)
 */
static int format_M(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%02u", m);
	return 2;
}

/*
 *  %n -> A newline character
 */
static int format_n(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	buffer[0] = '\n';
	return 1;
}

/*
 *  %p -> Either "AM" or "PM", noon is treated as "PM" and midnight as "AM"
 */
static int format_p(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	if (h < 12) {
		buffer[0] = 'A';
		buffer[1] = 'M';
	} else {
		buffer[0] = 'P';
		buffer[1] = 'M';
	}
	return 2;
}

/*
 *  %P -> Like %p but in lowercase: "am" or "pm"
 */
static int format_P(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	if (h < 12) {
		buffer[0] = 'a';
		buffer[1] = 'm';
	} else {
		buffer[0] = 'p';
		buffer[1] = 'm';
	}
	return 2;
}

/*
 *  %q -> The milliseconds as a decimal number (range 000 to 999)
 */
static int format_q(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 4, "%03u", us / 1000);
	return 3;
}

/*
 *  %Q -> The microseconds as a decimal number (range 000 to 999)
 */
static int format_Q(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 4, "%03u", us % 1000);
	return 3;
}

/*
 *  %r -> The time in a.m. or p.m. notation, this is equivalent to %I:%M:%S %p
 */
static int format_r(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	char *p = h < 12 ? "AM" : "PM";
	if (h > 12)
		h = h - 12;
	if (h == 0)
		h = 12;
	snprintf(buffer, 12, "%02u:%02u:%02u %s", h, m, s, p);
	return 11;
}

/*
 *  %R -> The time in 24-hour notation (%H:%M)
 */
static int format_R(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 6, "%02u:%02u", h, m);
	return 5;
}

/*
 *  %s -> The number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC)
 */
static int format_s(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	return snprintf(buffer, 21, "%lld", t - VMS_1970);
}

/*
 *  %S -> The second as a decimal number (range 00 to 59)
 */
static int format_S(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%02u", s);
	return 2;
}

/*
 *  %t -> A tab character
 */
static int format_t(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	buffer[0] = '\t';
	return 1;
}

/*
 *  %T -> The time in 24-hour notation (%H:%M:%S)
 */
static int format_T(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 9, "%02u:%02u:%02u", h, m, s);
	return 8;
}

/*
 *  %u -> The day of the week as a decimal, range 1 to 7, Monday being 1
 */
static int format_u(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	buffer[0] = '0' + (char)((t / 864000000000 + 2) % 7 + 1);
	return 1;
}

/*
 *  %v -> VMS timestamp, hexadecimal
 */
static int format_v(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 19, "0x%llX", t);
	return 18;
}

/*
 *  %x -> The preferred date representation without the time
 */
static int format_x(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 11, "%04u-%02u-%02u", Y, M, D);
	return 10;
}

/*
 *  %X -> The preferred time representation without the date
 */
static int format_X(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 9, "%02u:%02u:%02u", h, m, s);
	return 8;
}

/*
 *  %y -> The year as a decimal number without a century (range 00 to 99)
 */
static int format_y(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 3, "%02u", Y % 100);
	return 2;
}

/*
 *  %Y -> The year as a decimal number including the century
 */
static int format_Y(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 5, "%04u", Y);
	return 4;
}

/*
 *  %. -> The milliseconds and microseconds as a decimal number (%q%Q)
 */
static int format_dot(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {
	
	snprintf(buffer, 7, "%06u", us);
	return 6;
}

/*
 *  %% -> A literal '%' character
 */
static int format_percent(char *buffer, long long t, unsigned Y, unsigned M, unsigned D, unsigned h, unsigned m, unsigned s, unsigned us) {

	buffer[0] = '%';
	return 1;
}

/*
 *  List of all the conversion specifier characters, maximum post conversion length and conversionn function
 */
static t_spec specs[] = {
	{'a', 3, format_a},
	{'A', 9, format_A},
	{'b', 3, format_b},
	{'B', 9, format_B},
	{'C', 2, format_C},
	{'d', 2, format_d},
	{'D', 8, format_D},
	{'e', 2, format_e},
	{'F', 10, format_F},
	{'h', 3, format_h},
	{'H', 2, format_H},
	{'I', 2, format_I},
	{'j', 3, format_j},
	{'k', 2, format_k},
	{'l', 2, format_l},
	{'m', 2, format_m},
	{'M', 2, format_M},
	{'n', 1, format_n},
	{'p', 2, format_p},
	{'P', 2, format_P},
	{'q', 3, format_q},
	{'Q', 3, format_Q},
	{'r', 11, format_r},
	{'R', 5, format_R},
	{'s', 20, format_s},
	{'S', 2, format_S},
	{'t', 1, format_t},
	{'T', 8, format_T},
	{'u', 1, format_u},
	{'v', 18, format_v},
	{'x', 10, format_x},
	{'X', 8, format_X},
	{'y', 2, format_y},
	{'Y', 4, format_Y},
	{'.', 6, format_dot},
	{'%', 1, format_percent},
	{0, 0, 0}
};

/*
 *  Compute the max possible length of the formated string
 */
static int format_max_length(const char *format) {
	
	int	length = 0;
	for (int i = 0; format[i]; i++) {
		if (format[i] == '%' && format[i + 1] != '\0') {
			int j;
			for (j = 0; specs[j].c; j++) {
				if (format[i + 1] == specs[j].c) {
					length += specs[j].max_length;
					break;
				}
			}
			if (specs[j].c == 0) {
				length += 2;
			}
			i++;
		} else {
			length++;
		}
	}
	return length;
}

/*
 *  string = Time:format(format_string)
 */
int datetime_format(lua_State *L) {

	t_datetime *self = (t_datetime *)luaL_checkudata(L, 1, LTIME_MT_DATETIME);
	if (lua_gettop(L) == 1 || lua_type(L, 2) != LUA_TSTRING)
		luaL_error(L, LTIME_ERR_DATETIME_MISSING_FORMAT);
	const char *format = lua_tostring(L, 2);
	//printf("Format max length: %d\n", format_max_length(format));
	char buffer[format_max_length(format) + 1];
	int cursor = 0;
	unsigned Y, M, D, h, m, s, us;
	fromVMS(self->t, &Y, &M, &D, &h, &m, &s, &us);
	for (int i = 0; format[i]; i++) {
		if (format[i] == '%' && format[i + 1] != '\0') {
			int j;
			for (j = 0; specs[j].c; j++) {
				if (format[i + 1] == specs[j].c) {
					cursor += specs[j].func(&buffer[cursor], self->t, Y, M, D, h, m, s, us);
					break;
				}
			}
			if (specs[j].c == 0) {
				strncpy(&buffer[cursor], &format[i], 2);
				cursor += 2;
			}
			i++;
		} else {
			buffer[cursor++] = format[i];
		}
	}
	buffer[cursor] = '\0';
	lua_pushstring(L, buffer);
	return 1;
}
