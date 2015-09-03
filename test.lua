-- Some tests
print "LTime module test for Lua 5.3"

package.cpath = "./?.so;" .. package.cpath
local ltime = require"ltime"

-- Shortcuts
local T, E = ltime.Time, ltime.Epoch




-- Version
print( "Time module ", ltime.VERSION)

-- Create a time
local now = T()
-- convert to string
local strtime = tostring(now)

print("Now", now)

-- test __eq:
assert(now == T(strtime))
assert(tostring(now) == strtime)

-- Arithmetics


print "\nAdding to Time"

print( "Time + Number = Time", now + 3600 )
print( "Time + Epoch = Time ", now + E"02:00:00" )
print( "Time + String = Time", now + "03:00:00" )
assert(now+3600 == now + E"01:00:00")
assert(now+E"01:00:00" == now + "01:00:00")

print "\nSubstracting from Time"

print( "Time - Number = Time", now - 3600 )
print( "Time - Epoch = Time ", now - E"02:00:00" )
print( "Time - String = Time", now - "03:00:00" )
print( "Time - Time = Epoch",  now - T"2013-09-01" )
assert(now-3600 == now - E"01:00:00")
assert(now-E"01:00:00" == now - "01:00:00")


-- This will bomb out as "not defined"
--print( "-Time ", -now)
--print( "Time * Number ", now * 5)
--print( "Time / Number ", now / 5)

print "\nTime modulo operator"

print( "Time % Number = Number", now % 3600 )
print( "Time % Epoch = Epoch  ", now % E"01:00:00" )
print( "Time % String = Epoch ", now % "01:00:00" )


print "\nTime methods:"

print( "Time:floor(Number)    ", now:clone():floor(1800) )
print( "Time:floor(Epoch)     ", now:clone():floor(E"00:30:00") )
print( "Time:floor(String)    ", now:clone():floor("00:30:00") )

print( "Time:ceil(Number)     ", now:clone():ceil(1800) )
print( "Time:ceil(Epoch)      ", now:clone():ceil(E"00:30:00") )
print( "Time:ceil(String)     ", now:clone():ceil("00:30:00") )


print "\nOther time methods"

print( "Time:time()            ", now:time() )
print( 'Time:time("12:00:00")  ', now:time("12:00:00") )

print( "Time:date()            ", now:date() )
print( 'Time:date("2013-08-01")', now:date("2013-08-01"))

print( 'Time:doy()             ', now:doy() )

print( 'Time:weekday()         ', now:weekday() )

print( 'Time:leap()            ', now:leap() )

print( 'Time:mjd()             ', now:mjd() )
print( 'Time:mjd(56546.5)      ', now:mjd(56546.5) )

print( 'Time:vms()             ', now:vms() )
print( 'Time:vms(4.8856176e+16)', now:vms(4.8856176e+16) )
print( 'Time:vms("0xAE4C7CE0776CD9")', now:vms("0xAE4C7CE0776CD9") )

assert(now:vms"*x" == "00AE4C7CE0776CD9")
assert((now+1e-7):vms"*x" == "00AE4C7CE0776CDA")



local d = E"01:00:00"
print("Epoch               ", d) 

print "Unary minus"
print( "-Epoch = Epoch        ", -d )


print "\nAdding to Epoch"

print( "Epoch + Number = Epoch", d + 3600 )
print( "Epoch + Epoch = Epoch ", d + E"02:00:00" )
print( "Epoch + String = Epoch", d + "03:00:00" )
print( "Epoch + Time = Time",    d + T(strtime) )


print "\nSubstracting from Epoch" 

print( "Epoch - Number = Epoch", d - 3600 )
print( "Epoch - Epoch = Epoch ", d - E"02:00:00" )
print( "Epoch - String = Epoch", d - "03:00:00" )


print "\nMultiply/Divide Epoch by scalar"

print( "Epoch * Number = Number", d * -1.5 )
print( "Number * Epoch = Epoch", -5 * d )
print( "Epoch / Number = Epoch", d / 5 )
-- print( "Number / Epoch = Epoch", 5 / d ) -- error


print "\nModulo operator"

local d = E"12:34:56.789"

print( "Epoch % Number = Number", d % (12*60+34) )
assert(d % (12*60+34) == 56.789)

print( "Epoch % Epoch = Epoch ", d % E"00:12:34" )
assert(d % E"00:12:34" == E"00:00:56.789000")

print( "Epoch % String = Epoch", d % "00:12:34" )
assert(d % "00:12:34" == E"00:00:56.789000")



print "\nOther epoch methods"

print( 'Epoch:seconds()       ', d:seconds() )
print( 'Epoch:days()          ', d:days() )


assert( (E(123) > 120) == true )
assert( (E(120) > 123) == false )
assert( (E(999) >= 999) == true )
assert( (E(999) <= 999) == true )
assert( (E(999) == 999) == false )	-- not of the same type!



-- in Lua 5.3, we have integers that can hold the full timestamp.
if _VERSION=="Lua 5.3" then
	assert(T"1969-06-11 16:12:43.000001":vms()==0x7BF5A162B54F8A, "Precision loss due to non 64 bit integers")
	assert(tostring(T():vms(0x7BF5A162B54F8A))=="1969-06-11 16:12:43.000001", "Precision loss due to non 64 bit integers")

end

-- Segfault on 0.7 when overflowing
for i=16,50 do
	local x = T(2^i)
	print(x:vms("*x"), x:format("%Y-%m-%d %x %X"), x, x:format"/%s/" )
end
-- print(T(49420232625460839))

for i=16,50 do
	local x = E(2^i)
	print(x)
end

-- Concat operation
print("Current UTC time is " .. T())
print(T() .. " is the current UTC time is")
print( T() .. T() )

print("1234 seconds is " .. E(1234))
print(E(1234) .. " is 1234 seconds")
print(E(1234) .. E(-1234))

print("\n"..ltime.VERSION.." OK")



