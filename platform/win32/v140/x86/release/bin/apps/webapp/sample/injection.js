
/*****************************************************************************/
// CoTExt
var CoTExt = {
	'csObject' : __SIRIUS_APP__,				// cs interface object
	'cs' : null,								// entrix cs interface module
	'api' : {},									// App interface
};
// CoTExt
/*****************************************************************************/



/*****************************************************************************/
// CoTExt.cs communicator define
CoTExt.cs = (function(){
	var callbacks = {};

	var initialize = function() {
		console.log('initialize');
		if (CoTExt.csObject && typeof CoTExt.csObject !== 'undefined') {
			CoTExt.csObject.setMessageCallback('AttendantToApp', function(name, args) {
				$("#debug").val('AttendantToApp :' + args[0]);
				console.info('%c[CoTExt.cs.receiveMessage] ' + args[0], 'color:blue');
			});
		}
	};

	var sendMessageToCSS = function(obj) {
		var jsonStr = JSON.stringify(obj);
		$("#debug").val('AppToAttendant :' + jsonStr);
		if (typeof CoTExt.csObject !== 'undefined')
			CoTExt.csObject.sendMessage('AppToAttendant', [jsonStr]);
	};

	initialize();

	return {
		'sendMessageToCSS' : sendMessageToCSS
	};
})();
// CoTExt.cs communicator define
/*****************************************************************************/



/*****************************************************************************/
// CoTExt.api
CoTExt.api.setTimeout = function(data) {
	CoTExt.cs.sendMessageToCSS({
		'name' : 'setTimeout',
		'data' : {
			'time' : data
		}
	});
};

