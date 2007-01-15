--  _ __   __ _ ___ __  __
-- | '_ \ / _` |_ _|  \/  | naim
-- | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
-- | | | | |_| || || |  | | Lua support Copyright 2006-2007 Joshua Wise <joshua@joshuawise.com>
-- |_| |_|\__,_|___|_|  |_| ncurses-based chat client
--
-- netlib.lua
-- library for very basic network stuff
-- based off joshua's tafs.lua
-- Andrew Chin (achin@andrewchin.net)


if not naim.socket or not naim.buffer then
	naim.debug("I NEED SOCKETS!!1!mrow")
end

if not naim.hooks then
	naim.debug("no naim.hooks")
end

if naim.netlib then
	naim.netlib.dereg()
end

naim.netlib = {
	activecoroutines = {},
	hooks = {},

}


naim.netlib.hooks.preselect = naim.hooks.add('preselect',
	function(rd, wr, ex, n)
		for k,v in pairs(naim.netlib.activecoroutines) do
			if v.sock then
				n = v.sock:preselect(rd, wr, ex, n)
			end
		end
		return true, n
	end, 100)

naim.netlib.hooks.postselect = naim.hooks.add('postselect',
	function(rd, wr, ex)
		for k,v in pairs(naim.netlib.activecoroutines) do
			if v.sock then
				e = v.sock:postselect(rd, wr, ex, v.buf)
				if e then
					naim.echo("Postselect error: " .. e)
					v.error = e
				end
			end
			if coroutine.status(v.coroutine) ~= "dead" then
				local resumeval,error = coroutine.resume(v.coroutine, v.args)
				if resumeval == false then
					naim.echo("Coroutine error: " .. error)
					table.remove(naim.netlib.activecoroutines, k)
				end
			else
				table.remove(naim.netlib.activecoroutines, k)
			end
		end
	end, 100)

function naim.netlib.httpget(ctbl, host, path)	-- must be called from coroutine context
	ctbl.sock = naim.socket.new()
	ctbl.buf = naim.buffer.new()
	ctbl.buf:resize(32768)
	ctbl.sock:connect(host, 80)
	while not ctbl.sock:connected() do
		coroutine.yield()
	end
	ctbl.sock:send(
		"GET "..path.." HTTP/1.1\n"..
		"Host: "..host.."\n"..
		"User-Agent: Mozilla/5.0 (not really) naimbot\n"..
		"Connection: close\n"..
		"\n")
	local data = ""
	while ctbl.sock:connected() do
		while ctbl.sock:connected() and ctbl.buf:peek():len() == 0 do
			coroutine.yield()
		end
		data = data .. ctbl.buf:take()
	end
	return data
end


function naim.netlib.register (match_string, func) 
	local f
	if type(match_string) == "string" then
		f = function (conn, sn, dest, text, flags)
			if text:match(match_string) then
				local arpt = text:match(match_string)
				local ctbl = {}
				table.insert(naim.netlib.activecoroutines, ctbl)
				ctbl.coroutine = coroutine.create( function(args) conn:msg(dest, func(args)) end )
				ctbl.args = {ctbl = ctbl, data = arpt, sn = sn}
			end
		end
	end
	if type(match_string) == "function" then
		f = function (conn, sn, dest, text, flags)
				local arpt = match_string(text, sn)
				if not arpt or arpt == false then return end
				local ctbl = {}
				table.insert(naim.netlib.activecoroutines, ctbl)
				ctbl.coroutine = coroutine.create( function(args) conn:msg(dest, func(args)) end )
				ctbl.args = {ctbl = ctbl, data = arpt, sn = sn}
			
		end
	end
	return naim.hooks.add('proto_recvfrom', f, 100)
end

function naim.netlib.unregister(r) 
	naim.hooks.del('proto_recvfrom', r)
end

function naim.netlib.dereg()
	for k,v in pairs(naim.netlib.hooks) do
		naim.hooks.del(k,v)
	end
	naim.netlib = nil
end

naim.debug("[netlib.lua] loaded")
