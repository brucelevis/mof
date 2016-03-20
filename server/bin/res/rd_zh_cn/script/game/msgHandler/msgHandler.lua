--msg
g_MsgHandlers = {}
function registerHandler(name, handler)
	g_MsgHandlers[name] = handler;
end

function getHandler(name)
	return  g_MsgHandlers[name]
end

function luaHandleNetMsg(sessionId, name, req)
	local handler = getHandler(name)
	if handler == nil then
		print("no handler to handle "..name)
		return -1
	else
		print("handle net msg"..name)
		handler(sessionId, req)
	end
end
