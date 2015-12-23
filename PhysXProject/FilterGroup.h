#ifndef FILTER_GROUP_H
#define FILTER_GROUP_H

struct FilterGroup
{
	enum Enum
	{
		BALL = (1 << 0),
		PIECE = (1 << 1),
		FLOOR = (1 << 2)
	};
};

#endif