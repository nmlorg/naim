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

naim.help = {}

naim.variables = {}

naim.connections = {}
setmetatable(naim.connections, naim.internal.rometatable("connections"))

function naim.internal.expandstring(s)
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

function naim.call(table, ...)
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

	if arg[1] == nil or type(arg[1]) == "string" then
		conn = naim.curconn()
	else
		if isconn(arg[1]) then
			conn = arg[1]
			arg = shift(arg)
		elseif arg[1].conn == nil then
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
		assert(#newarg == i-1)
		if (arg ~= nil) then
			newarg[i] = arg
		end
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

naim.commands.help.min = 0
naim.commands.help.max = 1
function naim.commands.help.func(conn, arg)
	if #arg == 0 then
		arg[1] = "topics"
	end
	naim.echo("Help on " .. tostring(arg[1]) .. ":")

	local function helpcmd(cmd, t)
		local str
		local i

		str = "Usage: /" .. cmd
		i = 0
		while (i < t.min) do
			str = str .. " <" .. t.args[i+1] .. ">"
			i = i + 1
		end
		while (i < t.max) do
			str = str .. " [<" .. t.args[i+1] .. ">"
			i = i + 1
		end
		i = t.min
		while (i < t.max) do
			str = str .. "]"
			i = i + 1
		end
		naim.echo(str)
		if t.desc ~= nil then
			naim.echo(t.desc)
		end
	end

	if arg[1] == "keys" then
		naim.echo("Current key bindings can be viewed at any time with /bind:")
		naim.call(naim.commands.bind)
		naim.echo("Key names beginning with ^ are entered by holding down the Ctrl key while pressing the listed key: ^N is Ctrl+N.")
		naim.echo("Key names beginning with M- are entered by holding down the Alt key while pressing the key, or by pressing Esc first, then typing the key: M-a is Alt+A.")
		naim.echo("IC is Ins and DC is Del on the numeric keypad. NPAGE and PPAGE are PgDn and PgUp.")
		naim.echo("Type /bind <keyname> \"<script>\" to change a key binding.")
	elseif arg[1] == "settings" or arg[1] == "variables" then
		naim.echo("Current configuration settings can be viewed at any time with /set:")
		naim.call(naim.commands.set)
		naim.echo("Type /set <varname> \"<new value>\" to change a configuration variable.")
	elseif arg[1] == "commands" then
		for cmd,t in pairs(naim.commands) do
			helpcmd(cmd, t)
			naim.echo("")
		end
	end

	if naim.commands[arg[1]] ~= nil then
		helpcmd(arg[1], naim.commands[arg[1]])
	end

	if naim.help[arg[1]] ~= nil then
		naim.echo(naim.help[arg[1]])
	end

	naim.echo("Use the scroll keys (PgUp and PgDn or Ctrl-R and Ctrl-Y) to view the entire help.")
end
