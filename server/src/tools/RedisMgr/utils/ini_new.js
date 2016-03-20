var fs = require('fs');


var Ini = function(filename)
{
	var self = this;
	this.data = null;

	this.load = function(filename) 
	{
		var filecontent = require('fs').readFileSync(filename, "utf8");
		var line;
		var section = "";
		var endPos = 0;
		do{
			var endPos = filecontent.indexOf('\r\r');
			line = filecontent.substr(0, endPos);

			filecontent = filecontent.substr(endPos + 1);
			console.log(line);

		}
		while (endPos >= 0);
	}

	this.getSections = function()
	{
		var sections = [];
		for (var a in self.data)
		{
			sections.push(a);
		}

		return sections;
	}

	this.getKeys = function(section)
	{
		var keys = [];

		var keyval = self.data[section];
		if (typeof keyval == 'object')
		{
			for (var key in keyval)
			{
				keys.push(key);
			}
		}

		return keys;
	}

	this.getValue = function(section, key)
	{
		var value = undefined;

		var keyval = self.data[section];
		if (typeof keyval == 'object')
		{
			value = keyval[key];
		}

		if (value == undefined)
		{
			if (arguments.length > 2)
			{
				value = arguments[2];
			}
			else
			{
				value = "";
			}
		}

		return value;
	}
}

exports.load = function(filename)
{
	var ini = new Ini;
	ini.load(filename);
	return ini;
}