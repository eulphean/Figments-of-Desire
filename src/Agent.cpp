#include "Agent.h"

void Agent::setup(ofxBox2d &box2d, AgentProperties agentProps) {
  // Populate color slots.
  populateSlots();
  createTexture(agentProps.meshSize); // Design the look of this creature.
  
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
  
  // Calculate a targetPerceptionRad based on the size of the mesh
  auto area = agentProps.meshSize.x * agentProps.meshSize.y;
  targetPerceptionRad = sqrt(area/PI);
  
  // Force weights for various body activities. 
  attractWeight = 0.1;
  randWeight = 0.3;
  
  // Healths to keep track when to execute something again.
  tickleHealth = 100;
  targetHealth = 100;
  
  applyRandomForce = true;
  attractTarget = false;
  repelTarget = false;
}

void Agent::update() {
  // Use box2d circle to update the mesh.
  updateMesh();
  
  // Apply behavioral forces on the body.
  applyBehaviors();
}

void Agent::draw(bool debug) {
  // Draw the meshes.
  // Draw the soft bodies.
  ofPushStyle();
    for(auto v: vertices) {
      ofNoFill();
      ofSetColor(ofColor::red);
      v->draw();
    }
  ofPopStyle();
  
  // Bind the fbo.
  fbo.getTexture().bind();
  mesh.draw();
  fbo.getTexture().unbind();

  if (debug) {
    auto centroid = mesh.getCentroid();
    ofPushMatrix();
      ofTranslate(centroid);
      ofNoFill();
      ofSetColor(ofColor::white);
      ofDrawCircle(0, 0, targetPerceptionRad * 1.5);
    ofPopMatrix();
  }
}

void Agent::createTexture(ofPoint meshSize) {
  // Create a simple fbo. 
  fbo.allocate(meshSize.x, meshSize.y, GL_RGBA);
  fbo.begin();
    ofClear(255, 255, 255, 0);
    ofBackground(ofColor::green);
    int gap = meshSize.x / maxSlots;
  
    // Create the slots in the fbo. 
    for (int i = 0, x = 0; i < maxSlots; i++, x+=gap) {
      ofSetColor(colorSlots.at(i));
      ofDrawRectangle(x, 0, gap, ofGetHeight());
    }
  fbo.end();
}

void Agent::populateSlots() {
  colorSlots.clear();
  for (int i = 0; i < maxSlots; i++) {
    int randIdx = ofRandom(colors.size());
    colorSlots.push_back(colors.at(randIdx));
  }
}

void Agent::applyBehaviors() {
  handleTickle();
  handleAttraction();
  handleRepulsion();
}

void Agent::handleTickle() {
  // Check for tickle health.
  if (tickleHealth == 0) {
    applyRandomForce = true;
    tickleHealth = 100;
  } else {
    tickleHealth -= 0.1;
  }
  
  // Random force/ Force all vertices.
  if (applyRandomForce) {
    // force()
    for (auto &v: vertices) {
      v -> addForce(glm::vec2(ofRandom(-20, 20), ofRandom(-20, 20)), randWeight);
    }
    applyRandomForce = false;
  }
}

void Agent::handleAttraction() {
  // New target to go to.
  if (targetHealth == 0) {
    // Calculate a random target and go there
    attractTargetPos = glm::vec2(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
    attractTarget = true;
    targetHealth = 100;
  } else {
    targetHealth -= 1.0;
  }

  // New target? Add an impulse in that direction.
  if (attractTarget) {
    // seek()
    for (auto &v: vertices) {
        v->addAttractionPoint(attractTargetPos.x, attractTargetPos.y, attractWeight);
        v->setRotation(ofRandom(120));
    }
    attractTarget = false;
  }
}

void Agent::handleRepulsion() {
  // Initiate a repel on all the vertices and then stop.
  // Repulsion impulse and then stop.
  if (repelTarget) {
    for (auto &v: vertices) {
      v->addRepulsionForce(repelTargetPos.x, repelTargetPos.y, 0.8);
    }
    
    repelTarget = false;
  }
  
  // Once or keep applying the force randomly?
}

glm::vec2 Agent::getCentroid() {
  return mesh.getCentroid();
}

void Agent::clean() {
  // Removes vertices.
  ofRemove(vertices, [&](std::shared_ptr<ofxBox2dCircle> c){
      return true;
  });
  
  // Remove joints.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
      return true;
  });
}

void Agent::setAttractionTarget(glm::vec2 target) {
  attractTargetPos = target;
  attractTarget = true;
}

void Agent::setRepulsionTarget(glm::vec2 target) {
  repelTargetPos = target; 
  repelTarget = true;
}

void Agent::setRandomForce() {
  applyRandomForce = true;
}

std::shared_ptr<ofxBox2dCircle> Agent::getRandomVertex() {
  int randV = ofRandom(vertices.size());
  auto v = vertices[randV];
  return v;
}

void Agent::createMesh(AgentProperties agentProps) {
  //auto a = ofRandom(50, ofGetWidth() - 50); auto b = ofRandom(50, ofGetHeight() - 50);
  auto a = ofPoint(100, 100);
  auto meshOrigin = glm::vec2(a.x, a.y);
  
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = agentProps.meshDimensions.x;
  int nCols = agentProps.meshDimensions.y;
  
  // Width, height for mapping the correct texture coordinate.
  int w = agentProps.meshSize.x;
  int h = agentProps.meshSize.y;
  
  auto texture = fbo.getTexture();
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = meshOrigin.x + w * x / (nCols - 1);
      float iy = meshOrigin.y + h * y / (nRows - 1);
      
      cout << "ix, iy: " << ix << ", " << iy << endl;
     
      mesh.addVertex({ix, iy, 0});
      
      float texX = ofMap(ix - meshOrigin.x, 0, fbo.getTexture().getWidth(), 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(iy - meshOrigin.y, 0, fbo.getTexture().getHeight(), 0, 1, true); // Map the calculated y coordinate from 0 - 1
      mesh.addTexCoord({texX, texY});
    }
  }

  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < nRows - 1; y++)
  {
      for (int x = 0; x < nCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * nCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));
      }
  }
}

void Agent::createSoftBody(ofxBox2d &box2d, AgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  
  vertices.clear();
  joints.clear();

  int vId = 0;
  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.z); // bounce, density, friction
    vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, ofRandom(3, agentProps.vertexRadius));
    vertex -> setFixedRotation(true);
    vertex->setData(new VertexData(agentProps.agentId, vId)); // Data to identify current agent.
    vertices.push_back(vertex);
    vId++; 
  }
  
  int meshRows = agentProps.meshDimensions.x;
  int meshColumns = agentProps.meshDimensions.y;
  
  // Create Box2d joints for the mesh.
  for (int y = 0; y < meshRows; y++) {
    for (int x = 0; x < meshColumns; x++) {
      int idx = x + y * meshColumns;
      
      // Do this for all columns except last column.
      // NOTE: Connect current vertex with the next vertex in the same row.
      if (x != meshColumns - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int rightIdx = idx + 1;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[rightIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y); // frequency, damping
        joints.push_back(joint);
      }
      
      
      // Do this for each row except the last row. There is no further joint to
      // be made there.
      if (y != meshRows - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int downIdx = x + (y + 1) * meshColumns;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[downIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y);
        joints.push_back(joint);
      }
    }
  }
}

void Agent::updateMesh() {
  auto meshPoints = mesh.getVertices();
  
  for (int j = 0; j < meshPoints.size(); j++) {
    // Get the box2D vertex position.
    glm::vec2 pos = vertices[j] -> getPosition();
    
    // Update mesh point's position with the position of
    // the box2d vertex.
    auto meshPoint = meshPoints[j];
    meshPoint.x = pos.x;
    meshPoint.y = pos.y;
    mesh.setVertex(j, meshPoint);
  }
}

// Avoid texture right now.
//      // Since, we have ofDisableArbTex, we map the coordinates from 0 - 1.
//      float texX = ofMap(ix, 0, agentProps.textureDimensions.x, 0, 1, true); // Map the calculated x coordinate from 0 - 1
//      float texY = ofMap(iy, 0, agentProps.textureDimensions.y, 0, 1, true); // Map the calculated y coordinate from 0 - 1
//      mesh.addTexCoord(glm::vec2(texX, texY));


//      glm::vec2 currentPos = glm::vec2(v->getPosition().x, v->getPosition().y);
//      auto desired = targetPos - currentPos; // Target - Current
//      desired = glm::normalize(desired);
//      auto steer = v->getVelocity() - desired;
//      v->addImpulseForce(steer, maxSteerImpulse);
//      v->addForce(steer, 0.2);
//      v->setRotation(ofRandom(60));
