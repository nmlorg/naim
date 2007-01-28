--  _ __   __ _ ___ __  __
-- | '_ \ / _` |_ _|  \/  | naim
-- | | | | | | || || |\/| | Copyright 1998-2006 Daniel Reed <n@ml.org>
-- | | | | |_| || || |  | | Lua support Copyright 2006-2007 Joshua Wise <joshua@joshuawise.com>
-- |_| |_|\__,_|___|_|  |_| ncurses-based chat client
--
-- tafs.lua
-- Downloads the latest TAFs on request and parses.

require("netlib")

if netlib._APIVERSION ~= 1 then
	error("tafs.lua requires a version 1 netlib; your netlib is version "..netlib._APIVERSION)
end

if not jaw then
	jaw = {}
end

if not jaw.tafs then
	jaw.tafs = {}
end

if jaw.tafs.hook then
	naim.hooks.del('proto_recvfrom', jaw.tafs.hook)
	jaw.tafs.hook = nil
end

function jaw.tafs.getmetar(arpt)	-- must be called from a netlib context
	local data = netlib.httpget("adds.aviationweather.noaa.gov",
		"/metars/index.php?station_ids="..arpt.."&std_trans=standard&chk_metars=on&hoursStr=most+recent+only&submitmet=Submit")
	local result
	if data:match('<FONT FACE="Monospace,Courier">(.-)</FONT>') then
		result = data:match('<FONT FACE="Monospace,Courier">(.-)</FONT>')
	else
		naim.echo(data:gsub("&","&amp;"):gsub("<","&lt;"):gsub(">","&gt;"):gsub("\n","<BR>"))
		result = "Download failure"
	end
	return result
end

function jaw.tafs.metartranslate(metar)
	local translated,untranslated = "", ""
	local gotadvancedtemp = false
	local oldtemp = ""
	
	metar = metar:gsub("\n","")
	
	local token = metar:match("(.-) ") -- drop the first token
	metar = metar:sub(token:len()+2)
	while metar:len() ~= 0 do
		local token = metar:match("(.-) ")
		if not token then token = metar end
		metar = metar:sub(token:len()+2)
		
		if token:match("^%d%d%d%d%d%dZ$") then
			translated = translated .. "Issued at " .. token:sub(3,4) .. ":" .. token:sub(5,6) .. " GMT. "
		elseif token:match("^...%d%dKT$") then
			local winddir = token:sub(1,3)
			if winddir == "VRB" then
				winddir = "variable directions"
			else
				winddir = winddir .. " degrees"
			end
			local speed = token:sub(4,5)
			if speed:sub(1,1) == "0" then speed = speed:sub(2) end
			translated = translated .. "Winds from ".. winddir .. " at ".. speed .. " knots. "
		elseif token:match("^...%d%dG%d%dKT$") then
			local winddir = token:sub(1,3)
			if winddir == "VRB" then
				winddir = "variable directions"
			else
				winddir = winddir .. " degrees"
			end
			local speed = token:sub(4,5)
			if speed:sub(1,1) == "0" then speed = speed:sub(2) end
			local gust = token:sub(7,8)
			if gust:sub(1,1) == "0" then gust = gust:sub(2) end
			translated = translated .. "Winds from ".. winddir .. " at ".. speed .. " knots, gusting ".. gust .." knots. "
		elseif token:match("^10SM$") then
			translated = translated .. "10+ mile visibility. "
		elseif token:match("^.*SM$") then
			translated = translated .. token:sub(1,token:len()-2).." mile visibility. "
		elseif token:match("^SLP%d%d%d$") then
			-- sea level pressure, uninteresting. must go before clouds
		elseif token:match("^RMK$") then
			-- remarks, uninteresting
		elseif token:match("^%u%u%u%d%d%d$") then
			local cloudtype
			local cloudtypes = {
				["BKN"] = "Broken clouds",
				["FEW"] = "Few clouds",
				["OVC"] = "Overcast skies",
				["SCT"] = "Scattered clouds",
			}
			cloudtype = cloudtypes[token:sub(1,3)] or ("Unknown cloud type " .. token:sub(1,3))
			local altitude = token:sub(4,6)
			while altitude:sub(1,1) == "0" do
				altitude = altitude:sub(2)
			end
			if altitude == "" then
				altitude = "ground level"
			elseif altitude:len() == 1 then
				altitude = altitude .. "00 feet"
			else
				altitude = altitude:sub(1,altitude:len()-1) .. "," .. altitude:sub(altitude:len()) .. "00 feet"
			end
			translated = translated .. cloudtype.." at "..altitude..". "
		elseif token:match("^A%d%d%d%d$") then
			translated = translated .. "Barometric pressure "..token:sub(2,3).."."..token:sub(4,5).."\" Hg. "
		elseif token:match("^T%d%d%d%d%d%d%d%d") then
			local temp = token:sub(3,4) .. "." .. token:sub(5,5) .. "C"
			local dp = token:sub(7,8) .. "." .. token:sub(9,9) .. "C"
			if dp:sub(1,1) == "0" then temp = temp:sub(2) end
			if dp:sub(1,1) == "0" then dp = dp:sub(2) end
			temp = ((token:sub(2,2) == "1") and "-" or "") .. temp
			dp = ((token:sub(6,6) == "1") and "-" or "") .. dp
			translated = translated .. "Temperature " .. temp ..". Dew point " .. dp .. ". "
			gotadvancedtemp = true
		elseif token:match("^M?%d%d/M?%d%d$") then
			local temp,dp = token:match("^(M?%d%d)/(M?%d%d)")
			local proc = function(t)
				local neg = ""
				if t:sub(1,1) == "M" then
					neg = "-"
					t = t:sub(2)
				end
				if t:sub(1,1) == "0" then
					t = t:sub(2)
				end
				return neg .. t
			end
			oldtemp = "Temperature " .. proc(temp) .. "C. Dew point " .. proc(dp) .. "C. "
		elseif token:match("^CAVOK") then
			translated = translated .. "Ceilings and Visibilities OK. "
		elseif token:match("^NOSIG") then
			translated = translated .. "No significant change expected in the next 2 hours. "
		elseif token:match("^CLR") then
			translated = translated .. "Skies clear. "
		elseif token:match("^AUTO") then
			translated = translated .. "Automatically observed. "
		elseif token:match("^Q%d%d%d%d") then
			local t = token:sub(2,5)
			if t:sub(1,1) == "" then 
				t = t:sub(2)
			end
			translated = translated .. "Barometric pressure "..t.." millibars. "
		elseif token:match("^SLP%d%d%d") then
			translated = translated .. "Sea level barometric pressure 10"..token:sub(4,5).."."..token:sub(6,6) .. " millibars. "
		elseif token:match("^%d%d%d%d$") then
			if token == "9999" then
				translated = translated .. "Visibility unlimited. "
			else
				translated = translated .. "Visibility "..token.." meters. "
			end
		elseif token:match("^AO2") then
			-- they have a precip sensor; who cares?
		elseif token:match("^P%d%d%d%d") then
			local precip = token:sub(2,3) .. "." .. token:sub(4,5)
			if precip:sub(1,1) == "0" then
				precip = precip:sub(2)
			end
			translated = translated .. precip .. "\" of precipitation in the past hour. "
		elseif token:match("^[+-]?%u+$") and ((token:len() - (token:match("^[+-]") and 1 or 0)) % 2) == 0 then
			local descriptor = ""
			local trailer = ""
			if token:sub(1,1) == "+" then
				descriptor = "heavy "
				token = token:sub(2)
			end
			if token:sub(1,1) == "-" then
				descriptor = "light "
				token = token:sub(2)
			end
			if token:sub(1,2) == "VC" then 
				trailer = "in the vicinity"
				token = token:sub(3)
			end
			while token:len() > 0 do
				local minitoken = token:sub(1,2)
				local translations = {
					["MI"] = "shallow ", ["BC"] = "patches of ", ["PR"] = "partial ", ["TS"] = "thunderstorming ",
					["BL"] = "blowing ", ["SH"] = "showers of ", ["DR"] = "drifting ", ["FZ"] = "freezing ",
					["DZ"] = "drizzle", ["RA"] = "rain", ["SN"] = "snow", ["SG"] = "snow grains",
					["IC"] = "ice crystals", ["PL"] = "ice pellets", ["GR"] = "hail", ["GS"] = "small hail",
					["UP"] = "precipitation of unknown variant",
					["BR"] = "mist", ["FG"] = "fog", ["FU"] = "smoke", ["VA"] = "volcanic ash",
					["SA"] = "sand", ["HZ"] = "haze", ["PY"] = "spray", ["DU"] = "widespread dust",
					["SQ"] = "squall", ["SS"] = "sandstorm", ["DS"] = "duststorm", ["PO"] = "well developed dust/sand whirls",
					["FC"] = "funnel cloud", }
				if translations[minitoken] then
					descriptor = descriptor .. translations[minitoken]
				else
					descriptor = descriptor .. "["..minitoken.."] "
				end
				token = token:sub(3)
			end
			translated = translated .. descriptor:sub(1,1):upper() .. descriptor:sub(2) .. trailer .. ". "
		elseif token == "" then
			-- nothing interesting
		else
			untranslated = untranslated .. token .. " "
		end
	end
	if not gotadvancedtemp then
		translated = translated .. oldtemp
	end
	while translated:sub(1,1) == " " do
		translated = translated:sub(2)
	end
	if untranslated ~= "" then
		translated = translated .. "Untranslated tokens: "..untranslated
	end
	return translated
end

jaw.tafs.hook = naim.hooks.add('proto_recvfrom', function(conn, sn, dest, text, flags)
	if text:match("^!metar ([A-Z0-9a-z]*)$") then
		local d = text:match("^!metar ([A-Z0-9a-z]*)$"):upper()
		netlib.createcontext(function ()
			conn:msg(dest, sn..", METAR for "..d..": "..jaw.tafs.getmetar(d))
		end)
	elseif text:match("^!metar translate ([A-Z0-9a-z]*)$") then
		local d = text:match("^!metar translate ([A-Z0-9a-z]*)$"):upper()
		netlib.createcontext(function ()
			conn:msg(dest, sn..", Translated METAR for "..d..": "..jaw.tafs.metartranslate(jaw.tafs.getmetar(d)))
		end)
	end
end, 100)

naim.echo("[tafs.lua] loaded")
