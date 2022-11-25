#pragma once
#include <functional>
#include "framework.h"

class UNITY_INTERFACE_EXPORT Action
{
public:
	using Key = int;
	Action() = default;
	~Action() = default;
	Action(const Action&) = default;
	Action(Action&&) = default;
	Action& operator=(const Action&) = default;
	Action& operator=(Action&&) = default;

	virtual Key GetKey() const = 0;
	virtual bool DoAction(const int time) = 0;
};