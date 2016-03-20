var redisHelper = require('./redishelper.js');
var ini = require('../utils/ini.js');
var utils = require('../utils/utils.js');

var configdir = '../config/'
var cfgs = [];

var dbCfg = {
	ip : '192.168.160.37',
	port : 6379,
	index : 15,
	pwd : ''
}

loadConfig();


function robotCfg() {
	this.roles = [];
	this.pets = [];
}

function loadConfig(){
	var robots = new robotCfg;

	var roleCfgFile = configdir + 'robot_role.ini';
	var roleIni = ini.loadIni(roleCfgFile);

	var roles = [];
	var roleData = roleIni.data;
	for (var section in roleData)
	{
		var role = {}
		var roleProps = roleData[section];

		role.isRobot = 1;
		role.roleId = utils.getHashVal(roleProps, 'roleid');
		role.roleName = utils.getHashVal(roleProps, 'rolename');
		role.lvl = utils.getHashVal(roleProps, 'lvl');
		role.roleType = utils.getHashVal(roleProps, 'roletype');
		role.petList = utils.getHashVal(roleProps, 'petlist');  //宠物列表
		role.petPvpForm = utils.getHashVal(roleProps, 'petpvpform');  //宠物竞技场阵型
		role.cups = utils.getHashVal(roleProps, 'cups');   //杯数

		roles.push(role);
	}

	robots.roles = roles;

	var petCfgFile = configdir + 'robot_pet.ini';
	var petIni = ini.loadIni(petCfgFile);
	var pets = [];
	var petData = petIni.data;
	for (var section in petData){
		var pet = {};
		var petProps = petData[section];

		pet.petId = utils.getHashVal(petProps, 'petid');
		pet.petMod = utils.getHashVal(petProps, 'petmod');
		pet.lvl = utils.getHashVal(petProps, 'lvl');
		pet.growth = utils.getHashVal(petProps, 'growth');

		pet.batk = utils.getHashVal(petProps, 'batk');
		pet.bdef = utils.getHashVal(petProps, 'bdef');
		pet.bhp = utils.getHashVal(petProps, 'bhp');
		pet.bdodge = utils.getHashVal(petProps, 'bdodge');
		pet.bhit = utils.getHashVal(petProps, 'bhit');

		pet.skills = utils.getHashVal(petProps, 'skills');
		pet.owner = utils.getHashVal(petProps, 'owner');

		pet.phys = utils.getHashVal(petProps, 'phys');
		pet.capa = utils.getHashVal(petProps, 'capa');
		pet.inte = utils.getHashVal(petProps, 'inte');
		pet.stre = utils.getHashVal(petProps, 'stre');

		pets.push(pet);
	}

	robots.pets = pets;

	return robots;
}

var ctx = redisHelper.connect(dbCfg);
doWork();

function doWork(){
	var ctx = redisHelper.connect(dbCfg);

	var robots = loadConfig();

	for (var i = 0; i < robots.roles.length; i++){
		console.log(robots.roles[i]);
		tryAddOneRobotRole(ctx, robots.roles[i]);
	}

	for (var i = 0; i < robots.pets.length; i++){
		console.log(robots.pets[i]);
		tryAddOneRobotPet(ctx, robots.pets[i]);
	}
}


function tryAddOneRobotPet(dbCtx, pet){
	var petKey = "pet:" + pet.petId;
	dbCtx.send_command("exists", [petKey], function(err, data){
		if (err){
			console.log("try add robot pet fail " + petKey);
			return;
		}

		if (data == "1"){
			console.log("already exists " + petKey);
			return;
		}

		addOneRobotPet(dbCtx, pet);
	})
}


function tryAddOneRobotRole(dbCtx, role){
	var roleKey = "role:" + role.roleId;
	dbCtx.send_command("exists", [roleKey], function(err, data){
		if (err){
			console.log("try add robot role fail " + roleKey);
			return;
		}

		if (data == "1"){
			console.log("already exists " + roleKey);
			return;
		}

		var roleNameKey = "rolename:" + role.roleName;
		dbCtx.send_command("exists", [roleNameKey], function(err, data){
			if (err){
				console.log("try add robot role fail " + roleNameKey);
				return;
			}

			if (data == "1"){
				console.log("already exists " + roleNameKey);
				return;
			}

			addOneRobotRole(dbCtx, role);
		})
	})
}


function addOneRobotPet(dbCtx, pet){
	if (!pet.petId || !pet.petMod){
		return;
	}

	var petKey = "pet:" + pet.petId;

	var petProps = [];

	petProps.push(petKey);


	petProps.push('petid');
	petProps.push(pet.petId);
	petProps.push('petmod');
	petProps.push(pet.petMod);
	petProps.push('lvl');
	petProps.push(pet.lvl || '1');
	petProps.push('growth');
	petProps.push(pet.growth || '40');

	petProps.push('batk');
	petProps.push(pet.batk || '1');
	petProps.push('bdef');
	petProps.push(pet.bdef || '1');
	petProps.push('bhp');
	petProps.push(pet.bhp || '1');
	petProps.push('bdodge');
	petProps.push(pet.bdodge || '1');
	petProps.push('bhit');
	petProps.push(pet.bhit || '1');

	petProps.push('skills');
	petProps.push(pet.skills || '');
	petProps.push('owner');
	petProps.push(pet.owner || '1');

	petProps.push('phys');
	petProps.push(pet.phys || '1');
	petProps.push('capa');
	petProps.push(pet.capa || '1');
	petProps.push('inte');
	petProps.push(pet.inte || '1');
	petProps.push('stre');
	petProps.push(pet.stre || '1');

	ctx.send_command("hmset", petProps, function(err, reply){
		if (err){
			console.log("set pet prop fail");
			return;
		}
	});
}

function addOneRobotRole(dbCtx, role){
	var roleKey = "role:" + role.roleId;

	var roleProps = [];

	roleProps.push(roleKey);

	roleProps.push("lvl");
	roleProps.push(role.lvl);
	roleProps.push("roletype");
	roleProps.push(role.roleType);
	roleProps.push("rolename");
	roleProps.push(role.roleName);
	roleProps.push("RolePropPetList");
	roleProps.push(role.petList);
	roleProps.push("petpvpform");
	roleProps.push(role.petPvpForm);
	roleProps.push("petpvpcups");
	roleProps.push(role.cups);
	ctx.send_command("hmset", roleProps, function(err, reply){
		if (err){
			console.log("set role prop fail");
			return;
		}
	});

	var petPvpKey = "paihang:petpvp_bronze";
	var cups = utils.atoi(role.cups);
	var roleId = utils.atoi(role.roleId);
	ctx.send_command("zadd", [petPvpKey, cups, roleId], function(err, reply){
		if (err){
			console.log("add pai hang jjc fail " + err);
			return;
		}
	});
}
