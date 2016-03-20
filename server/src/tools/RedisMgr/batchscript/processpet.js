var redisHelper = require('./redishelper.js');

var srcCfg = {
	ip : '192.168.160.101',
	port : 6379,
	index : 0,
	pwd : ''
}

var destCfg = {
	ip : '192.168.160.101',
	port : 6379,
	index : 10,
	pwd : ''
}

//binary buffer for dump and restore
//plain text buffer for data process
var srcCtx = redisHelper.connect(srcCfg);
var bsrcCtx = redisHelper.connect(srcCfg, true);
var destCtx = redisHelper.connect(destCfg);
var bdestCtx = redisHelper.connect(destCfg, true);


var start = 630000;
var end =   650000;
doWork();

function doWork(){
	for (var i = start; i < end; i++){
		backupPet(srcCtx, bsrcCtx, destCtx, bdestCtx, i);
	}
}

function backupPet(srcCtx, bsrcCtx, destCtx, bdestCtx, petId){
	var petKey = "pet:" + petId;

	srcCtx.hget(petKey, "owner", function(err, roleId){
		if (err){
			console.log("error occurs on " + petKey);
			return;
		}

		if (!roleId){
			console.log(petKey + " does not exist");
			return;
		}

		var roleKey = "role:" + roleId;
		srcCtx.hget(roleKey, "RolePropPetList", function(err, petListStr){
			if (err){
				console.log("error occurs when get " + roleKey + "'s pet list");
				return;
			}

			var isFind = false;
			if (petListStr){
				petList = petListStr.split(",");
				for (var i = 0; i < petList.length; i++){
					if (petList[i] == petId){
						isFind = true;
						break;
					}
				}
			}

			if (isFind){
				console.log(petKey + " is ok");
				return;
			}

			redisHelper.moveTo(bsrcCtx, bdestCtx, petKey, true);
		});
	})
}