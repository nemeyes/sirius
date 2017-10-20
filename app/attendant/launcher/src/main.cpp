#include "ofMain.h"
#include "ofApp.h"

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") 
//========================================================================
int main() {
	//ofAppGlutWindow windows;
	//ofSetupOpenGL(&windows, 1280, 1024, OF_WINDOW);			// <-------- setup the GL context
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}
