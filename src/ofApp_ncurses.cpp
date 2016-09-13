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
/** status msgs ***/
    string tok;
    char delimiter='\n';
    while(getline(stream1, tok, delimiter)) {
        worldStr.push_back(tok);
    }
    
    mMainWindow->erase();
    //mMainWindow->moveTo(0,0);
    mMainWindow->box();
    
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
 
    
    
    if((status==FINDING || status==WAITING_RESPONSE) && finder.size()>0){
            for(int i = 0; i < faceFinder.size(); i++) {
                    ofRectangle object = faceFinder.getObject(i);
                    mMainWindow->moveTo(
                                                                         ofMap(object.x+facesRectangle.x, 0, videoWidth, 0, mMainWindow->getWidth()),
                                                                         ofMap(object.y+facesRectangle.y, 0, videoHeight, 0, mMainWindow->getHeight())
                                                                         );
                    mMainWindow->attrOn(nc::Win::COLOR_2);
                    mMainWindow->print("FACE " + ofToString( faceFinder.getLabel(i)) );
                    mMainWindow->attrOff(nc::Win::COLOR_2);
                }
        }
    
    for(std::size_t i = 0; i < detections.size(); i++){
            vector<string> infoStr;
            if( (ofGetElapsedTimeMillis()/1000) - timeLastDetectionFromAPI < 2 ){
                    int x1=facesRectangle.x + detections[i].position.x/ratioW;
                    int y1=facesRectangle.y + detections[i].position.y/ratioH;
                    mMainWindow->moveTo(
                                                         ofMap(x1, 0, videoWidth, 0, mMainWindow->getWidth()),
                                                         ofMap(y1, 0, videoHeight, 0, mMainWindow->getHeight())
                                                      );
                    infoStr.push_back ("  beard " + ofToString(detections[i].beard) + '\n') ;
                    infoStr.push_back ("  age "  + ofToString(detections[i].age) + '\n') ;
                    infoStr.push_back ("  glasses " + ofToString(detections[i].glasses) + '\n') ;
                    infoStr.push_back ("  gender " + ofToString(detections[i].gender) + '\n') ;
                    infoStr.push_back ("  smile " + ofToString(detections[i].smile) + '\n') ;
                    for (int i=0; i<infoStr.size(); i++){
                            //mMainWindow->moveTo(1, i % mMainWindow->getHeight());
                            mMainWindow->print(infoStr[i]);
                        }
            
                }
        }
    
    mMainWindow->refresh();
    
}
