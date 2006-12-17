function naim.internal.expandString(s)
	s = string.gsub(s, "$%((.*)%)",
		function (n)
			return loadstring("return "..n)()
		end)
	s = string.gsub(s, "$\{([a-zA-Z0-9:_]+)\}",
		function (n)
			return tostring(naim.variables[n])
		end)
	s = string.gsub(s, "$([a-zA-Z0-9:_]+)",
		function (n)
			return tostring(naim.variables[n])
		end)
	return s
end

function naim.internal.rwmetatable(prototype)
	return({
		__index = function(table, key)
			if prototype[key] ~= nil then
				return prototype[key]
			elseif prototype["get_"..key] ~= nil then
				return prototype["get_"..key](table.handle)
			else
				return nil
			end
		end,
		__newindex = function(table, key, value)
			if prototype["set_"..key] ~= nil then
				prototype["set_"..key](table.handle, key, value)
			elseif prototype["get_"..key] ~= nil then
				error(key .. " is a read-only attribute",2)
			else
				rawset(table, key, value)
			end
		end,
		__tostring = function(table)
			return(table.name)
		end,
	})
end

function naim.internal.rometatable(name)
	return({
		__newindex = function (t,k,v)
			error("only naim can update the " .. name .. " table", 2)
		end
	})
end



naim.variables = {}
naim.connections = {}

setmetatable(naim.connections, naim.internal.rometatable("connections"))

function naim.internal.newconn(winname, handle)
	setmetatable(naim.connections, {})
	naim.connections[winname] = {
		handle = handle,
		name = winname,
		windows = {},
		buddies = {},
	}
	setmetatable(naim.connections[winname], naim.internal.rwmetatable(naim.prototypes.connections))
	setmetatable(naim.connections[winname].windows, naim.internal.rometatable("windows"))
	setmetatable(naim.connections[winname].buddies, naim.internal.rometatable("buddies"))
	setmetatable(naim.connections, naim.internal.rometatable("connections"))
end

function naim.internal.delconn(winname)
	setmetatable(naim.connections, {})
	naim.connections[winname] = nil
	setmetatable(naim.connections, naim.internal.rometatable("connections"))
end

function naim.internal.newwin(conn, winname, handle)
	setmetatable(conn.windows, {})
	conn.windows[winname] = {
		handle = handle,
		conn = conn,
		name = winname,
	}
	setmetatable(conn.windows[winname], naim.internal.rwmetatable(naim.prototypes.windows))
	setmetatable(conn.windows, naim.internal.rometatable("windows"))
end

function naim.internal.delwin(conn, winname)
	setmetatable(conn.windows, {})
	conn.windows[winname] = nil
	setmetatable(conn.windows, naim.internal.rometatable("windows"))
end

function naim.internal.newbuddy(conn, account, handle)
	setmetatable(conn.buddies, {})
	conn.buddies[account] = {
		handle = handle,
		conn = conn,
		name = account,
	}
	setmetatable(conn.buddies[account], naim.internal.rwmetatable(naim.prototypes.buddies))
	setmetatable(conn.buddies, naim.internal.rometatable("buddies"))
end

function naim.internal.changebuddy(conn, account, newaccount)
	setmetatable(conn.buddies, {})
	conn.buddies[newaccount] = conn.buddies[account]
	conn.buddies[account] = nil
	setmetatable(conn.buddies, naim.internal.rometatable("buddies"))
end

function naim.internal.delbuddy(conn, account)
	setmetatable(conn.buddies, {})
	conn.buddies[account] = nil
	setmetatable(conn.buddies, naim.internal.rometatable("buddies"))
end

naim.call = function(table, ...)
	local conn
	local min = table.min
	local max = table.max

	local isconn = function(conn)
		for k,v in pairs(naim.connections) do
			if v == conn then
				return(k)
			end
		end
		return(nil)
	end
	local shift = function(t)
		local n = {}

		for i,v in ipairs(t) do
			if i > 1 then
				n[i-1] = v
			end
		end
		return(n)
	end

	arg.n = nil

	if type(arg[1]) == "string" then
		conn = naim.curconn()
	else
		if isconn(arg[1]) then
			conn = arg[1]
			arg = shift(arg)
		elseif arg1[1].conn == nil then
			naim.echo("invalid first argument to " .. tostring(table))
			return
		else
			conn = arg[1].conn
			arg[1] = tostring(arg[1])
		end
	end

	if #arg == 1 then
		local newarg = {}
		local i = 1

		arg = arg[1]
		while i < max and arg ~= nil do
			newarg[i],arg = naim.internal.pullword(arg)
			i = i+1
		end
		newarg[i] = arg
		arg = newarg
	end

	if #arg > max then
		naim.echo("Command requires at most " .. max .. " arguments.")
		return
	elseif #arg < min then
		naim.echo("Command requires at least " .. min .. " arguments.")
		return
	end

	table.func(conn, arg)
end
