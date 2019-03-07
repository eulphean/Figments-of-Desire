#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(0);
  ofSetCircleResolution(20);
  ofDisableArbTex();
  
  box2d.init();
  box2d.setGravity(-0.5, 0.5);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  startRepelling = false;
  debug = false;
  
  // Boundaries
  ofRectangle bounds;
  bounds.x = 0; bounds.y = 0;
  bounds.width = ofGetWidth(); bounds.height = ofGetHeight();
  box2d.createBounds(bounds);
  
  agentNum = 0;
  //serial.setup("/dev/cu.usbmodem1411", 9600);
}

void ofApp::contactStart(ofxBox2dContactArgs &e) {
  
}

// Joint creation sequence.
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
  if (agents.size() > 0) {
    if(e.a != NULL && e.b != NULL) {
      if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle
          && e.a->GetBody() && e.b->GetBody()) {
        // Extract Agent pointers.
        Agent* agentA = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData())->agent;
        Agent* agentB = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData())->agent;
        
        // Really long routine to evaluate if two vertices belonging to two different agents
        // can actually bond with each other or not. Take a look at the conditions under which
        // this bonding actually happens.
        evaluateBonding(e.a->GetBody(), e.b->GetBody(), agentA, agentB);
      }
    }
  }
}



//--------------------------------------------------------------
void ofApp::update(){
  box2d.update();
  //handleSerial();
  
  // GUI props.
  updateAgentProps();
  
  // Update agents.
  for (auto &a : agents) {
    a.update();
  }

  // Should create/destroy interAgentJoints?
  interAgentJointCreateDestroy();
}

//--------------------------------------------------------------
void ofApp::draw(){
  ofPushStyle();
    // Draw InterAgentJoints
    for (auto j: interAgentJoints) {
      ofSetColor(ofColor::red);
      ofSetLineWidth(3);
      j->draw();
    }
  ofPopStyle();
  
  for (auto a: agents) {
    a.draw(debug);
  }
  
  // Health parameters
  if (hideGui) {
    gui.draw();
  }
}

void ofApp::interAgentJointCreateDestroy() {
//  // Joint creation based on when two bodies collide at certain vertices.
//  if (collidingBodies.size()>0) {
//    // We have pending bodies to create a joint with.
//    auto jointList = collidingBodies[0]->GetJointList();
//    auto j = std::make_shared<ofxBox2dJoint>();
//    j->setup(box2d.getWorld(), collidingBodies[0], collidingBodies[1], frequency, damping); // Use the interAgentJoint props.
//    j->setLength(ofRandom(100, 150));
//    interAgentJoints.push_back(j);
//
//    // Get agent IDs and set the repulsion targets.
//    auto id1 = reinterpret_cast<VertexData*>(collidingBodies[0]->GetUserData())->agentId;
//    auto id2 = reinterpret_cast<VertexData*>(collidingBodies[0]->GetUserData())->agentId;
//    auto &agent1 = agents.at(id1);
//    auto &agent2 = agents.at(id2);
//    agent1.setRepulsionTarget(&agent2, id2);
//    agent2.setRepulsionTarget(&agent1, id1);
//
//    collidingBodies.clear();
//  }
//
//  // Joint destruction based on a predetermined force between agents.
//  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> c){
//      auto f = c->getReactionForce(ofGetElapsedTimef());
//      if (f.length() > maxJointForce) {
//        box2d.getWorld()->DestroyJoint(c->joint);
//        return true;
//      }
//
//      return false;
//  });
}

void ofApp::updateAgentProps() {
    // Create Soft Body payload to create objects.
  agentProps.meshDimensions = ofPoint(meshRows, meshColumns);
  agentProps.meshSize = ofPoint(meshWidth, meshHeight);
  agentProps.vertexRadius = vertexRadius;
  agentProps.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  agentProps.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
  agentProps.agentId = agentNum; 
}

void ofApp::createAgent() {
  Agent a;
  a.setup(box2d, agentProps);
  agents.push_back(a);
  agentNum = agents.size();
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("Inter Mesh Settings");
  
    // Mesh parameters.
    meshParams.setName("Mesh Params");
    meshParams.add(meshRows.set("Mesh Rows", 5, 5, 100)); // Add the current value
    meshParams.add(meshColumns.set("Mesh Columns", 5, 5, 100));
    meshParams.add(meshWidth.set("Mesh Width", 100, 10, ofGetWidth()/2));
    meshParams.add(meshHeight.set("Mesh Height", 100, 10, ofGetHeight()/2));
  
    // Vertex parameters
    vertexParams.setName("Vertex Params");
    vertexParams.add(vertexRadius.set("Vertex Radius", 6, 1, 30));
    vertexParams.add(vertexDensity.set("Vertex Density", 1, 0, 5));
    vertexParams.add(vertexBounce.set("Vertex Bounce", 0.3, 0, 1));
    vertexParams.add(vertexFriction.set("Vertex Friction", 1, 0, 1));
  
    // Joint parameters
    jointParams.setName("Joint Params");
    jointParams.add(jointFrequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    jointParams.add(jointDamping.set("Joint Damping", 1.0f, 0.0f, 5.0f));
  
    // InterAgentJoint parameters
    interAgentJointParams.setName("InterAgentJoint Params");
    interAgentJointParams.add(frequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    interAgentJointParams.add(damping.set("Joint Damping", 1.0f, 0.0f, 10.0f));
    interAgentJointParams.add(maxJointForce.set("Max Joint Force", 6.f, 1.f, 20.0f));
  
    settings.add(meshParams);
    settings.add(vertexParams);
    settings.add(jointParams);
    settings.add(interAgentJointParams);
  
    gui.setup(settings);
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::cleanInterAgentJoints() {
  // Actually destroy the joint... Else, the joint disappears but it still sit between the bodies.
  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> j){
      box2d.getWorld()->DestroyJoint(j->joint);
      return true;
  });
  
  interAgentJoints.clear();
  collidingBodies.clear();
}

void ofApp::enableRepulsion() {
//    // Enable repelling on the agent.
//    for (auto &j: interAgentJoints) {
//      auto bodyA = j->joint->GetBodyA();
//      auto bodyB = j->joint->GetBodyB();
//      
//      // Agent A (centroid will be the repulsion point)
//      auto agentIdA = reinterpret_cast<VertexData*>(bodyA -> GetUserData()) -> agentId;
//      auto &agentA = agents.at(agentIdA);
//      auto centroidA = agentA.getCentroid();
//      
//      // Agent B (centroid will be the repulsion point)
//      auto agentIdB = reinterpret_cast<VertexData*>(bodyB -> GetUserData()) -> agentId;
//      auto &agentB = agents.at(agentIdB);
//      auto centroidB = agentB.getCentroid();
//      
//      // Set a repulsion target for agent A
//      //agentA.setRepulsionTarget(&agentB, agentIdB);
//      agentB.setRepulsionTarget(&agentA, agentIdA);
//    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if (key == 'd') {
    debug = !debug;
  }
  
  if (key == 'n') {
    createAgent(); 
  }
  
  if (key == 'c') {
    // [WARNING] For some reason, these events are still fired when trying to clean things as one could be in the
    // middle of a step function. Disabling and renabling the events work as a good solution for now.
    box2d.disableEvents();
    
    collidingBodies.clear();
    cleanInterAgentJoints();
    for (auto &a : agents) {
      a.clean(box2d);
    }
    agents.clear();
    agentNum = 0;
    
    box2d.enableEvents();
  }
  
  if (key == 'j') {
    box2d.disableEvents();
    cleanInterAgentJoints();
    box2d.enableEvents();
  }
  
  if (key == 'h') {
    hideGui = !hideGui;
  }
  
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a.setRandomForce();
    }
  }
  
  if (key == 'r') {
    enableRepulsion();
  }
}

void ofApp::mousePressed(int x, int y, int button) {
   for (auto &a: agents) {
    a.setAttractionTarget(glm::vec2(x, y));
  }
}

void ofApp::exit() {
  box2d.disableEvents();
  gui.saveToFile("InterMesh.xml");
}

void ofApp::handleSerial() {
  while (serial.available() > 0)
    {
        // Read the byte.
        char b = serial.readByte();
      
        // End of line character.
        if (b == '\n')
        {
            // Skip
            cout<< "New line" << "\n";
        }
        else
        {
            // If it's not the line feed character, add it to the buffer.
            cout << "Received: " << b << "\n";
            startRepelling = b - '0';
            cout << "Replling: " << startRepelling << "\n";
        }
    }
}

// Massive important function that determines when the 2 bodies actually bond.
void ofApp::evaluateBonding(b2Body *bodyA, b2Body *bodyB, Agent *agentA, Agent *agentB) {
  if (agentA != agentB) {
    // Visual similarly
    bool isSimilar = hasVisualSimilarities(agentA, agentB);
    if (isSimilar) {
      // Is AgentA's partner AgentB
      // Is AgentB's partner AgentA
      if (agentA -> getPartner() == agentB && agentB -> getPartner() == agentA) {
        // Vertex level checks. Is this vertex bonded to
        // anything except itself?
        bool a = canVertexBond(bodyA, agentA);
        bool b = canVertexBond(bodyB, agentB);
        if (a && b) {
          // Prepare for bond.
          collidingBodies.push_back(bodyA);
          collidingBodies.push_back(bodyB);
        }
      }
    }
  }
}

bool ofApp::hasVisualSimilarities(Agent *agentA, Agent *agentB) {
  // How many common colors are between the two arrays?
  int commonColorsNum = 0;
  int uncommonColorsNum = 0;
  bool a; bool b;

  auto colors = agentA -> colors;
  for (int i = 0; i < colors.size(); i++) {
    a = ofContains(agentA -> colorSlots, colors.at(i));
    b = ofContains(agentB -> colorSlots, colors.at(i));
    if (a & b) {
      commonColorsNum++;
    }

    if (!a && !b) {
      // Color isn't in a and b.
    } else {
      // It's in one but not the other.
      uncommonColorsNum++;
    }
  }

  // Should be some common and some uncommon colors.
  return commonColorsNum >= 2 && uncommonColorsNum >=1;
}

bool ofApp::canVertexBond(b2Body* body, Agent *curAgent) {
  // If it joins anything except itself, then it cannot join.
  auto curEdge = body -> GetJointList();
  // Traverse the joint doubly linked list.
  while (curEdge && curEdge != curEdge -> next) {
    // Other agent that this joint might be joined to
    auto otherAgent = reinterpret_cast<VertexData*>(curEdge->other->GetUserData())->agent;

    // It's not the same body? That means it's bonded with someone else already. 
    if (otherAgent != curAgent) {
      return false;
    }
    curEdge = curEdge -> next;
  }

  return true;
}



//
//int ofApp::findOtherAgent(b2Body *body, int curAgentId) {
////  if (body == NULL) {
////    return -1;
////  }
////
////  auto curEdge = body -> GetJointList();
////  // Traverse the joint doubly linked list.
////  while (curEdge && curEdge != curEdge -> next) {
////    // Get the other body and check if its agentId is same as
////    // bodyBAgentId
////    if (curEdge->other == NULL) {
////      ofLogWarning("A Null Pointer came around");
////    }
////
////    auto otherAgentId = reinterpret_cast<VertexData*>(curEdge->other->GetUserData())->agentId;
////    // It's not the same body? That means it's jointed with someone else.
////    if (otherAgentId != curAgentId) {
////      return otherAgentId;
////    }
////
////    curEdge = curEdge -> next;
////  }
////
////  return curAgentId;
//}
//
//
////  if (agentA != agentB) { // Hope it's not the same agent
////    // Agent instances
////    auto agent1 = agents.at(agentA);
////    auto agent2 = agents.at(agentB);
////
////
////    // These two for loops ensure that A doesn't bond with anybody except B
////    // And B doesn't bond with anybody except A
////    for (auto v : agent1.vertices) {
////      int otherAgentId = findOtherAgent(v->body, agentA);
////      if (otherAgentId == agentA || otherAgentId == agentB) {
////        // Cool. Keep going through each other.
////      } else {
////        // No bonding.
////        return;
////      }
////    }
////
////    for (auto v : agent2.vertices) {
////      int otherAgentId = findOtherAgent(v->body, agentB);
////      if (otherAgentId == agentB || otherAgentId == agentA) {
////        // Cool. Keep going through each other.
////      } else {
////        // No bonding.
////        return;
////      }
////    }
////
////  }
////
////  return false;


  // Agent Level Checks
  // 0
  // Both the agents should be different.

  // 1
  // Does AgentA's colors match with AgentB's colors. Does the visual appearance match
  // for both the agents to bond?

  // 2
  // Is AgentA bonded with anybody? If it's bonded with AgentB and itself, then keep going
  // Is AgentB bonded with anybody? If it's bonded with AgentA and itself, then keep going

  // Vertex level check
  // 3
  // If it's bonded to anything else except itself
  // then it cannot bond.
  // Two agents are not the same
