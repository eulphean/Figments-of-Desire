#include "Memory.h"
#include "Agent.h"

Memory::Memory(ofxBox2d &box2d) {
  mem = std::make_shared<ofxBox2dCircle>();
  mem -> setPhysics(0.3, 0.3, 0.3); // bounce, density, friction
  mem -> setup(box2d.getWorld(), ofGetWidth()/2, ofGetHeight()/2, 5); // ofRandom(3, agentProps.vertexRadius)
  mem -> setFixedRotation(true);
  mem -> setData(new VertexData(NULL)); // No agent pointer for this.
}

void Memory::update() {

}

void Memory::draw() {
  ofPushMatrix();
    ofTranslate(mem->getPosition());
    ofPushStyle();
      ofSetColor(ofColor::red);
      ofDrawCircle(0, 0, mem->getRadius());
    ofPopStyle();
  ofPopMatrix();
}

