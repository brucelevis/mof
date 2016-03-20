var redisHelper = require('./redishelper.js');

var srcCfg = {
	ip : '192.168.160.37',
	port : 11481,
	index : 1,
	pwd : '123456'
}

var destCfg = {
	ip : '192.168.160.37',
	port : 11481,
	index : 1,
	pwd : '1234567'
}



//binary buffer for dump and restore
//plain text buffer for data process
var srcCtx = redisHelper.connect(srcCfg);
var bsrcCtx = redisHelper.connect(srcCfg, true);
var destCtx = redisHelper.connect(destCfg);
var bdestCtx = redisHelper.connect(destCfg, true);


var restoreKeys = ["role", "illustrations", "honor",
					"BackBag", "questFinish", "constellpro", 
					"PlayerEquip", "finishelitecopy", "totem",
					"wardrobedata", "mailcustom", "honorcond"];

function restoreRoleData(srcCtx, bsrcCtx, destCtx, bdestCtx, roleId){

	for (var i = 0; i < restoreKeys.length; i++) {
		var prevKey = restoreKeys[i] + ":" + roleId;
		backupAndRestoreKey(srcCtx, bsrcCtx, destCtx, bdestCtx, prevKey);
	}

/*
	var roleKey = "role:" + roleId;
	var bagKey = "BackBag:" + roleId;
	var equipKey = "PlayerEquip:" + roleId;

	//exe for the first time
	backupAndRestoreKey(srcCtx, bsrcCtx, destCtx, bdestCtx, roleKey);
	backupAndRestoreKey(srcCtx, bsrcCtx, destCtx, bdestCtx, bagKey);
	backupAndRestoreKey(srcCtx, bsrcCtx, destCtx, bdestCtx, equipKey);
	*/

}


var roleIds = [
	1550853935
];

for (i = 0; i < roleIds.length; i++) {
	var roleId = roleIds[i];
	//showRoleInfo(srcCtx, roleId, showKeyInfo, "src");
	//showRoleInfo(destCtx, roleId, showKeyInfo, "dest");
	//first step
	restoreRoleData(srcCtx, bsrcCtx, destCtx, bdestCtx, roleId);
	//second step
	//restoreRolePet(srcCtx, bsrcCtx, destCtx, bdestCtx, roleId);
}


function showKeyInfo(param, key, type, info){
	console.log(param + "'s " + key + " is hash");
	if (type == "hash"){
		showHash(info);
	}
	console.log();
}

function showHash(info){
	for(var k in info){
		console.log(k + ": " + info[k]);
	}
}

function showRoleInfo(dbCtx, roleId, type, info) {
	var keyInfo = "role:" + roleId;
	opKeyInfo(dbCtx, keyInfo, showKeyInfo, info);
}

function opKeyInfo(dbCtx, key, callback, param){
	dbCtx.send_command("type", [key], function(err, reply){
		if (err){
			console.log("execute exists " + key + " fail");
			return;
		}

		if (reply == "string"){
			dbCtx.send_command("get", [key], function(err, info){
				callback(param, key, reply, info);
				return;
			})
		}
		else if (reply == "hash"){

			dbCtx.send_command("hgetall", [key], function(err, info){
				callback(param, key, reply, info);
				return;
			})
		}

	})
}

function backupAndRestoreKey(srcCtx, bsrcCtx, destCtx, bdestCtx, key){
	var backupKey = "backup:" + key;
	destCtx.send_command("rename", [key, backupKey], function(err, reply){
		if (err){
			console.log("rename failed with err: " + key + " " + err);
			//return;
		}

		redisHelper.copyTo(bsrcCtx, bdestCtx, key, true);
	})
}

function restoreRolePet(srcCtx, bsrcCtx, destCtx, bdestCtx, roleId){
	var self = this;
	//删除现有的宠物
	(function(){
		var roleKey = "role:" + roleId;
		destCtx.hget(roleKey, "RolePropPetList", function(err, prevPetListStr){
			if (err){
				console.log("error occurs when get " + roleKey + "'s pet list");
				return;
			}

			if (prevPetListStr == null){
				prevPetListStr = "";
			}

			prevPetList = prevPetListStr.split(",");
			prevPetKeyList = [];
			for (var i = 0; i < prevPetList.length; i++){
				if (!prevPetList[i]){
					continue;
				}

				prevPetKeyList.push("pet:" + prevPetList[i]);
			}

			if (prevPetKeyList.length == 0){
				self.restoreSrc(roleKey);
			} else {
				var count = 0;
				for (var i = 0; i < prevPetKeyList.length; i++){
					var petKey = prevPetKeyList[i];
					var backupPetKey = "backup:" + petKey;
					destCtx.send_command("rename", [petKey, backupPetKey], function(err, reply) {
						if (err){
							console.log(err);
						}

						console.log("rename " + petKey + " to " + backupPetKey);
						count++;
						if (count == prevPetKeyList.length) {
							self.restoreSrc(roleKey);
						}
					});
				}
			}
		});
	})();

	this.restoreSrc = function(roleKey){
		srcCtx.hget(roleKey, "RolePropPetList", function(err, petListStr){
			if (err){
				console.log("error occurs when get " + roleKey + "'s pet list");
				return;
			}

			if (!petListStr){
				console.log("empty pet list");
				return;
			}


			destCtx.send_command("hset", [roleKey, "RolePropPetList", petListStr], function(err, reply){
				if (err){
					console.log("set ")
					return;
				}

				console.log("set role pet list " + petListStr);

				petList = petListStr.split(",");
				for (var i = 0; i < petList.length; i++){
					if (!petList[i]){
						continue;
					}

					var petKey = "pet:" + petList[i];
					console.log(petKey);
					redisHelper.copyTo(bsrcCtx, bdestCtx, petKey, true);
				}

			});

		})
	}
}