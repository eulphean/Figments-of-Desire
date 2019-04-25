#include "Memory.h"
#include "Agent.h" 

Memory::Memory(ofxBox2d &box2d, glm::vec2 location) {
  mem = std::make_shared<ofxBox2dCircle>();
  mem -> setPhysics(0.3, 0.3, 0.3); // bounce, density, friction
  mem -> setup(box2d.getWorld(), location.x, location.y, 6);
  mem -> setFixedRotation(true);
  mem -> setVelocity(ofRandom(-3, 3), ofRandom(-3, 3)); // Random velocity
  mem -> setData(new VertexData(NULL)); // No agent pointer for this.
  
  curTime = ofGetElapsedTimeMillis();
  maxTime = ofRandom(120000, 180000);
  shouldRemove = false; 
  
  // 2 - 3 minutes = 120000 - 180000
}

void Memory::update() {
  elapsedTime = ofGetElapsedTimeMillis() - curTime;
  if (elapsedTime >= maxTime) {
    shouldRemove = true; 
  }
}

void Memory::draw() {
  ofPushMatrix();
    ofTranslate(mem->getPosition());
    ofPushStyle();
      auto opacity = ofMap(elapsedTime, 0, maxTime, 255, 50, true);
      ofSetColor(ofColor::red, opacity);
      ofDrawCircle(0, 0, mem->getRadius());
    ofPopStyle();
  ofPopMatrix();
}

