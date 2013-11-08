var fs = require("fs");

generateDocs("Dota Module");

function generateDocs(filename){
	var str = fs.readFileSync(filename + ".json", {"encoding": "utf8"});
	var obj = JSON.parse(str);
	
	if(!obj["Introduction"]) throw new Error(filename + " -- Docs must have an introduction");
	
	generateHTMLPage(filename, generateWikiDocs(filename, obj));
}

function generateWikiDocs(filename, obj){
	
	var str = "";
	
	str += "## Introduction\n";
	str += "{#pre-intro}\n\n";
	
	str += obj["Introduction"] + "\n\n";
	
	var methodList = [];
	var constantsList = [];
	
	str += "{#post-intro}\n\n";
	
	if(obj["Methods"]){
		str += "## Methods\n\n";
		
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
			
			str += "### " + methodName;
			str += " {#func-" + wikifyLink2(methodName) + "}\n";
			str += "    " + method["Signature"] + "\n\n";
			str += method["Description"] + "\n\n";
			
			if(method["Example"]){
				str += "Example:\n\n    " + method["Example"].split('\n').join('\n    ') + "\n\n";
			}
		}
	}
	
	if(obj["Constants"]){
		str += "## Constants\n\n";
		
		for(var cteName in obj["Constants"]){
			if(!obj["Constants"].hasOwnProperty(cteName)) continue;
			constantsList.push(cteName);
		}
		
		constantsList.sort();
		
		for (var i = 0; i < constantsList.length; i++) {
			var cteName = constantsList[i];
			var ctes = obj["Constants"][cteName];
			
			str += "### " + cteName;
			str += " {#ctes-" + wikifyLink2(cteName) + "}\n";
			for (var j = 0; j < ctes.length; j++) {
				var cte = ctes[j];
				str += "    " + cte + "\n";
			}
			
			str += "\n\n";
		}
	}
	
	var simpleMD = str.replace(/\{#([a-z0-9_-]+)\}/ig, "");
	
	fs.writeFileSync("out/md/" + filename + ".md", simpleMD, {"encoding": "utf8"});
	return {
		str: str,
		methodList: methodList,
		constantsList: constantsList
	};
}

function wikifyLink(link){
	return link.replace(/\s+/gi, "_").replace(/[^a-zA-Z0-9_]/gi, "").toLowerCase();
}

function wikifyLink2(link){
	return link.replace(/\s+/gi, "-").replace(/[^a-zA-Z0-9_]/gi, "").toLowerCase();
}

function wikifyLinks(str){
	return str.replace(/\[\[([a-zA-Z0-9_ ]+)\]\]/gi, function(all, link){
		var target = "docs_" + wikifyLink(link);
		return '<a href="' + target + '">'+link+'</a>';
	});
}

function generateHTMLPage(filename, obj){
	
	var content = require("markdown").markdown.toHTML(obj.str);
	content = "<h1>Developers &mdash; " + filename + "</h1>\n" + content;
	
	content = wikifyLinks(content);
	
	content = content.replace(/<h3>([^\{]+)\s*\{#([a-z0-9_-]+)\}\s*<\/h3>/ig, function(all, title, link){
		return '<hr /><h3 id="' + link + '">' + title.trim() + '</h3>';
	});
	
	content = content.replace("<p>{#post-intro}</p>", '<br clear="both" />')
	
	content = content.replace(/\{#([a-z0-9_-]+)\}/ig, function(all, link){
		if(link == "pre-intro"){
			var nav = '<div class="well pull-right"><ul class="nav nav-list">';
			nav += '<li class="nav-header">Table of contents</li>';
			
			
			if(obj.methodList.length > 0){
				nav += '<li class="nav-header">Methods</li>';
				for (var i = 0; i < obj.methodList.length; i++) {
					nav += '<li><a onclick="window.location.hash=\'func-' + wikifyLink2(obj.methodList[i]) + '\'" href="#">' + obj.methodList[i] + '</a></li>';
					
				}
			}
			
			if(obj.constantsList.length > 0){
				nav += '<li class="nav-header">Constants</li>';
				for (var i = 0; i < obj.constantsList.length; i++) {
					nav += '<li><a onclick="window.location.hash=\'ctes-' + wikifyLink2(obj.constantsList[i]) + '\'" href="#">' + obj.constantsList[i] + '</a></li>';
					
				}
			}
			
			
			/*
			  <li class="nav-header">List header</li>
			  <li class="active"><a href="#">Home</a></li>
			  <li><a href="#">Library</a></li>
			  ...*/
			nav += '</ul></div>';
			return nav;
		}
		
		return '<a id="' + link + '"></a>';
	});
	content = content.replace(/<h2>/gi, "<hr /><h2>");
	content = content.replace(/<\/h2>\s*<hr \/>/gi, "</h2>");
	
	var html = '<div class="d2w-container">\n<div id="content">\n' + content + '\n<br clear="both"/>\n</div>\n</div>';
	
	fs.writeFileSync("out/html/docs_" + wikifyLink(filename) + ".html", html, {"encoding": "utf8"});
}