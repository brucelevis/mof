local super_ips = {"192.168.188.101", "127.0.0.1", "113.107.234.126"}

function is_super_ip(ip)
	for i,v in ipairs(super_ips)
	do
		if ip == v then
			return 1
		end
	end

	return 0
end
