// Copyright Vicente Hotz. All Rights Reserved.

#include "InventoryItemDefinition.h"
#include "InventoryItemFragment.h"

UInventoryItemFragment* UInventoryItemDefinition::FindFragmentByClass(TSubclassOf<UInventoryItemFragment> FragmentClass) const
{
	if (!FragmentClass) return nullptr;

	for (UInventoryItemFragment* Fragment : Fragments)
	{
		if (Fragment && Fragment->IsA(FragmentClass))
		{
			return Fragment;
		}
	}
	return nullptr;
}
