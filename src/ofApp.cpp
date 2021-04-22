#include "ofApp.h"

ofApp::ofApp()
 {

}

void ofApp::setup(){
    _videoDevices = _grabber.listDevices();
    _videoDeviceId.set("video device ID", 0, 0, _videoDevices.size());
    _videoDeviceId.addListener(this, &ofApp::onDeviceIdChanged);
    _grabber.setup(640, 480);


    contour.setMinAreaRadius(10);
    contour.setMaxAreaRadius(80);
    contour.setSortBySize(true);
    contour.setFindHoles(false);


    _host.addListener(this, &ofApp::onHostChanged);
    _port.addListener(this, &ofApp::onPortChanged);
    _host.set("host", "localhost");
    _port.set("port", 6448, 1000, 50000);
    _address.set("address", "/wek/inputs");

    _width.set("width", 10, 1, 20);
    _height.set("height", 10, 1, 20);

    _color.set("color", ofColor(0, 255, 0));
    _threshold.addListener(this, &ofApp::onThresholdChanged);
    _threshold.set("threshold", 128,0, 255);
    _minArea.set("minArea", 10,0,255);
    _maxArea.set("maxArea", 150,0,255);
    _trackHueSaturation.set("trackHueSaturation", false);


    _pixels.allocate(_width, _height, OF_IMAGE_COLOR);
    _oscSender.setup(_host, _port);
    _gui.setup(nullptr, ImGuiConfigFlags_ViewportsEnable );
}

void ofApp::update(){
    _grabber.update();
    if(_grabber.isFrameNew()){
        _pixels = _grabber.getPixels();
        if(_mode == Mode::SCALEDOWN){
            _pixels.resize(_width, _height);
            sendPixels();
        } else if(_mode == Mode::BLOBTRACKER) {
            contour.setTargetColor(_color, _trackHueSaturation ? ofxCv::TRACK_COLOR_HS : ofxCv::TRACK_COLOR_RGB);
            contour.setMinAreaRadius(_minArea);
            contour.setMaxAreaRadius(_maxArea);
            contour.findContours(_grabber);
        }
    }
}

void ofApp::draw(){
    if(_mode == Mode::SCALEDOWN){
        auto width = 640.0/_width;
        auto height = 480.0/_height;

        ofPushStyle();
        for(auto x = 0; x < _width; x++){
            for(auto y = 0; y < _height; y++){
                ofSetColor(_pixels.getColor(x, y));
                ofDrawRectangle(x * width, y * height, width, height);
            }
        }
        ofPopStyle();
    }
    if(_mode == Mode::BLOBTRACKER){
        _grabber.draw(0, 0, 640, 480);
        contour.draw();
        for(auto i = 0; i < contour.size(); i++){
            ofVec2f center = ofxCv::toOf(contour.getCenter(i));
            ofLogNotice() << "center " <<i<<": "<< center;
            if(i == 0){
                // TODO: normalize
                sendPosition(center.x, center.y);
            }
            ofDrawCircle(center.x, center.y, 10);
        }
    }
    
       //required to call this at beginning
    _gui.begin();
    
    ImGui::SetNextWindowSize( ofVec2f(ofGetWidth()/4, ofGetHeight()));
    ImGui::SetNextWindowPos( ofVec2f( ofGetWidth()/4*3, 0));
    ImGui::Begin("Preferences");
    if (ImGui::CollapsingHeader("Input"))
    {
        std::vector<std::string> labels;
        for(auto & device : _videoDevices){
            labels.push_back(device.deviceName);
        }
        if(_videoDevices.size()){
            if (ImGui::BeginCombo("camera", _videoDevices[_videoDeviceId].deviceName.c_str()))
            {
                for(auto i = 0; i < _videoDevices.size(); i++){
                    if (ImGui::Selectable(_videoDevices[i].deviceName.c_str(), _videoDeviceId == i)){
                        _videoDeviceId = i;
                    }
                    if (_videoDeviceId == i){
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
    }
    if (ImGui::CollapsingHeader("Processing")) {
        std::vector<std::string> modeLabels= {"scole down", "blob tracker"};
        if (ImGui::BeginCombo("mode", modeLabels[_mode].c_str()))
        {
            if (ImGui::Selectable("scale down", _mode == Mode::SCALEDOWN)){
                _mode = Mode::SCALEDOWN;
            }
            if (_mode == Mode::SCALEDOWN){
                ImGui::SetItemDefaultFocus();
            }
            if (ImGui::Selectable("blob tracker", _mode == Mode::BLOBTRACKER)){
                _mode = Mode::BLOBTRACKER;
            }
            if (_mode == Mode::BLOBTRACKER){
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if(_mode == Mode::SCALEDOWN){
            {
                int value = _width;
                if(ImGui::InputInt(_width.getName().c_str(), &value)){
                    _width = value;
                }
            }
            {
                int value = _height;
                if(ImGui::InputInt(_height.getName().c_str(), &value)){
                    _height = value;
                }
            }
        }
        if(_mode == Mode::BLOBTRACKER){
            {
                ofColor color = _color;
                float col[3] = { (float)(color.r)/255, float(color.g)/255, float(color.b)/255};
                if(ImGui::ColorEdit3(_color.getName().c_str(), col)) {
                    _color = ofColor(col[0]*255, col[1]*255, col[2]*255);
                }
            }
            {
                bool value = _trackHueSaturation;
                if(ImGui::Checkbox(_trackHueSaturation.getName().c_str(), &value)) {
                    _trackHueSaturation = value;
                }
            }
            {
                int value = _threshold;
                if(ImGui::InputInt(_threshold.getName().c_str(), &value)){
                    _threshold = ofClamp(value, _threshold.getMin(), _threshold.getMax());
                }
            }
            {
                int value = _minArea;
                if(ImGui::InputInt(_minArea.getName().c_str(), &value)){
                    _minArea = ofClamp(value, _minArea.getMin(), _minArea.getMax());
                }
            }
            {
                int value = _maxArea;
                if(ImGui::InputInt(_maxArea.getName().c_str(), &value)){
                    _maxArea = ofClamp(value, _maxArea.getMin(), _maxArea.getMax());
                }
            }
        }
    }

    if (ImGui::CollapsingHeader("Output"))
    {
        {
            auto value = _host.get();
            char * cString = new char[value.size()];
            strcpy(cString, value.c_str());
            if(ImGui::InputText(_host.getName().c_str(), cString, 200)){
                _host = cString;
            }
        }
        {
            int port = _port;
            if(ImGui::InputInt(_port.getName().c_str(), &port)){
                _port = port;
            }
        }
        {
            auto value = _address.get();
            char * cString = new char[value.size()];
            strcpy(cString, value.c_str());
            if(ImGui::InputText(_address.getName().c_str(), cString, 200)){
                _address = cString;
            }
        }

    }
    ImGui::End();

    // ImGui::ShowDemoWindow();
    _gui.end();
}

void ofApp::keyPressed(int key){}
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){
    if(_mode == Mode::BLOBTRACKER){
        if(x < _grabber.getWidth() && y < _grabber.getHeight()){
            _color = _grabber.getPixels().getColor(x, y);
        }
    }
}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::mouseEntered(int x, int y){}
void ofApp::mouseExited(int x, int y){}
void ofApp::windowResized(int w, int h){}
void ofApp::gotMessage(ofMessage msg){}
void ofApp::dragEvent(ofDragInfo dragInfo){}

void ofApp::sendPixels(){ 
    ofxOscMessage msg;
    msg.setAddress(_address);
    for(auto x = 0; x < _pixels.getWidth(); x++){
        for(auto y = 0; y < _pixels.getHeight(); y++){
            auto color = _pixels.getColor(x, y);
            msg.addFloatArg(color.r);
            msg.addFloatArg(color.g);
            msg.addFloatArg(color.b);
        }
    }
    _oscSender.sendMessage(msg);
}
void ofApp::sendPosition(int x, int y){ 
    ofxOscMessage msg;
    msg.setAddress(_address);
    msg.addFloatArg(x);
    msg.addFloatArg(y);
    _oscSender.sendMessage(msg);
}


void ofApp::onPortChanged(int & value){
    _oscSender.setup(_host, _port);
}
void ofApp::onHostChanged(std::string & value){
    _oscSender.setup(_host, _port);
}
void ofApp::onThresholdChanged(int & value){
    contour.setThreshold(_threshold);
}
void ofApp::onDeviceIdChanged(int & value){
    _grabber.close();
    _grabber.setDeviceID(_videoDeviceId);
    _grabber.setup(_width, _height);
}
