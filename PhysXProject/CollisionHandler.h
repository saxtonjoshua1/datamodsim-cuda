#include <iostream>
#include <PxPhysicsAPI.h>

#include "FilterGroup.h"

#ifndef COLLISION_HANDLER_H
#define COLLISION_HANDLER_H

using namespace physx;

class CollisionHandler : public PxSimulationEventCallback {
public:
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);

	void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
	void onWake(PxActor **actors, PxU32 count) {}
	void onSleep(PxActor **actors, PxU32 count) {}
	void onTrigger(PxTriggerPair *pairs, PxU32 count) {}
};

#endif