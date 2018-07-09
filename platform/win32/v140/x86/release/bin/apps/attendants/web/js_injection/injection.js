var isCoTYoutubePlayer = false;
if (window.frameElement && window.frameElement.id && window.frameElement.id === 'CoT_youtube_player')
	isCoTYoutubePlayer = true;


if ((self === top && !CoTExt) || isCoTYoutubePlayer) {
	console.info('[CoTExt]', document.location.toString());

	/*****************************************************************************/
	// set MediaSource is null;
	window.MediaSource = null;
	window.WebKitMediaSource = null;
	/*****************************************************************************/

	/*****************************************************************************/
	// overwrite window.close
	window.close = function() {
		CoTExt.api.launchApp({'ip':CoTExt.properties['SERVER']['ICS'], 'port':5000, 'name':'MAIN', 'csType':'ICS'});
	}
	/*****************************************************************************/


	/*****************************************************************************/
	// CoTExt
	var CoTExt = {
		'csObject' : __SIRIUS_APP__,				// cs interface object
		'cs' : null,								// cs interface module
		'main' : null,								// main
		'util' : null,								// util
		'video' : null,								// video & audio module
		'object' : null,							// HbbTV Object module
		'OIPF' : {},								// OIPF
		'OIPF_CLASS' : {},							// OIPF CLASS
		'api' : {},									// App interface
		'properties' : {
			'SERVICE_ID' : 'SKY_PORTAL',
			'SERVER' : {
				'ICS' : '112.220.230.26',
				'VCS' : '192.168.2.2',
				'SKY_BIZ_TICKET' : '18.195.164.82',
				'SKY_ON_DEMAND' : '52.28.229.107',
				'SKY_UK_NOWTV_HOTEL' : '52.56.241.227',
/*
				'SKY_PORTAL' : '18.196.89.101',
				'SKY_Q_POC1' : '35.158.84.167',
*/
				'SKY_PORTAL' : '52.79.168.144',
				'SKY_Q_POC1' : '52.78.45.132',
			},
			'URL' : {
				'MAIN' : 'http://112.220.230.26:8080/cot/sky/sky_portal/index.html',
				'SKY_PORTAL' : 'http://112.220.230.26:8080/cot/sky/sky_portal/index.html',
				'NOW_TV' : 'http://publish-tv.prd.p.ovp.sky.com/',
				'SKY_STORE' : 'http://qs.int.skystore.com/smarttv/smarttv.html',
				'SKY_HOTEL_TV' : 'http://112.220.230.26:8080/cot/sky/sky_hotel_tv/index.html?res=hd',
				'SKY_Q_POC1' : 'http://cot.lph.cool/?res=hd',
				'SKY_Q_POC2' : 'http://112.220.230.26:8080/cot/sky/sky_qms/index.html?res=hd',
				'SKY_MOZART' : 'http://qs.int.skystore.com/skygo/skygo.html',
			},
			'ENC_KEY' : 'CoTExt',
			'timeout' : 0,
			'objectType' : {
				'application/oipfApplicationManager' : 'appmgr',
				'application/oipfConfiguration' : 'oipfcfg',
				'video/broadcast' : 'broadcast'
				//'application/oipfRecordingScheduler' : 'recorder',			// ARD에서만 사용됨.
				//'valups/system' : 'system',
			},
			'mediaMimeType' : [
				'media/CoT',
				'video/mpeg', 'video/mp4', 'video/mpeg4',
				'audio/mpeg', 'audio/mp4'
				//'video/mpeg', 'video/mp2t', 'video/vnd.dlna.mpeg-tts', 'video/mp4',
				//'audio/mpeg', 'audio/mp4', 'audio/3gpp', 'audio/x-wav', 'audio/vnd.dts.hd', 'audio/amr', 'audio/amr-wb', 'audio/amr-wb+', 'audio/mp4', 'audio/3gpp', 'audio/ac3', 'audio/eac3'
			],
			'objectDumyVideo' : 'CoTExt_object_video',
			'objectDumyBroadcast' : 'CoTExt_object_broadcast',
			'checkVideoRedirect' : false,			// video open전 source 파일 존재 여부를 ajax로 확인
			'existBroadcast' : true,				// broadcast 여부
			'blankVideoUrl' : 'http://112.220.230.26:8080/cot/video/blank.ogv'
		},
		'CoTYoutubePlayer' : null
	};

	if (!CoTExt.properties['existBroadcast'])
		delete CoTExt.properties.objectType['video/broadcast'];

	if (isCoTYoutubePlayer) {
		parent.CoTExt.CoTYoutubePlayer = this;
		CoTExt.csObject = parent.CoTExt.csObject;
		CoTExt.cs = parent.CoTExt.cs;
	}
	// CoTExt
	/*****************************************************************************/



	if (!isCoTYoutubePlayer) {
		/*****************************************************************************/
		// CoTExt.cs communicator define
		CoTExt.cs = (function(){
			var callbacks = {};

			var initialize = function() {
				if (CoTExt.csObject && typeof CoTExt.csObject !== 'undefined') {
					CoTExt.csObject.setMessageCallback('AttendantToApp', function(name, args) {
						console.info('%c[CoTExt.cs.receiveMessage] ' + args[0], 'color:blue');
						var obj = JSON.parse(args[0]);
						if (typeof callbacks[obj.name] === 'function')
							callbacks[obj.name](obj.data);
					});
				}
			};

			var receiveMessageFromCSS = function(obj) {
				if (typeof obj === 'object' && obj.name !== undefined && typeof obj.callback === 'function')
					callbacks[obj.name] = obj.callback;
			};

			var sendMessageToCSS = function(obj) {
				var jsonStr = JSON.stringify(obj);
				if (typeof CoTExt.csObject !== 'undefined')
					CoTExt.csObject.sendMessage('AppToAttendant', [jsonStr]);
				console.info('%c[CoTExt.cs.sendMessage] ' + jsonStr, 'color:green');
			};

			initialize();

			return {
				'sendMessageToCSS' : sendMessageToCSS,
				'receiveMessageFromCSS' : receiveMessageFromCSS
			};
		})();
		// CoTExt.cs communicator define
		/*****************************************************************************/



		/*****************************************************************************/
		// KeyEvent
		var KeyEvent = {
			VK_LEFT			: 37,
			VK_UP			: 38,
			VK_RIGHT		: 39,
			VK_DOWN			: 40,
			VK_ENTER		: 13,
			VK_BACK			: 8,
			VK_0			: 48,
			VK_1			: 49,
			VK_2			: 50,
			VK_3			: 51,
			VK_4			: 52,
			VK_5			: 53,
			VK_6			: 54,
			VK_7			: 55,
			VK_8			: 56,
			VK_9			: 57,
			VK_RED			: 116,			// F5
			VK_GREEN		: 117,			// F6
			VK_YELLOW		: 118,			// F7
			VK_BLUE			: 119,			// F8
			VK_PREV			: 112,			// F1
			VK_NEXT			: 113,			// F2
			VK_PLAY			: 114,			// F3
			VK_PAUSE		: 115,			// F4
			VK_PLAY_PAUSE	: 120,			// F9
			VK_STOP			: 121,			// F10
			VK_REWIND		: 122,			// F11
			VK_FAST_FWD		: 123,			// F12
			VK_PAGE_UP		: 33,			// PAGE UP
			VK_PAGE_DOWN	: 34,			// PAGE DOWN
			VK_INFO			: 47			// HELP
		};
		for (var attr in KeyEvent) {
			if (KeyEvent.hasOwnProperty(attr))
				window[attr] = KeyEvent[attr];
		}
		// KeyEvent
		/*****************************************************************************/



		/*****************************************************************************/
		//OIPF object

		if (CoTExt.properties['existBroadcast']) {
			// page 194
			CoTExt.OIPF['video/broadcast'] = {
				//width : 0,
				//height : 0,
				fullScreen : null,
				data : null,
				playState : 2,					// 0-unrealized; 1-connecting; 2-presenting; 3-stopped;

				_channelConfig : null,			// for getChannelConfig
				getChannelConfig : function() {
					console.info('CoTExt.OIPF["video/broadcast"].getChannelConfig');
					return (this._channelConfig = this._channelConfig || new CoTExt.OIPF_CLASS['ChannelConfig']());
				},
				bindToCurrentChannel : function() {
					console.info('%c>>>todo CoTExt.OIPF["video/broadcast"].bindToCurrentChannel', 'color:red;');
					this._liveBind();
					return null;				// todo Channel
				},
				//createChannelObject : function(idType, dsd, sid) {},
				createChannelObject : function(idType, onid, tsid, sid, sourceID, ipBroadcastID) {
					console.info('%c>>>todo CoTExt.OIPF["video/broadcast"].createChannelObject', 'color:red;', arguments);
					return null;				// todo Channel
				},
				setChannel : function(channel, trickplay, contentAccessDescriptorURL) {
					console.info('%c>>>todo CoTExt.OIPF["video/broadcast"].setChannel', 'color:red;', arguments);
					// if channel is null then App transition to BI App.
					this._channelTune({
						'act' : 'SET',
						'channel' : channel
					});
				},
				prevChannel : function() {
					console.info('CoTExt.OIPF["video/broadcast"].prevChannel');
					this._channelTune({
						'act' : 'PREV',
						'channel' : {}
					});
				},
				nextChannel : function() {
					console.info('CoTExt.OIPF["video/broadcast"].nextChannel');
					this._channelTune({
						'act' : 'NEXT',
						'channel' : {}
					});
				},
				setFullScreen : function(fullscreen) {			// Boolean fullscreen
					console.info('CoTExt.OIPF["video/broadcast"].setFullScreen', arguments);
					this.fullScreen = fullscreen;
				},
				release : function() {
					console.info('%c>>>todo CoTExt.OIPF["video/broadcast"].release', 'color:red;');
				},
				stop : function() {
					this.innerHTML = '';
					this._prevSize = {'width' : null, 'height' : null, 'left' : null, 'top' : null};
					this.playState = 0;		 // unrealized
					CoTExt.api.stopBroadcast();
				},

				// todo
				onfocus : null,								// function() {},
				onblur : null,								// function() {},
				onFullScreenChange : null,					// function() {},
				onChannelChangeError : null,				// function(channel, errorState) {},
				onChannelChangeSucceeded : null,			// function(channel) {},
				onPlayStateChange : null,					// function(state, error) {},

				// CoT function
				_prevSize : {'width' : null, 'height' : null, 'left' : null, 'top' : null},
				_liveBind : function() {
					console.info('CoTExt.OIPF["video/broadcast"]._liveBind', this);
					var size = {'width' : null, 'height' : null, 'left' : null, 'top' : null};
					if (this.style.display !== 'none') {
						var rect = this.getBoundingClientRect();
						size.width = CoTExt.util.convertSize(this.getAttribute('width'), rect.width);
						size.height = CoTExt.util.convertSize(this.getAttribute('height'), rect.height);
						size.left = rect.left + this.scrollLeft;
						size.top = rect.top + this.scrollTop;
						if (size.width == '0' && size.height == '0' && size.left == '0' && size.top == '0')
							return;

						var ratio = 1;
						//var ratio = window.innerWidth / screen.width;
						size.width = Math.round(size.width * ratio);
						size.height = Math.round(size.height * ratio);
						size.left = Math.round(size.left * ratio);
						size.top = Math.round(size.top * ratio);

						this.fullScreen = !!(size.width == screen.width && size.height == screen.height);
						//this.fullScreen = (size.width == screen.width && size.height == screen.height) ? true : false;
						if (this._prevSize.width != size.width || this._prevSize.height != size.height || this._prevSize.left != size.left || this._prevSize.top != size.top) {
							this.playState = 2;			 // presenting
							CoTExt.video.stop();
							CoTExt.cs.sendMessageToCSS({
								'name' : 'bindLive',
								'data' : {
									'width' : size.width,
									'height' : size.height,
									'left' : size.left,
									'top' : size.top,
									'fullScreen' : this.fullScreen ? 'Y' : 'N'
								}
							});
							this.innerHTML = '<video id="' + CoTExt.properties['objectDumyBroadcast'] + '" width="100%" height="100%" src="' + CoTExt.properties['blankVideoUrl'] + '" autoplay="true" />';
							setTimeout(() => {this.style.opacity = 1;}, 500);
						}

						this._prevSize = {'width' : size.width, 'height' : size.height, 'left' : size.left, 'top' : size.top};
					}
					else {
						this._prevSize = {'width' : null, 'height' : null, 'left' : null, 'top' : null};
					}
				},
				_channelTune : function(data) {
					this._currentChannel = null;
					CoTExt.cs.sendMessageToCSS({
						'name' : 'tuneChannel',
						'data' : data
					});
					if (typeof onChannelChangeError === 'function')
						console.info('%c>>>todo [CoTExt.OIPF["video/broadcast"]._channelTune] onChannelChangeError', 'color:red', this.onChannelChangeError);
					if (typeof onChannelChangeSucceeded === 'function')
						console.info('%c>>>todo [CoTExt.OIPF["video/broadcast"]._channelTune] onChannelChangeSucceeded', 'color:red', this.onChannelChangeSucceeded);
				},
				_currentChannel : null			// for currentChannel property

				/* HbbTV_2.0.1 Not Included
				playerCapabilities : null,
				allocationMethod : 1,			// 1-STATIC_ALLOCATION, 2-DYNAMIC_ALLOCATION
				setVolume : function(volume) {},
				getVolume : function() {},
				*/
			};
			// Extensions to video/broadcast for current channel information
			Object.defineProperty(HTMLObjectElement.prototype, 'currentChannel', {get : function() { 
				if (this.type !== 'video/broadcast')
					return null;
				console.info('%c>>>todo CoTExt.OIPF["video/broadcast"].currentChannel', 'color:red;');
				return null;			// todo
				//return (this._currentChannel = this._currentChannel || new CoTExt.OIPF_CLASS['Channel']('CURRENT'));
			}});
		}


		CoTExt.OIPF['media/CoT'] = {
			// CoT function
			_data : null,			// for original data
			_oriType : '',			// for original type
			_isAudio : function() {
				return this._oriType.startsWith('audio/');
			},
			_initData : function(data) {
				this._data = data;
				if (data !== null && data !== undefined && data.length > 0) {
					console.info('CoTExt.OIPF["media/CoT"]._initData', data);
					this.removeAttribute('data');
					if (!this._isAudio())
						this.innerHTML = '<video id="' + CoTExt.properties['objectDumyVideo'] + '" width="100%" height="100%" src="' + CoTExt.properties['blankVideoUrl'] + '" autoplay="true" />';
					setTimeout(() => {CoTExt.video.setVideoData(data);}, 50);
				}
			},

			//data : null,
			playPosition : 0,
			playTime : 0,
			playState : 0,			// 0-stopped, 1-playing, 2-paused, 3-connecting, 4-buffering, 5-finished, 6-error
			error : null,			// 0 - A/V format not supported, 1 - cannot connect to server or connection lost. 2 - unidentified error. 3 - insufficient resources. 4 - content corrupt or invalid. 5 - content not available. 6 - content not available at given position
			speed : 0,
			onPlayStateChange : null,
			//width : 0,
			//height : 0,
			fullScreen : null,
			onFullScreenChange : null,
			onfocus : null,
			onblur : null,
			play : function(speed) {
				CoTExt.video.play(speed == undefined ? 1 : speed);
			},
			stop : function() {
				CoTExt.video.stop();
			},
			seek : function(pos) {
				CoTExt.video.seek(pos == undefined ? 0 : pos);
			},
			setVolume : function(volume) {
				console.info('%c>>>todo CoTExt.OIPF["media/CoT"].setVolume', 'color:red;', arguments);
			},
			setFullScreen : function(fullScreen) {
				console.info('%c>>>todo CoTExt.OIPF["media/CoT"].setFullScreen', 'color:red;', arguments);
				this.fullScreen = fullScreen;
			},
			focus : function() {
				console.info('%c>>>todo CoTExt.OIPF["media/CoT"].focus', 'color:red;');
			}
		};


		// page 63
		CoTExt.OIPF['application/oipfApplicationManager'] = {
			onLowMemory : null,						// function() {},
			onApplicationLoadError : null,			// function(appl) {},

			_ownerApplication : null,				// for getOwnerApplication
			getOwnerApplication : function(doc) {
				console.info('CoTExt.OIPF["application/oipfApplicationManager"].getOwnerApplication', arguments);
				return (this._ownerApplication = this._ownerApplication || new CoTExt.OIPF_CLASS['Application']());
			}

			/* HbbTV_2.0.1 Not Included
			onApplicationLoaded : function(appl) {},
			onApplicationUnloaded : function(appl) {},
			onWidgetInstallation : function(wd, state, reason) {},
			onWidgetUninstallation : function(wd, state) {},
			widgets : null,		 //WidgetDescriptorCollection
			getApplicationVisualizationMode : function() {},
			getChildApplications : function(appl) {},
			gc : function() {},
			installWidget : function(uri) {},
			uninstallWidget : function(wd) {},
			*/
		};


		// page 78
		CoTExt.OIPF['application/oipfConfiguration'] = {
			configuration : null			// readonly Configuration

			/* HbbTV_2.0.1 Not Included
			localSystem : null,
			onIpAddressChange : function(item, ipAddress) {}
			*/
		};


		/*
		// page 161
		CoTExt.OIPF['application/oipfRecordingScheduler'] = {
			record : function(programme) {
				console.info('%c>>>todo CoTExt.OIPF["application/oipfRecordingScheduler"].record', 'color:red;', arguments);
				return null;			// ScheduledRecording
			},
			recordAt : function(startTime, duration, repeatDays, channelID) {
				console.info('%c>>>todo CoTExt.OIPF["application/oipfRecordingScheduler"].recordAt', 'color:red;', arguments);
				return null;			// ScheduledRecording
			},
			getScheduledRecordings : function() {
				console.info('%c>>>todo CoTExt.OIPF["application/oipfRecordingScheduler"].getScheduledRecordings', 'color:red;', arguments);
				return null;			// ScheduledRecordingCollection
			},
			getChannelConfig : function() {
				console.info('%c>>>todo CoTExt.OIPF["application/oipfRecordingScheduler"].getChannelConfig', 'color:red;', arguments);
				return null;			// ChannelConfig
			},
			remove : function(recording) {
				console.info('%c>>>todo CoTExt.OIPF["application/oipfRecordingScheduler"].remove', 'color:red;', arguments);
			},
			createProgrammeObject : function() {
				console.info('%c>>>todo CoTExt.OIPF["application/oipfRecordingScheduler"].createProgrammeObject', 'color:red;', arguments);
				return null;			// Programme
			},
			getInProgressRecordings : function() {
				console.info('%c>>>todo CoTExt.OIPF["application/oipfRecordingScheduler"].getInProgressRecordings', 'color:red;', arguments);
				return null;			// ScheduledRecordingCollection
			},
		};
		*/
		//OIPF object
		/*****************************************************************************/



		/*****************************************************************************/
		//OIPF Class object

		// page 67
		CoTExt.OIPF_CLASS['Application'] = function() {
			this._privateData = null;			// readonly ApplicationPrivateData

			this.show = function() {
				console.info('%c>>>todo CoTExt.OIPF_CLASS["Application"].show', 'color:red;');
			};
			this.hide = function() {
				console.info('%c>>>todo CoTExt.OIPF_CLASS["Application"].hide', 'color:red;');
			};
			this.createApplication = function(uri, createChild) {
				console.info('%c>>>todo CoTExt.OIPF_CLASS["Application"].createApplication', 'color:red;', arguments);
				if (uri.startsWith('dvb://'))
					;			// todo
				else			// uri startsWith http or https App transition to BI App.
					window.location.href = uri;
				return null;			// todo Application
			};
			this.destroyApplication = function() {
				CoTExt.api.launchApp({'ip':CoTExt.properties['SERVER']['ICS'], 'port':5000, 'name':'MAIN', 'csType':'ICS'});
				//window.location.href = CoTExt.properties['URL']['MAIN'];
				//CoTExt.api.closeSession();
			};

			/* HbbTV_2.0.1 Not Included
			this._visible = true;							// readonly Boolean
			this._active = true;							// readonly Boolean
			this._permissions = null;						// readonly StringCollection
			this._isPrimaryReceiver = true;					// readonly Boolean
			this.window = window;							// readonly Window
			this.onApplicationActivated = null;
			this.onApplicationDeactivated = null;
			this.onApplicationShown = null;
			this.onApplicationHidden = null;
			this.onApplicationPrimaryReceiver = null;
			this.onApplicationNotPrimaryReceiver = null;
			this.onApplicationTopmost = null;
			this.onApplicationNotTopmost = null;
			this.onApplicationDestroyRequest = null;
			this.onApplicationHibernateRequest = null;
			this.onKeyPress = null;
			this.onKeyUp = null;
			this.onKeyDown = null;
			this.activateInput = function(gainFocus) {};
			this.deactivateInput = function() {};
			this.startWidget = function(wd, createChild) {};
			this.stopWidget = function(wd, createChild) {};
			*/
		};
		Object.defineProperty(CoTExt.OIPF_CLASS['Application'].prototype, 'privateData', {get : function() {
			console.info('CoTExt.OIPF_CLASS["Application"].privateData');
			return (this._privateData = this._privateData || new CoTExt.OIPF_CLASS['ApplicationPrivateData']());
		}});


		// page 71
		CoTExt.OIPF_CLASS['ApplicationPrivateData'] = function() {
			this._keyset = null;							// readonly Keyset
			this._currentChannel = null;					// readonly Channel

			this.getFreeMem = function() {
				console.info('%c>>>todo CoTExt.OIPF_CLASS["ApplicationPrivateData"].getFreeMem', 'color:red;');
				return null;		// todo Integer
			};

			/* HbbTV_2.0.1 Not Included
			this._wakeupApplication = true;					// readonly Boolean
			this._wakeupOITF = true;						// readonly Boolean
			this.prepareWakeupApplication = function(URI, token, time) {};
			this.prepareWakeupOITF = function(time) {};
			this.clearWakeupToken = function(time) {};
			*/
		};
		Object.defineProperties(CoTExt.OIPF_CLASS['ApplicationPrivateData'].prototype, {
			'keyset' : {
				get : function() {
					//console.info('CoTExt.OIPF_CLASS["ApplicationPrivateData"].keyset');
					return (this._keyset = this._keyset || new CoTExt.OIPF_CLASS['Keyset']());
				}
			},
			'currentChannel' : {
				get : function() {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["ApplicationPrivateData"].currentChannel', 'color:red;', this._currentChannel);
					return null;			// todo
					//return (this._currentChannel = this._currentChannel ? this._currentChannel : new CoTExt.OIPF_CLASS['Channel']('CURRENT'));
				}
			}
		});

		/*
		Object.defineProperty(CoTExt.OIPF_CLASS['ApplicationPrivateData'].prototype, 'keyset', {get : function() {
			//console.info('CoTExt.OIPF_CLASS["ApplicationPrivateData"].keyset');
			return (this._keyset = this._keyset || new CoTExt.OIPF_CLASS['Keyset']());
		}});
		Object.defineProperty(CoTExt.OIPF_CLASS['ApplicationPrivateData'].prototype, 'currentChannel', {get : function() {
			console.info('%c>>>todo CoTExt.OIPF_CLASS["ApplicationPrivateData"].currentChannel', 'color:red;', this._currentChannel);
			return null;			// todo
			//return (this._currentChannel = this._currentChannel ? this._currentChannel : new CoTExt.OIPF_CLASS['Channel']('CURRENT'));
		}});
		*/


		// page 72
		CoTExt.OIPF_CLASS['Keyset'] = function() {
			this.RED			= 0x1;					// VK_RED
			this.GREEN			= 0x2;					// VK_GREEN
			this.YELLOW			= 0x4;					// VK_YELLOW
			this.BLUE			= 0x8;					// VK_BLUE
			this.NAVIGATION		= 0x10;				// VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_ENTER and VK_BACK
			this.VCR			= 0x20;					// VK_PLAY, VK_PAUSE, VK_STOP, VK_NEXT, VK_PREV, VK_FAST_FWD, VK_REWIND, VK_PLAY_PAUSE
			this.SCROLL			= 0x40;					// VK_PAGE_UP and VK_PAGE_DOWN
			this.INFO			= 0x80;					// VK_INFO
			this.NUMERIC		= 0x100;				// 0 to 9
			this.ALPHA			= 0x200;				// alphabetic
			this.OTHER			= 0x400;				// the other

			this._value = 0;							// readonly Integer
			this.maximumValue = 0x7FF;					// readonly Integer
			this.supportsPointer = false;				// Boolean

			this.setValue = function(value, otherKeys) {
				this._value = value;
				/*
				CoTExt.cs.sendMessageToCSS({
					'name' : 'setKeySet',
					'data' : {
						'value' : value
					}
				});
				*/
				return value;
			};
			this.getKeyIcon = function(code) {			// 32*32 pixels
				console.info('%c>>>todo CoTExt.OIPF_CLASS["Keyset"].getKeyIcon', 'color:red;', arguments);
				return null;			// todo String
			};

			/* HbbTV_2.0.1 Not Included
			this.otherKeys = [];						// readonly Integer Array
			this.maximumOtherKeys = [];					// readonly Integer Array
			this.getKeyLabel = function(code) {};
			*/
		};
		Object.defineProperty(CoTExt.OIPF_CLASS['Keyset'].prototype, 'value', {get : function() {
			return this._value;
		}});


		// page 218
		CoTExt.OIPF_CLASS['ChannelConfig'] = function() {
			this._channelList = null;					// readonly ChannelList

			/* HbbTV_2.0.1 Not Included
			this._favouriteLists = null;				// FavouriteListCollection
			this._currentFavouriteList= null;			// FavouriteList
			this._currentChannel = null;				// Channel
			this.onChannelScan = null;
			this.onChannelListUpdate = null;
			this.createFilteredList = function(blocked, favourite, hidden, favouriteListID) {};
			this.startScan = function(options, scanParameters) {};
			this.stopScan = function() {};
			this.createChannelList = function(bdr) {};
			this.createChannelObject = function(idType, onid, tsid, sid, sourceID, ipBroadcastID) {};
			this.createChannelScanParametersObject = function(idType) {};
			this.createChannelScanOptionsObject = function() {};
			*/
		};
		Object.defineProperty(CoTExt.OIPF_CLASS['ChannelConfig'].prototype, 'channelList', {get : function() {
			console.info('%c>>>todo CoTExt.OIPF_CLASS["ChannelConfig"].channelList', 'color:red;', this._channelList);
			return null;			// todo
		}});


		// page 224
		CoTExt.OIPF_CLASS['ChannelList'] = [];
		CoTExt.OIPF_CLASS['ChannelList'].getChannel = function(channelID) {
			console.info('%c>>>todo CoTExt.OIPF_CLASS["ChannelList"].getChannel', 'color:red;', arguments);
			return null;			// todo Channel
		};
		CoTExt.OIPF_CLASS['ChannelList'].getChannelByTriplet = function(onid, tsid, sid, nid) {
			console.info('%c>>>todo CoTExt.OIPF_CLASS["ChannelList"].getChannelByTriplet', 'color:red;', arguments);
			return null;			// todo Channel
		};
		/* HbbTV_2.0.1 Not Included
		this.getChannelBySourceID = function(sourceID) {}
		*/


		// page 225
		CoTExt.OIPF_CLASS['Channel'] = function(option) {
			this.channelType = null;					// readonly Integer
			this.ccid = null;							// readonly String
			this.dsd = null;							// readonly String
			this.idType = null;							// readonly Integer
			this.nid = null;							// readonly Integer
			this.onid = null;							// readonly Integer
			this.tsid = null;							// readonly Integer
			this.sid = null;							// readonly Integer
			this.name = null;							// String
			this.majorChannel = null;					// readonly Integer

			if (option === 'CURRENT') {
				try {
					var channel = JSON.parse(CoTExt.main.buffer['CHANNEL']);
					for (attr in channel) {
						if (channel.hasOwnProperty(attr))
							this[attr] = channel[attr];
					}
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Channel"].currentChannel', 'color:red', this);
				}
				catch (e) {
					console.error(e.toString());
				}
				/*
				CoTExt.main.buffer['currentChannelObj'] = this;
				CoTExt.cs.sendMessageToCSS({
					'command' : 'request',
					'name' : 'CHANNEL-GET',
					'data' : {
						'act' : 'CURRENT'
					}
				});
				*/
			}

			/* HbbTV_2.0.1 Not Included
			this.tunerID = null;						// readonly String
			this.sourceID = null;						// readonly Integer
			this.freq = null;							// readonly Integer
			this.cni = null;							// readonly Integer
			this.minorChannel = null;					// readonly Integer
			this.favourite = null;						// readonly Boolean
			this.favIDs = null;							// readonly StringCollection
			this.locked = null;							// readonly Boolean
			this.manualBlock = null;					// readonly Boolean
			this.ipBroadcastID = null;					// readonly String
			this.channelMaxBitRate = null;				// readonly Integer
			this.channelTTR = null;						// readonly Integer
			this.recordable = null;						// readonly Boolean
			// clientMetadata == true
			this.longName = null;						// String
			this.description = null;					// String
			this.authorised = null;						// readonly Boolean
			this.genre = null;							// StringCollection
			this.hidden = null;							// Boolean
			this.is3D = null;							// readonly Boolean
			this.isHD = null;							// readonly Boolean
			this.logoURL = null;						// String
			this.getField = function(fieldId) {};
			this.getLogo = function(width, height) {};
			*/
		};


		// page 225
		CoTExt.OIPF_CLASS['Configuration'] = function() {
			this._preferredAudioLanguage;				// String
			this._preferredSubtitleLanguage;			// String
			this._preferredUILanguage;					// String
			this._countryId;							// String

			/* HbbTV_2.0.1 Not Included
			this.regionId;								// Integer
			this.pvrPolicy;								// Integer
			this.pvrSaveEpisodes;						// Integer
			this.pvrSaveDays;							// Integer
			this.pvrStartPadding;						// Integer
			this.pvrEndPadding;							// Integer
			this.preferredTimeShiftMode;				// Integer
			this.getText = function(key) {};
			this.setText = function(key, value) {};
			*/
		};
		Object.defineProperties(CoTExt.OIPF_CLASS['Configuration'].prototype, {
			'preferredAudioLanguage' : {
				get : function() {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].preferredAudioLanguage', 'color:red;');
					return _preferredAudioLanguage;
				},
				set : function(data) {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].preferredAudioLanguage setter', 'color:red;', data);
					this._preferredAudioLanguage = data;
				}
			},
			'preferredSubtitleLanguage' : {
				get : function() {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].preferredSubtitleLanguage', 'color:red;');
					return _preferredSubtitleLanguage;
				},
				set : function(data) {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].preferredSubtitleLanguage setter', 'color:red;', data);
					this._preferredSubtitleLanguage = data;
				}
			},
			'preferredUILanguage' : {
				get : function() {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].preferredUILanguage', 'color:red;');
					return _preferredUILanguage;
				},
				set : function(data) {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].preferredUILanguage setter', 'color:red;', data);
					this._preferredUILanguage = data;
				}
			},
			'countryId' : {
				get : function() {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].countryId', 'color:red;');
					return _countryId;
				},
				set : function(data) {
					console.info('%c>>>todo CoTExt.OIPF_CLASS["Configuration"].countryId setter', 'color:red;', data);
					this._countryId = data;
				}
			}
		});

		//OIPF Class object
		/*****************************************************************************/



		/*****************************************************************************/
		//HTMLObjectElement.prototype override methods

		HTMLObjectElement.prototype._setAttribute = HTMLObjectElement.prototype.setAttribute;
		HTMLObjectElement.prototype.setAttribute = function(name, value) {
			this._setAttribute(name, value);

			console.info('[HTMLObjectElement.setAttribute]', name, value);
			if (name === 'type' && CoTExt.properties['mediaMimeType'].indexOf(value) > -1) {
				this._setAttribute(name, 'media/CoT');
				if (typeof this.play !== 'function') {
					for (var attr in CoTExt.OIPF['media/CoT']) {
						if (!this[attr])
							this[attr] = CoTExt.OIPF['media/CoT'][attr];
					}
				}
				this._oriType = value;
				CoTExt.video.setVideo(this);
			}
			else if (name === 'data' && this.type === 'media/CoT') {
				this._initData(value);
			}
		};


		Object.defineProperties(HTMLObjectElement.prototype, {
			'_data' : {writable : true, value : null},
			'data' : {get : function() { return this._data; }, set : function(data) {console.info('[Object.data.setter]', data, this.type); this._data = data; if (this.type === 'media/CoT') this._initData(data);}}
		});

		//HTMLObjectElement.prototype override methods
		/*****************************************************************************/
	} // if (!isCoTYoutubePlayer) {



	/*****************************************************************************/
	//HTMLVideoElement.prototype override properties

	// 0-stopped, 1-playing, 2-paused, 3-connecting, 4-buffering, 5-finished, 6-error
	Object.defineProperty(HTMLMediaElement.prototype, '_playState', {writable : true, value : 0});

	Object.defineProperties(HTMLVideoElement.prototype, {
		'_currentTime' : {writable : true, value : 0},
		'currentTime' : {get : function() {return this._currentTime;}, set : function(currentTime) {CoTExt.video.seek(currentTime * 1000);}},

		'_duration' : {writable : true, value : ''},
		'duration' : {get : function() {return this._duration;}, set : function(duration) {this._duration = duration;}},

		'_ended' : {writable : true, value : false},
		'ended' : {get : function() {return this._ended;}, set : function(ended) {this._ended = ended;}},

		'_error' : {writable : true, value : ''},
		'error' : {get : function() {return this._error;}, set : function(error) {this._error = error;}},

		'_networkState' : {writable : true, value : 1},
		'networkState' : {get : function() {return this._networkState;}, set : function(networkState) {this._networkState = networkState;}},

		'_paused' : {writable : true, value : true},
		'paused' : {get : function() {return this._paused;}, set : function(paused) {this._paused = paused;}},

		'_readyState' : {writable : true, value : 0},
		'readyState' : {get : function() {return this._readyState;}, set : function(readyState) {this._readyState = readyState;}},

		'_seeking' : {writable : true, value : false},
		'seeking' : {get : function() {return this._seeking;}, set : function(seeking) {this._seeking = seeking;}},

		'_buffered' : {writable : true, value : {length : 0, start : function(idx) {return null;}, end : function(idx) {return null;}}},
		'buffered' : {get : function() {return this._buffered;}}
	});

	Object.defineProperties(HTMLSourceElement.prototype, {
		'_src' : {writable : true, value : ''},
		'src' : {get : function() {return this._src;}, set : function(src) {console.info('[HTMLSourceElement.src.setter]', src); CoTExt.video.setVideoData(src); this._src = src;}}
	});

	//HTMLVideoElement.prototype override properties
	/*****************************************************************************/



	/*****************************************************************************/
	//HTMLVideoElement.prototype override methods
	
	/*
	HTMLVideoElement.prototype.addTextTrack()
	HTMLVideoElement.prototype.canPlayType()
	HTMLVideoElement.prototype.load()
	HTMLMediaElement.prototype.canPlayType()
	*/

	HTMLVideoElement.prototype.play = function() {
		if (this._playState != 1)
			CoTExt.video.play(1);
	};

	HTMLVideoElement.prototype.pause = function() {
		if (this._playState != 2)
			CoTExt.video.play(0);
	};

	//HTMLVideoElement.prototype override methods
	/*****************************************************************************/



	/*****************************************************************************/
	// CoTExt.video start
	CoTExt.video = (function(){
		var videoObj = null;
		var playPositionInterval = 1000;
		var observerResize = new MutationObserver(function(){});
		var observerChangeSrc = new MutationObserver(function() {});
		var isOpen = false;
		var prevSize = {'width' : null, 'height' : null, 'left' : null, 'top' : null};
		var vodInfo = null;


		var init = function() {
			if (isCoTYoutubePlayer) {
				if (videoObj === null)
					findVideo();
			}
			else {
				findVideo();
				setInterval(() => {CoTExt.video.findVideo();}, 500);
				//setInterval(function() {CoTExt.video.findVideo();}, 500);
			}

			window.onbeforeunload = function() {
				stop();			// vod stop 없이 페이지 이동시 방어코드
				return null;
			};
		};


		var findVideo = function() {
			if (videoObj !== null) {
				if (videoObj.closest('body') === null) {
					console.info('[CoTExt.video.findVideo]', 'video object removed!');
					stop();
					videoObj = null;
				}
				return;
			}

			var doc = document;
			// check video tag
			try {
				var videos = doc.getElementsByTagName('video');
				if (videos && videos.length > 0) {
					for (var i = 0; i < videos.length; i++) {
						var id = videos[i].getAttribute('id');
						if (id === null || (id !== CoTExt.properties['objectDumyVideo'] && id !== CoTExt.properties['objectDumyBroadcast'])) {
							setVideo(videos[i]);
							return;
						}
					}
				}
			}
			catch(e) {}

			// check audio tag
			try {
				var audio = doc.getElementsByTagName('audio');
				if (audio && audio.length > 0) {
					setVideo(audio[0]);
					return;
					/*
					for (var i = 0; i < audio.length; i++) {
						setVideo(audio[i]);
						return;
					}
					*/
				}
			}
			catch(e) {}
		};


		var setVideo = function(obj) {
			console.info('[CoTExt.video.setVideo] ', vodInfo);
			videoObj = obj;

			if (vodInfo !== null) {			// STB에서 VoD를 재생하는 중에 Recover 된 경우
				if (vodInfo.apply) {
					vodInfo = null;
				}
				else {
					//vodInfo.apply = true;
					onEvent({'type' : 'onVodPlayState', 'data' : {'playState' : vodInfo.playState, 'error' : vodInfo.error}});
					onEvent({'type' : 'onVodPlayTime', 'data' : {'playTime' : vodInfo.playTime}});
					onEvent({'type' : 'onVodPlayPosition', 'data' : {'playPosition' : vodInfo.playPosition}});
				}
			}

			// observe size changed
			observerResize.disconnect();
			if (!isAudioTag()) {
				observerResize = new MutationObserver(function(mutations) {
					mutations.forEach(function(mutation) {
						if (mutation.attributeName === 'class' || mutation.attributeName === 'style' || mutation.attributeName === 'width')
							resize();
					});
				});
				observerResize.observe(videoObj, {attributes:true, attributeOldValue:false});
			}


			if (isVideoTag() || isAudioTag()) {
				videoObj = obj.id ? document.getElementById(obj.id) : obj;
				videoObj.removeAttribute('autoplay');
				videoObj.removeAttribute('poster');

				// observe src changed
				observerChangeSrc.disconnect();
				observerChangeSrc = new MutationObserver(function(mutations) {
					mutations.forEach(function(mutation) {
						if (mutation.attributeName === 'src') {
							console.info('[CoTExt.video.setVideo] src changed : ', mutation.oldValue + ' => ' + videoObj.src);
							var src = videoObj.getAttribute('src');
							if (src === null || src === undefined || src === '')
								stop();
							else
								setVideoData(src);
						}
					});
				});
				observerChangeSrc.observe(videoObj, {attributes:true, attributeOldValue:true});

				// videoObj에 source가 나중에 추가될 경우
				videoObj.addEventListener('DOMNodeInserted', function(event) {
					setTimeout(event => {
						if (event.srcElement.nodeName.toUpperCase() === 'SOURCE') {
							var source = videoObj.getElementsByTagName('source');
							if (source && source.length > 0) {
								for (var i = 0; i < source.length; i++) {
									var src = source[i].getAttribute('src');
									if (src !== null && src.length > 0) {
										setVideoData(src);
										break;
									}
								}
							}
						}
					}, 100, event);
				}, false);

				// check src attribute
				var src = videoObj.getAttribute('src');
				if (src !== null && src.length > 0) {
					setVideoData(src);
					return;
				}

				// check source child node
				var childNodes = videoObj.childNodes;
				if (childNodes !== null && childNodes.length > 0) {
					for (var i = 0; i < childNodes.length; i++) {
						if (childNodes[i].nodeName.toUpperCase() === 'SOURCE') {
							src = childNodes[i].getAttribute('src') || childNodes[i].src;
							if (src !== null && src.length > 0) {
								setVideoData(src);
								return;
							}
						}
					}
				}
			}
			else {
				// HbbTV Object
				if (!window['video'])
					window['video'] = videoObj;

				if (videoObj.data !== null && videoObj.data !== undefined && videoObj.data.length > 0)
					videoObj._initData(videoObj.data);
				else if (videoObj.getAttribute('data') && videoObj.getAttribute('data') !== undefined && videoObj.getAttribute('data').length > 0)
					videoObj._initData(videoObj.getAttribute('data'));
			}
		};


		// check video/audio data
		var prevSrc = {'url' : '', 'time' : null};
		var setVideoData = function(src) {
			console.info('[CoTExt.video.setVideoData]', src);

			//var doc = document;
			if (videoObj === null || src === null || src === '') {
				stop();
				return;
			}

			// blank video url이면 skip
			if (src === CoTExt.properties['blankVideoUrl'])
				return;

			// 0.5초안에 같은 url 들어 왔는지 체크
			if (src === prevSrc.url && (new Date()).getTime() - prevSrc.time.getTime() < 500)
				return;
			prevSrc.url = src;
			prevSrc.time = new Date();


			if (vodInfo === null || vodInfo.apply || !videoObj.hasAttribute('cloud-recovery'))
				stop();		 // 기존 VoD Stop

			if (!src.startsWith('http')) {
				if (src.indexOf(0) === '/')
					src = window.location.origin + src;
				else {
					var href = window.location.href;
					src = href.substring(0, href.lastIndexOf('/')) + '/' + src;
				}
			}

			if (!isCoTYoutubePlayer && CoTExt.properties['checkVideoRedirect'] && !(src.toLowerCase().startsWith('http:') && document.location.protocol === 'https:')) {
				// https에서는 http로 ajax가 불가능함.
				var xhr = new XMLHttpRequest();
				xhr.open('HEAD', src, true);
				xhr.onreadystatechange = function() {
					if (xhr.readyState === 4) {
						if (xhr.status === 200) {
							console.info('[CoTExt.video.setVideoData] XMLHttpRequest', xhr.status, src);
							open(this.responseURL != null ? this.responseURL : src);
						}
						else {
							console.info('[CoTExt.video.setVideoData] XMLHttpRequest error', xhr.status, src);
							onEvent({'type' : 'onVodPlayState', 'data' : {'playState':6, 'error':5}});
						}
					}
				};
				xhr.send();
			}
			else {
				open(src);
			}
		};


		var initVideo = function() {
			if (isVideoTag() || isAudioTag()) {
				videoObj._currentTime = 0;
				videoObj._duration = 0;
				videoObj._ended = false;
				videoObj._error = '';
			}
			else {
				videoObj.playPosition = 0;
				videoObj.playTime = 0;
			}
		};


		var open = function(src) {
			console.info('[CoTExt.video.open]', src, videoObj);
			if (videoObj === null)
				return;

			videoObj._src = src;

			if (isVideoTag() && !isCoTYoutubePlayer)			// make video hole
				videoObj.setAttribute('src', CoTExt.properties['blankVideoUrl']);
			isOpen = true;

			console.info('[CoTExt.video.open]', vodInfo, videoObj.hasAttribute('cloud-recovery'));
			if (vodInfo !== null && !vodInfo.apply && videoObj.hasAttribute('cloud-recovery'))
				return;
			if (vodInfo !== null)
				vodInfo.apply = true;
			videoObj.removeAttribute('cloud-recovery');

			initVideo();
			var info = videoObj.hasAttribute('cloud-info') ? videoObj.getAttribute('cloud-info') : '';
			var size = getVideoSize();
			CoTExt.cs.sendMessageToCSS({
				'name' : 'openVod',
				'data' : {
					'data' : src,
					'width' : size.width,
					'height' : size.height,
					'left' : size.left,
					'top' : size.top,
					'fullScreen' : (size.width == screen.width && size.height == screen.height) ? 'Y' : 'N',
					'playPositionInterval' : videoObj.hasAttribute('cloud-live') ? 0 : playPositionInterval,
					'info' : info
				}
			});
			play(1);
			prevSize = {'width' : size.width, 'height' : size.height, 'left' : size.left, 'top' : size.top};
		};


		var resize = function() {
			if (!isOpen || isAudioTag())
				return;

			var size = getVideoSize();
			if (prevSize.width != size.width || prevSize.height != size.height || prevSize.left != size.left || prevSize.top != size.top) {
				CoTExt.cs.sendMessageToCSS({
					'name' : 'resizeVod',
					'data' : {
						'width' : parseInt(size.width),
						'height' : parseInt(size.height),
						'left' : parseInt(size.left),
						'top' : parseInt(size.top),
						'fullScreen' : (size.width == screen.width && size.height == screen.height) ? 'Y' : 'N'
					}
				});
			}
			prevSize = {'width' : size.width, 'height' : size.height, 'left' : size.left, 'top' : size.top};
		};


		var play = function(speed) {
			if (videoObj === null || !isOpen)
				return;

			CoTExt.cs.sendMessageToCSS({
				'name' : 'playVod',
				'data' : {
					'speed' : parseInt(speed),
					'loop' : isLoop() ? 'Y' : 'N'
				}
			});
		};


		var seek = function(pos) {
			if (videoObj === null || !isOpen)
				return;

			CoTExt.cs.sendMessageToCSS({
				'name' : 'seekVod',
				'data' : {
					'pos' : parseInt(pos)
				}
			});
		};


		var stop = function(forceFlag) {
			if (isOpen || forceFlag || vodInfo) {
				isOpen = false;
				CoTExt.cs.sendMessageToCSS({
					'name' : 'stopVod',
					'data' : {}
				});
			}
		};


		var repeat = function() {
			if (videoObj === null || !isOpen)
				return;

			CoTExt.cs.sendMessageToCSS({
				'name' : 'repeatVod',
				'data' : {}
			});
		};


		var onEvent = function(event) {
			if (videoObj === null) {
				if (vodInfo !== null)
					return;
				console.info('[CoTExt.video.onEvent] videoObj is null => call stopVoD');
				stop();
				return;
			}

			var video;
			var doc = document;
			if (isVideoTag() || isAudioTag()) {
				if (videoObj.id !== null && videoObj.id !== '')
					video = doc.getElementById(videoObj.id);
				else
					video = videoObj;

				switch (event.type) {
				case 'onVodPlayPosition' :
					video._currentTime = Math.ceil(event.data.playPosition / 1000);
					video.dispatchEvent(new Event('timeupdate'));
					break;

				case 'onVodPlayTime' :
					video._duration = Math.ceil(event.data.playTime / 1000);
					video.dispatchEvent(new Event('durationchange'));
					break;

				case 'onVodPlayState' :
					videoObj._playState = Number(event.data.playState);
					switch (videoObj._playState) {
					case 0 : // stopped
						//isOpen = false;
						break;

					case 1 : // playing
						video._paused = false;
						video._ended = false;
						video._readyState = 4;
						video._networkState = 1;
						video._seeking = false;
						//video.dispatchEvent(new Event('canplay'));
						video.dispatchEvent(new Event('playing'));
						video.dispatchEvent(new Event('play'));
						break;

					case 2 : // paused
						video._paused = true;
						video._ended = false;
						video._readyState = 4;
						video._networkState = 1;
						video._seeking = false;
						//video.dispatchEvent(new Event('canplay'));
						video.dispatchEvent(new Event('pause'));
						break;

					case 3 : // connecting
						break;

					case 4 : // buffering
						video._paused = true;
						video._ended = false;
						video._readyState = 4;
						video._networkState = 2;
						video._seeking = true;
						video.dispatchEvent(new Event('loadstart'));
						//video.dispatchEvent(new Event('pause'));
						break;

					case 5 : // finished
						video._paused = true;
						video._ended = true;
						video._readyState = 4;
						video._networkState = 1;
						video._seeking = false;
						video.dispatchEvent(new Event('ended'));
						if (!isLoop())
							stop();
						else
							repeat();
						break;

					case 6 : // error
						video._paused = true;
						video._ended = false;
						video._readyState = 4;
						video._networkState = 1;
						video._seeking = false;

						if (event.data.error === null)
							event.data.error = 2;			// MEDIA_ERR_NETWORK - error occurred when downloading
						video._error = Number(event.data.error);
						video.dispatchEvent(new Event('error'));
						break;
					}
					break;
				}
			}
			else {			// Object tag
				video = videoObj;

				switch (event.type) {
				case 'onVodPlayPosition' :
					video.playPosition = Number(event.data.playPosition);
					break;

				case 'onVodPlayTime' :
					video.playTime = Number(event.data.playTime);
					break;

				case 'onVodPlayState' :
					if (video.playState == 1 && (Number(event.data.playState) === 3 || Number(event.data.playState) === 4))
						break;

					video.playState = Number(event.data.playState);
					if (video.playState === 6) {
						if (event.data.error === null)
							event.data.error = 5;		// content not available.

						switch (Number(event.data.error)) {
						case 1 :			// MEDIA_ERR_ABORTED - fetching process aborted by user
							video.error = 4;			// content corrupt or invalid.
							break;

						case 2 :			// MEDIA_ERR_NETWORK - error occurred when downloading
							video.error = 1;			// cannot connect to server or connection lost.
							break;

						case 3 :			// MEDIA_ERR_DECODE - error occurred when decoding
							video.error = 5;			// content not available.
							break;

						case 4 :			// MEDIA_ERR_SRC_NOT_SUPPORTED - audio/video not supported
							video.error = 0;			// A/V format not supported.
							break;

						default :
							video.error = 2;			// unidentified error.
							break;
						}
					}

					if (typeof video.onPlayStateChange === 'function')
						video.onPlayStateChange();

					break;
				}
			}
		};


		var isVideoTag = function() {
			return (videoObj === null) ? null : (videoObj.nodeName.toUpperCase() === 'VIDEO');
		};


		var isAudioTag = function() {
			return (videoObj === null) ? null : (videoObj.nodeName.toUpperCase() === 'AUDIO');
		};


		var getVideoSize = function() {
			if (videoObj === null)
				return null;

			var size = {'width' : '0', 'height' : '0', 'left' : '0', 'top' : '0'};

			var rect;
			if (isVideoTag()) {
				if (videoObj.id)
					rect = document.getElementById(videoObj.id).getBoundingClientRect();
				else
					rect = videoObj.getBoundingClientRect();

				size.width = CoTExt.util.convertSize(videoObj.getAttribute('width'), rect.width);
				size.height = CoTExt.util.convertSize(videoObj.getAttribute('height'), rect.height);
				size.left = rect.left + videoObj.scrollLeft;
				size.top = rect.top + videoObj.scrollTop;
			} 
			else if (isAudioTag()) {
				// size is '0'
			}
			else {
				if (!videoObj._isAudio()) {
					var obj = document.getElementById(CoTExt.properties['objectDumyVideo']);
					rect = obj.getBoundingClientRect();

					if (rect.width == 0 && rect.height == 0) {
						rect = videoObj.getBoundingClientRect();
						size.width = CoTExt.util.convertSize(videoObj.getAttribute('width'), rect.width);
						size.height = CoTExt.util.convertSize(videoObj.getAttribute('height'), rect.height);
						size.left = rect.left + videoObj.scrollLeft;
						size.top = rect.top + videoObj.scrollTop;
					}
					else {
						size.width = rect.width;
						size.height = rect.height;
						size.left = rect.left + obj.scrollLeft;
						size.top = rect.top + obj.scrollTop;
					}

					videoObj.fullScreen = !!(size.width == screen.width && size.height == screen.height);
				}
			}

			// 정수값으로 전달하기 위해 보정
			var ratio = 1;
			//var ratio = window.innerWidth / screen.width;
			size.width = Math.round(Number(size.width) * ratio);
			size.height = Math.round(Number(size.height) * ratio);
			size.left = Math.round(Number(size.left) * ratio) + (isCoTYoutubePlayer ? window.parent.document.getElementById('CoT_youtube_player').offsetLeft : 0);
			size.top = Math.round(Number(size.top) * ratio) + (isCoTYoutubePlayer ? window.parent.document.getElementById('CoT_youtube_player').offsetTop : 0);

			// full size인 경우 left, top을 0으로 변경
			if (size.width == screen.width && size.height == screen.height) {
				size.left = 0;
				size.top = 0;
			}

			// Youtube preview인 경우 size 보정
			if (isCoTYoutubePlayer && size.width != 1280) {
				size.width += 10;
				size.height += 7;
			}

			return size;
		};


		var setVodInfo = function(info) {
			console.info('[CoTExt.video.setVodInfo]', info);
			if (info === null) {
				vodInfo = null;
			}
			else {
				vodInfo = {
					'apply' : false,
					'playState' : info.playState,
					'url' : info.url,
					'playTime' : info.playTime,
					'playPosition' : info.playPosition
				};
				setTimeout(() => {CoTExt.video.setVodInfo(null);}, 1000);
			}
		};


		var isLoop = function() {
			return !!(videoObj !== null && videoObj.hasAttribute('loop'));
		};


		return {
			'init' : init,
			'findVideo' : findVideo,
			'setVideo' : setVideo,
			'setVideoData' : setVideoData,
			'open' : open,
			'play' : play,
			'seek' : seek,
			'stop' : stop,
			'onEvent' : onEvent,
			'videoObj' : videoObj,
			'setVodInfo' : setVodInfo
		};
	})();
	// CoTExt.video end
	/*****************************************************************************/



	if (!isCoTYoutubePlayer) {
		/*****************************************************************************/
		// CoTExt.object start
		CoTExt.object = (function(){
			var objBroadcast = null;
			var observerResize = new MutationObserver(function(){});

			var init = function() {
				findObject();

				// video/broadcast object가 없을 경우 live broadcast를 종료한다.
				if (objBroadcast === null)
					CoTExt.api.stopBroadcast();
			};


			var findObject = function() {
				// check object tag
				var obj = document.getElementsByTagName('object');
				if (obj && obj.length > 0) {
					for (var i = 0; i < obj.length; i++) {
						try {
							var type = obj[i].getAttribute('type');
							if (CoTExt.properties['objectType'][type] || CoTExt.properties['mediaMimeType'].indexOf(type) > -1)
								setObject(type, obj[i]);
							else
								console.info('%c[CoTExt.object.findObject] type', 'color:red', type);
						}
						catch(e) {
							console.info('%c[CoTExt.object.findObject] catch: ' + e.toString(), 'color:red');
						}
					}
				}
			};


			var setObject = function(type, object) {
				console.info('[CoTExt.object.setObject]', type, object);

				// check object for video
				if (CoTExt.properties['mediaMimeType'].indexOf(type) > -1) {
					object._setAttribute('type', 'media/CoT');
					if (typeof object.play !== 'function') {
						for (var attr in CoTExt.OIPF['media/CoT']) {
							if (!object[attr])
								object[attr] = CoTExt.OIPF['media/CoT'][attr];
						}
					}
					object._oriType = type;
					CoTExt.video.setVideo(object);
				}

				if (CoTExt.OIPF[type]) {
					if (type === 'application/oipfConfiguration')
						CoTExt.OIPF[type]['configuration'] = new CoTExt.OIPF_CLASS['Configuration']();

					for (var attr in CoTExt.OIPF[type]) {
						if (!object[attr])
							object[attr] = CoTExt.OIPF[type][attr];
					}
				}

				if (CoTExt.properties['existBroadcast']) {
					if (type === 'video/broadcast') {
						objBroadcast = object;
						objBroadcast.style.opacity = 0;
						object._liveBind();

						// check broadcast resize
						observerResize.disconnect();
						observerResize = new MutationObserver(function(mutations) {
							mutations.forEach(function(mutation) {
								if (mutation.attributeName === 'class' || mutation.attributeName === 'style' || mutation.attributeName === 'width') {
									object._liveBind();
								}
							});
						});
						observerResize.observe(object, {attributes: true, attributeOldValue: false});
					}
				}
			};


			return {
				'init' : init,
				'setObject' : setObject,
				'objBroadcast' : objBroadcast
			};
		})();
		// CoTExt.object end
		/*****************************************************************************/
	} // if (!isCoTYoutubePlayer) {



	/*****************************************************************************/
	// CoTExt.util start
	CoTExt.util = (function(){
		(function(e,r){"object"==typeof exports?module.exports=r():"function"==typeof define&&define.amd?define(r):e.GibberishAES=r()})(this,function(){"use strict";var e=14,r=8,n=!1,f=function(e){try{return unescape(encodeURIComponent(e))}catch(r){throw"Error on UTF-8 encode"}},c=function(e){try{return decodeURIComponent(escape(e))}catch(r){throw"Bad Key"}},t=function(e){var r,n,f=[];for(16>e.length&&(r=16-e.length,f=[r,r,r,r,r,r,r,r,r,r,r,r,r,r,r,r]),n=0;e.length>n;n++)f[n]=e[n];return f},a=function(e,r){var n,f,c="";if(r){if(n=e[15],n>16)throw"Decryption error: Maybe bad key";if(16===n)return"";for(f=0;16-n>f;f++)c+=String.fromCharCode(e[f])}else for(f=0;16>f;f++)c+=String.fromCharCode(e[f]);return c},o=function(e){var r,n="";for(r=0;e.length>r;r++)n+=(16>e[r]?"0":"")+e[r].toString(16);return n},d=function(e){var r=[];return e.replace(/(..)/g,function(e){r.push(parseInt(e,16))}),r},u=function(e,r){var n,c=[];for(r||(e=f(e)),n=0;e.length>n;n++)c[n]=e.charCodeAt(n);return c},i=function(n){switch(n){case 128:e=10,r=4;break;case 192:e=12,r=6;break;case 256:e=14,r=8;break;default:throw"Invalid Key Size Specified:"+n}},b=function(e){var r,n=[];for(r=0;e>r;r++)n=n.concat(Math.floor(256*Math.random()));return n},h=function(n,f){var c,t=e>=12?3:2,a=[],o=[],d=[],u=[],i=n.concat(f);for(d[0]=L(i),u=d[0],c=1;t>c;c++)d[c]=L(d[c-1].concat(i)),u=u.concat(d[c]);return a=u.slice(0,4*r),o=u.slice(4*r,4*r+16),{key:a,iv:o}},l=function(e,r,n){r=S(r);var f,c=Math.ceil(e.length/16),a=[],o=[];for(f=0;c>f;f++)a[f]=t(e.slice(16*f,16*f+16));for(0===e.length%16&&(a.push([16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16]),c++),f=0;a.length>f;f++)a[f]=0===f?x(a[f],n):x(a[f],o[f-1]),o[f]=s(a[f],r);return o},v=function(e,r,n,f){r=S(r);var t,o=e.length/16,d=[],u=[],i="";for(t=0;o>t;t++)d.push(e.slice(16*t,16*(t+1)));for(t=d.length-1;t>=0;t--)u[t]=p(d[t],r),u[t]=0===t?x(u[t],n):x(u[t],d[t-1]);for(t=0;o-1>t;t++)i+=a(u[t]);return i+=a(u[t],!0),f?i:c(i)},s=function(r,f){n=!1;var c,t=M(r,f,0);for(c=1;e+1>c;c++)t=g(t),t=y(t),e>c&&(t=k(t)),t=M(t,f,c);return t},p=function(r,f){n=!0;var c,t=M(r,f,e);for(c=e-1;c>-1;c--)t=y(t),t=g(t),t=M(t,f,c),c>0&&(t=k(t));return t},g=function(e){var r,f=n?D:B,c=[];for(r=0;16>r;r++)c[r]=f[e[r]];return c},y=function(e){var r,f=[],c=n?[0,13,10,7,4,1,14,11,8,5,2,15,12,9,6,3]:[0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11];for(r=0;16>r;r++)f[r]=e[c[r]];return f},k=function(e){var r,f=[];if(n)for(r=0;4>r;r++)f[4*r]=F[e[4*r]]^R[e[1+4*r]]^j[e[2+4*r]]^z[e[3+4*r]],f[1+4*r]=z[e[4*r]]^F[e[1+4*r]]^R[e[2+4*r]]^j[e[3+4*r]],f[2+4*r]=j[e[4*r]]^z[e[1+4*r]]^F[e[2+4*r]]^R[e[3+4*r]],f[3+4*r]=R[e[4*r]]^j[e[1+4*r]]^z[e[2+4*r]]^F[e[3+4*r]];else for(r=0;4>r;r++)f[4*r]=E[e[4*r]]^U[e[1+4*r]]^e[2+4*r]^e[3+4*r],f[1+4*r]=e[4*r]^E[e[1+4*r]]^U[e[2+4*r]]^e[3+4*r],f[2+4*r]=e[4*r]^e[1+4*r]^E[e[2+4*r]]^U[e[3+4*r]],f[3+4*r]=U[e[4*r]]^e[1+4*r]^e[2+4*r]^E[e[3+4*r]];return f},M=function(e,r,n){var f,c=[];for(f=0;16>f;f++)c[f]=e[f]^r[n][f];return c},x=function(e,r){var n,f=[];for(n=0;16>n;n++)f[n]=e[n]^r[n];return f},S=function(n){var f,c,t,a,o=[],d=[],u=[];for(f=0;r>f;f++)c=[n[4*f],n[4*f+1],n[4*f+2],n[4*f+3]],o[f]=c;for(f=r;4*(e+1)>f;f++){for(o[f]=[],t=0;4>t;t++)d[t]=o[f-1][t];for(0===f%r?(d=m(w(d)),d[0]^=K[f/r-1]):r>6&&4===f%r&&(d=m(d)),t=0;4>t;t++)o[f][t]=o[f-r][t]^d[t]}for(f=0;e+1>f;f++)for(u[f]=[],a=0;4>a;a++)u[f].push(o[4*f+a][0],o[4*f+a][1],o[4*f+a][2],o[4*f+a][3]);return u},m=function(e){for(var r=0;4>r;r++)e[r]=B[e[r]];return e},w=function(e){var r,n=e[0];for(r=0;4>r;r++)e[r]=e[r+1];return e[3]=n,e},A=function(e,r){var n,f=[];for(n=0;e.length>n;n+=r)f[n/r]=parseInt(e.substr(n,r),16);return f},C=function(e){var r,n=[];for(r=0;e.length>r;r++)n[e[r]]=r;return n},I=function(e,r){var n,f;for(f=0,n=0;8>n;n++)f=1===(1&r)?f^e:f,e=e>127?283^e<<1:e<<1,r>>>=1;return f},O=function(e){var r,n=[];for(r=0;256>r;r++)n[r]=I(e,r);return n},B=A("637c777bf26b6fc53001672bfed7ab76ca82c97dfa5947f0add4a2af9ca472c0b7fd9326363ff7cc34a5e5f171d8311504c723c31896059a071280e2eb27b27509832c1a1b6e5aa0523bd6b329e32f8453d100ed20fcb15b6acbbe394a4c58cfd0efaafb434d338545f9027f503c9fa851a3408f929d38f5bcb6da2110fff3d2cd0c13ec5f974417c4a77e3d645d197360814fdc222a908846eeb814de5e0bdbe0323a0a4906245cc2d3ac629195e479e7c8376d8dd54ea96c56f4ea657aae08ba78252e1ca6b4c6e8dd741f4bbd8b8a703eb5664803f60e613557b986c11d9ee1f8981169d98e949b1e87e9ce5528df8ca1890dbfe6426841992d0fb054bb16",2),D=C(B),K=A("01020408102040801b366cd8ab4d9a2f5ebc63c697356ad4b37dfaefc591",2),E=O(2),U=O(3),z=O(9),R=O(11),j=O(13),F=O(14),G=function(e,r,n){var f,c=b(8),t=h(u(r,n),c),a=t.key,o=t.iv,d=[[83,97,108,116,101,100,95,95].concat(c)];return e=u(e,n),f=l(e,a,o),f=d.concat(f),T.encode(f)},H=function(e,r,n){var f=T.decode(e),c=f.slice(8,16),t=h(u(r,n),c),a=t.key,o=t.iv;return f=f.slice(16,f.length),e=v(f,a,o,n)},L=function(e){function r(e,r){return e<<r|e>>>32-r}function n(e,r){var n,f,c,t,a;return c=2147483648&e,t=2147483648&r,n=1073741824&e,f=1073741824&r,a=(1073741823&e)+(1073741823&r),n&f?2147483648^a^c^t:n|f?1073741824&a?3221225472^a^c^t:1073741824^a^c^t:a^c^t}function f(e,r,n){return e&r|~e&n}function c(e,r,n){return e&n|r&~n}function t(e,r,n){return e^r^n}function a(e,r,n){return r^(e|~n)}function o(e,c,t,a,o,d,u){return e=n(e,n(n(f(c,t,a),o),u)),n(r(e,d),c)}function d(e,f,t,a,o,d,u){return e=n(e,n(n(c(f,t,a),o),u)),n(r(e,d),f)}function u(e,f,c,a,o,d,u){return e=n(e,n(n(t(f,c,a),o),u)),n(r(e,d),f)}function i(e,f,c,t,o,d,u){return e=n(e,n(n(a(f,c,t),o),u)),n(r(e,d),f)}function b(e){for(var r,n=e.length,f=n+8,c=(f-f%64)/64,t=16*(c+1),a=[],o=0,d=0;n>d;)r=(d-d%4)/4,o=8*(d%4),a[r]=a[r]|e[d]<<o,d++;return r=(d-d%4)/4,o=8*(d%4),a[r]=a[r]|128<<o,a[t-2]=n<<3,a[t-1]=n>>>29,a}function h(e){var r,n,f=[];for(n=0;3>=n;n++)r=255&e>>>8*n,f=f.concat(r);return f}var l,v,s,p,g,y,k,M,x,S=[],m=A("67452301efcdab8998badcfe10325476d76aa478e8c7b756242070dbc1bdceeef57c0faf4787c62aa8304613fd469501698098d88b44f7afffff5bb1895cd7be6b901122fd987193a679438e49b40821f61e2562c040b340265e5a51e9b6c7aad62f105d02441453d8a1e681e7d3fbc821e1cde6c33707d6f4d50d87455a14eda9e3e905fcefa3f8676f02d98d2a4c8afffa39428771f6816d9d6122fde5380ca4beea444bdecfa9f6bb4b60bebfbc70289b7ec6eaa127fad4ef308504881d05d9d4d039e6db99e51fa27cf8c4ac5665f4292244432aff97ab9423a7fc93a039655b59c38f0ccc92ffeff47d85845dd16fa87e4ffe2ce6e0a30143144e0811a1f7537e82bd3af2352ad7d2bbeb86d391",8);for(S=b(e),y=m[0],k=m[1],M=m[2],x=m[3],l=0;S.length>l;l+=16)v=y,s=k,p=M,g=x,y=o(y,k,M,x,S[l+0],7,m[4]),x=o(x,y,k,M,S[l+1],12,m[5]),M=o(M,x,y,k,S[l+2],17,m[6]),k=o(k,M,x,y,S[l+3],22,m[7]),y=o(y,k,M,x,S[l+4],7,m[8]),x=o(x,y,k,M,S[l+5],12,m[9]),M=o(M,x,y,k,S[l+6],17,m[10]),k=o(k,M,x,y,S[l+7],22,m[11]),y=o(y,k,M,x,S[l+8],7,m[12]),x=o(x,y,k,M,S[l+9],12,m[13]),M=o(M,x,y,k,S[l+10],17,m[14]),k=o(k,M,x,y,S[l+11],22,m[15]),y=o(y,k,M,x,S[l+12],7,m[16]),x=o(x,y,k,M,S[l+13],12,m[17]),M=o(M,x,y,k,S[l+14],17,m[18]),k=o(k,M,x,y,S[l+15],22,m[19]),y=d(y,k,M,x,S[l+1],5,m[20]),x=d(x,y,k,M,S[l+6],9,m[21]),M=d(M,x,y,k,S[l+11],14,m[22]),k=d(k,M,x,y,S[l+0],20,m[23]),y=d(y,k,M,x,S[l+5],5,m[24]),x=d(x,y,k,M,S[l+10],9,m[25]),M=d(M,x,y,k,S[l+15],14,m[26]),k=d(k,M,x,y,S[l+4],20,m[27]),y=d(y,k,M,x,S[l+9],5,m[28]),x=d(x,y,k,M,S[l+14],9,m[29]),M=d(M,x,y,k,S[l+3],14,m[30]),k=d(k,M,x,y,S[l+8],20,m[31]),y=d(y,k,M,x,S[l+13],5,m[32]),x=d(x,y,k,M,S[l+2],9,m[33]),M=d(M,x,y,k,S[l+7],14,m[34]),k=d(k,M,x,y,S[l+12],20,m[35]),y=u(y,k,M,x,S[l+5],4,m[36]),x=u(x,y,k,M,S[l+8],11,m[37]),M=u(M,x,y,k,S[l+11],16,m[38]),k=u(k,M,x,y,S[l+14],23,m[39]),y=u(y,k,M,x,S[l+1],4,m[40]),x=u(x,y,k,M,S[l+4],11,m[41]),M=u(M,x,y,k,S[l+7],16,m[42]),k=u(k,M,x,y,S[l+10],23,m[43]),y=u(y,k,M,x,S[l+13],4,m[44]),x=u(x,y,k,M,S[l+0],11,m[45]),M=u(M,x,y,k,S[l+3],16,m[46]),k=u(k,M,x,y,S[l+6],23,m[47]),y=u(y,k,M,x,S[l+9],4,m[48]),x=u(x,y,k,M,S[l+12],11,m[49]),M=u(M,x,y,k,S[l+15],16,m[50]),k=u(k,M,x,y,S[l+2],23,m[51]),y=i(y,k,M,x,S[l+0],6,m[52]),x=i(x,y,k,M,S[l+7],10,m[53]),M=i(M,x,y,k,S[l+14],15,m[54]),k=i(k,M,x,y,S[l+5],21,m[55]),y=i(y,k,M,x,S[l+12],6,m[56]),x=i(x,y,k,M,S[l+3],10,m[57]),M=i(M,x,y,k,S[l+10],15,m[58]),k=i(k,M,x,y,S[l+1],21,m[59]),y=i(y,k,M,x,S[l+8],6,m[60]),x=i(x,y,k,M,S[l+15],10,m[61]),M=i(M,x,y,k,S[l+6],15,m[62]),k=i(k,M,x,y,S[l+13],21,m[63]),y=i(y,k,M,x,S[l+4],6,m[64]),x=i(x,y,k,M,S[l+11],10,m[65]),M=i(M,x,y,k,S[l+2],15,m[66]),k=i(k,M,x,y,S[l+9],21,m[67]),y=n(y,v),k=n(k,s),M=n(M,p),x=n(x,g);return h(y).concat(h(k),h(M),h(x))},T=function(){var e="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",r=e.split(""),n=function(e){var n,f,c=[],t="";for(Math.floor(16*e.length/3),n=0;16*e.length>n;n++)c.push(e[Math.floor(n/16)][n%16]);for(n=0;c.length>n;n+=3)t+=r[c[n]>>2],t+=r[(3&c[n])<<4|c[n+1]>>4],t+=void 0!==c[n+1]?r[(15&c[n+1])<<2|c[n+2]>>6]:"=",t+=void 0!==c[n+2]?r[63&c[n+2]]:"=";for(f=t.slice(0,64)+"\n",n=1;Math.ceil(t.length/64)>n;n++)f+=t.slice(64*n,64*n+64)+(Math.ceil(t.length/64)===n+1?"":"\n");return f},f=function(r){r=r.replace(/\n/g,"");var n,f=[],c=[],t=[];for(n=0;r.length>n;n+=4)c[0]=e.indexOf(r.charAt(n)),c[1]=e.indexOf(r.charAt(n+1)),c[2]=e.indexOf(r.charAt(n+2)),c[3]=e.indexOf(r.charAt(n+3)),t[0]=c[0]<<2|c[1]>>4,t[1]=(15&c[1])<<4|c[2]>>2,t[2]=(3&c[2])<<6|c[3],f.push(t[0],t[1],t[2]);return f=f.slice(0,f.length-f.length%16)};return"function"==typeof Array.indexOf&&(e=r),{encode:n,decode:f}}();return{size:i,h2a:d,expandKey:S,encryptBlock:s,decryptBlock:p,Decrypt:n,s2a:u,rawEncrypt:l,rawDecrypt:v,dec:H,openSSLKey:h,a2h:o,enc:G,Hash:{MD5:L},Base64:T}});

		var encrypt = function(data) {
			try {
				return GibberishAES.enc(data, CoTExt.properties['ENC_KEY']);
			}
			catch(e) {console.error('[CoTExt.util.encrypt] error', e);}
			return null;
		};
		

		var decrypt = function(data) {
			try {
				return GibberishAES.dec(data, CoTExt.properties['ENC_KEY']);
			}
			catch(e) {console.error('[CoTExt.util.decrypt] error', e);}
			return null;
		};

		var convertSize = function(attr, rect) {
			if (attr && attr.indexOf('%') < 0)
				return attr.toString().toUpperCase().replace('PX','') ;
			else
				return rect;
		};

		var fireKeyDown = function(key) {
			var doc = document;
			var eventObj = doc.createEvent('Events');
			eventObj.initEvent('keydown', true, true);
			eventObj.which = key;
			eventObj.keyCode = key;
			doc.dispatchEvent(eventObj);
		};

		var fireKeyPress = function(key) {
			var doc = document;
			var eventObj = doc.createEvent('Events');
			eventObj.initEvent('keypress', true, true);
			eventObj.which = key;
			eventObj.keyCode = key;
			doc.dispatchEvent(eventObj);
		};

		return {
			'convertSize' : convertSize,
			'fireKeyDown' : fireKeyDown,
			'fireKeyPress' : fireKeyPress,
			'encrypt' : encrypt,
			'decrypt' : decrypt
		};
	})();
	// CoTExt.util end
	/*****************************************************************************/


	/*****************************************************************************/
	// CoTExt.main start
	CoTExt.main = (function(){

		var init = function() {
			var doc = document;
			console.info('[CoTExt.main.init]', doc.location.toString());
			if (!isCoTYoutubePlayer) {
				clearHistory();
				doc.body.style.width = screen.width + 'px';
				doc.body.style.height = screen.height + 'px';
				doc.body.style.overflow = 'hidden';
				CoTExt.object.init();
			}
			CoTExt.video.init();

//			if (isCoTYoutubePlayer)
			if (isCoTYoutubePlayer || isHbbTV())
				observeDOMNodeInserted();

			doc.addEventListener('keydown', function(e) {
				//console.log(">>>keydown ", e.which);
				switch (e.which) {
				case 46 :		// EXIT for Loewe
				case 34 :		// EXIT for Innopia Dongle
				case 27 :		// EXIT
					//CoTExt.api.launchCSApp({'csType' : 'ICS', 'name' : 'MAIN'});
					CoTExt.api.launchApp({'ip':CoTExt.properties['SERVER']['ICS'], 'port':5000, 'name':'MAIN', 'csType':'ICS'});
					break;
				case 220 :		// Andriod Menu
					if (isHbbTV())
						CoTExt.util.fireKeyDown(116);
					break;
				}
			}, true);

			doc.addEventListener('keyup', function(e) {
				//console.log(">>>keyup ", e.which);
				CoTExt.util.fireKeyPress(e.which);
			}, true);
		};


		var clearHistory = function() {
			window.history.pushState(null, null, window.location.href);
			window.onpopstate = function() {
				window.history.go(1);
			};
		};

		var isHbbTV = function() {
			var url = document.location.toString();
			return (url != CoTExt.properties['URL']['MAIN'] && url != CoTExt.properties['URL']['NOW_TV'] 
				&& url != CoTExt.properties['URL']['SKY_STORE'] && url != CoTExt.properties['URL']['SKY_HOTEL_TV'] 
				&& url != CoTExt.properties['URL']['SKY_MOZART'] );
		}


		var observeDOMNodeInserted = function() {
			var doc = document;
			doc.addEventListener('DOMNodeInserted', function(event) {
				if (CoTExt.video.videoObj !== null)
					return;

				//console.info('DOMNodeInserted', event.srcElement.nodeName, event.srcElement);
				switch (event.srcElement.nodeName.toUpperCase()) {
				case 'VIDEO' :
					if (event.srcElement.id === CoTExt.properties['objectDumyVideo'] || event.srcElement.id === CoTExt.properties['objectDumyBroadcast'])
						break;
					// else일 경우 아래 구문 실행됨.
				case 'AUDIO' :
					var tagList = ['video', 'audio'];
					for (var i = 0; i < tagList.length; i++) {
						var videos = doc.getElementsByTagName(tagList[i]);
						if (videos && videos.length > 0) {
							//console.info('[CoTExt.main.observeDOMNodeInserted] findVideo DOMNodeInserted videos.length : ' + videos.length);
							setTimeout(() => {CoTExt.video.setVideo(videos[0]);}, 50);
							break;
						}
					}
					break;

				case 'SOURCE' :
					var source = doc.getElementsByTagName('source');
					for (var i = 0; i < source.length; i++) {
						var src = source[i].getAttribute('src');
						if (src !== null && src !== '') {
							CoTExt.video.setVideoData(src);
							break;
						}
					}
					break;

				case 'OBJECT' :
					CoTExt.object.setObject(event.srcElement.type, event.srcElement);
					break;

				case 'DIV' :
					if (event.srcElement.innerHTML) {
						var html = event.srcElement.innerHTML.toUpperCase();
						if (html.indexOf('<VIDEO') >= 0) {
							console.info('>>>>>>>>> event.srcElement.innerHTML', event.srcElement.innerHTML);
							CoTExt.video.findVideo();
						}
						else if (html.indexOf('<OBJECT') >= 0) {
							console.info('%c>>>todo event.srcElement.innerHTML', 'color:red;', event.srcElement.innerHTML);
						}
					}
					break;
				}
			}, false);
		};


		var ait={},isZDF=function(){return'http://hbbtv.zdf.de'==window.location.origin},initZDF=function(){isZDF()&&(ait.ZDF={'//current.ait/11.1':'http://hbbtv.zdf.de/zdfstart/index.php','//current.ait/11.2':'http://hbbtv.zdf.de/zdfm/index.php','//current.ait/11.6':'http://hbbtv.zdf.de/zdfepg/index.php','//current.ait/11.7':'http://hbbtv.zdf.de/zdfnews/index.php'})},getAITUrl=function(a){var b=document.createElement('a');b.href=a;var c=ait.ZDF[b.pathname];return c?c+b.hash+(c.indexOf('?')>0?'&':'?')+b.search.substring(1):null};


		return {
			'init' : init
		};
	})();
	// CoTExt.main start
	/*****************************************************************************/



	if (!isCoTYoutubePlayer) {
		/*****************************************************************************/
		// CoTExt.cs Event define
		CoTExt.cs.receiveMessageFromCSS({
			'name' : 'onMove',
			'callback' : function(data) {
/*
				if (data.name === 'MAIN' || data.name === CoTExt.properties['SERVICE_ID']) {
					if (data.vodInfo !== undefined && data.vodInfo.playState !== undefined)
						CoTExt.video.setVodInfo(data.vodInfo);

					if (typeof CoTExt.api.eventListener['onMove'] === 'function') {
						CoTExt.api.eventListener['onMove']({
							'history' : data.history
						});
						CoTExt.api.setServiceInfo(CoTExt.properties['SERVICE_ID']);
						CoTExt.api.setTimeout(CoTExt.properties['timeout']);
					}
				}
				else {
*/
					var url;
					if (data.name.startsWith('http'))
						url = data.name;
					else
						url = CoTExt.properties['URL'][data.name] || CoTExt.properties['URL']['MAIN'];

					CoTExt.api.setServiceInfo(CoTExt.properties['SERVICE_ID']);
					setTimeout(() => {window.location.href = url;}, 10);
//				}
			}
		});

		CoTExt.cs.receiveMessageFromCSS({
			'name' : 'onVodPlayState',
			'callback' : function(data) {
				var cotExt = CoTExt;
				if (CoTExt.CoTYoutubePlayer)
					cotExt = CoTExt.CoTYoutubePlayer.CoTExt;
				cotExt.video.onEvent({
					'type' : 'onVodPlayState',
					'data' : data
				});
			}
		});

		CoTExt.cs.receiveMessageFromCSS({
			'name' : 'onVodPlayPosition',
			'callback' : function(data) {
				var cotExt = CoTExt;
				if (CoTExt.CoTYoutubePlayer)
					cotExt = CoTExt.CoTYoutubePlayer.CoTExt;
				cotExt.video.onEvent({
					'type' : 'onVodPlayPosition',
					'data' : data
				});
			}
		});

		CoTExt.cs.receiveMessageFromCSS({
			'name' : 'onVodPlayTime',
			'callback' : function(data) {
				var cotExt = CoTExt;
				if (CoTExt.CoTYoutubePlayer)
					cotExt = CoTExt.CoTYoutubePlayer.CoTExt;
				cotExt.video.onEvent({
					'type' : 'onVodPlayTime',
					'data' : data
				});
			}
		});

		CoTExt.cs.receiveMessageFromCSS({
			'name' : 'onTimeout',
			'callback' : function() {
				if (typeof CoTExt.api.eventListener['onTimeout'] === 'function')
					CoTExt.api.eventListener['onTimeout']();
			}
		});
		// CoTExt.cs Event define
		/*****************************************************************************/



		/*****************************************************************************/
		// CoTExt.api
		CoTExt.api.callback = {};
		CoTExt.api.eventListener = {};

		CoTExt.api.stopBroadcast = function() {
			if (!CoTExt.properties['existBroadcast'])
				return;

			CoTExt.cs.sendMessageToCSS({
				'name' : 'stopLive',
				'data' : {}
			});
		};

		CoTExt.api.closeSession = function() {
			CoTExt.cs.sendMessageToCSS({
				'name' : 'closeSession',
				'data' : {}
			});
		};

		CoTExt.api.setHistory = function(data, serviceId) {
			CoTExt.cs.sendMessageToCSS({
				'name' : 'setHistory',
				'data' : {
					name : (serviceId != undefined ? serviceId : CoTExt.properties['SERVICE_ID']),
					history : data
				}
			});
		};

		CoTExt.api.setTimeout = function(data) {
			CoTExt.cs.sendMessageToCSS({
				'name' : 'setTimeout',
				'data' : {
					'time' : parseInt(data)
				}
			});
		};

		// data = {csType:'ICS', ip:'127.0.0.1', port:'3390', name:'MAIN'}
		CoTExt.api.launchApp = function(data) {
			data.csType = data.csType === 'ICS' ? 'ICS' : 'VCS';
			CoTExt.api.setTimeout(0);
			CoTExt.cs.sendMessageToCSS({
				'name' : 'launchApp',
				'data' : {
					'csType' : data.csType,
					'ip' : data.ip,
					'port' : parseInt(data.port),
					'name' : data.name || ''
				}
			});
		};

		// data = {csType;'ICS', appId:'1', regionCode:'KOR'}
		CoTExt.api.launchCSApp = function(data) {
			CoTExt.cs.sendMessageToCSS({
				'name' : 'launchCSApp',
				'data' : {
					'csType' : data.csType || 'ICS',
					'appId' : data.appId,
					'name' : data.name || ''
				}
			});
		};

		CoTExt.api.startAnimation = function(obj) {};

		CoTExt.api.getUserInfo = function(cb, serviceId) {
			CoTExt.cs.sendMessageToCSS({
				'name' : 'getUserInfo',
				'data' : {
					name : (serviceId != undefined ? serviceId : CoTExt.properties['SERVICE_ID']),
				}
			});
			CoTExt.api.callback['getUserInfo'] = cb;
		};
		CoTExt.cs.receiveMessageFromCSS({
			'name' : 'cbGetUserInfo',
			'information' : function(data) {
				var userInfo = {'user' : null, 'pin' : null,'device' : null};
				if (data.userInfo && data.userInfo.length > 0) {
					try {
						userInfo = JSON.parse(CoTExt.util.decrypt(data.userInfo));
					} catch (e) {}
				}
				if (userInfo === null)
					userInfo = {'user' : null, 'pin' : null,'device' : null};

				//console.log('cbGetUserInfo', userInfo);
				if (typeof CoTExt.api.callback['getUserInfo'] === 'function')
					CoTExt.api.callback['getUserInfo'](userInfo);
			}
		});

		CoTExt.api.setUserInfo = function(info, serviceId) {
			var encryptInfo = CoTExt.util.encrypt(JSON.stringify(info));
			if (encryptInfo === null)
				encryptInfo = '';
			CoTExt.cs.sendMessageToCSS({
				'name' : 'setUserInfo',
				'data' : {
					'name' : (serviceId != undefined ? serviceId : CoTExt.properties['SERVICE_ID']),
					'userInfo' : encryptInfo,
					'customerId' : info.user
				}
			});
		};

		CoTExt.api.launchService = function(serviceName) {
			console.info('[CoTExt.api.launchService]', serviceName);
			switch (serviceName) {
			case 'SKY_Q_POC1' :
			case 'SKY_ON_DEMAND' :
			case 'SKY_BIZ_TICKET' :
			case 'SKY_UK_NOWTV_HOTEL' :
				CoTExt.api.setServiceInfo(serviceName);
				CoTExt.api.setHistory('', serviceName);
				CoTExt.api.launchApp({'ip':CoTExt.properties['SERVER'][serviceName], 'port':5000, 'name':'MAIN', 'csType':'ICS'});
				break;
			case 'SKY_VR' :
				break;
			case 'SKY_Q_POC2' :
				CoTExt.api.setServiceInfo(serviceName);
				if (CoTExt.properties['URL'][serviceName] !== undefined)
					window.location.href = CoTExt.properties['URL'][serviceName];
				else
					window.location.href = serviceName;
				break;
			default :
				CoTExt.api.setServiceInfo('ETC');
				if (CoTExt.properties['URL'][serviceName] !== undefined)
					window.location.href = CoTExt.properties['URL'][serviceName];
				else
					window.location.href = serviceName;
				break;
			}
		};

		CoTExt.api.getDeviceId = function(cb) {
			CoTExt.api.getDeviceInfo(function(data) {
				if (typeof cb === 'function')
					cb(data.deviceId);
			});
		}
		CoTExt.api.getDeviceInfo = function(cb) {
			CoTExt.cs.sendMessageToCSS({
				'name' : 'getDeviceInfo',
				'data' : {}
			});
			CoTExt.api.callback['getDeviceInfo'] = cb;
		};
		CoTExt.cs.receiveMessageFromCSS({
			'name' : 'cbGetDeviceInfo',
			'callback' : function(data) {
				console.log('cbGetDeviceInfo', data);
				if (typeof CoTExt.api.callback['getDeviceInfo'] === 'function')
					CoTExt.api.callback['getDeviceInfo'](data);
			}
		});

		CoTExt.api.setServiceInfo = function(data) {
			CoTExt.cs.sendMessageToCSS({
				'name' : 'setServiceInfo',
				'data' : {
					'id' : data
				}
			});
		};

		// evt : 'onTimeout'
		// evt : 'onMove'
		// evt : 'onError'
		CoTExt.api.addEventListener = function(evt, cb) {
			console.info('[CoTExt.api.addEventListener]', evt, cb);
			CoTExt.api.eventListener[evt] = cb;
		};

		// CoTExt.api
		/*****************************************************************************/
	} // if (!isCoTYoutubePlayer) {


	if (document.readyState === 'complete' || document.readyState === 'loaded' || document.readyState === 'interactive')
		CoTExt.main.init();
	else
		document.addEventListener('DOMContentLoaded', CoTExt.main.init, false);
}
