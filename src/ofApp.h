#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxImGui.h"
#include "ofxCv.h"

enum Mode {
	SCALEDOWN,
	BLOBTRACKER
};

class ofApp : public ofBaseApp{
	public:
		ofApp();
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void sendPixels();
		void sendPosition(int x, int y);

		void onPortChanged(int & value);
		void onHostChanged(std::string & value);
		void onThresholdChanged(int & value);
		void onDeviceIdChanged(int & value);

		ofVideoGrabber _grabber;
		ofPixels _pixels;
		ofxOscSender _oscSender;

		ofParameter<std::string> _host;
		ofParameter<int> _port;
		ofParameter<std::string> _address;

		ofParameter<int> _width;
		ofParameter<int> _height;

		ofxCv::ContourFinder contour;
  		ofParameter<ofColor> _color;
		ofParameter<int> _threshold;
		ofParameter<bool> _trackHueSaturation;
		ofParameter<int> _minArea;
		ofParameter<int> _maxArea;

		std::vector<ofVideoDevice> _videoDevices;
		ofParameter<int> _videoDeviceId;


    	ofxImGui::Gui _gui;

		Mode _mode;
	};
