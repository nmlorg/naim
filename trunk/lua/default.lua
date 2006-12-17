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

naim.sockets = {}

naim.connections = {}
setmetatable(naim.connections, naim.internal.rometatable("connections"))

function naim.internal.varsub(s, t)
	s = string.gsub(s, "$%((.*)%)",
		function(n)
			return(loadstring("return " .. n)())
		end)
	s = string.gsub(s, "$\{([a-zA-Z0-9:_]+)\}",
		function(n)
			return(t[n])
		end)
	s = string.gsub(s, "$([a-zA-Z0-9:_]+)",
		function(n)
			return(t[n])
		end)
	return(s)
end

function naim.internal.expandstring(s)
	return(naim.internal.varsub(s, naim.variables))
end

function naim.internal.newconn(winname, handle)
	setmetatable(naim.connections, {})
	naim.connections[winname] = {
		handle = handle,
		name = winname,
		windows = {},
		buddies = {},
		groups = {},
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

naim.help = {
	topics = {
		"In the top-right corner of your screen is a \"window list\" window, which will display one window for each online buddy once you have signed on. The window highlighted at the top is the current window. Use the Tab key to cycle through the listed windows, or /jump buddyname to jump directly to buddyname's window.",
		"",
		"The window list window will hide itself automatically to free up space for conversation, but it will come back if someone other than your current buddy messages you. You can press Ctrl-N to go to the next waiting (yellow) buddy when this happens.",
	},
	about = {
		"naim is a console-mode chat client.",
		"naim supports AIM, IRC, ICQ, and Lily.",
		"",
		"Copyright 1998-2006 Daniel Reed <n@ml.org>",
		"http://naim.n.ml.org/",
	},
}

naim.commands.help = {
	min = 0,
	max = 1,
	desc = "Display topical help on using naim",
	args = { "topic" },
	func = function(conn, arg)
		local found

		if #arg == 0 then
			arg[1] = "topics"
		end

		local function helpcmd(cmd, t)
			local str
			local i

			str = "<font color=\"#FF00FF\">Usage</font>: <font color=\"#00FF00\">/" .. cmd .. "</font>"
			i = 0
			while (i < t.min) do
				str = str .. " <font color=\"#00FF00\">&lt;" .. t.args[i+1] .. "&gt;</font>"
				i = i + 1
			end
			while (i < t.max) do
				str = str .. " [<font color=\"#FFFF00\">&lt;" .. t.args[i+1] .. "&gt;</font>"
				i = i + 1
			end
			i = t.min
			while (i < t.max) do
				str = str .. "]"
				i = i + 1
			end
			naim.curwin():x_hprint(str .. "<br>")
			if t.desc ~= nil then
				naim.curwin():x_hprint("<b>" .. t.desc .. "</b><br>")
			end
		end

		found = true
		if arg[1] == "keys" then
			naim.echo("Current key bindings can be viewed at any time with <font color=\"#00FF00\">/bind</font>:")
			naim.call(naim.commands.bind)
			naim.curwin():x_hprint("Key names beginning with ^ are entered by holding down the Ctrl key while pressing the listed key: ^N is Ctrl+N.<br>")
			naim.curwin():x_hprint("Key names beginning with M- are entered by holding down the Alt key while pressing the key, or by pressing Esc first, then typing the key: M-a is Alt+A.<br>")
			naim.curwin():x_hprint("IC is Ins and DC is Del on the numeric keypad. NPAGE and PPAGE are PgDn and PgUp.<br>")
			naim.curwin():x_hprint("Type <font color=\"#00FF00\">/bind &lt;keyname&gt; \"&lt;script&gt;\"</font> to change a key binding.<br>")
		elseif arg[1] == "settings" or arg[1] == "variables" then
			naim.echo("Current configuration settings can be viewed at any time with <font color=\"#00FF00\">/set</font>:")
			naim.call(naim.commands.set)
			naim.curwin():x_hprint("Type <font color=\"#00FF00\">/set &lt;varname&gt; \"&lt;new value&gt;\"</font> to change a configuration variable.<br>")
		elseif arg[1] == "commands" then
			naim.echo("Help on " .. tostring(arg[1]) .. ":")
			for cmd,t in pairs(naim.commands) do
				helpcmd(cmd, t)
				naim.curwin():x_hprint("<br>")
			end
		else
			found = false
		end

		if naim.commands[arg[1]] ~= nil then
			if not found then
				naim.echo("Help on " .. tostring(arg[1]) .. ":")
				found = true
			end
			helpcmd(arg[1], naim.commands[arg[1]])
		end

		if type(naim.help[arg[1]]) == "string" then
			if not found then
				naim.echo("Help on " .. tostring(arg[1]) .. ":")
				found = true
			end
			naim.curwin():x_hprint(naim.help[arg[1]] .. "<br>")
		elseif type(naim.help[arg[1]]) == "table" then
			if not found then
				naim.echo("Help on " .. tostring(arg[1]) .. ":")
				found = true
			end
			for i,v in ipairs(naim.help[arg[1]]) do
				naim.curwin():x_hprint(v .. "<br>")
			end
		end

		if not found then
			naim.echo("No help available for " .. arg[1] .. ".")
		--else
		--	naim.echo("Use the scroll keys (PgUp and PgDn or Ctrl-R and Ctrl-Y) to view the entire help.")
		end
	end,
}

naim.hooks.add('proto_chat_joined', function(conn, chat)
	assert(conn.groups[string.lower(chat)] == nil)
	conn.groups[string.lower(chat)] = {
		members = {},
	}

	if naim.variables.autonames ~= 1 then
		conn.windows[string.lower(chat)]:echo("You are now participating in the " .. chat .. " group.")
	else
		conn.windows[string.lower(chat)]:echo("You are now participating in the " .. chat .. " group. Checking for current participants...")
	end
end, 100)

naim.hooks.add('proto_chat_synched', function(conn, chat)
	assert(conn.groups[string.lower(chat)])
	conn.groups[string.lower(chat)].synched = true

	local maxlen = 0

	for member,info in pairs(conn.groups[string.lower(chat)].members) do
		local mlen = member:len() + (info.admin and 1 or 0)

		if mlen > maxlen then
			maxlen = mlen
		end
	end

	local t = {}

	for member,info in pairs(conn.groups[string.lower(chat)].members) do
		local mlen = member:len() + (info.admin and 1 or 0)

		table.insert(t, (info.admin and "@" or "") .. member .. string.rep("&nbsp;", maxlen-mlen))
	end

	local p = "Users in group " .. chat .. ":"

	conn.windows[string.lower(chat)]:echo(p .. string.rep("&nbsp;", (maxlen+1)-math.fmod(p:len(), maxlen+1)) .. table.concat(t, "&nbsp;"))
end, 100)

naim.hooks.add('proto_chat_left', function(conn, chat)
	conn.groups[string.lower(chat)] = nil
end, 100)

naim.hooks.add('proto_chat_kicked', function(conn, chat)
	conn.groups[string.lower(chat)] = nil
end, 100)

naim.hooks.add('proto_chat_oped', function(conn, chat, by)
	conn.groups[string.lower(chat)].admin = true
end, 100)

naim.hooks.add('proto_chat_deoped', function(conn, chat, by)
	conn.groups[string.lower(chat)].admin = nil
end, 100)

naim.hooks.add('proto_chat_user_joined', function(conn, chat, who, extra)
	conn.groups[string.lower(chat)].members[who] = {}

	if conn.groups[string.lower(chat)].synched then
		if extra and extra ~= "" then
			conn.windows[string.lower(chat)]:echo("<font color=\"#00FFFF\">" .. who .. "</font> (" .. extra .. ") has joined group " .. chat .. ".")
		else
			conn.windows[string.lower(chat)]:echo("<font color=\"#00FFFF\">" .. who .. "</font> has joined group " .. chat .. ".")
		end
	end
end, 100)

naim.hooks.add('proto_chat_user_left', function(conn, chat, who, reason)
	conn.groups[string.lower(chat)].members[who] = nil

	if reason and reason ~= "" then
		conn.windows[string.lower(chat)]:echo("<font color=\"#00FFFF\">" .. who .. "</font> has left group " .. chat .. " (</b><body>" .. reason .. "</body><b>).")
	else
		conn.windows[string.lower(chat)]:echo("<font color=\"#00FFFF\">" .. who .. "</font> has left group " .. chat .. ".")
	end
end, 100)

naim.hooks.add('proto_chat_user_kicked', function(conn, chat, who, by, reason)
	conn.groups[string.lower(chat)].members[who] = nil

	if reason and reason ~= "" then
		conn.windows[string.lower(chat)]:echo("<font color=\"#00FFFF\">" .. who .. "</font> has been kicked out of group " .. chat .. " by <font color=\"#00FFFF\">" .. by .. "</font> (</b><body>" .. reason .. "</body><b>).")
	else
		conn.windows[string.lower(chat)]:echo("<font color=\"#00FFFF\">" .. who .. "</font> has been kicked out of group " .. chat .. " by <font color=\"#00FFFF\">" .. by .. "</font>.")
	end
end, 100)

naim.hooks.add('proto_chat_user_oped', function(conn, chat, who, by)
	conn.groups[string.lower(chat)].members[who].admin = true
end, 100)

naim.hooks.add('proto_chat_user_deoped', function(conn, chat, who, by)
	conn.groups[string.lower(chat)].members[who].admin = nil
end, 100)

naim.hooks.add('proto_chat_user_nickchanged', function(conn, chat, who, newnick)
	conn.groups[string.lower(chat)].members[newnick] = conn.groups[string.lower(chat)].members[who]
	conn.groups[string.lower(chat)].members[who] = nil
end, 100)

naim.hooks.add('preselect', function(rfd, wfd, efd, maxfd)
	for k,v in pairs(naim.sockets) do
	end
end, 100)

naim.hooks.add('postselect', function(rfd, wfd, efd)
	for k,v in pairs(naim.sockets) do
	end
end, 100)
