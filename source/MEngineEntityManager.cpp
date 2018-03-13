#include "Interface/MEngineEntityManager.h"
#include "Interface/MEngineSettings.h"
#include "MEngineEntityManagerInternal.h"
#include "MEngineComponentManagerInternal.h"
#include <MUtilityIDBank.h>
#include <MUtilityIntrinsics.h>
#include <MUtilityLog.h>
#include <MUtilityMath.h>
#include <MUtilityPlatformDefinitions.h>
#include <cassert>

#define LOG_CATEGORY_ENTITY_MANAGER "MEngineEntityManager"

using namespace MEngine;
using namespace MEngineEntityManager;
using MUtility::MUtilityIDBank;

// ---------- LOCAL ----------

namespace MEngineEntityManager
{
	int32_t GetEntityIndex(EntityID ID);
	uint32_t CalcComponentIndiceListIndex(ComponentMask entityComponentMask, ComponentMask componentType);
	ComponentMask RemoveComponentsFromEntityByIndex(ComponentMask componentMask, int32_t entityIndex);

	std::vector<EntityID>*		m_Entities;
	std::vector<ComponentMask>*	m_ComponentMasks;
	std::vector<std::vector<uint32_t>>* m_ComponentIndices;
	MUtilityIDBank* m_IDBank;
}

// ---------- INTERFACE ----------

EntityID MEngine::CreateEntity() // TODODB: Take component mask and add the components described by the mask
{
	EntityID ID = m_IDBank->GetID();
	m_Entities->push_back(ID);
	m_ComponentMasks->push_back(MUtility::EMPTY_BITSET);
	m_ComponentIndices->push_back(std::vector<uint32_t>());

	return ID;
}

bool MEngine::DestroyEntity(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_IDBank->IsIDActive(ID))
	{
		if(Settings::HighLogLevel)
			MLOG_WARNING("Attempted to destroy entity using an inactive entity ID; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);

		return false;
	}
#endif

	int32_t entityIndex = GetEntityIndex(ID);
	if (entityIndex >= 0)
	{
		if ((*MEngineEntityManager::m_Entities)[entityIndex] == ID)
		{
			RemoveComponentsFromEntityByIndex((*m_ComponentMasks)[entityIndex], entityIndex);

			m_Entities->erase(m_Entities->begin() + entityIndex);
			m_ComponentMasks->erase(m_ComponentMasks->begin() + entityIndex);
			m_ComponentIndices->erase(m_ComponentIndices->begin() + entityIndex);
			m_IDBank->ReturnID(ID);
			return true;
		}
	}
	return false;
}

ComponentMask MEngine::AddComponentsToEntity(ComponentMask componentMask, EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to add component(s) to entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentMask), LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
	else if (!m_IDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to add components to an entity that doesn't exist; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
#endif

	std::vector<uint32_t>& componentIndices = (*m_ComponentIndices)[ID];
	while (componentMask != MUtility::EMPTY_BITSET)
	{
		ComponentMask singleComponentMask = MUtility::GetHighestSetBit(componentMask);
		uint32_t componentIndiceListIndex = CalcComponentIndiceListIndex((*m_ComponentMasks)[ID], singleComponentMask);
		componentIndices.insert(componentIndices.begin() + componentIndiceListIndex, MEngineComponentManager::AllocateComponent(singleComponentMask, ID));
		(*m_ComponentMasks)[ID] |= singleComponentMask;

		componentMask &= ~MUtility::GetHighestSetBit(componentMask);
	}

	return MUtility::EMPTY_BITSET;
}

ComponentMask MEngine::RemoveComponentsFromEntity(ComponentMask componentMask, EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to removed component(s) from entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentMask), LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
	else if (!m_IDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to remove component(s) from an entity that doesn't exist; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
#endif

	int32_t entityIndex = GetEntityIndex(ID);
	if(entityIndex >= 0)
	{
		if((*m_Entities)[entityIndex] == ID)
			return RemoveComponentsFromEntityByIndex(componentMask, entityIndex);
	}

	return componentMask;
}

// TODODB: Validate component mask in debug mode
void MEngine::GetEntitiesMatchingMask(ComponentMask componentMask, std::vector<EntityID>& outEntities, MaskMatchMode matchMode)
{
	for (int i = 0; i < m_Entities->size(); ++i)
	{
		ComponentMask currentMask = (*m_ComponentMasks)[i];
		bool isMatch = false;
		switch (matchMode)
		{
			case MaskMatchMode::Any:
			{
				isMatch = ((currentMask & componentMask) != 0);
			} break;

			case MaskMatchMode::Partial:
			{
				isMatch = ((currentMask & componentMask) == componentMask);
			} break;

			case MaskMatchMode::Exact:
			{
				isMatch = ((currentMask & componentMask) == componentMask && MUtility::PopCount(componentMask) == MUtility::PopCount(currentMask));
			} break;

		default:
			MLOG_ERROR("Received unknown matchMode", LOG_CATEGORY_ENTITY_MANAGER);
			break;
		}

		if (isMatch)
			outEntities.push_back(i);	
	}
}

MEngine::Component* MEngine::GetComponentForEntity(ComponentMask componentType, EntityID entityID) // TODODB: Rename last parameter to ID for consistency
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentType == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to get component for entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
	else if (!m_IDBank->IsIDActive(entityID))
	{
		MLOG_WARNING("Attempted to get component for an entity that doesn't exist; ID = " << entityID, LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
	else if (MUtility::PopCount(componentType) != 1)
	{
		MLOG_WARNING("Attempted to get component for an entity using a component mask containing more or less than one component; mask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
#endif

	int32_t entityIndex = GetEntityIndex(entityID);
	if (entityIndex >= 0)
	{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
		if (((*m_ComponentMasks)[entityIndex] & componentType) == 0)
		{
			MLOG_WARNING("Attempted to get component of type " << MUtility::BitSetToString(componentType) << " for an entity that lacks that component type; entity component mask = " << MUtility::BitSetToString((*m_ComponentMasks)[entityID]), LOG_CATEGORY_ENTITY_MANAGER);
			return nullptr;
		}
#endif

		uint32_t componentIndex = (*m_ComponentIndices)[entityIndex][CalcComponentIndiceListIndex((*m_ComponentMasks)[entityIndex], componentType)];
		return MEngineComponentManager::GetComponent(componentType, componentIndex);
	}

	return nullptr;
}

ComponentMask MEngine::GetComponentMask(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_IDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get component mask from an entity that doesn't exist; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);
		return INVALID_MENGINE_COMPONENT_MASK;
	}
#endif

	return (*m_ComponentMasks)[ID];
}

// ---------- INTERNAL ----------

void MEngineEntityManager::Initialize()
{
	m_Entities			= new std::vector<EntityID>();
	m_ComponentMasks	= new std::vector<ComponentMask>();
	m_ComponentIndices	= new std::vector<std::vector<uint32_t>>();
	m_IDBank			= new MUtilityIDBank();
}

void MEngineEntityManager::Shutdown()
{
	delete m_Entities;
	delete m_ComponentMasks;
	delete m_ComponentIndices;
	delete m_IDBank;
}

void MEngineEntityManager::UpdateComponentIndex(EntityID ID, ComponentMask componentType, uint32_t newComponentIndex)
{
	int32_t entityIndex = GetEntityIndex(ID);
	if (entityIndex >= 0)
	{
		uint32_t componentIndexListIndex = CalcComponentIndiceListIndex((*m_ComponentMasks)[entityIndex], componentType);
		(*m_ComponentIndices)[entityIndex][componentIndexListIndex] = newComponentIndex;
		return;
	}
}

// ---------- LOCAL ----------

int32_t MEngineEntityManager::GetEntityIndex(EntityID ID) // TODODB: This function does not scale well; need to get rid of it or redesign
{
	for (int i = 0; i < m_Entities->size(); ++i)
	{
		if ((*m_Entities)[i] == ID)
		{
			return i;
		}
	}

	MLOG_ERROR("Failed to find entity with ID " << ID << " even though it is marked as active", LOG_CATEGORY_ENTITY_MANAGER);
	return -1;
}

uint32_t MEngineEntityManager::CalcComponentIndiceListIndex(ComponentMask entityComponentMask, ComponentMask componentType)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if(MUtility::PopCount(componentType) != 1)
		MLOG_ERROR("A component meask containing more or less than i set bit was supplied; only the highest set bit will be considered", LOG_CATEGORY_ENTITY_MANAGER);
#endif
	
	uint32_t componentTypeBitIndex = MUtilityMath::FastLog2(componentType); // Find the index of the bit signifying the component type
	uint64_t shiftedMask = componentTypeBitIndex != 0 ? (entityComponentMask << (MEngineComponentManager::MAX_COMPONENTS - componentTypeBitIndex)) : 0; // Shift away all bits above the component type bit index
	return static_cast<uint32_t>(MUtility::PopCount(shiftedMask)); // Return the number of set bits left in the shifted mask
}

ComponentMask MEngineEntityManager::RemoveComponentsFromEntityByIndex(ComponentMask componentMask, int32_t entityIndex)
{
	ComponentMask failedComponents = 0ULL;
	std::vector<uint32_t>& componentIndices = (*MEngineEntityManager::m_ComponentIndices)[entityIndex];
	while (componentMask != MUtility::EMPTY_BITSET)
	{
		ComponentMask singleComponentMask = MUtility::GetHighestSetBit(componentMask);
		uint32_t componentIndiceListIndex = CalcComponentIndiceListIndex((*m_ComponentMasks)[entityIndex], singleComponentMask);
		if (MEngineComponentManager::ReturnComponent(singleComponentMask, componentIndices[componentIndiceListIndex]))
		{
			componentIndices.erase(componentIndices.begin() + componentIndiceListIndex);
			(*m_ComponentMasks)[entityIndex] &= ~MUtility::GetHighestSetBit(componentMask);
		}
		else
			failedComponents &= MUtility::GetHighestSetBit(componentMask);

		componentMask &= ~MUtility::GetHighestSetBit(componentMask);
	}

	return failedComponents;
}