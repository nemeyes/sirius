window.setTimeout(function(){
	document.activeElement.blur();
}, 1);

window.alert = function(m){console.log(m);}
window.confirm = function(m){console.log(m);}
window.prompt = function(m){console.log(m);}

var orgOpen=window.open;
window.open=function(u, w, wi) {
	return orgOpen(u, '_self', wi);
}
