#pragma once
#include <list>

/// <summary>
/// Simple class for reusing ids. 
/// Used by shader generators.
/// </summary>
class IdManager
{
	int nextId;
	std::list<int> freedIds;

public:
	IdManager();
	int Get();
	void Free(int id);
	void Reset();
};

