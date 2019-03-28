#include "Azra.h"

Azra::Azra(ofxBox2d &box2d, AgentProperties agentProps) {
  // Assign Azra's mesh origin (right corner).
  ofPoint p = ofPoint(ofGetWidth() - agentProps.meshSize.x - 10, ofGetHeight() - agentProps.meshSize.y - 20);
  agentProps.meshOrigin = p;
  
  // Color palette
  palette = { ofColor::fromHex(0x064789), ofColor::fromHex(0xF48327), ofColor::fromHex(0xF72E57), ofColor::fromHex(0x7CC934), ofColor::fromHex(0x2D3D30), ofColor::fromHex(0xFF6B6C), ofColor::fromHex(0xFFC145), ofColor::fromHex(0x5D3DFF) }; 
  
  this->numMessages = 550;
  setup(box2d, agentProps);

  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y);
}
