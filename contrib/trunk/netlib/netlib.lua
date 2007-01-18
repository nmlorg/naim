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

if not naim then
	error("netlib must be invoked from inside naim")
end

if not naim.socket or not naim.buffer or not naim.hooks then
	error("Your naim is not new enough; please get naim from svn")
end

if naim.netlib then
	naim.netlib.dereg()
end

if netlib and netlib.dereg then netlib.dereg() end

netlib = {
	contexts = {},
	hooks = {},
	-- ugh
	_NAME = "netlib",
	_M = netlib,
	_PACKAGE = "",
	_APIVERSION = 1,	-- our specific extension
}

netlib.hooks.preselect = naim.hooks.add('preselect',
	function(rd, wr, ex, n)
		for cr,socks in pairs(netlib.contexts) do
			for sock,t in pairs(socks) do
				n = sock:preselect(rd, wr, ex, n)
			end
		end
		return true, n
	end, 100)

netlib.hooks.postselect = naim.hooks.add('postselect',
	function(rd, wr, ex)
		for cr,socks in pairs(netlib.contexts) do
			for sock,t in pairs(socks) do
				t.error = sock:postselect(rd, wr, ex, t.buf)
			end

			if coroutine.status(cr) ~= "dead" then
				local resumeval,error = coroutine.resume(cr)
				if resumeval == false then
					naim.echo("[netlib] coroutine error: " .. error)
					netlib.contexts[cr] = nil
				end
			else
				netlib.contexts[cr] = nil
			end
		end
	end, 100)

function netlib.addsocket(sock, buf)
	if not coroutine.running() then
		error("netlib function called from non-coroutine context")
	end
	if not netlib.contexts[coroutine.running()] then
		error("netlib function called from non-netlib context")
	end
	netlib.contexts[coroutine.running()][sock] = { buf = buf }
end

function netlib.delsocket(sock)
	if not coroutine.running() then
		error("netlib function called from non-coroutine context")
	end
	if not netlib.contexts[coroutine.running()] then
		error("netlib function called from non-netlib context")
	end
	netlib.contexts[coroutine.running()][sock] = nil
end

function netlib.httpget(host, path)	-- must be called from netlib context
	local sock, buf = naim.socket.new(), naim.buffer.new()
	netlib.addsocket(sock, buf)
	buf:resize(32768)
	sock:connect(host, 80)
	while not sock:connected() do
		coroutine.yield()
	end
	sock:send(
		"GET "..path.." HTTP/1.1\n"..
		"Host: "..host.."\n"..
		"User-Agent: Mozilla/5.0 (not really) naimbot\n"..
		"Connection: close\n"..
		"\n")
	local data = ""
	while sock:connected() do
		while sock:connected() and buf:peek():len() == 0 do
			coroutine.yield()
		end
		data = data .. buf:take()
	end
	netlib.delsocket(sock)
	return data
end

function netlib.createcontext(f)
	local thecoro
	if type(f) == "coroutine" then
		thecoro = f
	end
	if type(f) == "function" then
		thecoro = coroutine.create(f)
	end
	netlib.contexts[thecoro] = {}
	coroutine.resume(thecoro)
end

function netlib.dereg()
	for k,v in pairs(netlib.hooks) do
		naim.hooks.del(k,v)
	end
	netlib = nil
end

naim.debug("[netlib.lua] loaded")

module("netlib")	-- have to do this last. d'oh
