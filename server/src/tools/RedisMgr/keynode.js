//keynode.js
var section = 0;
var single = 1;
var Node = function(){
	this.type = null;
	this.str = null;
	this.children = [];

}


function parse(root, keys, depth){
	if (keys.length == 0){
		return;
	}

	var i = 0;
	while (i < keys.length){

		var segs = keys[i].split(':');
		if (segs.length <= depth + 1){
			var node = new Node;
			node.type = single;
			node.str = keys[i];
			node.children = null;
			root.children.push(node);
			i++;
			continue;
		}

		var curseg = segs[depth];
		var subkeys = [];
		subkeys.push(keys[i]);
		var j = i + 1;
		while (j < keys.length){
			var segs = keys[j].split(':');
			if (segs.length <= depth + 1 || segs[depth] != curseg){
				break;
			}

			subkeys.push(keys[j]);
			j++;
		}

		i = j;

		console.log(curseg);
		var node = new Node;
		node.type = section;
		node.str = curseg;
		root.children.push(node);
		parse(node, subkeys, depth + 1);
	}
}
