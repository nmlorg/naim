-- weather.lua by Andrew Chin
-- gets weather info from yahoo
-- based off some stuff by Joshua Wise

-- uses netlib

assert(naim.netlib)
naim.weather={ history={}}

function naim.weather.getdata(args)
	
	local data = naim.netlib.httpget(args.ctbl, "weather.yahooapis.com", "/forecastrss?p="..args.data)

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

if naim.weather.hook then naim.netlib.unregister(naim.weather.hook) naim.weather.hook = nil end

naim.weather.hook = naim.netlib.register(function (text, sn)
	textmatch = text:match("^!weather (%d%d%d%d%d)$")
	if textmatch then
		naim.weather.history[sn] = textmatch
		return textmatch
	elseif text:match("^!weather$") then
		if naim.weather.history[sn] then return naim.weather.history[sn] else return nil end
	end
end, function (args)
	return naim.weather.getdata(args)
end)

naim.echo("[weather_em.lua] loaded.")
