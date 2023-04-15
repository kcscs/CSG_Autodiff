#include "IdManager.h"

IdManager::IdManager()
{
	nextId = 1;
}

int IdManager::Get()
{
	if (freedIds.begin() != freedIds.end()) {
		int id = *freedIds.begin();
		freedIds.erase(freedIds.begin());
		return id;
	}
	return nextId++;
}

void IdManager::Free(int id)
{
	freedIds.push_back(id);
}

void IdManager::Reset()
{
	nextId = 1;
	freedIds.clear();
}
