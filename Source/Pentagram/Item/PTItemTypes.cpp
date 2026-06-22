#include "PTItemTypes.h"

bool FItemData::CanSell() const
{
    return bCanSell;
}

int32 FItemData::GetSellPrice() const
{
    if (!bCanSell)
    {
        return 0;
    }

    if (SellPriceOverride >= 0)
    {
        return SellPriceOverride;
    }

    return FMath::Max(BuyPrice / 4, 1);
}
