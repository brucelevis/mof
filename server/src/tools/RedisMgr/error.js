//error.js
var html = ['<html>', 
	'<head></head>', 
	'<body>', 
		'<h1>The page you request dosn\'n exist</h1>', 
	'</body>', 
	'</html>'].join('');

exports.handle = function(query, response){
	response.write(html);
	response.end();
}