//querykey
var redisctx = require('./redisctx.js')

function formatHashHtml(key, keyvals){
	var html = [];

	html.push("<div class='" + key + "'>");
	html.push("键: <input id='hkey' type='text' width='50px' />");
	html.push("值: <input id='val' type='text' width='50px' />");
	html.push("<input type='button' value='设置' onclick='updateHash(this)'/>");
	html.push("</div>");

	html.push("<div><table>");
	html.push("<tr><td>key</td><td>" + key + "</td></tr>");
	html.push("<tr><td>type</td><td>hash</td></tr>");
	html.push("<tr><td>hkey</td><td>");

	html.push("<table>")
	for (hkey in keyvals){
		html.push("<tr><td>" + hkey + "</td><td>")
		var val = keyvals[hkey];
		html.push(val + "</td></tr>")
	}
	html.push("</table>")

	html.push("</td></tr>");
	html.push("</table></div>");
	return html.join("");
}

function formatStringHtml(key, val){
	var html = [];

	html.push("<div class='" + key + "'>");
	html.push("值: <input id='val' type='text' width='50px' />");
	html.push("<input type='button' value='设置' onclick='updateString(this)'/>");
	html.push("</div>");

	html.push("<div><table>");
	html.push("<tr><td>key</td><td>" + key + "</td></tr>");
	html.push("<tr><td>type</td><td>string</td></tr>");
	html.push("<tr><td>value</td><td>" + val + "</td></tr>");
	html.push("</table></div>");
	return html.join("");
}

function formatSetHtml(key, members){
	var html = [];

	html.push("<div><table>");
	html.push("<tr><td>key</td><td>" + key + "</td></tr>");
	html.push("<tr><td>type</td><td>set</td></tr>");
	html.push("<tr><td>members</td><td>");

	html.push("<table>")
	for (var i = 0; i < members.length; i++){
		html.push("<tr><td>" + members[i] + "</td></tr>")
	}
	html.push("</table>")

	html.push("</td></tr>");
	html.push("</table></div>");
	return html.join("");
}

exports.handle = function(query, response){
	var ip = query.ip ? query.ip : process.defrediscfg.ip;
	var port = query.port ? query.port : process.defrediscfg.port;
	var ctx = redisctx.getRedisCtx(port, ip);

	console.log(query);

	var key = query.key;
	ctx.getKeyDetail(key, function(detail){
		var html = "";
		if (detail.type == 'hash'){
			html = formatHashHtml(detail.key, detail.keyvals);
		}
		else if (detail.type == 'string'){
			html = formatStringHtml(detail.key, detail.val);
		}
		else if (detail.type == 'set'){
			html = formatSetHtml(detail.key, detail.members);
		}
		response.write(html);
		response.end();
	});
}