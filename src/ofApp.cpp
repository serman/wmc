

#include "ofApp.h"
#include <math.h>
using namespace ofxCv;
using namespace cv;

//------------------------------------------------------------------------------
void ofApp::setup()
{
    ofLogToFile(ofToDataPath("log.txt",true) );
    ofSetLogLevel(OF_LOG_VERBOSE);

    fileName="";

    ofSetFrameRate(8);
    sender.setup(HOST, PORT);
    loadCameras();
    
    ofSetLogLevel(logLevel);
    ofSetLogLevel(OF_LOG_VERBOSE);
    int serverRecvPort = 12000;
    string dst="localhost";
    myosc.setup(serverRecvPort);
    
    if(useLocalVideo==false){
        // initialize connection
        for(std::size_t i = 0; i < NUM_CAMERAS; i++)
        {
            IPCameraDef& cam = getNextCamera();
            
            auto grabber = std::make_shared<Video::IPVideoGrabber>();

            
            grabber->setCameraName(cam.getName());
            grabber->setURI(cam.getURL());
            grabber->connect(); // connect immediately

            // if desired, set up a video resize listener
            ofAddListener(grabber->videoResized, this, &ofApp::videoResized);
            grabbers.push_back(grabber);
        }
        
/*        for(std::size_t i = 0; i < grabbers.size(); i++){
            grabbers[i]->update();
            if(grabbers[i]->getWidth()<1000){ //TODO niapa de elegir fuente
                faceTrackingGrabber=i;
            }
            else{
                microsoftGrabber=i;
            }
        }*/
        faceTrackingGrabber=0;
        microsoftGrabber=1;
        ratioW=1280.0/videoWidth;
        ratioH=1024.0/videoHeight;
        
    }else{
        videoPlayer.load(videoName);
        //videoPlayer.setLoopState(OF_LOOP_NORMAL);
        videoPlayer.play();
        //videoPlayer.setLoopState(OF_LOOP_NORMAL);
        videoPlayer.getPlayer()->getTotalNumFrames();
        
        ratioW=1;
        ratioH=1;
    }
    
   // grabFrame.allocate(videoWidth,videoHeight,OF_IMAGE_COLOR);
   // grabberMat = toCv(grabFrame);
    //imgMat2=cv::Mat(512, 512, CV_8UC3) * 255;
    //grabberMat=cv::Mat(videoWidth, videoHeight, CV_8UC3) * 255;
    
    ofSetVerticalSync(true);
    finder.setup("haarcascade_upperbody.xml");
    finder.setPreset(ObjectFinder::Fast); //accurate sensitive fast //este lo deamos en fast
                //info de los presets: https://github.com/kylemcdonald/ofxCv/blob/master/libs/ofxCv/src/ObjectFinder.cpp
    finder.getTracker().setSmoothingRate(.2);
    faceFinder.setup("haarcascade_frontalface_alt2.xml");
    faceFinder.setPreset(ObjectFinder::Sensitive);
    
    //finder.setMinNeighbors(3);
    //finder.setMultiScaleFactor(1.07);
    //cam.initGrabber(640, 480);
    
/***
 arduino
 ***/
    
    serial.listDevices();
/*    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();*/
    if(serialPort>=0){
        serial.setup(serialPort, 9600);
    }
    
    if(HEADLESS){
        ofLogNotice() << "HEADLESS";
        setupNC();
    }
}


//------------------------------------------------------------------------------
void ofApp::videoResized(const void* sender, ofResizeEventArgs& arg)
{
    // find the camera that sent the resize event changed
    for(std::size_t i = 0; i < NUM_CAMERAS; i++)
    {
        if(sender == &grabbers[i])
        {
            std::stringstream ss;
            ss<< "videoResized: ";
            ss<< "Camera connected to: " << grabbers[i]->getURI() + " ";
            ss<< "New DIM = " << arg.width << "/" << arg.height;
            ofLogVerbose("ofApp") << ss.str();
        }
    }
}



//------------------------------------------------------------------------------
void ofApp::update()
{
    //ofLogNotice() << "uupdate init" << ofGetFrameNum() <<endl;
    
    
    // update the cameras
    if(status==HIBERN &&  ( (ofGetElapsedTimeMillis()/1000) - timeHibernStarted > 5 ) ){
        status=FINDING;
    }
    if(status==WAITING_RESPONSE){ //checking for a network timeout
        if( (ofGetElapsedTimeMillis()-lastTimeFaceDetectedAndSentToAPI) > NETWORK_TIMEOUT)
            status=FINDING;
    }
    /*** ACTUALIZACION DE IMAGENES DE CAMARA ***/
    if(useLocalVideo==false){
        grabbers[faceTrackingGrabber]->update();

        grabberMat = toCv(grabbers[faceTrackingGrabber]->getPixels());
      //  grabbers[faceTrackingGrabber]->getPixel
        toOf(grabberMat,grabFrame.getPixels());
        grabFrame.update();
        
    }else{
        videoPlayer.update();
        ofLogNotice() << " videoPlayer inside" << videoPlayer.getCurrentFrame() <<endl;
        if(videoPlayer.isFrameNew() ){
            ofLogNotice() << " videoPlayer FrameNew" << videoPlayer.getCurrentFrame() <<endl;

            //unsigned char * pixels ;
            //pixels = videoPlayer.getPixels();
            //grabFrame.setFromPixels(pixels, videoWidth,videoHeight, OF_IMAGE_COLOR);
            grabberMat = toCv(videoPlayer.getPixels());
            toOf(grabberMat,grabFrame.getPixels());
            grabFrame.update();
        }
    }
    /*****  *****/
    
       
    
    if(grabberMat.rows >100 && ( status==FINDING || status==WAITING_RESPONSE ) ){
        //Mat imgMat = toCv(grabFrame);

        finder.update( grabberMat );
        int detectedFaces=0;
        if(finder.size()>0 && status==FINDING ){ //Si est‡ en modo busqueda y ha encontrado algo...
            for(int i=0; i<finder.size(); i++){
                ofRectangle r= finder.getObject(i);
                cv::Rect mroi=cv::Rect(r.x,r.y,r.width, r.height);
                //cout << r.x << " " << r.y <<" " << r.width << "\n";
                //Mat imgMat = toCv(grabFrame);
                //Mat imgMat3=imgMat(mroi);
                 imgMat2=grabberMat(mroi);
                //FaceFinder
                //imgMat2=toCv(grabFrameEnvio);
                //Mat imgMat3=toCv(grabFrameEnvio);
                faceFinder.update(imgMat2);
                if(faceFinder.size()>0){
                    lastFaceTrackingIdSent=finder.getTracker().getLabelFromIndex(i);                    ofLog(OF_LOG_NOTICE) << "id cara" << lastFaceTrackingIdSent;
                    detectedFaces++;
                    break;
                }
            }
            if(!HEADLESS){
            //cv::Mat tmpMat;
            imgMat2.copyTo(tmpMat);
            // https://github.com/kylemcdonald/ofxCv/issues/163
            // Sin este copyTo una submatriz (ya que al usar roi no se copian pixels) no se copiar’a correctamente y se ve raro.
            }
        }
        
        if (detectedFaces>0){
            if( (ofGetElapsedTimeMillis()-lastTimeFaceDetectedAndSentToAPI) > MIN_TIME_API_QUERY){
                lastTimeFaceDetectedAndSentToAPI=ofGetElapsedTimeMillis();
                saveFrameAndNotify();
                status=WAITING_RESPONSE;
            }
        }
    }
    oscRcvUpdate();
    if(status!=SHOOTING){
        updateServoPosition();
    }
    ofLogNotice() << "videoplayergetposition " << videoPlayer.getPosition() <<endl;
        std::stringstream infoStream;
    infoStream<< "  frameRAte: " << ofToString( ofGetFrameRate() ) << endl;
    infoStream<< "  current video frame: " << videoPlayer.getPosition() <<endl;
    infoStream<< "  path " << videoPlayer.getMoviePath() <<endl;
    infoStream<< "  playing " << videoPlayer.isPlaying() <<endl;
    infoStream<< "  loaded " << videoPlayer.getPlayer() <<endl;
    infoStream<< "  frame num " << ofGetFrameNum() <<endl;
    infoStream<< "   isUsingTexture " << videoPlayer.isUsingTexture() <<endl;
    infoStream<< "   isUsingTexture " << videoPlayer.getTotalNumFrames() <<endl;

    

}

//------------------------------------------------------------------------------
void ofApp::draw(){
    std::stringstream infoStream;
    //cout << "Draw init" << videoPlayer.getCurrentFrame() << endl;
                    //ofLogNotice() << "Drive" <<endl;
    if(!HEADLESS){
        drawGraphic();
    }
    else{
        int f=round( ofGetFrameRate()); if(f<1) f=1;
        if(ofGetFrameNum()%f==0){
            ofLogVerbose("personas" + ofToString(finder.size()) + " ___ Caras: " + ofToString(faceFinder.size()));
        }
    }
    


    if(useLocalVideo==false){
        ofSetHexColor(0xffffff);
        float kbps = grabbers[faceTrackingGrabber]->getBitRate() / 1000.0f; // kilobits / second, not kibibits / second
        float fps = grabbers[faceTrackingGrabber]->getFrameRate();
        
        // ofToString formatting available in 0072+
        infoStream<< "          NAME: " << grabbers[faceTrackingGrabber]->getCameraName() << endl;
        infoStream<< "          HOST: " << grabbers[faceTrackingGrabber]->getHost() << endl;
        infoStream<< "           FPS: " << ofToString(fps,  2) << endl;
        infoStream<< "          Kb/S: " << ofToString(kbps, 2) << endl;
     //   infoStream<< " #Bytes Recv'd: " << ofToString(grabbers[faceTrackingGrabber]->getNumBytesReceived(),  0) << endl;
      //  infoStream<< "#Frames Recv'd: " << ofToString(grabbers[faceTrackingGrabber]->getNumFramesReceived(), 0) << endl;
        infoStream<< "Width: " << ofToString(grabbers[faceTrackingGrabber]->getWidth()) << endl;
        infoStream<< "Height: " << ofToString(grabbers[faceTrackingGrabber]->getHeight()) << endl;
        infoStream<< "Auto Reconnect: " << (grabbers[faceTrackingGrabber]->getAutoReconnect() ? "YES" : "NO") << endl;
        infoStream<< " Needs Connect: " << (grabbers[faceTrackingGrabber]->getNeedsReconnect() ? "YES" : "NO") << endl;
        infoStream<< "Time Till Next: " << grabbers[faceTrackingGrabber]->getTimeTillNextAutoRetry() << " ms" << endl;
        infoStream<< "Num Reconnects: " << ofToString(grabbers[faceTrackingGrabber]->getReconnectCount()) << endl;
        infoStream<< "Max Reconnects: " << ofToString(grabbers[faceTrackingGrabber]->getMaxReconnects()) << endl;
        infoStream<< "  Connect Fail: " << (grabbers[faceTrackingGrabber]->hasConnectionFailed() ? "YES" : "NO");
        infoStream<< "  frameRAte: " << ofToString(ofGetFrameRate() ) << endl;
        
    }
    else{
        infoStream<< "  frameRAte: " << ofToString( ofGetFrameRate() ) << endl;
        infoStream<< "  current video frame: " << videoPlayer.getPosition() <<endl;
        infoStream<< "  path " << videoPlayer.getMoviePath() <<endl;
                infoStream<< "  playing " << videoPlayer.isPlaying() <<endl;
                        infoStream<< "  loaded " << videoPlayer.getPlayer() <<endl;
        infoStream<< "  frame num " << ofGetFrameNum() <<endl;
                infoStream<< "   isUsingTexture " << videoPlayer.isUsingTexture() <<endl;
                infoStream<< "   isUsingTexture " << videoPlayer.getTotalNumFrames() <<endl;
    }
    
    switch(status){
        case 0:         infoStream<< "  Status: HIBERN" << endl; break;
        case 1:         infoStream<< "  Status: SHOOTING" << endl; break;
        case 2:         infoStream<< "  Status: FINDING" << endl; break;
        case 3:         infoStream<< "  Status: WAITING RESPONSE" << endl; break;
    }
    infoStream << "personas" << ofToString(finder.size()) << " ___ Caras: " << ofToString(faceFinder.size() );
    
    
    if(!HEADLESS){
        ofDrawBitmapString(infoStream.str(), 10, 10+12);
        drawDetection();
    }
    else{
        drawNC(infoStream);
        int f=round( ofGetFrameRate()*4);
        if(f<1) f=1;
        if(ofGetFrameNum()%f==0){
            ofLogVerbose() << "logverbose 2" << infoStream.str();
        }
    }
    
    
    /******* POST DRAW UPDATES ****/
    if(status==SHOOTING){
        status=HIBERN; //TODO mover esto al update
        timeHibernStarted=ofGetElapsedTimeMillis()/1000;
    }
    ofLogVerbose() << "Draw End" << videoPlayer.getCurrentFrame() << endl;
}

void ofApp::drawGraphic(){
    
    ofBackground(0,0,0);
    ofEnableAlphaBlending();
    ofSetColor(0,80);
    ofDrawRectangle(5,5, 150, 40);
    
    ofSetColor(255);
    
    ofDisableAlphaBlending();
    
    grabFrame.draw(0,0,grabFrame.getWidth(),grabFrame.getHeight());
    finder.draw();
    ofSetColor(255,0,0);
    ofPushMatrix();
    if((status==FINDING || status==WAITING_RESPONSE) && finder.size()>0){
        ofTranslate(facesRectangle.x, facesRectangle.y);
        faceFinder.draw();
    }
    ofPopMatrix();
    ofSetColor(255,255,255);
    //toOf(imgMat2,grabFrameDetectBody.getPixelsRef());
    //grabFrameDetectBody.setImageType(OF_IMAGE_COLOR_ALPHA);
    //grabFrameDetectBody.update();
    //      grabFrameDetectBody.setImageType(OF_IMAGE_COLOR_ALPHA);
    //        grabFrameDetectBody.setImageType(OF_IMAGE_COLOR);
    //grabFrameDetectBody.draw(800,100);
    
    drawMat(tmpMat, 750, 100);
    
    ofDrawBitmapStringHighlight(ofToString(finder.size()), 10, 20);
    //ofLog(OF_LOG_NOTICE, "file name file " + fileName);
    
    
    // SECTION dibuja una aproximacion del angulo del motor
    ofPushMatrix();
    ofTranslate(grabFrame.getWidth()/2, 20);
    unsigned int ang=(unsigned int) servoMsg[1];
    int minAng=270-(maxAngleServo/2);
    int maxAng=270+(maxAngleServo/2);
    
    int x1 = 200*cos(ofDegToRad(maxAng-ang));
    int y1 = -200*sin(ofDegToRad(maxAng-ang));
    
    //ofLogNotice("angulo char" + ofToString((int)ang));
    ofDrawEllipse(0, 0, 200, 200);
    ofDrawLine(0, 0, x1, y1);
    ofFill();
    ofDrawEllipse(100*cos(ofDegToRad(minAng)), -100*sin(ofDegToRad(minAng)) , 20, 20);
    ofDrawEllipse(100*cos(ofDegToRad(maxAng)), -100*sin(ofDegToRad(maxAng)) , 20, 20);
    ofPopMatrix();
    
}

void ofApp::drawDetection(){
    ofSetColor(0, 0, 200);
    ofNoFill();
    ofPushMatrix();
    ofTranslate(facesRectangle.x, facesRectangle.y);
    for(std::size_t i = 0; i < detections.size(); i++){

            std::stringstream ss;
        if(status==SHOOTING){
            
            ofFill();
            
        }
        if( (ofGetElapsedTimeMillis()/1000) - timeLastDetectionFromAPI < 2 ){
            if( targetMoved.width >0  ) {
                ofPushStyle();
                
                // toOf(r);
                ofSetColor(200, 000, 200);

                ofDrawRectangle( targetMoved.x+targetMoved.width/2-75-facesRectangle.x, targetMoved.y+targetMoved.height/2-75-facesRectangle.y, 150, 150 );
                ofPopStyle();

            }
            ofDrawRectangle(detections[i].position.x/ratioW,
                            detections[i].position.y/ratioH,
                            detections[i].position.width/ratioW,
                            detections[i].position.height/ratioH);
            ss<< "  beard " << ofToString(detections[i].beard ) << endl;
            ss<< "  age "  << ofToString(detections[i].age ) << endl;
            ss<< "  glasses " << ofToString(detections[i].glasses ) << endl;
            ss<< "  gender " << ofToString(detections[i].gender ) << endl;
            ss<< "  smile " << ofToString(detections[i].smile ) << endl;
            ofDrawBitmapString(ss.str(), detections[i].position.x-90,detections[i].position.y );
        }
    }
    ofPopMatrix();
}

//------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if(key == ' ')
    {
        // initialize connection
        for(std::size_t i = 0; i < NUM_CAMERAS; i++)
        {
            ofRemoveListener(grabbers[i]->videoResized, this, &ofApp::videoResized);
            auto c = std::make_shared<Video::IPVideoGrabber>();
            IPCameraDef& cam = getNextCamera();
            c->setUsername(cam.getUsername());
            c->setPassword(cam.getPassword());
            Poco::URI uri(cam.getURL());
            c->setURI(uri);
            c->connect();
            
            grabbers[i] = c;
        }
    }
    if(key == 'a' || key == 'A'){
        ofxOscMessage m;
        m.setAddress("/video");
        m.addIntArg(1);
       // m.addFloatArg(3.5f);
       // m.addStringArg("hello");
       // m.addFloatArg(ofGetElapsedTimef());
        sender.sendMessage(m);
    }
    if(key == 's' ){
        //string fileName = "/Users/sergiogalan/temporalborrar/snapshot.png";
        /*grabFrameEnvio.setFromPixels(grabbers[microsoftGrabber]->getPixels(), grabbers[microsoftGrabber]->getWidth(), grabbers[microsoftGrabber]->getHeight(), OF_IMAGE_COLOR);
        grabFrameEnvio.update();
        grabFrameEnvio.saveImage(fileName);
        ofLog(OF_LOG_NOTICE, "saved file " + fileName);*/
        saveFrameAndNotify();
    }
    if(key == 'b' ){//bang
        killThatOne();
    }
    
}

void ofApp::getContour(){
    //facesRectangle.set(0,0,0,0);
    for(int i=0; i<finder.size(); i++){
        if(i==0){
            facesRectangle=finder.getObject(i);
        }
        else{
            facesRectangle.growToInclude(finder.getObject(i));
        }
    }
}



void ofApp::saveFrameAndNotify(){
    
    //string fileName = "/Users/sergiogalan/temporalborrar/snapshot.png";
    if(useLocalVideo==false){ //si usamos la camara

        grabbers[microsoftGrabber]->update();
        grabFrameEnvio.setFromPixels(
                                     grabbers[microsoftGrabber]->getPixels(),
                                     grabbers[microsoftGrabber]->getWidth(),
                                     grabbers[microsoftGrabber]->getHeight(),
                                     OF_IMAGE_COLOR);
        //grabFrameEnvio.update();
        getContour();
        
        
                cout << grabbers[microsoftGrabber]->getWidth() << " " << grabbers[microsoftGrabber]->getHeight()  << " " << ratioW  << " " << ratioH;
        
        //TODO testear que esto fncione
        grabFrameEnvio.crop(facesRectangle.x*ratioW, facesRectangle.y*ratioH, facesRectangle.width*ratioW, facesRectangle.height*ratioH);
        
        grabFrameEnvio.save(fileName);
        //grabFrame.save("/Users/sergiogalan/temporalborrar/snapshot.jpg");
        
    }
    else{
        unsigned char * pixels ;
        //pixels = videoPlayer.getPixels();
        //grabFrameEnvio.setFromPixels(pixels, videoWidth,videoHeight, OF_IMAGE_COLOR);
        //grabFrameEnvio.update();
        getContour();
        cv::Rect r = cv::Rect(facesRectangle.x, facesRectangle.y, facesRectangle.width, facesRectangle.height);
        cv::Mat m1=grabberMat(r);
        cv::Mat m2tmp;
        m1.copyTo(m2tmp);
         toOf(m2tmp,grabFrameEnvio);
        grabFrameEnvio.update();
        
//        grabFrameEnvio.crop(facesRectangle.x, facesRectangle.y, facesRectangle.width, facesRectangle.height);
        grabFrameEnvio.save(fileName);
    }
    ofLog(OF_LOG_VERBOSE, ofToString( ofGetElapsedTimeMillis()/1000) + ": saved file " + fileName);
    ofxOscMessage m;
    m.setAddress("/video");
    m.addIntArg(1);
    //m.addFloatArg(3.5f);
    //m.addStringArg("hello");
    m.addFloatArg(ofGetElapsedTimef());
    sender.sendMessage(m);
}


void ofApp::updateServoPosition(){
    //No quiero estar moviendo todo el rato....
    if(ofGetElapsedTimeMillis()-timeLastMotorRotation>400 && finder.size()>0){
        int p1=finder.getObject(0).position.x +finder.getObject(0).width/2;
      //  p1=ofGetMouseX();
        int resolucion = videoWidth;
//        unsigned char angulo= (p1/resolucion) *2* aperturaCamara - aperturaCamara+90;
  //      int angulo2 = (p1/resolucion) *2* aperturaCamara - aperturaCamara+90;
        unsigned char angulo= ofMap(p1,0,resolucion,maxAngleServo,0);
        
        //ofLogNotice("angulo " + ofToString((int)angulo));
        if(angulo>=0 && angulo <= maxAngleServo){ //verificaci—n redundante pero por si acaso
            servoMsg[1]=angulo;
            if(serial.isInitialized()){
                bool byteWasWritten = serial.writeBytes(&servoMsg[0],3);
                if ( !byteWasWritten )
                    ofLogError("byte was not written to serial port");
            }
            
        }
        timeLastMotorRotation=ofGetElapsedTimeMillis();
        
    }
    /*
     val = chr(int(-posFaceX/resolucion*2*aperturaCamara-aperturaCamara+125))
     print(chr(int(-posFaceX/resolucion*2*aperturaCamara-aperturaCamara+125)), "x:", str(posFaceX))
     arduinoSerial.write(val)
     */
}

//------------------------------------------------------------------------------
IPCameraDef& ofApp::getNextCamera()
{
    nextCamera = (nextCamera + 1) % ipcams.size();
    return ipcams[nextCamera];
}

//------------------------------------------------------------------------------
void ofApp::loadCameras()
{
    
    // all of these cameras were found using this google query
    // http://www.google.com/search?q=inurl%3A%22axis-cgi%2Fmjpg%22
    // some of the cameras below may no longer be valid.
    
    // to define a camera with a username / password
    //ipcams.push_back(IPCameraDef("http://148.61.142.228/axis-cgi/mjpg/video.cgi", "username", "password"));
    
    ofLog(OF_LOG_VERBOSE, "---------------Loading Streams---------------");
    
    ofxXmlSettings XML;
    
    if(XML.loadFile("streams.xml"))
    {
        XML.pushTag("streams");
        std::string tag = "stream";
        
        std::size_t nCams = static_cast<std::size_t>(XML.getNumTags(tag));
        
        for (std::size_t n = 0; n < nCams; ++n)
        {
            std::string username = XML.getAttribute(tag, "username", "", n);
            std::string password = XML.getAttribute(tag, "password", "", n);
            
            std::string auth = XML.getAttribute(tag, "auth-type", "NONE", n);
            
            IPCameraDef::AuthType authType = IPCameraDef::AuthType::NONE;
            
            if (auth.compare("NONE") == 0)
            {
                authType = IPCameraDef::AuthType::NONE;
            }
            else if (auth.compare("BASIC") == 0)
            {
                authType = IPCameraDef::AuthType::BASIC;
            }
            else if (auth.compare("COOKIE") == 0)
            {
                authType = IPCameraDef::AuthType::COOKIE;
            }
            
            IPCameraDef def(XML.getAttribute(tag, "name", "", n),
                            XML.getAttribute(tag, "url", "", n),
                            username,
                            password,
                            authType);
            
            
            std::string logMessage = "STREAM LOADED: " + def.getName() +
            " url: " +  def.getURL() +
            " username: " + def.getUsername() +
            " password: " + def.getPassword() +
            " auth: " + std::to_string(static_cast<int>((def.getAuthType())));
            
            ofLogVerbose() << logMessage;
            
            ipcams.push_back(def);
        }
        
        XML.popTag();
        
        fileName = XML.getValue("file","");
        useLocalVideo= (bool)XML.getValue("useLocalVideo",1);
        useLocalVideo= (bool)XML.getValue("useLocalVideo",1);
        videoName= XML.getValue("videoName","abierto.mp4");
        serialPort= XML.getValue("serialPort",-1);
        HEADLESS= XML.getValue("headless",0);
        
#ifdef NCURSES
        HEADLESS=1;
#endif
        logLevel= static_cast<ofLogLevel>( XML.getValue("logLevel",1));
        
        ofLog(OF_LOG_VERBOSE, "filename: " + fileName);
        
    }
    else
    {
        ofLog(OF_LOG_ERROR, "Unable to load streams.xml.");
    }
    
    ofLog(OF_LOG_VERBOSE, "-----------Loading Streams Complete----------");
    
    nextCamera = ipcams.size();
}
