#include "SuperAgent.h"

void SuperAgent::setup(Agent *agent1, Agent *agent2, std::shared_ptr<ofxBox2dJoint> joint) {
  agentA = agent1;
  agentB = agent2;
  joints.push_back(joint);
  maxExchangeCounter = 10;
  curExchangeCounter = 0;
}

void SuperAgent::update(ofxBox2d &box2d, int maxJointForce) {
  // Max Force based on which the joint breaks.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j) {
    auto force = j->getReactionForce(ofGetElapsedTimef());
    return true;
    // Both AgentA and AgentB want to break the bonds?
    // Then Break it. 
//    if (!agentA -> canBond() && !agentB -> canBond()) {
//      box2d.getWorld()->DestroyJoint(j->joint);
//      return true;
//    } else {
//      return false;
//    }
  });
  
  if (joints.size() == 0) {
//    agentA -> setPartner(NULL);
//    agentB -> setPartner(NULL);
    shouldRemove = true;
  } else {
    // When it's a super agent, that means it's bonded.
    // Check if it's ready to swap messages.
    if (curExchangeCounter <= 0) {
      std::vector<Message>::iterator aMessage = agentA -> curMsg;
      std::vector<Message>::iterator bMessage = agentB -> curMsg;
      
      Message swap = Message(aMessage->location, aMessage->color, aMessage->size, aMessage->message);
      
      // Assign A message
      aMessage->color = bMessage->color;
      aMessage->size = bMessage->size;
      aMessage->message = bMessage->message;
      
      // Assign B message
      bMessage->color = swap.color;
      bMessage->size = swap.size;
      bMessage->message = swap.message;
      
      // Change the iteretor to point to a unique message now
      aMessage = agentA -> messages.begin() + ofRandom(0, agentA -> messages.size() - 1);
      bMessage = agentB -> messages.begin() + ofRandom(0, agentB -> messages.size() - 1);
      
      agentA -> curMsg = aMessage;
      agentB -> curMsg = bMessage; 
      
      // Create new textures the two agents as they have just gone through a swap.
      agentA -> createTexture(agentA -> getTextureSize());
      agentB -> createTexture(agentB -> getTextureSize());
      
      // Reset exchange counter since
      curExchangeCounter = maxExchangeCounter;
    } else {
      curExchangeCounter -= 1;
    }
  }
}

void SuperAgent::draw() {
  for (auto j : joints) {
    ofPushStyle();
      ofSetColor(ofColor::red);
      ofSetLineWidth(3);
      j->draw();
    ofPopStyle();
  }
}

// Check if super body already exists. 
bool SuperAgent::contains(Agent *agent1, Agent *agent2) {
  if (agent1 == agentA) {
    if (agent2 == agentB) {
      return true;
    } else {
      return false;
    }
  } else if (agent2 == agentB) {
    if (agent1 == agentA) {
      return true;
    } else {
      return false;
    }
  }
}

void SuperAgent::clean(ofxBox2d &box2d) {
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
    box2d.getWorld()->DestroyJoint(j->joint);
    return true;
  });
  
  joints.clear();
}
