#include "Azra.h"

Azra::Azra(ofxBox2d &box2d, AgentProperties agentProps) {
  // Assign Azra's mesh origin (right corner).
  ofPoint p = ofPoint(ofGetWidth() - agentProps.meshSize.x - 10, ofGetHeight() - agentProps.meshSize.y - 20);
  agentProps.meshOrigin = p;
  agentProps.vertexRadius = 4.0; 
  
  palette = { ofColor::fromHex(0xFFBE0B), ofColor::fromHex(0xFB5607), ofColor::fromHex(0xFF006E), ofColor::fromHex(0x8338EC), ofColor::fromHex(0x3A86FF) };
  
  this->numBogusMessages = 500;
  
  // Force weights for body actions
  stretchWeight = 0.8;
  repulsionWeight = 1.5;
  attractionWeight = 0.5; // Can this be changed when the other agent is trying to attack me?
  seekWeight = 0.4; // Probably seek with a single vertex.
  tickleWeight = 2.5;
  maxVelocity = 10; 
  
  setup(box2d, agentProps, "azra.txt");
  
  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);
}
