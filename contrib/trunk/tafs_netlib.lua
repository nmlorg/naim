if not naim.netlib then
	naim.debug("netlib is not loaded.  Please load it and reload this script")
end

assert(naim.netlib)

naim.tafs = {}
function naim.tafs.getmetar(args)	-- must be called from a netlib registered function
	local data = naim.netlib.httpget(args.ctbl, "adds.aviationweather.noaa.gov",
		"/metars/index.php?station_ids="..args.data.."&std_trans=standard&chk_metars=on&hoursStr=most+recent+only&submitmet=Submit")
	local result
	if data:match('<FONT FACE="Monospace,Courier">(.-)</FONT>') then
		result = data:match('<FONT FACE="Monospace,Courier">(.-)</FONT>')
	else
		naim.echo(data:gsub("&","&amp;"):gsub("<","&lt;"):gsub(">","&gt;"):gsub("\n","<BR>"))
		result = "Download failure"
	end
	return result
end
if naim.tafs.hook then naim.netlib.unregister(naim.tafs.hook) naim.tafs.hook = nil end

-- register a recvfrom hook
-- args has .ctbl, .data (which is the matched text) and .sn which is the speaker

naim.tafs.hook = naim.netlib.register("^!metar ([A-Z0-9a-z]*)$", function (args)
	return args.sn..", METAR for "..args.data..": "..naim.tafs.getmetar(args)
end)

naim.echo("[tafs_em.lua] loaded")
