#include "Amay.h"

// Customize filters and textures for Amay

Amay::Amay(ofxBox2d &box2d, AgentProperties agentProps) {  
  // Assign Amay's mesh origin (left corner).
  agentProps.meshOrigin = ofPoint(10, 20);
  agentProps.vertexRadius = 7; 
  
  // Assign a color palette
  palette = { ofColor::fromHex(0x540D6E), ofColor::fromHex(0x982A41), ofColor::fromHex(0xFFEEB9), ofColor::fromHex(0x3BCEAC), ofColor::fromHex(0x0EAD69) };
  
  this->numBogusMessages = 550;
  
  // Force weight for body actions. This is heavier, so more weight.
  stretchWeight = 1.0;
  repulsionWeight = 2.0;
  attractionWeight = 1.5; // Can this be changed when the other agent is trying to attack me?
  seekWeight = 0.4; // Probably seek with a single vertex.
  tickleWeight = 2.5;
  maxVelocity = 10; 
  
  setup(box2d, agentProps, "amay.txt"); // TODO: Actually pass a pointer to all the messages later (for now it's assigned randomly)

  // Post process filters.
  this->filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);
}
