#include "ofMain.h"
#include "ofApp.h"

int main( ){
	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 2);		// set GL version, x, y -> x.y
	settings.setSize(800, 600);		// set size, in pixels, of window
	ofCreateWindow(settings);			// create window with custom settings
	ofRunApp(new ofApp());				// run the app
}
