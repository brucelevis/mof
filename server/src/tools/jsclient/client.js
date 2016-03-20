//client.js
var proxy = require('./proxy.js');
var nethandler = require('./nethandler.js');
var requestor = require('./requestor.js');
var os = require('os');
var utils = require('./utils.js');
var questmgr = require('./questmgr.js');
var util = require('util');
var events = require('events');

var kUnconnected = 0;
var kConnected = 1;
var kLoginIng = 2;
var kLogined = 3;
var kSelectingRole = 4;
var kSelectedRole = 5;
var kInitingData = 6;
var kDoLogic = 7;


var Client = function()
{
	events.EventEmitter.call(this);

	var self = this;
	this.idx = null;
	this.state = null;
	this.substate = 0;
	this.proxy = null;
	this.requestor = null;
	this.serverconfig = null;
	this.account = null;
	this.rolename = null;
	this.isnewuser = 0;

	this.curquests = [];
	this.finishquests = [];
	this.triggerrecv = true;

	this.onLogin = function(ret)
	{
		console.log(ret);
		console.log(self.isnewuser);

		if (ret.err == 0)
		{
			self.requestor.getRoles();
		}
		else if (self.isnewuser == 0)
		{
			self.isnewuser = 1;
			self.requestor.register(self.account, '123456');
		}
		else
		{
			console.log('login failed');
		}
	}

	this.onGetRoles = function(ret)
	{
		utils.logDebug('onGetRoles');
		if (ret.roles.length == 0)
		{
			console.log("get rand name");
			self.requestor.getRandName();
		}
		else
		{
			self.requestor.selectRole(ret.roles[0].roleid);
		}
	}

	this.index = 0;

	this.onGetRandName = function(ret)
	{
		console.log('onGetRandName ' + ret.rolename);
		var type = utils.getRand(4, 6);

		//var name = "test_test_" + self.index++;
		self.requestor.createRole(type, ret.rolename);
		//self.requestor.createRole(type, "abctest_testfdsfsa");
	}

	this.onCreateRole = function(ret)
	{
		utils.logDebug('onCreateRole');

		self.requestor.selectRole(ret.roleid);
	}
	
	this.onSelectRole = function(ret)
	{
		utils.logDebug('onSelectRole');
		self.removeLoginListener();

		self.addLogicListener();

		//self.proxy.disconnect();
		setInterval(self.doLogic, 100);


		//init the role
		/*
		self.requestor.getSkills();
		self.requestor.getBackBag();
		self.requestor.getEquip();
		self.requestor.getConstell();
		self.requestor.getQuests();
		*/
	}

	this.onEnterWorldBoss = function(ret){
		console.log(ret);
		if (ret.err == 0){
			self.isInWorldBoss = 2;
			console.log("enter boss scene success");
		}
		else{
			self.isInWorldBoss = -1;
			console.log("enter boss scene fail");
		}

	}

	this.addInitListener = function()
	{
	}

	this.removeInitListener = function()
	{

	}

	this.addLoginListener = function()
	{
		self.on('login', self.onLogin);
		self.on('getRoles', self.onGetRoles);
		self.on('getRandName', self.onGetRandName);
		self.on('createRole', self.onCreateRole);
		self.on('selectRole', self.onSelectRole);
	}

	this.removeLoginListener = function()
	{
		self.removeListener('login', self.onLogin);
		self.removeListener('getRoles', self.onLogin);
		self.removeListener('getRandName', self.onGetRandName);
		self.removeListener('createRole', self.onCreateRole);
		self.removeListener('selectRole', self.onSelectRole);
	}

	this.addLogicListener = function()
	{
		self.on('enterWorldBoss', self.onEnterWorldBoss);
	}


	this.test = function(){
		self.requestor.heartBeat();
	}
	
	this.run = function()
	{
		self.state = kUnconnected;
		self.substate = 0;

		self.proxy.connectServer(self.serverconfig, function()
		{
			console.log("client " + self.idx + " connect ")
			self.addLoginListener();

			if (process.argv.length > 2){
				self.account = self.idx + 'test__' + process.argv[2];
			}
			else{
				self.account = self.idx + "test_gamepdc";
			}

			self.state = kLoginIng;
			self.requestor.login(self.account, '123456');

			console.log("login with " + self.account);



		});
	}

	this.doQuest = function()
	{
		if (self.triggerrecv)
		{
			var quests = questmgr.getAllQuest();
			for (var questid in quests)
			{
				if (self.finishquests[questid] == 1)
				{
					continue;
				}

				var quest = quests[questid];
				if (self.finishquests[quest.prevquest] != 1)
				{
					continue;
				}

				self.requestor.recvQuest(questid);
			}

			self.triggerrecv = false;
			return;
		}

		for (var questid in self.curquests)
		{
			var questprogresses = self.curquests[questid].progresses;
			var quest = questmgr.getQuest(questid);

			var isfinish = true;
			for (var i = 0; i < quest.conditions.length; i++)
			{
				var cond = quest.conditions[i];
				if (cond.type == 'dialog')
				{
					if (cond.num != questprogresses[i])
					{
						isfinish = false;
						self.requestor.dialog(cond.target, cond.num);
						questprogresses[i] = cond.num;
					}
				}
				else
				{
					if (cond.num > questprogresses[i])
					{
						isfinish = false;
						//do something to finish the quest
						if (cond.type == 'finishcopy')
						{
							self.requestor.finishCopy(cond.target);
							questprogresses[i]++;
						}
						else if (cond.type == 'upequip')
						{

						}
						else if (cond.type == 'studyskill')
						{

						}
					}
				}
			}

			if (isfinish)
			{
				self.requestor.finishQuest(questid);
				self.triggerrecv = true;
			}
		}

	}

	this.count = 0;
	this.isInWorldBoss = 0;
	this.doLogic= function()
	{
		/*if (self.isInWorldBoss == 0){
			self.isInWorldBoss = 1;
			self.requestor.enterWorldBoss();
		}
		else if (self.isInWorldBoss == 1){
			console.log("wait enter world boss return");
		}
		else if (self.isInWorldBoss == 2){
			console.log("attack");
			self.requestor.attackWorldBoss();
		}
		else if (self.isInWorldBoss == -1){
			console.log("enter world boss fail");
			self.isInWorldBoss = -2;
		}*/

		//self.requestor.registerOrder();

		//self.requestor.chatWorld("very good fsdaf" + self.count++);


		var randnum;

		randnum = utils.getRand(1, 100000);

		if (randnum % 100 == 0)
		{
			var cityid = 501101 + utils.getRand(0, 3) * 1000;
			self.requestor.enterCity(cityid);
		}

		if (randnum % 10000 == 0)
		{
			//self.requestor.finishCopy(501211);
			self.requestor.chatWorld('我是机器人' + self.idx + '号!');
		}

		//if (randnum % 7 == 0){
		//}
	}

	this.onDisconnect = function()
	{
		console.log("client " + self.idx + " discconect ");
		self.state = kUnconnected;
	}

	this.logClientId = function()
	{
		console.log('client ' + self.idx);
	}

};

util.inherits(Client, events.EventEmitter);

exports.createClient = function(serverconfig, idx)
{
	var c = new Client;
	c.idx = idx;
	c.serverconfig = serverconfig;


	var clienthandler = nethandler.createNetHandler();
	var clientproxy = proxy.createProxy(clienthandler, c);
	var clientrequestor = requestor.createRequestor(clientproxy);

	c.proxy = clientproxy;
	c.requestor = clientrequestor;


	return c;
}