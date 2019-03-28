#include "Amay.h"

// Customize filters and textures for Amay

Amay::Amay(ofxBox2d &box2d, AgentProperties agentProps) {  
  // Assign Amay's mesh origin (left corner).
  agentProps.meshOrigin = ofPoint(10, 20);
  
  // Assign a color palette
  palette = { ofColor::fromHex(0x141414), ofColor::fromHex(0x662121), ofColor::fromHex(0xA87300), ofColor::fromHex(0x39AA08), ofColor::fromHex(0x5AAAAA) };
  
  // Messages
  this->numMessages = 600;
  setup(box2d, agentProps); // TODO: Actually pass a pointer to all the messages later (for now it's assigned randomly)

  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y);
}
