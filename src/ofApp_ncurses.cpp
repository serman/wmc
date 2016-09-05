//
//  ofApp_ncurses.cpp
//  wmc
//
//  Created by Sergio Galan on 05/09/16.
//
//

#include "ofApp.h"

shared_ptr<nc::Win> mMainWindow;

vector<string> worldStr;

void ofApp::setupNC(){
    ofxNcurses::setup();
    ofxNcurses::hideCursor();
    
    mMainWindow         = ofxNcurses::addWindow(0, 0, ofxNcurses::getWidth(), ofxNcurses::getHeight(), true, true);
    mMainWindow->box();
}
