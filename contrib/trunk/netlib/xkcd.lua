--  _ __   __ _ ___ __  __
-- | '_ \ / _` |_ _|  \/  | naim
-- | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
-- | | | | |_| || || |  | | Lua support Copyright 2006-2007 Joshua Wise <joshua@joshuawise.com>
-- |_| |_|\__,_|___|_|  |_| ncurses-based chat client
--
-- xkcd.lua
-- Downloads the latest xkcd RSS on request and informs of the latest xkcd.

require("netlib")

if netlib._APIVERSION ~= 1 then
	error("tafs.lua requires a version 1 netlib; your netlib is version "..netlib._APIVERSION)
end

if not jaw then
	jaw = {}
end
if not jaw.xkcd then
	jaw.xkcd = {}
end
if jaw.xkcd.hook then
	naim.hooks.del('proto_recvfrom', jaw.xkcd.hook)
end

jaw.xkcd.hook = naim.hooks.add('proto_recvfrom', function (conn, sn, dest, text, flags)
	if text:match("^!xkcd") then
		netlib.createcontext(function()
			local data = netlib.httpget("xkcd.com", "/rss.xml")
			if data:match("<item>[^<]*<title>([^<]*)</title>[^<]*<link>([^<]*)") then
				title,link = data:match("<item>[^<]*<title>([^<]*)</title>[^<]*<link>([^<]*)")
				conn:msg(dest, sn .. ", latest xkcd is " .. title .. " ( " .. link .. " )")
			else
				conn:msg(dest, sn .. ", no xkcd RSS data!")
			end
		end)
	end
end, 100)

naim.echo("[xkcd.lua] loaded")
