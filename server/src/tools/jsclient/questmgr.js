//datacfg.js
var ini = require('./ini.js');
var utils = require('./utils.js');

var configdir = './config/'
var cfgs = [];

var QuestCondition = function()
{
	this.type = '';
	this.target = 0;
	this.num = 0;
}

var QuestDef = function()
{
	this.id = 0;
	this.minlvl = 50;
	this.maxlvl = -1;
	this.type = -1;
	this.prevquest = -1;
	this.conditions = [];
}

var quests = null;

exports.getAllQuest = function()
{
	return quests;
}

exports.getQuest = function(questid)
{
	var quest = quests[questid];
	if (typeof quest != 'object')
	{
		quest = null;
	}

	return quest;
}

exports.load = function()
{
	var path = configdir + 'quests.ini';
	var inicfg = ini.loadIni(path);
	var content = inicfg.data;

	quests = [];
	for (var section in content)
	{
		var questid = utils.atoi(section);
		var keyvals = content[section];

		var quest = new QuestDef();
		{
			quest.id = questid;

			quest.minlvl = utils.atoi(utils.getHashVal(keyvals, 'minLvl'));
			quest.maxlvl = utils.atoi(utils.getHashVal(keyvals, 'maxLvl'));
			quest.type = utils.atoi(utils.getHashVal(keyvals, 'type'));
			quest.prevquest = utils.atoi(utils.getHashVal(keyvals, 'preQuestId'));

			var conditions = utils.getHashVal(keyvals, 'conditions');
			var arr = conditions.split(';');
			for (var i = 0; i < arr.length; i++)
			{
				var conddesc = arr[i].split(' ');
				if (conddesc.length < 2 || conddesc[0] == '')
				{
					quest = null;
					break;
				}

				var tardesc = conddesc[1].split('*');
				if (tardesc.length != 2)
				{
					quest = null;
					break;
				}

				var target = utils.atoi(tardesc[0]);
				var num = utils.atoi(tardesc[1]);
				if (isNaN(target) || isNaN(target))
				{
					quest = null;
					break;
				}

				var questcond = new QuestCondition;
				questcond.type = conddesc[0];
				questcond.target = target;
				questcond.num = num;
				quest.conditions.push(questcond);
			}
		}
		if (quest == null)
		{
			console.log('load quest ' + questid + ' failed');
		}
		else
		{
			quests[questid] = quest;
		}
	}
}