-- weather.lua by Andrew Chin
-- gets weather info from yahoo
-- based off some stuff by Joshua Wise

-- uses netlib

require("netlib")

if netlib._APIVERSION < 1 then
	error("weather.lua requires version 1 of netlib;  your version is "..netlib._APIVERSION)
end

if not naim then
	error("Need to be run from within naim")
end

if not naim.weather then
	naim.weather={ history={}}
end

function naim.weather.getdata(args)
	local data = netlib.httpget("weather.yahooapis.com", "/forecastrss?p="..args)
	city = data:match("<yweather:location.*city=\"([%w/\. ]+)\".*/>")

	condition = data:match("<yweather:condition.*text=\"([%w/\. ]+)\".*/>")
	 if condition then tosend = "Currently in " .. city .. ": " .. condition else tosend = "" end

	temp = data:match("<yweather:condition.*temp=\"(%d+)\".*/>")
	if temp then tosend = tosend .. " temp=" .. temp end
	tempunit = data:match("<yweather:units.*temperature=\"(%a)\".*/>")
	if tempunit then tosend = tosend .. tempunit else tempunit = "" end

	windchill = data:match("<yweather:wind.*chill=\"(%d+)\".*/>")
	if windchill then tosend = tosend .. " windchill=" .. windchill end

	humidity = data:match("<yweather:atmosphere.*humidity=\"(%d+)\".*/>")
	if humidity then tosend = tosend .. " humidity=" .. humidity .. "%" end

	forecastday = data:match("<yweather:forecast.*day=\"(%w+)\".*/>")
	if forecastday then
	tosend = tosend .. " " .. forecastday .. ":"

	forecastlow = data:match("<yweather:forecast.*low=\"(%d+)\".*/>")
	if forecastlow then tosend = tosend .. " low=" .. forecastlow end

	forecasthigh = data:match("<yweather:forecast.*high=\"(%d+)\".*/>")
	if forecasthigh then tosend = tosend .. " high=" .. forecasthigh end

	forecasttext = data:match("<yweather:forecast.*text=\"([%w/\. ]+)\".*/>")
	if forecasttext then tosend = tosend .. " " .. forecasttext end
	end

	if tosend == "" then
		return "bad zipcode?"
	else
		naim.echo("trying to send .. \"" .. tosend .. "\"")
		return tosend
	end
end

if naim.weather.hook then naim.hooks.del('proto_recvfrom',naim.weather.hook) naim.weather.hook = nil end

naim.weather.hook = naim.hooks.add('proto_recvfrom', function (conn, sn, dest, text, flags)
	if not dest then rcpt = sn else rcpt = dest end
	if text:match("^!weather (%d%d%d%d%d)$") then
		local zip = text:match("^!weather (%d%d%d%d%d)$")
		naim.weather.history[sn] = zip
		netlib.createcontext(function() conn:msg(rcpt,naim.weather.getdata(zip) ) end)
	elseif text:match("^!weather$") then
		if not naim.weather.history[sn] then
			conn:msg(rcpt,"What zipcode do you want the weather for?")
		else
			netlib.createcontext(function() conn:msg(rcpt, naim.weather.getdata(naim.weather.history[sn]))end)
		end
	
	end

end, 100)

naim.echo("[weather.lua] loaded.")
