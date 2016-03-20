//net handler.js

var utils = require('./utils.js');

var NetHandler = function()
{
	this.handle = function(client, netmsg)
	{
		var type = netmsg.readInt();
		var id = netmsg.readInt();

		if (process.handlers[type] == undefined)
		{
			console.log('net msg type ' + type + ' id ' + id);
			return;
		}

		var handler = process.handlers[type][id];
		if (handler == undefined)
		{
			console.log('net msg type ' + type + ' id ' + id);
			return;
		}
		handler(client, netmsg);
	}
}


process.handlers = [];
function addHandler(type, id, handle) 
{
	if (process.handlers[type] == undefined)
	{
		process.handlers[type] = [];
	}

	process.handlers[type][id] = handle;
}


function createNetHandler()
{
	var nethandler = new NetHandler;
	return nethandler;
}

function getRetErr(netmsg)
{
	var ret = {};
    ret.err = netmsg.readInt();
    return ret;
}

function validateError(netmsg){
	var ret = netmsg.readInt();
	return ret;
}



(function(){
	console.log("add default handler");

	addHandler(1, 1, function onLogin(client, netmsg)
	{
		var result = getRetErr(netmsg);
		client.emit('login', result);
	});

	addHandler(0, 12, function onGetRoles(client, netmsg)
	{
		var result = getRetErr(netmsg);
		var rolenum = netmsg.readInt();

		var roles = [];
		for (var i = 0; i < rolenum; i++)
		{
			var role = {};

			role.roletype = netmsg.readInt();
			role.lvl = netmsg.readInt();
			role.rolename = netmsg.readStr();
			role.cratetime = netmsg.readInt();
			role.s = netmsg.readInt();
			role.roleid = netmsg.readInt();
			role.scenetypeid = netmsg.readInt();
			role.x = netmsg.readInt();
			role.y = netmsg.readInt();
			role.proplist = netmsg.readStr();
			roles.push(role);
		}

		result.rolenum = rolenum;
		result.roles = roles;
		client.emit('getRoles', result);
	});

	addHandler(0, 4, function onGetRandName(client, netmsg)
	{
		var result = getRetErr(netmsg);
		result.rolename = netmsg.readStr();
		client.emit('getRandName', result);
	});


	addHandler(0, 6, function onCreateRole(client, netmsg)
	{
		var result = getRetErr(netmsg);
		if (result.err == 0){
			result.roleid = netmsg.readInt();
		}
		client.emit('createRole', result);
	});

	addHandler(0, 8, function onSelectRole(client, netmsg)
	{
		var result = getRetErr(netmsg);
		result.roleid = netmsg.readInt();

		client.emit('selectRole', result);
	});

	addHandler(2, 1, function onEnterRoom(client, netmsg)
	{
		var result = getRetErr(netmsg);
		result.sceneid = netmsg.readInt();
		result.scenetype = netmsg.readInt();
		client.emit('enterRoom', result);
	});

	addHandler(2, 3, function onEnterCity(client, netmsg)
	{
		if (!validateError(netmsg))
		{
			return;
		}

		var cityid = netmsg.readInt();
		console.log('enter city ' + cityid + ' successfull');
	});

	addHandler(2, 4, function onNotifyEnterCity(client, netmsg){

	});

	addHandler(2, 5, function onNotifyLeaveCity(client, netmsg){

	})

	addHandler(5, 3, function onRecvQuest(client, netmsg)
	{
		var questid = netmsg.readInt();
		var err = netmsg.readInt();

		if (err != 0)
		{
			console.log('recv quest failed');
			return;
		}

		var quest = {};
		quest.questid = questid;
		quest.progresses = [];
		quest.progresses.push(0);
		quest.progresses.push(0);
		quest.progresses.push(0);
		quest.progresses.push(0);
		client.curquests[questid] = quest;
	});

	addHandler(5, 4, function onNotifyAddQuest(client, netmsg)
	{
		var num = netmsg.readInt();
		for (var i = 0; i < num; i++)
		{
			var questid = netmsg.readInt();

			var quest = {};
			quest.questid = questid;
			quest.progresses = [];
			quest.progresses.push(0);
			quest.progresses.push(0);
			quest.progresses.push(0);
			quest.progresses.push(0);
			client.curquests[questid] = quest;
		}
	});

	addHandler(8, 35, function onResWorldChat(client, netmsg){
		var err = netmsg.readInt();
	});

	addHandler(8, 19, function onNotifyWorldChat(client, netmsg){
		//var senderId = netmsg.readInt();
		//var senderSex = netmsg.readInt();
		//var senderName = netmsg.readStr();
		//var message = netmsg.readStr();
		//var sceneTemplateId = netmsg.readInt();
		//console.log(senderName + " say " + message);
	});

	addHandler(30, 2, function onEnterWorldBoss(client, netmsg)
	{
		/*
    def_int(err)                            // 1表示无错
    def_int(sceneid)                        // 由服务器通知sceneid
    def_int(bossid)                         // bossid == monsterid
    def_int(bossHP)                         // boss 当前 HP
    def_int(reborn_seconds)                 // 复活秒数
    */
		console.log("enter world boss return");

		var result = getRetErr(netmsg);
    	result.sceneid = netmsg.readInt();
    	result.bossid = netmsg.readInt();
    	result.bosshp = netmsg.readInt();
    	result.reborn = netmsg.readInt();
		client.emit('enterWorldBoss', result);
	});

	addHandler(5, 9, function onFinishQuest(client, netmsg)
	{
		var questid = netmsg.readInt();
		var err = netmsg.readInt();

		if (err != 0)
		{
			console.log('finish quest failed');
			return;
		}

		console.log('finish quest ' + questid);

		client.finishquests[questid] = 1;
		delete client.curquest[questid];
	});


	addHandler(5, 1, function onGetQuestState(client, netmsg)
	{
		if (!validateError(netmsg))
		{
			//return;
		}

		var dailycount = netmsg.readInt();

		var curnum = netmsg.readInt();
		for (var i = 0; i < curnum; i++)
		{
			var quest = {};

			var questid = netmsg.readInt();
			quest.questid = questid;

			quest.progresses = [];
			var progressnum = netmsg.readInt();
			for (var j = 0; j < progressnum; j++)
			{
				var progress = netmsg.readInt();
				quest.progresses.push(progress);
			}

			client.curquests[questid] = quest;
		}

		var finishquests = [];
		var finishnum = netmsg.readInt();
		for (var i = 0; i < finishnum; i++)
		{
			var questid = netmsg.readInt();
			client.finishquests[questid] = 1;
		}

		console.log('cur quests');
		for (var key in client.curquests)
		{
			console.log(key);
		}
		console.log('finish quests');
		for (var key in client.finishquests)
		{
			console.log(key);
		}

		client.setDoLogic();
	});

})();


exports.addHandler = addHandler;
exports.createNetHandler = createNetHandler;