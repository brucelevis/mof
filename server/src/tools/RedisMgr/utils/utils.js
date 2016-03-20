//utils.js

function getRand(min, max)
{
    var r = Math.random();
    var m = r * 0xFFFFFFFF;

    var i = m.toFixed(0);

    var diff = max - min + 1;
    return i % diff + min;
}

function getHashVal(hash, key)
{
	var val = hash[key];
	if (val == 'undefined')
	{
		if (arguments.length > 2)
		{
			val = arguments[2];
		}
		else
		{
			value = '';
		}
	}

	return val;
}

function atoi(str)
{
	var ret = parseInt(str);
	if (isNaN(ret))
	{
		if (arguments.length > 1)
		{
			ret = arguments[1];
		}
		else
		{
			ret = 0;
		}
	}
	return ret;
}

var debug = 1;
function logDebug(info)
{
	if (debug)
	{
		console.log(info);
	}
}


exports.getRand = getRand;
exports.getHashVal = getHashVal;
exports.atoi = atoi;
exports.logDebug = logDebug;