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
    
    mMainWindow = ofxNcurses::addWindow(0, 0, ofxNcurses::getWidth(), ofxNcurses::getHeight(), false, false);
    mMainWindow->box();
}


void ofApp::drawNC(stringstream &stream1){
    worldStr.clear();
    ofLogNotice() << "DRAW NCURSE 1" << stream1.str() <<endl;
    ofLogNotice() <<  "DRAW NCURSE 2" << videoPlayer.getCurrentFrame() <<endl;
    
/** status msgs ***/
    string tok;
    char delimiter='\n';
    while(getline(stream1, tok, delimiter)) {
        worldStr.push_back(tok);
    }
    
    mMainWindow->erase();
    //mMainWindow->moveTo(0,0);
    
    for (int i=0; i<worldStr.size(); i++){
        mMainWindow->moveTo(1, i % mMainWindow->getHeight());
        mMainWindow->print(worldStr[i]);
    }
    

    for(int i = 0; i < finder.size(); i++) {
        ofRectangle object = finder.getObject(i);
        
        mMainWindow->moveTo(
                            ofMap(object.x, 0, videoWidth, 0, mMainWindow->getWidth()),
                            ofMap(object.y, 0, videoHeight, 0, mMainWindow->getHeight())
                            );
        
        
        mMainWindow->attrOn(nc::Win::COLOR_1);
        mMainWindow->print("BODY " + ofToString( finder.getLabel(i)) );
        mMainWindow->attrOff(nc::Win::COLOR_1);
 
    }
 
    
    
    mMainWindow->moveTo(10,mMainWindow->getHeight() / 2);
    mMainWindow->attrOn(nc::Win::COLOR_1);
    mMainWindow->print("HELLO");
    mMainWindow->attrOff(nc::Win::COLOR_1);
    
    mMainWindow->refresh();
    
}
