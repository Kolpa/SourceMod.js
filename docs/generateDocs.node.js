var fs = require("fs");

generateDocs("Dota Module");

function generateDocs(filename){
	var str = fs.readFileSync(filename + ".json", {"encoding": "utf8"});
	var obj = JSON.parse(str);
	
	if(!obj["Introduction"]) throw new Error(filename + " -- Docs must have an introduction");
	
	generateWikiDocs(filename, obj);
}

function generateWikiDocs(filename, obj){
	
	var str = "";
	
	str += "## Introduction\n";
	str += obj["Introduction"] + "\n\n";
	
	if(obj["Methods"]){
		str += "## Methods\n\n";
		
		var methodList = [];
		
		for(var methodName in obj["Methods"]){
			if(!obj["Methods"].hasOwnProperty(methodName)) continue;
			methodList.push(methodName);
		}
		
		methodList.sort();
		
		for (var i = 0; i < methodList.length; i++) {
			var methodName = methodList[i];
			var method = obj["Methods"][methodName];
			
			if(!method["Signature"]) throw new Error(filename + " -- Method " + methodName + " must have a signature");
			if(!method["Description"]) throw new Error(filename + " -- Method " + methodName + " must have a description");
			
			str += "### " + methodName + "\n";
			str += "    " + method["Signature"] + "\n\n";
			str += method["Description"] + "\n\n";
			
			if(method["Example"]){
				str += "Example:\n\n    " + method["Example"].split('\n').join('\n    ') + "\n\n";
			}
		}
	}
	
	fs.writeFileSync("out/" + filename + ".md", str, {"encoding": "utf8"})
}