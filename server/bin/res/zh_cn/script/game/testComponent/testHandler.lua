--test msg handler
registerHandler("req_test_role",
	function(sessionId, req)
		local rawRole = getRawRole(sessionId)
		if rawRole == nil then
			print("get raw role fail")
			return;
		end

		print(cjson.encode(req));

		local luaRole = rawRole:getLuaRole()
		local testComponent = luaRole.mComponents["TestComponent"]
		testComponent:setTestValue(1000)
		luaRole:saveComponent(testComponent);
	end
)

registerHandler("req_enter_city",
	function(sessionId, req)
		local rawRole = getRawRole(sessionId)
		if rawRole == nil then
			print("get raw role fail")
			return;
		end

		print("lua enter city ", req.cityid, req.threadid)


		local luaRole = rawRole:getLuaRole()
		local testComponent = luaRole.mComponents["TestComponent"]
		testComponent:setTestValue(1000)
		luaRole:saveComponent(testComponent);

		local notify = {}
		notify.gold = 1000000
		sendNetPacket(sessionId, "notify_syn_gold", notify)
	end
)
