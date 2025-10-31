#include "Common.h"
#include "AvatarQuestCurrency.h"
#include "Random.h"

namespace AvatarQuest {

// Conversion ratios in base value units (Gold-based) for each currency.
// Non-convertible currencies use 0 to exclude them from value totals.
static constexpr int64_t kCurrencyToValue[] = {
    1,   // Gold: 1 gold = 1 value unit
    0    // Food: non-convertible
};
static_assert(sizeof(kCurrencyToValue)/sizeof(kCurrencyToValue[0]) == static_cast<size_t>(CurrencyType::COUNT),
    "kCurrencyToValue must match CurrencyType::COUNT");

const char* currencyTypeToString(CurrencyType t) {
    switch (t) {
        case CurrencyType::Gold:     return "Gold";
        case CurrencyType::Food:     return "Food";
        default: return "Currency";
    }
}

static inline int64_t getCount(const CurrencyBundle& b, CurrencyType t) {
    int key = static_cast<int>(t);
    auto it = b.amounts.find(key);
    if (it != b.amounts.end()) return it->second;
    return 0;
}

static inline void setCount(CurrencyBundle& b, CurrencyType t, int64_t v) {
    if (v <= 0) {
        // keep map tidy; remove zero/negative
        b.amounts.erase(static_cast<int>(t));
    } else {
        b.amounts[static_cast<int>(t)] = v;
    }
}

int64_t toValueTotal(const CurrencyBundle& b) {
    int64_t total = 0;
    for (int i = 0; i < static_cast<int>(CurrencyType::COUNT); ++i) {
        auto t = static_cast<CurrencyType>(i);
        int64_t c = getCount(b, t);
        int64_t unit = kCurrencyToValue[i];
        if (c > 0 && unit > 0) total += c * unit;
    }
    return total;
}

void normalize(CurrencyBundle& b) {
    // Convert only value currencies to base and recompose (Gold-only at present).
    int64_t value = toValueTotal(b);
    // Preserve non-convertible currencies (e.g., Food)
    UMap<int, int64_t> preserved;
    for (int i = 0; i < static_cast<int>(CurrencyType::COUNT); ++i) {
        if (kCurrencyToValue[i] == 0) {
            auto t = static_cast<CurrencyType>(i);
            int64_t cnt = getCount(b, t);
            if (cnt > 0) preserved[static_cast<int>(t)] = cnt;
        }
    }
    b.amounts.clear();
    // Recompose value currencies
    for (int i = static_cast<int>(CurrencyType::COUNT) - 1; i >= 0; --i) {
        int64_t unit = kCurrencyToValue[i];
        int64_t cnt = (unit > 0) ? (value / unit) : 0;
        if (cnt > 0) {
            setCount(b, static_cast<CurrencyType>(i), cnt);
            value -= cnt * unit;
        }
    }
    // Restore non-convertibles
    for (const auto& kv : preserved) b.amounts[kv.first] = kv.second;
}

void addCurrency(CurrencyBundle& b, CurrencyType t, int64_t amount) {
    if (amount == 0) return;
    int64_t prev = getCount(b, t);
    setCount(b, t, prev + amount);
    if (amount > 0) normalize(b); // keep normalized for positive additions
}

void addBundle(CurrencyBundle& dst, const CurrencyBundle& src) {
    for (int i = 0; i < static_cast<int>(CurrencyType::COUNT); ++i) {
        auto t = static_cast<CurrencyType>(i);
        int64_t amt = getCount(src, t);
        if (amt) addCurrency(dst, t, amt);
    }
}

bool spendValue(CurrencyBundle& from, int64_t value) {
    if (value <= 0) return true;
    int64_t total = toValueTotal(from);
    if (total < value) return false;
    int64_t remain = total - value;
    // Preserve non-convertibles
    UMap<int, int64_t> preserved;
    for (int i = 0; i < static_cast<int>(CurrencyType::COUNT); ++i) {
        if (kCurrencyToValue[i] == 0) {
            auto t = static_cast<CurrencyType>(i);
            int key = static_cast<int>(t);
            auto it = from.amounts.find(key);
            if (it != from.amounts.end() && it->second > 0) preserved[key] = it->second;
        }
    }
    from.amounts.clear();
    // Recompose remaining
    for (int i = static_cast<int>(CurrencyType::COUNT) - 1; i >= 0; --i) {
        int64_t unit = kCurrencyToValue[i];
        int64_t cnt = (unit > 0) ? (remain / unit) : 0;
        if (cnt > 0) {
            setCount(from, static_cast<CurrencyType>(i), cnt);
            remain -= cnt * unit;
        }
    }
    // Restore non-convertibles
    for (const auto& kv : preserved) from.amounts[kv.first] = kv.second;
    return true;
}

bool spendBundle(CurrencyBundle& from, const CurrencyBundle& cost) {
    // First, ensure non-convertible currencies can be paid exactly
    for (int i = 0; i < static_cast<int>(CurrencyType::COUNT); ++i) {
        int64_t unit = kCurrencyToValue[i];
        if (unit == 0) {
            auto t = static_cast<CurrencyType>(i);
            int64_t need = getCount(cost, t);
            if (need > 0) {
                if (getCount(from, t) < need) return false;
            }
        }
    }
    // Deduct non-convertibles
    for (int i = 0; i < static_cast<int>(CurrencyType::COUNT); ++i) {
        int64_t unit = kCurrencyToValue[i];
        if (unit == 0) {
            auto t = static_cast<CurrencyType>(i);
            int64_t need = getCount(cost, t);
            if (need > 0) setCount(from, t, getCount(from, t) - need);
        }
    }
    // Pay value-based part using spendValue
    int64_t valueNeed = toValueTotal(cost);
    if (valueNeed > 0) return spendValue(from, valueNeed);
    return true;
}

bool canAfford(const CurrencyBundle& from, const CurrencyBundle& cost) {
    // Non-convertibles must be available in exact counts
    for (int i = 0; i < static_cast<int>(CurrencyType::COUNT); ++i) {
        int64_t unit = kCurrencyToValue[i];
        if (unit == 0) {
            auto t = static_cast<CurrencyType>(i);
            if (getCount(from, t) < getCount(cost, t)) return false;
        }
    }
    // Value portion must be affordable by total
    return toValueTotal(from) >= toValueTotal(cost);
}

void createCurrency(CurrencyType t, int64_t amount, int tileID, CurrencyBundle& out) {
    out.amounts.clear();
    out.tileID = tileID;
    if (amount > 0) setCount(out, t, amount);
}

void createPurse(int64_t gold, int64_t food, int64_t /*unused0*/, int64_t /*unused1*/, CurrencyBundle& out) {
    out.amounts.clear();
    if (gold > 0) setCount(out, CurrencyType::Gold, gold);
    if (food > 0) setCount(out, CurrencyType::Food, food);
    normalize(out);
}

void createRandomPurse(int playerLevel, CurrencyBundle& out) {
    if (playerLevel < 1) playerLevel = 1;
    // Scale gold value approximately with level; add variance
    // Base: 1 gold per level with +/- 50% variance (floored at 0)
    int basePerLevel = 1;
    int variancePct = 50;
    int base = basePerLevel * playerLevel;
    int deltaPct = Random::getInt(-variancePct, variancePct);
    int64_t gold = static_cast<int64_t>(static_cast<double>(base) * (100 + deltaPct) / 100.0);
    if (gold < 0) gold = 0;
    // Compose into currencies
    out.amounts.clear();
    if (gold > 0) setCount(out, CurrencyType::Gold, gold);
}

}
