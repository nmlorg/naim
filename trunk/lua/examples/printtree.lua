function naim.printtree(t)
	visited = {}

	function doprint(t, depth)
		for k,v in pairs(t) do
			if type(v) == "table" then
				naim.echo(string.rep(" &nbsp;", depth) .. " " .. tostring(k) .. " = {")
				if visited[v] == nil then
					visited[v] = true
					doprint(v, depth+1)
				else
					naim.echo(string.rep(" &nbsp;", depth+1) .. " (already displayed)")
				end
				naim.echo(string.rep(" &nbsp;", depth) .. " }")
			else
				naim.echo(string.rep(" &nbsp;", depth) .. " " .. tostring(k) .. " = (" .. type(v) .. ")" .. tostring(v))
			end
		end
	end

	doprint(t, 0)
end
