#include "SuperAgent.h"

void SuperAgent::setup(Agent *agent1, Agent *agent2, std::shared_ptr<ofxBox2dJoint> joint) {
  agentA = agent1;
  agentB = agent2;
  joints.push_back(joint);
}

void SuperAgent::update(ofxBox2d &box2d, std::vector<std::shared_ptr<ofSoundPlayer>> &sounds, int maxJointForce, bool &enableSound) {
  // Max Force based on which the joint breaks.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j) {
    auto force = j->getReactionForce(ofGetElapsedTimef());
    if (abs(force.length()) > maxJointForce) {
      cout << "Breaking this joint" << endl;
      auto data = (SoundData *) j -> joint -> GetUserData();
//      if (enableSound) {
//        sounds[data->breakIdx]->play();
//      }
      
      box2d.getWorld()->DestroyJoint(j->joint);
      // Trigger the break sound here.
      
      return true;
    } else {
      return false;
    }
  });
  
  if (joints.size() == 0) {
    agentA -> setPartner(NULL);
    agentB -> setPartner(NULL);
    shouldRemove = true;
  }

  // Calculate the exertion force on the bonds
  // If exertion force is greater than a limit, break the bond.
  
  // Update partner conditions after every bond that breaks. Is it
  // still a partner? If not, set the pointer to null
  
  // Then come to behaviors when they are bonded.. Exert those behaviors on both AgentA and AgentB.
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
