#include "Azra.h"

Azra::Azra(ofxBox2d &box2d, AgentProperties agentProps) {
  // Assign Azra's mesh origin (right corner).
  ofPoint p = ofPoint(ofGetWidth() - agentProps.meshSize.x - 10, ofGetHeight() - agentProps.meshSize.y - 20);
  agentProps.meshOrigin = p;
  
  palette = { ofColor::fromHex(0xFFBE0B), ofColor::fromHex(0xFB5607), ofColor::fromHex(0xFF006E), ofColor::fromHex(0x8338EC), ofColor::fromHex(0x3A86FF) };
  
  this->numBogusMessages= 500;
  
  // Desire
  this->maxDesireCounter = 100;
  this->curDesireCounter = -this->maxDesireCounter; // Start with a negative number (No desire)
  this->desireIncrement = 0.01;
  
  setup(box2d, agentProps, "azra.txt");
  
  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);
}
