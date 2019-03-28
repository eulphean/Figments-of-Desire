#include "Amay.h"

// Customize filters and textures for Amay

Amay::Amay(ofxBox2d &box2d, AgentProperties agentProps) {  
  // Assign Amay's mesh origin (left corner).
  agentProps.meshOrigin = ofPoint(10, 20);
  
  // Assign a color palette
  palette = { ofColor::fromHex(0x5D3DFF), ofColor::fromHex(0xF48327), ofColor::fromHex(0xF72E57), ofColor::fromHex(0x7CC934), ofColor::fromHex(0x2D3D30), ofColor::fromHex(0xFF6B6C), ofColor::fromHex(0xFFC145), ofColor::fromHex(0x064789) };
  
  // Messages
  this->numMessages = 600;
  setup(box2d, agentProps); // TODO: Actually pass a pointer to all the messages later (for now it's assigned randomly)

  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y);
}
