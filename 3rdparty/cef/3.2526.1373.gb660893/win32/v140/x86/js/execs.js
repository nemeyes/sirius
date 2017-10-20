function changeAll(){
	for (var i=0; i<window.frames.length; i++){
		changeAElements(window.frames[i].document);
	}
	changeAElements(document);
}

function changeAElements(doc){
	var list = doc.getElementsByTagName('a');
	for(var i=0; i<list.length; i++){		
		var attr = list[i].getAttribute('target');
		//case of no target attr
		if ( (attr == null) || (attr != '_self') ){
			list[i].setAttribute('target', '_self');
		}
	}
}

var s = document.createElement('script');
s.setAttribute('id', 'csjs');
s.setAttribute('type', 'text/javascript');
s.text = "<!--insertJsHere!-->";

document.getElementsByTagName('head')[0].appendChild(s);

changeAll();


