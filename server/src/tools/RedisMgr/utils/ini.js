//ini.js

var Ini = function(filename)
{
	var self = this;
	this.data = null;

	this.load = function(filename) 
	{
		var q = require('querystring');
		var filecontent = require('fs').readFileSync(filename, "utf8");
		var sections = q.parse(filecontent, '[', ']');
		var t;

		self.data = [];
		for (var i in sections)
		{
			if (i == '' || sections[i] == '')
			{
				continue;
			}

			self.data[i] = [];

			t = q.parse(sections[i], '\r', '=');
			for (var j in t)
			{
				self.data[i][j] = t[j] || '';
			}
		}
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

exports.loadIni = function(filename)
{
	var ini = new Ini;
	ini.load(filename);
	return ini;
}