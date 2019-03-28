#include "SuperAgent.h"

void SuperAgent::setup(Agent *agent1, Agent *agent2, std::shared_ptr<ofxBox2dJoint> joint) {
  agentA = agent1;
  agentB = agent2;
  joints.push_back(joint);
  maxExchangeCounter = 50;
  curExchangeCounter = 0;
}

void SuperAgent::update(ofxBox2d &box2d, int maxJointForce) {
  // Max Force based on which the joint breaks.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j) {
    auto force = j->getReactionForce(ofGetElapsedTimef());
    // Both AgentA and AgentB want to break the bonds?
    // Then Break it. 
    if (!agentA -> canBond() && !agentB -> canBond()) {
      box2d.getWorld()->DestroyJoint(j->joint);
      return true;
    } else {
      return false;
    }
  });
  
  if (joints.size() == 0) {
    agentA -> setPartner(NULL);
    agentB -> setPartner(NULL);
    shouldRemove = true;
  } else {
    // When it's a super agent, that means it's bonded.
    // Check if it's ready to swap messages.
    
    if (curExchangeCounter <= 0) {
      auto& aMessage = agentA -> it;
      auto& bMessage = agentB -> it;
      Message swap = Message(aMessage->location, aMessage->color, aMessage->size);
      
      // Assign A message
      aMessage->color = bMessage->color;
      aMessage->size = bMessage->size;
      
      // Assign B message
      bMessage->color = swap.color;
      bMessage->size = swap.size;
      
      // Increment iterators
      if (aMessage == agentA -> messages.end()) {
        aMessage = agentA -> messages.begin();
      } else {
        aMessage++;
      }
      
      if (bMessage == agentB -> messages.end()) {
        bMessage = agentB -> messages.begin();
      } else {
        bMessage++;
      }
      
      agentA -> createTexture(agentA -> getTextureSize());
      agentB -> createTexture(agentB -> getTextureSize());
      
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
