#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <PxPhysicsAPI.h>

using namespace physx;
using namespace std;

class Actor {
private:
	PxRigidDynamic *body;
	string name;

public:
	Actor(PxPhysics *physicsEngine, string name, PxGeometry *shape, PxMaterial *material, float density);
	~Actor();

	operator PxRigidDynamic*();

	void SetPosition(float x, float y, float z);
	PxVec3 GetPosition();
	void SetLinearVelocity(float x, float y, float z);
	void SetPose(PxVec3 pos, PxQuat rot);
	bool IsSleeping() const;
};

#endif