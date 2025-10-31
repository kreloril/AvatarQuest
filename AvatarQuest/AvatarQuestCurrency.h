#pragma once

#include "Common.h"

namespace AvatarQuest {

// Currencies used across the game. Keep this enum small now and add over time.
enum struct CurrencyType : int {
    Gold = 0,  // primary money value
    Food,      // rations/food tokens (non-convertible to Gold)
    COUNT
};

// Bag/purse of money keyed by CurrencyType id
struct CurrencyBundle {
    // key = static_cast<int>(CurrencyType), value = count of that denomination
    UMap<int, int64_t> amounts;
    // Optional tile to render this bundle in UI/world (e.g., coin pile icon)
    int tileID = -1;
};

// Names and conversions
const char* currencyTypeToString(CurrencyType t);
// Convert an arbitrary bundle to base value units (Gold-based). Non-convertible
// currencies like Food contribute 0 to this total.
int64_t toValueTotal(const CurrencyBundle& b);
// Normalize only value-based currencies (currently Gold). Non-convertible
// currencies are left untouched.
void normalize(CurrencyBundle& b);

// Basic operations
void addCurrency(CurrencyBundle& b, CurrencyType t, int64_t amount);
void addBundle(CurrencyBundle& dst, const CurrencyBundle& src); // dst += src
// Spend a mixed cost: non-convertible currencies (e.g., Food) must be available
// in exact counts; value currencies (Gold) are spent by total value.
bool spendBundle(CurrencyBundle& from, const CurrencyBundle& cost);
// Spend by total value (Gold-based units). Non-convertible currencies are not affected.
bool spendValue(CurrencyBundle& from, int64_t value);
// Affordability check for mixed bundles as in spendBundle.
bool canAfford(const CurrencyBundle& from, const CurrencyBundle& cost);

// Creators
// Create a bundle with a single denomination and assign a render tile id
void createCurrency(CurrencyType t, int64_t amount, int tileID, CurrencyBundle& out);
// Back-compat helper: interpret as (gold, food, -, -)
void createPurse(int64_t gold, int64_t food, int64_t unused0, int64_t unused1, CurrencyBundle& out);
// Create a randomized purse based on an approximate player level (Gold only)
void createRandomPurse(int playerLevel, CurrencyBundle& out);

}
