#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Common.h"
#include "AvatarQuestCurrency.h"
#include "AvatarQuestRP.h"

using namespace AvatarQuest;

static std::string bundleToString(const CurrencyBundle& b) {
    auto count = [&](CurrencyType t){
        int key = static_cast<int>(t);
        auto it = b.amounts.find(key);
        return (it != b.amounts.end()) ? it->second : 0LL;
    };
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "P:%lld G:%lld S:%lld C:%lld | totalC:%lld | tileID:%d",
        (long long)count(CurrencyType::Platinum),
        (long long)count(CurrencyType::Gold),
        (long long)count(CurrencyType::Silver),
        (long long)count(CurrencyType::Copper),
        (long long)toCopperTotal(b),
        b.tileID);
    return std::string(buf);
}

TEST_CASE("Currency: normalize and totals", "[currency]") {
    CurrencyBundle b{};
    // 3 gold, 5 silver, 250 copper -> normalize should carry 250c -> 2s 50c
    createPurse(/*platinum*/0, /*gold*/3, /*silver*/5, /*copper*/250, b);
    WARN("After normalize: " << bundleToString(b));
    // Totals in copper: 3*10000 + 7*100 + 50 = 30000 + 700 + 50 = 30750
    REQUIRE(toCopperTotal(b) == 30750);
}

TEST_CASE("Currency: add and spend by copper", "[currency]") {
    CurrencyBundle wallet{}; createPurse(0,1,0,0, wallet); // 1 gold = 10000 c
    WARN("Initial:  " << bundleToString(wallet));
    REQUIRE(toCopperTotal(wallet) == 10000);
    // Spend 9999 copper
    REQUIRE(spendCopper(wallet, 9999) == true);
    WARN("Post-9999: " << bundleToString(wallet));
    REQUIRE(toCopperTotal(wallet) == 1); // 1 copper remains
    // Can't spend more than we have
    REQUIRE(spendCopper(wallet, 2) == false);
}

TEST_CASE("Currency: canAfford and spendBundle", "[currency]") {
    CurrencyBundle wallet{}; createPurse(0,0,50,0, wallet); // 50 silver = 5000 c
    CurrencyBundle cost{};   createPurse(0,0,10,75, cost);  // 10 s 75 c = 1075 c
    WARN("Wallet: " << bundleToString(wallet));
    WARN("Cost:   " << bundleToString(cost));
    REQUIRE(canAfford(wallet, cost));
    REQUIRE(spendBundle(wallet, cost));
    WARN("After:  " << bundleToString(wallet));
    REQUIRE(toCopperTotal(wallet) == 5000 - 1075);
}

TEST_CASE("Currency: random purse scales with level on average", "[currency][random]") {
    // Sample a few times and compare average totals
    auto avgTotal = [](int level){
        int64_t sum = 0;
        for (int i = 0; i < 16; ++i) {
            CurrencyBundle b{}; createRandomPurse(level, b);
            sum += toCopperTotal(b);
        }
        return static_cast<double>(sum) / 16.0;
    };
    double avg1  = avgTotal(1);
    double avg10 = avgTotal(10);
    WARN("avg1=" << avg1 << ", avg10=" << avg10);
    REQUIRE(avg10 > avg1);
}

TEST_CASE("Currency: sheet wallet add/spend integrates", "[currency][sheet]") {
    AvatarQuest::CharacterSheet cs{};
    WARN("Initial wallet: " << bundleToString(getWallet(cs)));
    REQUIRE(toCopperTotal(getWallet(cs)) == 0);
    // Add some funds
    addFunds(cs, CurrencyType::Gold, 1); // 10,000 copper
    WARN("After add G1:  " << bundleToString(getWallet(cs)));
    REQUIRE(toCopperTotal(getWallet(cs)) == 10000);
    // Spend
    CurrencyBundle cost{}; createPurse(0,0,5,50, cost); // 5s 50c = 550c
    WARN("Spend cost:     " << bundleToString(cost));
    REQUIRE(spendFunds(cs, cost));
    WARN("After spend:    " << bundleToString(getWallet(cs)));
    REQUIRE(toCopperTotal(getWallet(cs)) == 10000 - 550);
    // Tile stays as default unless explicitly set
    REQUIRE(getWallet(cs).tileID == -1);
    getWallet(cs).tileID = 123;
    WARN("Tile set ->     " << bundleToString(getWallet(cs)));
    REQUIRE(getWallet(cs).tileID == 123);
}
