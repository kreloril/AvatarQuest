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
        "Gold:%lld Food:%lld | totalValue:%lld | tileID:%d",
        (long long)count(CurrencyType::Gold),
        (long long)count(CurrencyType::Food),
    (long long)toValueTotal(b),
        b.tileID);
    return std::string(buf);
}

TEST_CASE("Currency: gold totals and spend", "[currency]") {
    CurrencyBundle b{};
    createCurrency(CurrencyType::Gold, 3, /*tile*/-1, b);
    WARN("After create: " << bundleToString(b));
    REQUIRE(toValueTotal(b) == 3);
    REQUIRE(spendValue(b, 2) == true);
    WARN("After spend2: " << bundleToString(b));
    REQUIRE(toValueTotal(b) == 1);
    REQUIRE(spendValue(b, 2) == false);
}

TEST_CASE("Currency: food exact spend and affordability", "[currency]") {
    CurrencyBundle wallet{}; createPurse(/*gold*/0, /*food*/5, 0, 0, wallet);
    CurrencyBundle eat{};    createPurse(/*gold*/0, /*food*/2, 0, 0, eat);
    WARN("Wallet: " << bundleToString(wallet));
    WARN("Eat:    " << bundleToString(eat));
    REQUIRE(canAfford(wallet, eat));
    REQUIRE(spendBundle(wallet, eat));
    WARN("After:  " << bundleToString(wallet));
    // Value unchanged (food doesn't add to value), but food count reduced
    REQUIRE(toValueTotal(wallet) == 0);
    // Spend more food than available should fail
    CurrencyBundle tooMuch{}; createPurse(0, 4, 0, 0, tooMuch);
    REQUIRE(!canAfford(wallet, tooMuch));
}

TEST_CASE("Currency: mixed costs (gold + food)", "[currency]") {
    CurrencyBundle wallet{}; createPurse(10, 1, 0, 0, wallet); // 10 gold, 1 food
    CurrencyBundle cost{};   createPurse(5, 1, 0, 0, cost);    // 5 gold, 1 food
    WARN("Wallet: " << bundleToString(wallet));
    WARN("Cost:   " << bundleToString(cost));
    REQUIRE(canAfford(wallet, cost));
    REQUIRE(spendBundle(wallet, cost));
    WARN("After:  " << bundleToString(wallet));
    REQUIRE(toValueTotal(wallet) == 5);
    // Another food needed but none remains
    CurrencyBundle needFood{}; createPurse(0, 1, 0, 0, needFood);
    REQUIRE(!canAfford(wallet, needFood));
}

TEST_CASE("Currency: random purse scales with level on average", "[currency][random]") {
    // Sample a few times and compare average totals
    auto avgTotal = [](int level){
        int64_t sum = 0;
        for (int i = 0; i < 16; ++i) {
            CurrencyBundle b{}; createRandomPurse(level, b);
            sum += toValueTotal(b);
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
    REQUIRE(toValueTotal(getWallet(cs)) == 0);
    // Add some funds
    addFunds(cs, CurrencyType::Gold, 10);
    addFunds(cs, CurrencyType::Food, 3);
    WARN("After add:      " << bundleToString(getWallet(cs)));
    REQUIRE(toValueTotal(getWallet(cs)) == 10);
    // Spend mixed
    CurrencyBundle cost{}; createPurse(2,1,0,0, cost); // 2 gold + 1 food
    WARN("Spend cost:     " << bundleToString(cost));
    REQUIRE(spendFunds(cs, cost));
    WARN("After spend:    " << bundleToString(getWallet(cs)));
    REQUIRE(toValueTotal(getWallet(cs)) == 8);
    // Tile stays as default unless explicitly set
    REQUIRE(getWallet(cs).tileID == -1);
    getWallet(cs).tileID = 123;
    WARN("Tile set ->     " << bundleToString(getWallet(cs)));
    REQUIRE(getWallet(cs).tileID == 123);
}

TEST_CASE("Currency: transfer between character sheets", "[currency][sheet][transfer]") {
    AvatarQuest::CharacterSheet a{};
    AvatarQuest::CharacterSheet b{};
    // Seed wallets
    addFunds(a, CurrencyType::Gold, 10);
    addFunds(a, CurrencyType::Food, 2);
    addFunds(b, CurrencyType::Gold, 1);
    WARN("A start: " << bundleToString(getWallet(a)));
    WARN("B start: " << bundleToString(getWallet(b)));

    // Transfer mixed bundle: 3 gold + 1 food
    CurrencyBundle give{}; createPurse(3,1,0,0, give);
    REQUIRE(transferFunds(a, b, give));
    WARN("A after 3G1F -> B: " << bundleToString(getWallet(a)));
    WARN("B after 3G1F <- A: " << bundleToString(getWallet(b)));
    REQUIRE(toValueTotal(getWallet(a)) == 7);
    REQUIRE(toValueTotal(getWallet(b)) == 4);

    // A has only 1 food left; try to transfer 2 food (should fail, no change)
    CurrencyBundle tooMuchFood{}; createPurse(0,2,0,0, tooMuchFood);
    REQUIRE(!transferFunds(a, b, tooMuchFood));
    REQUIRE(toValueTotal(getWallet(a)) == 7);
    REQUIRE(toValueTotal(getWallet(b)) == 4);

    // Transfer value-only via bundle: 5 gold from B to A (should fail; B has only 4)
    {
        CurrencyBundle fiveG{}; createPurse(5,0,0,0, fiveG);
        REQUIRE_FALSE(transferFunds(b, a, fiveG));
    }
    REQUIRE(toValueTotal(getWallet(a)) == 7);
    REQUIRE(toValueTotal(getWallet(b)) == 4);

    // Transfer 3 gold from B to A (succeeds)
    {
        CurrencyBundle threeG{}; createPurse(3,0,0,0, threeG);
        REQUIRE(transferFunds(b, a, threeG));
    }
    WARN("A after +3G: " << bundleToString(getWallet(a)));
    WARN("B after -3G: " << bundleToString(getWallet(b)));
    REQUIRE(toValueTotal(getWallet(a)) == 10);
    REQUIRE(toValueTotal(getWallet(b)) == 1);
}
