var ini = require('../utils/ini.js');
var utils = require('../utils/utils.js');
var redisHelper = require('./redishelper.js');

var configPath = '../config/server_1004.ini';
var dbCfg = {
	ip : '',
	port : 6384,
	index : 1,
	pwd : ''
}

/*var dbCfg = {
	ip : '121.9.235.39',
	port : 6379,
	index : 1,
	pwd : 'mof@!#xxoo__123*&'
}*/
//var compCfg = ini.loadIni('../config/server_1001.ini');

var compCfg = ini.loadIni(configPath)
var compData = compCfg.data;
var dbCtx = redisHelper.connect(dbCfg);


function display(roleId, item, num){
	var roleKey = "role:" + roleId;
	var items = "item " + item + "*" + num + ";"
	console.log("add " + items + " to offlineitem of " + roleKey);
}

function opData(roleId, item, num){
	var roleKey = "role:" + roleId;
	var items = "item " + item + "*" + num + ";"

	dbCtx.send_command("exists", [roleKey], function(err, data){
		if (err){
			console.log("get " + roleKey + " fail");
			return;
		}

		if (data == "0"){
			console.log(roleKey + " not exists");
			return;
		}
		
		dbCtx.send_command("hget", [roleKey, "offlineitem"], function(err, data){
			if (err){
				console.log("get offline item of " + roleKey + " fail");
				return;
			}

			if (!data){
				data = "";
			}

			var offlineitem = data;
			offlineitem += items;
			console.log("set " + roleKey + " offlineitem as " + offlineitem );

			/*
			dbCtx.send_command("hset", [roleKey, "offlineitem", offlineitem], function(err, data){
				if (err){
					console.log("set " + roleKey + " offlineitem as " + offlineitem + " fail");
					return;
				}

				console.log("set " + roleKey + " offlineitem as " + offlineitem + " success");
			});*/
		});

	})
}


function traverse(compData, callback){
	for (var roleId in compData){
		var roleCompData = compData[roleId];
		var item = utils.getHashVal(roleCompData, 'item');
		var num = utils.getHashVal(roleCompData, 'num');
		callback(roleId, item, num);
	}
}

traverse(compData, display);
traverse(compData, opData);