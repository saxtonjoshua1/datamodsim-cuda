#include "CollisionHandler.h"

void CollisionHandler::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	for (PxU32 i = 0; i < nbPairs; i++)
	{
		const PxContactPair& cp = pairs[i];

		if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			if ((pairHeader.actors[0]->getType() == PxActorType::eRIGID_DYNAMIC) &&
				(pairHeader.actors[1]->getType() == PxActorType::eRIGID_DYNAMIC))
			{
				std::cout << "Two-body dynamic collision!" << std::endl;
				break;
			} else if ((pairHeader.actors[0]->getType() == PxActorType::eRIGID_STATIC) ||
				(pairHeader.actors[1]->getType() == PxActorType::eRIGID_STATIC))
			{
				if ((strcmp(pairHeader.actors[0]->getName(), "Floor") == 0) ||
					(strcmp(pairHeader.actors[1]->getName(), "Floor") == 0))
				{
					std::cout << "Dynamic actor collided with plane." << std::endl;
					break;
				} else // must have hit cube
				{
					std::cout << "Dynamic actor collided with static object." << std::endl;
					break;
				}
			} else
			{
				std::cout << "YIKES!" << std::endl;
				break;
			}
		}
	}
}