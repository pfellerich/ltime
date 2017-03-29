LTime
=====

Easy hi-res UTC time and epoch handling for Lua, Version 0.9.1
By Pascal Fellerich (pascal.fellerich@ses.com) and Nicolas Griselle (nicolas.griselle@gmail.com)


## LTime module

The Ltime module can handle UTC timestamps ranging from 1858-11-17 to 9999-12-31.
Internally, it uses the VMS (or Smithsonian) representation, which is the number
of 100ns ticks since 1858-11-17, expressed in a 64 bit signed integer. This gives
a range up to the year 31000 without precision loss.

However, behavior of the module with out of range dates is undefined.

It only intended for fast and precise UTC time handling, it does not have timezone
support, and it does not have any fancy human readable date parsing routines.

Instead, it focuses on seamless integration of timestamps into "standard" arithmetics --
all operators are overloaded, whereever it makes sense.

The module exports the following items:
 * `.Time` - the Time (timestamp) constructor
 * `.Epoch` - the Epoch (duration, timespan) constructor
 * `.mktime` - a secondary Time constructor taking different parameters
 *  `.VERSION` - the LTime version string

## Creating Time and Epoch objects
```
tstamp = Ltime.Time(parameter)
```
Parameter can be:
 * Nil or absent, considered current time in microsecond precision
 * A number, considered number of seconds since 1970-01-01 00:00:00, for unix timestamp compatibility
 * A strict ISO 8601 string, YYYY-MM-DD[[T ]hh:mm:ss[.uuuuuu]]
 * A table with keys "year", "month", "day", "hour", "min", "sec", "usec"
 * Another Time object, in which case a cloned Time object is returned

```
tstamp = Ltime.mktime(year[, month[, day[, hours[, minutes[, seconds[, useconds]]]]]])
```
The parameters are in "big endian" order and represent their natural values; e.g. 
the month ranges from 1 to 12 here. This is essentially a shortcut to `.Time{year=...}`, 
provided for convenience

```
epoch = Ltime.Epoch(parameter)
```
Parameter can be:
 * Nil or absent, considered 0 seconds
 * A number, considered number of seconds
 * A deltatime string in the form '[+/-][DDDD ]hh:mm:ss[.uuuuuu]'
 * An other Epoch object in which case a cloned Epoch object is returned

```
version = Ltime.VERSION
```
Returns the current version of the library


## The Time Object

### Time:format
Return a formatted representation of the Time object
```
string = Time:format(format_string)
```
format_string can include the following conversion specifier characters :
 * %a -> The abreviated weekday in english
 * %A -> The full weekday in english
 * %b -> The abreviated month name in english
 * %B -> The full month name in english
 * %C -> The century number (year/100) as a 2-digit integer
 * %d -> The day of the month as a decimal number (range 01 to 31)
 * %D -> Equivalent to %m/%d/%y
 * %e -> Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space
 * %F -> Equivalent to %Y-%m-%d (the ISO 8601 date format)
 * %h -> Equivalent to %b
 * %H -> The hour as a decimal number using a 24-hour clock (range 00 to 23)
 * %I -> The hour as a decimal number using a 12-hour clock (range 01 to 12)
 * %j -> The day of the year as a decimal number (range 001 to 366)
 * %k -> The hour (24-hour  clock) as a decimal number (range 0 to 23), single digits are preceded by a blank
 * %l -> The hour (12-hour clock) as a decimal number (range  1  to  12), single digits are preceded by a blank
 * %m -> The month as a decimal number (range 01 to 12)
 * %M -> The minute as a decimal number (range 00 to 59)
 * %n -> A newline character
 * %p -> Either "AM" or "PM", noon is treated as "PM" and midnight as "AM"
 * %P -> Like %p but in lowercase: "am" or "pm"
 * %q -> The milliseconds as a decimal number (range 000 to 999)
 * %Q -> The microseconds as a decimal number (range 000 to 999)
 * %r -> The time in a.m. or p.m. notation, this is equivalent to %I:%M:%S %p
 * %R -> The time in 24-hour notation (%H:%M)
 * %s -> The number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC)
 * %S -> The second as a decimal number (range 00 to 59)
 * %t -> A tab character
 * %T -> The time in 24-hour notation (%H:%M:%S)
 * %u -> The day of the week as a decimal, range 1 to 7, Monday being 1
 * %x -> The preferred date representation without the time
 * %X -> The preferred time representation without the date
 * %y -> The year as a decimal number without a century (range 00 to 99)
 * %Y -> The year as a decimal number including the century
 * %. -> The milliseconds and microseconds as a decimal number (%q%Q)
 * %% -> A literal '%' character

### Time:clone
Clone Time object
```
tstamp2 = tstamp:clone()
```
The same effect can be achieved by writing
```
tstamp2 = Ltime.Time(tstamp)
```


### Time:date
Set/get date without affecting time of day
 * Setter: Time = Time:date(parameter)
 * Getter: Time2 = Time:date()
Parameter can be :
 * Nil, considered 1970-01-01 00:00:00
 * A number, considered number fo seconds since 1970-01-01 00:00:00
 * A strict ISO 8601 string, YYYY-MM-DD[Thh:mm:ss[.uuuuuu]]
 * A table with keys "year", "month", "day", "hour", "min", sec", "usec"
 * An other Time object
If parameter includes a time of day, it will be silently ignored


### Time:time
Set/get time of day without affecting date
 * Setter: Time = Time:time(parameter)
 * Getter: Epoch = Time:time()
Parameter can be :
 * Nil, considered 00:00:00
 * A number, considered number of seconds from 00:00:00
 * A deltatime string, [+]hh:mm:ss[.uuuuuu]
 * An Epoch object with value between 00:00:00.000000 and 23:59:59.999999

### Time:add
Add onto the same object.
```
Time = Time:add(parameter)
```
Parameter can be :
 * A number, considered number of seconds
 * A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
 * An Epoch object

### Time:sub
Substract from the same object
```
Time = Time:sub(parameter)
```
Parameter can be :
 * A number, considered number of seconds
 * A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
 * An Epoch object

### Time:floor
Round down the same object to the next "stop"
```
Time = Time:floor(parameter)
```
Parameter can be :
 * A number, considered number of seconds
 * A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
 * An Epoch object

### Time:ceil
Round up the same object to the next "stop"
```
Time = Time:ceil(parameter)
```
Parameter can be :
 * A number, considered number of seconds
 * A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
 * An Epoch object

### Time:leap
Return true if leap year, false otherwise
```
boolean = Time:leap()
```

### Time:doy
Set/get yearday without affecting time of day (1 = January 1st, ...)
 * Setter: Time = Time:doy(yearday)
 * Getter: yearday = Time:doy()

### Time:weekday
Get weekday (1 = Monday, ..., 7 = Sunday)
```
weekday = Time:weekday()
```

### Time:mjd
Set/get date via MJD (modified julian day) without affecting time of day
 * Setter: Time = Time:mjd(mjd)
 * Getter: mjd = Time:mjd()

### Time:vms
Set/get VMS timestamp (number of 100 nanoseconds ticks since 1858-11-17 00:00:00)
* Setter: Time = Time:vms(unixTimestamp)
* Getters: 
```
vmsTimestamp = Time:vms()
prefixedHexTimestamp = Time:vms("*h")     ('0x'..16 hex nibbles)
hexTimestamp = Time:vms("*x")     (16 hex nibbles)
binaryTimestamp = Time:vms("*b")  (8 bytes in network order)
```

Note: in Lua 5.3, this functions returns 64 bit integers, i.e. no
precision loss occurs.



## Epoch object

### Epoch:clone
Clone Epoch object
```
Epoch2 = Epoch:clone()
```

### Epoch:useconds
Get Epoch value as microseconds
```
useconds = Epoch:useconds()
```

### Epoch:mseconds
Get Epoch value as milliseconds
```
mseconds = Epoch:mseconds()
```

### Epoch:seconds, :tonumber
Get Epoch value as seconds
```
seconds = Epoch:seconds()
seconds = Epoch:tonumber()
```

### Epoch:minutes
Get Epoch value as minutes
```
minutes = Epoch:minutes()
```

### Epoch:hours
Get Epoch value as hours
```
hours = Epoch:hours()
```

### Epoch:days
Get Epoch value as days
```
days = Epoch:days()
```


## Arithmetic operations

The following operations are defined:


### Time Addition

Time + value => Time

value + Time => Time

`value` can be:
		* A number, considered number of seconds
		* A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
		* An Epoch object


### Time Difference

Time - Time => Epoch

Time - value => Time

`value` can be:
		* A number, considered number of seconds
		* A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
		* An Epoch object


### Time Modulo

Time % Epoch => Epoch

Time % string => Epoch (when string is [+/-][DDDD ]hh:mm:ss[.uuuuuu])

Time % value => number (value in seconds)


### Epoch Addition

Epoch + value => Epoch

value + Epoch => Epoch

`value` can be :
		* A number, considered number of seconds
		* A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
		* An Epoch object


### Epoch Difference

Epoch - Epoch => Epoch

Epoch - value => Epoch

value - Epoch => Epoch

`value` can be :
		* A number, considered number of seconds
		* A deltatime string, [+/-][DDDD ]hh:mm:ss[.uuuuuu]
		* An Epoch object


### Epoch Multiplication

Epoch * number => number

number * Epoch => Epoch

Note that this is not commutative in that the resulting type is different.
Also note that you cannot multiply two epoch objects (one operand must be a scalar)




