#include "Azra.h"

Azra::Azra(ofxBox2d &box2d, AgentProperties agentProps) {
  // Assign Azra's mesh origin (right corner).
  ofPoint p = ofPoint(ofGetWidth() - agentProps.meshSize.x - 10, ofGetHeight() - agentProps.meshSize.y - 20);
  agentProps.meshOrigin = p;
  
  // Color palette
  palette = { ofColor::fromHex(0xEDF67D), ofColor::fromHex(0xFC6DAB), ofColor::fromHex(0xC04CFD), ofColor::fromHex(0x724CF9), ofColor::fromHex(0x3F388C) }; 
  
  this->numMessages = 550;
  setup(box2d, agentProps);

  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y);
}
