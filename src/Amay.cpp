#include "Amay.h"

// Customize filters and textures for Amay

Amay::Amay(ofxBox2d &box2d, AgentProperties agentProps) {  
  // Assign Amay's mesh origin (left corner).
  agentProps.meshOrigin = ofPoint(10, 20);
  
  // Assign a color palette
  palette = { ofColor::fromHex(0x540D6E), ofColor::fromHex(0x982A41), ofColor::fromHex(0xFFEEB9), ofColor::fromHex(0x3BCEAC), ofColor::fromHex(0x0EAD69) };
  
  this->numBogusMessages = 550;
  
  setup(box2d, agentProps, "amay.txt"); // TODO: Actually pass a pointer to all the messages later (for now it's assigned randomly)

  // Post process filters.
  this->filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 10.f);
}
