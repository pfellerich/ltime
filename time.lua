--[[

	Yet another Date/Time library for Lua 5.2

	Author: P. Fellerich

	====================================================================


]]


-- Absolute time object
Datetime = { d = 0 }
Datetime.__index = Datetime


-- Epoch object - a delta time
Epoch = { d = 0 }
Epoch.__index = Epoch


--
-- local (private) functions: 
-- Create a new instance and set the "d" field
--
local function NewDatetime(d)
	return setmetatable({d=d}, Datetime)
end


local function NewEpoch(d)
	return setmetatable({d=d}, Epoch)
end



-- 
-- Compute timestamp from individual fields, UTC version
--
-- This is needed because the C library function depends on the
-- system TZ setting.
--
-- from the single unix specification:
-- tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
-- (tm_year-70)*31536000 + ((tm_year-69)/4)*86400 -
-- ((tm_year-1)/100)*86400 + ((tm_year+299)/400)*86400
local function gmmktime(Y,M,D,h,m,s)
	local int = math.floor
	Y, M, D = int(Y), int(M), int(D)
	local offset = {0,31,59,90,120,151,181,212,243,273,304,334}
	local d = offset[M] + D-1
	if M>2 and Y%4==0 and (Y%100~=0 or Y%400==0) then d = d+1 end
	local y=Y-1900
	return s + m*60 + h*3600 + int(d)*86400 +
		(y-70)*31536000 + int((y-69)/4)*86400 -
		int((y-1)/100)*86400 + int((y+299)/400)*86400
end


-- Constants
-- MJD of 1-JAN-2970 is 40587
local MJD_1970 		= 40587
local VMS_1970		= 40587*86400*1e7


--
-- Create a new datetime instance
-- (exported constructor)
--
function Datetime.new(d)
	local self
	
	if d==nil then
		return NewDatetime(os.time())
	end

	local t = type(d)

	-- create time stamp from table fields
	if t == 'table' then
		if getmetatable(d)==Datetime then
			self = d:clone()
		else
			self = NewDatetime(gmmktime(
				d.year or 1970, d.month or 1, d.day or 1,
				d.hour or 0, d.min or 0, d.sec or 0))
		end
	
	-- from (strict) ISO 8601 string
	elseif t == 'string' then
		local Y,M,D,h,m,s = d:match"^(%d+)%D(%d+)%D(%d+)[%sT](%d+):(%d+):([%d.]+)"
		if Y==nil then
			Y,M,D = d:match"^(%d+)%D(%d+)%D(%d+)"
			h,m,s = 0,0,0
		end
		if Y==nil then
			error("Illegal time specification", 2)
		end
		self = NewDatetime(gmmktime(Y,M,D,h,m,s))

	-- from number, which is considered seconds since 1970
	elseif t == 'number' then
		self = NewDatetime(d)
	
	-- unknown type
	else
		error("Datetime: illegal parameter to new() constructor", 2)
	end

	return self
end


-- special setter
function Datetime.mktime(...)
	return NewDatetime(gmmktime(...))
end


-- Clone
function Datetime:clone()
	return NewDatetime(self.d)
end


-- Format the value for use in MySQL
function Datetime:__tostring()
	return os.date("!%Y-%m-%d %H:%M:%S", self.d) or "????-??-??"
end


-- User formatting....
function Datetime:format(fmt)
	return os.date(fmt, self.d)
end


-- Unary minus
function Datetime:__unm()
	error("Unary minus not defined for Datetime",2)
end


-- Add an epoch and return a Datetime
function Datetime:__add(x)
	local dt
	if type(x)=='number' then
		dt = x
	elseif type(x)=='table' and getmetatable(x)==Epoch then
		dt = x.d
	elseif type(x)=='string' then -- try an Epoch constructor here
		dt = Epoch.new(x).d
	else
		error("Attempt to add illegal type to Datetime",2)
	end
	return NewDatetime(self.d + dt)
end


-- Subtract an Epoch and return Datetime
-- Subtract a Datetime and return Epoch
function Datetime:__sub(x)
	local tx = type(x)

	-- numbers are always considered seconds
	if tx=='number' then
		return NewDatetime(self.d - x)

	-- Epoch or Datetime objects
	elseif tx=='table' then
		local mt = getmetatable(x)
		if mt==Epoch then				-- D-E = D
			return NewDatetime(self.d - x.d)
		elseif mt==Datetime then		-- D-D = E
			return NewEpoch(self.d - x.d)
		end

	-- Strings shall represent epochs only
	elseif tx=='string' then
		local dt = Epoch.new(x)
		return NewDatetime(self.d - Epoch.new(x).d)
	end
	error("Attempt to subtract illegal type from Datetime",2)
end


function Datetime:__mul()
	error("Multiplication not defined for Datetime",2)
end


function Datetime:__div()
	error("Division not defined for Datetime",2)
end


-- The Modulo operator
function Datetime:__mod(x)
	local dt
	-- number type will return number!
	if type(x)=='number' then
		return self.d % x
	elseif type(x)=='table' and getmetatable(x)==Epoch then
		dt = x.d
	elseif type(x)=='string' then -- try an Epoch constructor here
		dt = Epoch.new(x).d
	else
		error("Attempt to perform modulo op with illegal type on Datetime",2)
	end
	return NewEpoch(self.d % dt)
end


--
-- Comparisons
--
function Datetime.__eq(a,b)
	return a.d == b.d
end

function Datetime.__lt(a,b)
	if type(a)=='table' and getmetatable(a)==Datetime then
		a = a.d
	else
		a = (Datetime.new(a)).d
	end

	if type(b)=='table' and getmetatable(b)==Datetime then
		b = b.d
	else
		b = (Datetime.new(b)).d
	end

	return a < b
end

function Datetime.__le(a,b)
	if type(a)=='table' and getmetatable(a)==Datetime then
		a = a.d
	else
		a = (Datetime.new(a)).d
	end

	if type(b)=='table' and getmetatable(b)==Datetime then
		b = b.d
	else
		b = (Datetime.new(b)).d
	end

	return a <= b
end




--[[

	Other methods:
	
	:clone() => return a clone of the datetime
	:floor() => see the above "mod" implementation - but works on the same object
	:ceil() => obvious after floor()
	:time() => set/get time of day w/o affecting the date
	:date() => set/get date part w/o affecting time
	:doy() => set/get day of year
	:mjd() => set/get MJD
	:leap() => get leap year status [bool]
	:add() => add onto the same object as opposed to newobj = obj + epoch
	:sub() => sub from the same object

--]]




-- pure info function
function Datetime:leap()
	local y = os.date("!%Y", self.d)
	return y%4==0 and (y%100~=0 or y%400==0)
end


-- round down to the next "stop"
function Datetime:floor(x)
	local dt
	if type(x)=='number' then
		dt = x
	elseif type(x)=='table' and getmetatable(x)==Epoch then
		dt = x.d
	elseif type(x)=='string' then -- try an Epoch constructor here
		dt = Epoch.new(x).d
	else
		error("Argument must be convertible to an Epoch",2)
	end
	self.d = self.d - self.d % dt
	return self
end

-- round up to the next "stop"
function Datetime:ceil(x)
	local dt
	if type(x)=='number' then
		dt = x
	elseif type(x)=='table' and getmetatable(x)==Epoch then
		dt = x.d
	elseif type(x)=='string' then -- try an Epoch constructor here
		dt = Epoch.new(x).d
	else
		error("Argument must be convertible to an Epoch",2)
	end
	self.d = self.d - self.d % dt + dt
	return self
end


-- TBD: setter to be implemented
function Datetime:doy()
	local doy = os.date("!%j", self.d)
	return doy
end

-- get the weekday (1=Monday, ..., 7=Sunday)
function Datetime:weekday()
	return math.floor(self.d/86400+3) % 7 + 1
end

-- Set or Get MJD
function Datetime:mjd(mjd)
	if type(mjd)=='number' then
		self.d = (mjd-MJD_1970)*86400
		return self
	else
		return self.d/86400 + MJD_1970
	end
end

--
-- Set or Get OpenVMS 'smithsonian' timestamp (100ns ticks MJD)
--
-- Resolution warning: current (2013) number of ticks requires 56 bits
-- for lossless representation. Doubles have only 52 bits available,
-- so the last 4 bits will be lost (16 ticks, i.e. error will be in the
-- range of 2 usec.
-- 
function Datetime:vms(ticks)
	if type(ticks)=='number' then
		self.d = (ticks-VMS_1970)/1e7
		return self
	else
		return self.d*1e7 + VMS_1970
	end
end




--[[
	
	Epoch
	
--]]

-- Epoch constructor
function Epoch.new(d)
	local self
	
	if d==nil then
		return NewEpoch(0)
	end

	local t = type(d)

	-- create time stamp from table fields
	if t == 'table' then
		if getmetatable(d)==Epoch then
			self = d:clone()
		else
			error("Epoch: illegal parameter to new() constructor", 2)
		end

	-- from deltatime string
	elseif t == 'string' then
		local D,h,m,s
		D,h,m,s = d:match"[+-]?(%d+) (%d+):(%d+):([%d.]+)"
		if D==nil then D, h,m,s = 0,d:match"[+-]?(%d+):(%d+):([%d.]+)" end
		if h==nil then error("Unparsable deltatime specification",2) end
		self = NewEpoch(((D*24+h)*60+m)*60+s)

	-- from number, which is considered seconds 
	elseif t == 'number' then
		self = NewEpoch(d)
	
	-- unknown type
	else
		error("Epoch: illegal parameter to new() constructor", 2)
	end

	return self
end


--
-- Clone
--
function Epoch:clone()
	return NewEpoch(self.d)
end


-- in seconds...
function Epoch:seconds()
	return self.d
end

--
-- Format the value for use with MySQL
--
function Epoch:__tostring()
	local t,h,m,s,ms,fmt
	local int = math.floor
	t = math.abs(self.d)
	h,m,s,ms = int(t/3600), int(t/60%60), int(t%60), t*1000%1000
	if self.d<0 then fmt = "-%02d:%02d:%02d"
	else fmt = "%02d:%02d:%02d" end
	if ms>0 then fmt=fmt..".%03d" end
	return fmt:format(h,m,s,ms)
end


-- Unary minus
function Epoch:__unm()
	self.d = -self.d
	return self
end


-- Add an epoch (seconds)
function Epoch.__add(a,b)
	if type(a)=='table' and getmetatable(a)==Epoch then
		a = a.d
	else
		a = tonumber(a)
	end

	if type(b)=='table' and getmetatable(b)==Epoch then
		b = b.d
	else
		b = tonumber(b)
	end
	return NewEpoch(a+b)
end


-- Subtract an epoch (seconds)
function Epoch.__sub(a,b)
	if type(a)=='table' and getmetatable(a)==Epoch then
		a = a.d
	else
		a = tonumber(a)
	end

	if type(b)=='table' and getmetatable(b)==Epoch then
		b = b.d
	else
		b = tonumber(b)
	end
	return NewEpoch(a-b)
end


-- Multiply an epoch by a scalar or vice versa
function Epoch.__mul(a,b)
	local self,x
	if type(b)=='number' then
		self,x = a,b
	elseif type(a)=='number' then 
		self,x = b,a
	else
		error("one operand must be a number", 2)
	end
	return NewEpoch(self.d * x)
end



-- Divide an epoch by a number or a number by an epoch
function Epoch.__div(a,b)
	local self,x
	if type(b)=='number' then
		self,x = a,b
	elseif type(a)=='number' then 
		self,x = b,a
	else
		error("one operand must be a number", 2)
	end
	return NewEpoch(self.d / x)
end


-- Comparisons
function Epoch.__eq(a,b)
	return a.d == b.d
end

function Epoch.__lt(a,b)
	if type(a)=='table' and getmetatable(a)==Epoch then
		a = a.d
	else
		a = (Epoch.new(a)).d
	end

	if type(b)=='table' and getmetatable(b)==Epoch then
		b = b.d
	else
		b = (Epoch.new(b)).d
	end

	return a < b
end

function Epoch.__le(x)
	if type(a)=='table' and getmetatable(a)==Epoch then
		a = a.d
	else
		a = (Epoch.new(a)).d
	end

	if type(b)=='table' and getmetatable(b)==Epoch then
		b = b.d
	else
		b = (Epoch.new(b)).d
	end

	return a <= b
end



-- Return epoch as seconds
function Epoch:seconds()
	return self.d
end

-- Return epoch as days
function Epoch:days()
	return self.d/86400
end



-- 
-- Return the module
--
return {
	Time    = Datetime.new,
	Epoch   = Epoch.new,
	mktime  = Datetime.mktime, -- special constructor Y M D h m s UTC
	VERSION = "Time v0.2"
}

-- eof
