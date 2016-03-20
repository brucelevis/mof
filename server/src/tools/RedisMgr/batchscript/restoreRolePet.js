var redisHelper = require('./redishelper.js');

var srcCfg = {
	ip : '',
	port : 6379,
	index : 0,
	pwd : ''
}

var destCfg = {
	ip : '192.168.160.37',
	//ip : '192.168.160.101',
	port : 6379,
	index : 13,
	//pwd : '',
	pwd : ''
}

//binary buffer for dump and restore
//plain text buffer for data process
var srcCtx = redisHelper.connect(srcCfg);
var bsrcCtx = redisHelper.connect(srcCfg, true);
var destCtx = redisHelper.connect(destCfg);
var bdestCtx = redisHelper.connect(destCfg, true);

//restoreRoleBackBag(srcCtx, bsrcCtx, destCtx, bdestCtx, 282237);
//restoreRolePlayerEquip(srcCtx, bsrcCtx, destCtx, bdestCtx, 282237);

function backupKey(srcCtx, bsrcCtx, destCtx, bdestCtx, key){
	var backupKey = "backup:" + key;
	destCtx.send_command("rename", [key, backupKey], function(err, reply){
		if (err){
			console.log("rename failed with err:" + err);
		}

		redisHelper.copyTo(bsrcCtx, bdestCtx, key, true);
	})
}

function restoreRolePlayerEquip(srcCtx, bsrcCtx, destCtx, bdestCtx, roleId){
	var playerEquipKey = "PlayerEquip:" + roleId;
	backupKey(srcCtx, bsrcCtx, destCtx, bdestCtx, playerEquipKey);
}

function restoreRoleBackBag(srcCtx, bsrcCtx, destCtx, bdestCtx, roleId){
	var backBagKey = "BackBag:" + roleId;
	backupKey(srcCtx, bsrcCtx, destCtx, bdestCtx, backBagKey);
}

function restoreRolePet(srcCtx, bsrcCtx, destCtx, bdestCtx, roleId){
	var roleKey = "role:" + roleId;
	var self = this;

	//删除现有的宠物
	(function(){
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
				self.restoreSrc();
			}
			else{
				destCtx.send_command("del", prevPetKeyList, function(err, reply){
					if (err){
						return;
					}

					self.restoreSrc();
				});
			}
		});
	})();

	this.restoreSrc = function(){
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