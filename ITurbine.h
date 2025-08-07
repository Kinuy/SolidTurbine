#pragma once

// Base interface for all turbines expl. horizontal and vertical turbines
class ITurbine {
	public:

	virtual ~ITurbine() = default;

	virtual int getNumberOfBlades() const = 0;
};