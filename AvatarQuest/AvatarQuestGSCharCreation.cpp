#include "AvatarQuestGSCharCreation.h"
#include "AvatarQuest/Fonts.h"
#include "AvatarQuestSkills.h"
#include "AvatarQuestRP.h"
#include "AvatarQuestClass.h"
#include "AvatarQuestProfile.h"
#include "AvatarQuestPortraits.h"

using namespace AvatarQuest;

GSCharCreation::~GSCharCreation() { }
void GSCharCreation::onEnter() {
    _font = Fonts::ui();
    SDL_Rect ws = Window::getWindowSize();
    const float winW = 880.0f;
    const float winH = 560.0f;
    const float wx = (float)ws.x + (float)ws.w * 0.5f - winW * 0.5f;
    const float wy = (float)ws.y + (float)ws.h * 0.5f - winH * 0.5f;
    _window.setPosition(wx, wy);
    _window.setSize(winW, winH);
    _window.setTitle(_font, "Character Creation");
    _window.setColors(Renderer::Color{30,30,40,220}, Renderer::Color{200,200,220,255});

    _prompt.setFont(_font);
    _prompt.setText("Enter your name:");

    _nameInput.setFont(_font);
    _nameInput.setPlaceholder("Hero");
    _nameInput.setMaxChars(24);
    _nameInput.setColors(Renderer::Color{20,20,20,220}, Renderer::Color{180,180,180,255}, SDL_Color{255,255,255,255}, SDL_Color{160,160,160,255});
    _nameInput.focus(true);
    _nameInput.setOnSubmit([this](const String& /*text*/){
        // Treat Enter as Next from NamePath; do not bypass the wizard
        _step = CharCreateStep::Stats;
        _nameInput.focus(false);
    });
    // Path combo: Custom vs Archetype (kept as ComboBox)
    _comboPath.setFont(_font);
    _comboPath.setColors(Renderer::Color{30,30,40,220}, Renderer::Color{200,200,220,255}, SDL_Color{255,255,255,255},
                         Renderer::Color{20,20,30,240}, Renderer::Color{200,200,220,255}, Renderer::Color{60,80,120,220});
    _comboPath.setItems(Vector<String>{"Custom","Archetype"});
    _comboPath.setSelectedIndex(0);
    _comboPath.setOnChanged([this](int idx){
        _mode = (idx == 1) ? CharCreateMode::Archetype : CharCreateMode::Custom;
        // Auto-fill preview when switching to Archetype
        if (_mode == CharCreateMode::Archetype) {
            int aidx = _comboArchetype.selectedIndex();
            const auto& arcs = getAllArchetypes();
            if (aidx >= 0 && aidx < (int)arcs.size()) {
                const auto& arc = arcs[aidx];
                Vector<std::pair<int,float>> kvs; kvs.reserve(arc.skillTargets.size());
                for (const auto& kv : arc.skillTargets) kvs.push_back(kv);
                std::sort(kvs.begin(), kvs.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
                _selectedSkillIds.clear();
                for (const auto& kv : kvs) { if ((int)_selectedSkillIds.size() >= 5) break; _selectedSkillIds.push_back(kv.first); }
                rebuildSkillLists();
            }
        }
    });

    // Skills selection: labels and empty selected list initially
    _lblAll.setFont(_font);
    _lblAll.setText("All Skills");
    _lblSelected.setFont(_font);
    _lblSelected.setText("Selected Skills");

    _allSkillsList.setFont(_font);
    _allSkillsList.setColors(Renderer::Color{20,20,30,200}, Renderer::Color{180,180,200,255}, Renderer::Color{60,80,120,220}, SDL_Color{230,230,240,255}, SDL_Color{255,255,255,255});
    _selectedSkillsList.setFont(_font);
    _selectedSkillsList.setColors(Renderer::Color{20,20,30,200}, Renderer::Color{180,180,200,255}, Renderer::Color{60,80,120,220}, SDL_Color{230,230,240,255}, SDL_Color{255,255,255,255});

    _btnAdd.setFont(_font);
    _btnAdd.setText("Add >>");
    _btnAdd.setColors(Renderer::Color{50,70,100,220}, Renderer::Color{70,100,140,230}, Renderer::Color{40,60,90,230}, Renderer::Color{200,200,230,255}, SDL_Color{255,255,255,255});
    _btnAdd.setOnClick([this]() {
        // Move selected from left to right
        int idx = _allSkillsList.selectedIndex();
        const String* label = _allSkillsList.selectedItem();
        if (!label || idx < 0) return;
        // Enforce max 5 selections
        if ((int)_selectedSkillIds.size() >= 5) return;
        // Map to actual SkillType via name
        SkillType t = SkillType::None;
        if (skillTypeFromString(label->c_str(), t)) {
            int key = static_cast<int>(t);
            if (std::find(_selectedSkillIds.begin(), _selectedSkillIds.end(), key) == _selectedSkillIds.end()) {
                _selectedSkillIds.push_back(key);
                rebuildSkillLists();
            }
        }
    });

    _btnRemove.setFont(_font);
    _btnRemove.setText("<< Remove");
    _btnRemove.setColors(Renderer::Color{100,70,50,220}, Renderer::Color{140,100,70,230}, Renderer::Color{90,60,40,230}, Renderer::Color{230,200,200,255}, SDL_Color{255,255,255,255});
    _btnRemove.setOnClick([this]() {
        int idx = _selectedSkillsList.selectedIndex();
        const String* label = _selectedSkillsList.selectedItem();
        if (!label || idx < 0) return;
        SkillType t = SkillType::None;
        if (skillTypeFromString(label->c_str(), t)) {
            int key = static_cast<int>(t);
            auto it = std::find(_selectedSkillIds.begin(), _selectedSkillIds.end(), key);
            if (it != _selectedSkillIds.end()) {
                _selectedSkillIds.erase(it);
                rebuildSkillLists();
            }
        }
    });

    // Archetypes list and editor button
    _archetypeList.setFont(_font);
    _archetypeList.setColors(Renderer::Color{20,20,30,200}, Renderer::Color{180,180,200,255}, Renderer::Color{60,80,120,220}, SDL_Color{230,230,240,255}, SDL_Color{255,255,255,255});
    // Populate archetype names
    {
        Vector<String> arcNames; for (const auto& a : getAllArchetypes()) arcNames.push_back(a.name);
        _archetypeList.setItems(std::move(arcNames));
    }
    _btnEditArchetype.setFont(_font);
    _btnEditArchetype.setText("Edit Archetype");
    _btnEditArchetype.setColors(Renderer::Color{50,70,100,220}, Renderer::Color{70,100,140,230}, Renderer::Color{40,60,90,230}, Renderer::Color{200,200,230,255}, SDL_Color{255,255,255,255});
    _btnEditArchetype.setOnClick([this]() {
        // Copy top skill targets from selected archetype into Selected Skills (up to 5)
        int idx = _comboArchetype.selectedIndex();
        if (idx < 0) return;
        const auto& arcs = getAllArchetypes();
        if (idx >= (int)arcs.size()) return;
        const auto& arc = arcs[idx];
        // Collect pairs and sort by target desc
        Vector<std::pair<int,float>> kvs; kvs.reserve(arc.skillTargets.size());
        for (const auto& kv : arc.skillTargets) kvs.push_back(kv);
        std::sort(kvs.begin(), kvs.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
        _selectedSkillIds.clear();
        for (const auto& kv : kvs) {
            if ((int)_selectedSkillIds.size() >= 5) break;
            _selectedSkillIds.push_back(kv.first);
        }
        rebuildSkillLists();
        _mode = CharCreateMode::Custom; // switch to Custom to allow editing
        _comboPath.setSelectedIndex(0);
    });

    // Gender segmented control: Male / Female
    _genderSeg.setFont(_font);
    _genderSeg.setColors(Renderer::Color{30,30,40,220}, Renderer::Color{200,200,220,255},
                         Renderer::Color{50,60,80,220}, Renderer::Color{70,100,140,230}, Renderer::Color{60,80,110,230},
                         SDL_Color{230,230,240,255}, SDL_Color{255,255,255,255});
    _genderSeg.setItems(Vector<String>{"Male","Female"});
    _genderSeg.setSelectedIndex(0);
    _genderSeg.setOnChanged([this](int idx){
        _isMale = (idx == 0);
        // Swap portrait source when gender changes
        if (_isMale && _pmMale.isLoaded()) {
            _portraitView.setTileMapTile(_pmMale.tile());
        } else if (!_isMale && _pmFemale.isLoaded()) {
            _portraitView.setTileMapTile(_pmFemale.tile());
        }
        _portraitView.setTileIndex(_portraitIndex);
    });

    // Portrait viewer setup: load male/female sheets (4x4 @ 128)
    _pmMale.load("assets/Tiles/MalePortraits.png", 128, 128, 4, 4);
    _pmFemale.load("assets/Tiles/FemalePortraits.png", 128, 128, 4, 4);
    _portraitView.setScaleMode(UI::UIImage::ScaleMode::Fit);
    _portraitView.setTint(Renderer::Color{255,255,255,255});
    _portraitView.useTileMap(true);
    if (_pmMale.isLoaded()) {
        _portraitView.setTileMapTile(_pmMale.tile());
    }
    _portraitView.setTileIndex(_portraitIndex);

    _btnPrevPortrait.setFont(_font); _btnPrevPortrait.setText("<<");
    _btnPrevPortrait.setColors(Renderer::Color{60,60,60,200}, Renderer::Color{80,80,80,220}, Renderer::Color{40,40,40,220}, Renderer::Color{200,200,200,255}, SDL_Color{255,255,255,255});
    _btnPrevPortrait.setOnClick([this](){ _portraitIndex = std::max(0, _portraitIndex - 1); _portraitView.setTileIndex(_portraitIndex); });
    _btnNextPortrait.setFont(_font); _btnNextPortrait.setText(">>");
    _btnNextPortrait.setColors(Renderer::Color{60,60,60,200}, Renderer::Color{80,80,80,220}, Renderer::Color{40,40,40,220}, Renderer::Color{200,200,200,255}, SDL_Color{255,255,255,255});
    _btnNextPortrait.setOnClick([this](){ _portraitIndex = std::min(15, _portraitIndex + 1); _portraitView.setTileIndex(_portraitIndex); });
    _lblPortraitIndex.setFont(_font);
    _lblPortraitIndex.setText("Portrait 1/16");

    // Archetype combo (dropdown) for class selection
    _comboArchetype.setFont(_font);
    _comboArchetype.setColors(Renderer::Color{30,30,40,220}, Renderer::Color{200,200,220,255}, SDL_Color{255,255,255,255},
                              Renderer::Color{20,20,30,240}, Renderer::Color{200,200,220,255}, Renderer::Color{60,80,120,220});
    {
        Vector<String> arcNames; for (const auto& a : getAllArchetypes()) arcNames.push_back(a.name);
        _comboArchetype.setItems(std::move(arcNames));
        _comboArchetype.setSelectedIndex(0);
        _comboArchetype.setOnChanged([this](int idx){
            // Preview selected archetype's top skills in the right list
            const auto& arcs = getAllArchetypes();
            if (idx >= 0 && idx < (int)arcs.size()) {
                const auto& arc = arcs[idx];
                Vector<std::pair<int,float>> kvs; kvs.reserve(arc.skillTargets.size());
                for (const auto& kv : arc.skillTargets) kvs.push_back(kv);
                std::sort(kvs.begin(), kvs.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
                _selectedSkillIds.clear();
                for (const auto& kv : kvs) { if ((int)_selectedSkillIds.size() >= 5) break; _selectedSkillIds.push_back(kv.first); }
                rebuildSkillLists();
            }
        });
    }

    // Navigation buttons
    _btnNext.setFont(_font); _btnNext.setText("Next >>");
    _btnNext.setColors(Renderer::Color{50,90,50,220}, Renderer::Color{70,120,70,230}, Renderer::Color{40,70,40,230}, Renderer::Color{200,230,200,255}, SDL_Color{255,255,255,255});
    _btnNext.setOnClick([this]() {
        switch (_step) {
            case CharCreateStep::NamePath: _nameInput.focus(false); _step = CharCreateStep::Stats; break;
            case CharCreateStep::Stats:    _step = CharCreateStep::Choose; break;
            case CharCreateStep::Choose:   _step = CharCreateStep::Summary; break;
            case CharCreateStep::Summary:  /* no-op */ break;
        }
    });
    _btnBack.setFont(_font); _btnBack.setText("<< Back");
    _btnBack.setColors(Renderer::Color{90,50,50,220}, Renderer::Color{120,70,70,230}, Renderer::Color{70,40,40,230}, Renderer::Color{230,200,200,255}, SDL_Color{255,255,255,255});
    _btnBack.setOnClick([this]() {
        switch (_step) {
            case CharCreateStep::NamePath: /* stay */ break;
            case CharCreateStep::Stats:    _step = CharCreateStep::NamePath; _nameInput.focus(true); break;
            case CharCreateStep::Choose:   _step = CharCreateStep::Stats; break;
            case CharCreateStep::Summary:  _step = CharCreateStep::Choose; break;
        }
    });

    // Stats distribution setup
    _lblStats.setFont(_font);
    _lblStats.setText("Distribute Attributes");
    _lblRemaining.setFont(_font);
    _lblRemaining.setText("Points remaining: 10");
    // Shared button colors for +/-
    const Renderer::Color btnN{50,70,100,220}, btnH{70,100,140,230}, btnP{40,60,90,230}, btnB{200,200,230,255};
    const SDL_Color btnT{255,255,255,255};
    // Initialize five core attributes with base 10
    const AttributeType kAttrs[] = { AttributeType::Strength, AttributeType::Dexterity, AttributeType::Intelligence, AttributeType::Endurance, AttributeType::Wisdom };
    _attrAlloc.clear();
    _attrRows.clear();
    for (AttributeType a : kAttrs) {
        _attrAlloc[(int)a] = 10;
        AttrRow row{}; row.type = a;
        row.label.setFont(_font); row.label.setText(attributeTypeToString(a));
        row.value.setFont(_font); row.value.setText("10");
    row.btnMinus.setFont(_font); row.btnMinus.setText("-");
        row.btnMinus.setColors(btnN, btnH, btnP, btnB, btnT);
        row.btnPlus.setFont(_font); row.btnPlus.setText("+");
        row.btnPlus.setColors(btnN, btnH, btnP, btnB, btnT);
        _attrRows.push_back(std::move(row));
    }
    // Disable click-time adjustment; we'll handle press-and-hold in update()
    for (auto& row : _attrRows) { row.btnMinus.setOnClick(nullptr); row.btnPlus.setOnClick(nullptr); }
    _prevPlusPressed.assign(_attrRows.size(), false);
    _prevMinusPressed.assign(_attrRows.size(), false);
    _accPlus.assign(_attrRows.size(), 0.0f);
    _accMinus.assign(_attrRows.size(), 0.0f);
    _holdPlus.assign(_attrRows.size(), -1.0f);
    _holdMinus.assign(_attrRows.size(), -1.0f);
    _didRepeatPlus.assign(_attrRows.size(), false);
    _didRepeatMinus.assign(_attrRows.size(), false);

    rebuildSkillLists();

    // Buttons initial layout under input (positions refined in render)
    _btnConfirm.setFont(_font);
    _btnConfirm.setText("Confirm");
    _btnConfirm.setColors(Renderer::Color{50,90,50,220}, Renderer::Color{70,120,70,230}, Renderer::Color{40,70,40,230}, Renderer::Color{200,230,200,255}, SDL_Color{255,255,255,255});
    _btnConfirm.setOnClick([this]() { 
        // Build a CharacterSheet from current selections
        CharacterSheet cs{};
        // Name
        cs.name = _nameInput.text().empty() ? String("Hero") : _nameInput.text();
        // Gender and portrait selection
        cs.isMale = _isMale;
        cs.portraitIndex = std::clamp(_portraitIndex, 0, 15);
        // Base level/XP and attributes from allocation
        setLevel(cs, 1);
        setExperience(cs, 0);
        setExperienceToNextLevel(cs, computeExperienceToNextLevel(cs));
        // Initialize core attributes to allocated values
        for (const auto& row : _attrRows) {
            int v = 10;
            auto it = _attrAlloc.find((int)row.type);
            if (it != _attrAlloc.end()) v = it->second;
            setAttribute(cs.attributes, row.type, v);
        }
        // Skills: apply archetype targets or mark selected skills with a starting rating
        if (_mode == CharCreateMode::Archetype) {
            const int idx = _comboArchetype.selectedIndex();
            const auto& arcs = getAllArchetypes();
            if (idx >= 0 && idx < (int)arcs.size()) {
                applyArchetype(cs, arcs[idx]);
            }
        } else {
            // Custom path: set selected skills to a moderate starting proficiency
            for (int sid : _selectedSkillIds) {
                setSkillRating(cs, static_cast<SkillType>(sid), 0.6f);
            }
        }
        // Recompute derived pools (HP/MP) based on stats
        recalcDerivedPools(cs, /*refill*/true);
        // Persist globally for the upcoming World state (non-invasive singleton)
        SetCurrentCharacter(cs);
        _next = AQStateId::World; 
    });

    _btnCancel.setFont(_font);
    _btnCancel.setText("Cancel");
    _btnCancel.setColors(Renderer::Color{90,50,50,220}, Renderer::Color{120,70,70,230}, Renderer::Color{70,40,40,230}, Renderer::Color{230,200,200,255}, SDL_Color{255,255,255,255});
    _btnCancel.setOnClick([this]() { _next = AQStateId::MainMenu; });
}

AQStateId GSCharCreation::handleEvents(float /*delta*/, Game::GameEvents& events) {
    // Ensure layout is up-to-date before hit-testing
    SDL_FRect cr = _window.contentRect();
    SDL_FRect lm = _font ? Text::measure(_font, "Mg") : SDL_FRect{0,0,0,20};
    const float labelY = cr.y;
    const float inputY = labelY + lm.h + 8.0f;
    _nameInput.setPosition(cr.x, inputY);
    // Match render layout (60% width) to avoid hit-rect overlap with controls on the right
    _nameInput.setSize(cr.w * 0.6f, lm.h + 8.0f);
    const float btnGap = 12.0f;
    const float btnW = (cr.w - btnGap) * 0.5f;
    const float btnH = lm.h + 10.0f;
    // Bottom-row button layout (match render) for correct hit-testing
    const bool showBack = (_step == CharCreateStep::Stats || _step == CharCreateStep::Choose || _step == CharCreateStep::Summary);
    const bool showNext = (_step == CharCreateStep::NamePath || _step == CharCreateStep::Stats || _step == CharCreateStep::Choose);
    const bool showConfirm = (_step == CharCreateStep::Summary);
    int slots = (showBack ? 1 : 0) + (showNext ? 1 : 0) + (showConfirm ? 1 : 0) + 1; // +1 for Cancel
    slots = std::max(2, slots);
    const float btnWBottom = (cr.w - btnGap * (slots - 1)) / slots;
    const float btnYBottom = cr.y + cr.h - btnH - 8.0f;
    float xb = cr.x;
    if (showBack) {
        _btnBack.setPosition(xb, btnYBottom);
        _btnBack.setSize(btnWBottom, btnH);
        xb += btnWBottom + btnGap;
    }
    if (showNext) {
        _btnNext.setPosition(xb, btnYBottom);
        _btnNext.setSize(btnWBottom, btnH);
        xb += btnWBottom + btnGap;
    }
    if (showConfirm) {
        _btnConfirm.setPosition(xb, btnYBottom);
        _btnConfirm.setSize(btnWBottom, btnH);
        xb += btnWBottom + btnGap;
    }
    _btnCancel.setPosition(xb, btnYBottom);
    _btnCancel.setSize(btnWBottom, btnH);

    // Route events via view controller: build active list based on step
    _vc.clear();
    if (_step == CharCreateStep::NamePath) {
        // Layout path combo to the right of name input (same as render)
        float cx = cr.x + cr.w * 0.6f + 12.0f;
        _comboPath.setPosition(cx, inputY);
        _comboPath.setSize(cr.w - (cx - cr.x), lm.h + 8.0f);
    // Gender segmented control below the name input (centered)
        const float genderY = inputY + (lm.h + 8.0f) + 8.0f;
    float segW = cr.w * 0.6f;
    float segX = cr.x + (cr.w - segW) * 0.5f;
    _genderSeg.setPosition(segX, genderY);
    _genderSeg.setSize(segW, lm.h + 8.0f);
    // Portrait label (index) above portrait, then portrait centered vertically between gender and bottom buttons
    const float pvSize = 160.0f;
    // Determine bottom button row Y to define available area
    const bool showBack = (_step == CharCreateStep::Stats || _step == CharCreateStep::Choose || _step == CharCreateStep::Summary);
    const bool showNext = (_step == CharCreateStep::NamePath || _step == CharCreateStep::Stats || _step == CharCreateStep::Choose);
    const bool showConfirm = (_step == CharCreateStep::Summary);
    int slots = (showBack ? 1 : 0) + (showNext ? 1 : 0) + (showConfirm ? 1 : 0) + 1; // +1 for Cancel
    slots = std::max(2, slots);
    const float btnYBottom = cr.y + cr.h - (lm.h + 10.0f) - 8.0f;
    const float availTop = genderY + (lm.h + 8.0f) + 12.0f;
    const float availBottom = btnYBottom - 8.0f;
    // Estimate label height using font metrics (we'll use lm.h here)
    const float idxH = lm.h;
    const float blockH = idxH + 6.0f + pvSize;
    const float availH = std::max(0.0f, availBottom - availTop);
    const float baseY = availTop + std::max(0.0f, (availH - blockH) * 0.5f);
    const float idxY = baseY;
    const float pvY = idxY + idxH + 6.0f; // space for the label height
        const float pvX = cr.x + (cr.w - pvSize) * 0.5f;
        _portraitView.setPosition(pvX, pvY);
        _portraitView.setSize(pvSize, pvSize);
        // Side buttons aligned to vertical center of portrait
        const float btnHsmall = lm.h + 6.0f;
        const float pvBtnW = 48.0f;
        const float pad = 10.0f;
        const float pvBtnY = pvY + (pvSize - btnHsmall) * 0.5f;
        _btnPrevPortrait.setPosition(pvX - pvBtnW - pad, pvBtnY);
        _btnPrevPortrait.setSize(pvBtnW, btnHsmall);
        _btnNextPortrait.setPosition(pvX + pvSize + pad, pvBtnY);
        _btnNextPortrait.setSize(pvBtnW, btnHsmall);
    // Index label (we'll center horizontally when rendering; y is set here for event hitbox symmetry)
    _lblPortraitIndex.setPosition(pvX, idxY);
        // VC
        _vc.add(&_nameInput, /*focusable=*/true, /*z=*/1);
        _vc.add(&_comboPath, true, 2);
        _vc.add(&_genderSeg, true, 2);
        _vc.add(&_btnPrevPortrait, true, 2);
        _vc.add(&_btnNextPortrait, true, 2);
        if (showNext) _vc.add(&_btnNext, true, 3);
    } else if (_step == CharCreateStep::Stats) {
        // Attribute distribution buttons
        // Layout rows similar to render so hit-testing is correct
        const float areaTop = inputY + (lm.h + 10.0f) + 8.0f;
        const float rowStartY = areaTop + lm.h + 8.0f;
        const float rowH = lm.h + 10.0f;
        const float nameW = cr.w * 0.35f;
        const float btnWattr = 36.0f;
        const float valW = 40.0f;
        for (size_t i = 0; i < _attrRows.size(); ++i) {
            float y = rowStartY + (float)i * (rowH + 6.0f);
            auto& row = _attrRows[i];
            row.label.setPosition(cr.x, y);
            row.btnMinus.setPosition(cr.x + nameW, y);
            row.btnMinus.setSize(btnWattr, rowH);
            row.value.setPosition(cr.x + nameW + btnWattr + 8.0f, y + 4.0f);
            row.btnPlus.setPosition(cr.x + nameW + btnWattr + 8.0f + valW + 8.0f, y);
            row.btnPlus.setSize(btnWattr, rowH);
        }
        for (auto& row : _attrRows) {
            _vc.add(&row.btnMinus, true, 1);
            _vc.add(&row.btnPlus, true, 1);
        }
        if (showBack) _vc.add(&_btnBack, true, 3);
        if (showNext) _vc.add(&_btnNext, true, 3);
    } else if (_step == CharCreateStep::Choose) {
        const float areaTop = inputY + (lm.h + 10.0f) + 8.0f;
        const float areaBottom = cr.y + cr.h - (btnH + 8.0f);
        const float areaHeight = std::max(100.0f, areaBottom - areaTop - 16.0f);
        if (_mode == CharCreateMode::Custom) {
            const float colGap = 12.0f;
            // Dynamically size the middle column so Add/Remove labels fit without clipping
            float labelAddW = Text::measure(_font, "Add >>").w;
            float labelRemW = Text::measure(_font, "<< Remove").w;
            float minBtnLabelW = std::max(labelAddW, labelRemW) + 24.0f; // padding for button chrome
            const float minColW = 160.0f; // ensure lists keep reasonable width
            float desiredMidW = std::max(80.0f, minBtnLabelW / 0.9f); // since midBtnW = midW*0.9
            float maxMidW = std::max(80.0f, cr.w - colGap - 2.0f * minColW);
            const float midGap = std::clamp(desiredMidW, 80.0f, maxMidW);
            const float colW = (cr.w - colGap - midGap) * 0.5f;
            const float leftX = cr.x;
            const float rightX = cr.x + colW + midGap + colGap;
            const float listY = areaTop + lm.h + 6.0f;
            const float listH = areaHeight - (lm.h + 6.0f);
            _allSkillsList.setPosition(leftX, listY);
            _allSkillsList.setSize(colW, listH);
            _selectedSkillsList.setPosition(rightX, listY);
            _selectedSkillsList.setSize(colW, listH);
            const float midX = cr.x + colW + colGap;
            const float midW = midGap;
            const float midBtnW = midW * 0.9f;
            const float midBtnX = midX + (midW - midBtnW) * 0.5f;
            const float addY = listY + 8.0f;
            const float remY = addY + btnH + 8.0f;
            _btnAdd.setPosition(midBtnX, addY);
            _btnAdd.setSize(midBtnW, btnH);
            _btnRemove.setPosition(midBtnX, remY);
            _btnRemove.setSize(midBtnW, btnH);
            // Keep enabled states in sync for correct event routing
            int selCount = (int)_selectedSkillIds.size();
            bool canAdd = selCount < 5 && _allSkillsList.selectedIndex() >= 0;
            bool canRemove = _selectedSkillsList.selectedIndex() >= 0 && selCount > 0;
            _btnAdd.setEnabled(canAdd);
            _btnRemove.setEnabled(canRemove);
            _vc.add(&_allSkillsList, true, 1);
            _vc.add(&_selectedSkillsList, true, 1);
            _vc.add(&_btnAdd, true, 2);
            _vc.add(&_btnRemove, true, 2);
            if (showBack) _vc.add(&_btnBack, true, 3);
            if (showNext) _vc.add(&_btnNext, true, 3);
        } else {
            const float comboW = cr.w * 0.5f;
            _comboArchetype.setPosition(cr.x, areaTop);
            _comboArchetype.setSize(comboW, lm.h + 10.0f);
            _vc.add(&_comboArchetype, true, 2);
            // Preview list
            const float previewTop = areaTop + lm.h + 16.0f;
            const float pvListY = previewTop + lm.h + 6.0f;
            const float pvListH = std::max(40.0f, (cr.y + cr.h - (btnH + 8.0f)) - pvListY - 8.0f);
            _selectedSkillsList.setPosition(cr.x, pvListY);
            _selectedSkillsList.setSize(cr.w, pvListH);
            _vc.add(&_selectedSkillsList, false, 0);
            _vc.add(&_btnEditArchetype, true, 2);
            if (showBack) _vc.add(&_btnBack, true, 3);
            if (showNext) _vc.add(&_btnNext, true, 3);
        }
        // defer back/next handles are added above per mode
    }
    else if (_step == CharCreateStep::Summary) {
        // Only need back/confirm on this screen; handled below via bottom-row
    }
    // Bottom-row buttons (added if shown above for Back/Next)
    if (showConfirm) _vc.add(&_btnConfirm, true, 3);
    _vc.add(&_btnCancel, true, 3);

    // Route events through controller
    _vc.handleEvents(events);
    if (_next != AQStateId::None) { AQStateId out = _next; _next = AQStateId::None; return out; }
    if (events.type == Game::GameEvents::EventType::None) return AQStateId::None;

    using Game::GameEvents;
    if (events.isEventType(Game::GameEvents::EventType::KeyPress)) {
        if (events.keyEvent.keyCode == SDLK_ESCAPE) {
            return AQStateId::MainMenu;
        }
    }
    return AQStateId::None;
}

void GSCharCreation::update(float delta) {
    (void)delta;
    _nameInput.update(delta);
    // Handle press-and-hold for +/- with safe single-click behavior (apply on release)
    if (_step == CharCreateStep::Stats) {
        for (size_t i = 0; i < _attrRows.size(); ++i) {
            auto& row = _attrRows[i];
            bool pPlus = row.btnPlus.pressed();
            bool pMinus = row.btnMinus.pressed();
            // Disable in-frame repeating: only apply one step on release
            if (!_prevPlusPressed[i] && pPlus) { /* pressed: no-op until release */ }
            if (!_prevMinusPressed[i] && pMinus) { /* pressed: no-op until release */ }

            if (_prevPlusPressed[i] && !pPlus) {
                adjustAttribute(row.type, +1);
            }
            if (_prevMinusPressed[i] && !pMinus) {
                adjustAttribute(row.type, -1);
            }

            _prevPlusPressed[i] = pPlus;
            _prevMinusPressed[i] = pMinus;
        }
    }
}

void GSCharCreation::render(float /*delta*/) {
    // Layout inside window's content area
    // Step-specific window title (visual separation per state)
    switch (_step) {
        case CharCreateStep::NamePath: _window.setTitle(_font, "Character Creation — Name & Path"); break;
        case CharCreateStep::Stats:    _window.setTitle(_font, "Character Creation — Assign Attributes"); break;
        case CharCreateStep::Choose:   _window.setTitle(_font, "Character Creation — Class & Skills"); break;
        case CharCreateStep::Summary:  _window.setTitle(_font, "Character Creation — Summary"); break;
    }
    _window.render();
    SDL_FRect cr = _window.contentRect();
    const float labelY = cr.y;
    _prompt.setPosition(cr.x, labelY);
    _prompt.render();

    // Name input just under label
    SDL_FRect lm = _font ? Text::measure(_font, "Mg") : SDL_FRect{0,0,0,20};
    const float inputY = labelY + lm.h + 8.0f;
    _nameInput.setPosition(cr.x, inputY);
    _nameInput.setSize(cr.w * 0.6f, lm.h + 8.0f);
    _nameInput.render();

    const float btnGap = 12.0f;
    const float btnH = lm.h + 10.0f;

    if (_step == CharCreateStep::NamePath) {
        // Path combo to the right of name input
        float cx = cr.x + cr.w * 0.6f + 12.0f;
        _comboPath.setPosition(cx, inputY);
        _comboPath.setSize(cr.w - (cx - cr.x), lm.h + 8.0f);
        _comboPath.render();
    // Gender segmented control below name input (centered)
        const float genderY = inputY + (lm.h + 8.0f) + 8.0f;
    float segW = cr.w * 0.6f;
    float segX = cr.x + (cr.w - segW) * 0.5f;
    _genderSeg.setPosition(segX, genderY);
    _genderSeg.setSize(segW, lm.h + 8.0f);
        _genderSeg.render();
    // Portrait viewer below gender, centered vertically; index label above; buttons left/right
    const float pvSize = 160.0f;
    // Compute bottom buttons row to get available vertical area
    const bool showBack = (_step == CharCreateStep::Stats || _step == CharCreateStep::Choose || _step == CharCreateStep::Summary);
    const bool showNext = (_step == CharCreateStep::NamePath || _step == CharCreateStep::Stats || _step == CharCreateStep::Choose);
    const bool showConfirm = (_step == CharCreateStep::Summary);
    int slots = (showBack ? 1 : 0) + (showNext ? 1 : 0) + (showConfirm ? 1 : 0) + 1; // +1 for Cancel
    slots = std::max(2, slots);
    const float btnYBottom = cr.y + cr.h - (btnH) - 8.0f;
    const float availTop = genderY + (lm.h + 8.0f) + 12.0f;
    const float availBottom = btnYBottom - 8.0f;
    // Update label text and measure to center perfectly
    String idxText = String("Portrait ") + std::to_string(_portraitIndex + 1) + "/16";
    _lblPortraitIndex.setText(idxText);
    SDL_FRect idxm = _lblPortraitIndex.measure();
    const float blockH = idxm.h + 6.0f + pvSize;
    const float availH = std::max(0.0f, availBottom - availTop);
    const float baseY = availTop + std::max(0.0f, (availH - blockH) * 0.5f);
    const float idxY = baseY;
    const float pvY = idxY + idxm.h + 6.0f;
        const float pvX = cr.x + (cr.w - pvSize) * 0.5f;
        _portraitView.setPosition(pvX, pvY);
        _portraitView.setSize(pvSize, pvSize);
    // Index label (centered above portrait)
    _lblPortraitIndex.setPosition(pvX + (pvSize - idxm.w) * 0.5f, idxY);
    _lblPortraitIndex.render();
        // Side buttons
        const float btnHsmall = lm.h + 6.0f;
        const float pvBtnW = 48.0f;
        const float pad = 10.0f;
        const float pvBtnY = pvY + (pvSize - btnHsmall) * 0.5f;
        _btnPrevPortrait.setPosition(pvX - pvBtnW - pad, pvBtnY);
        _btnPrevPortrait.setSize(pvBtnW, btnHsmall);
        _btnPrevPortrait.render();
        _btnNextPortrait.setPosition(pvX + pvSize + pad, pvBtnY);
        _btnNextPortrait.setSize(pvBtnW, btnHsmall);
        _btnNextPortrait.render();
    // Portrait image
    _portraitView.render();
    } else if (_step == CharCreateStep::Stats) {
        // Stats distribution UI
        const float areaTop = inputY + (lm.h + 10.0f) + 8.0f;
        _lblStats.setPosition(cr.x, areaTop);
        _lblStats.render();
        _lblRemaining.setPosition(cr.x + cr.w - _lblRemaining.measure().w, areaTop);
        _lblRemaining.render();
        const float rowStartY = areaTop + lm.h + 8.0f;
        const float rowH = lm.h + 10.0f;
        const float nameW = cr.w * 0.35f;
        const float btnW = 36.0f;
        const float valW = 40.0f;
        for (size_t i = 0; i < _attrRows.size(); ++i) {
            float y = rowStartY + (float)i * (rowH + 6.0f);
            auto& row = _attrRows[i];
            row.label.setPosition(cr.x, y);
            row.label.render();
            // Minus button
            row.btnMinus.setPosition(cr.x + nameW, y);
            row.btnMinus.setSize(btnW, rowH);
            row.btnMinus.render();
            // Value
            row.value.setPosition(cr.x + nameW + btnW + 8.0f, y + 4.0f);
            row.value.render();
            // Plus button
            row.btnPlus.setPosition(cr.x + nameW + btnW + 8.0f + valW + 8.0f, y);
            row.btnPlus.setSize(btnW, rowH);
            row.btnPlus.render();
        }
    } else if (_step == CharCreateStep::Choose) {
        // Choose step: either custom skills or archetype picker
        const float areaTop = inputY + (lm.h + 10.0f) + 8.0f;
        const float areaBottom = cr.y + cr.h - (btnH + 8.0f);
        const float areaHeight = std::max(100.0f, areaBottom - areaTop - 16.0f);

        if (_mode == CharCreateMode::Custom) {
            const float colGap = 12.0f;
            // Dynamically size the middle column so Add/Remove labels fit without clipping
            float labelAddW = Text::measure(_font, "Add >>").w;
            float labelRemW = Text::measure(_font, "<< Remove").w;
            float minBtnLabelW = std::max(labelAddW, labelRemW) + 24.0f; // padding for button chrome
            const float minColW = 160.0f; // ensure lists keep reasonable width
            float desiredMidW = std::max(80.0f, minBtnLabelW / 0.9f); // since midBtnW = midW*0.9
            float maxMidW = std::max(80.0f, cr.w - colGap - 2.0f * minColW);
            const float midGap = std::clamp(desiredMidW, 80.0f, maxMidW); // space for Add/Remove buttons between lists
            const float colW = (cr.w - colGap - midGap) * 0.5f;
            const float leftX = cr.x;
            const float rightX = cr.x + colW + midGap + colGap;

            _lblAll.setPosition(leftX, areaTop);
            _lblSelected.setPosition(rightX, areaTop);
            // Dynamic labels with cap indicator
            _lblAll.render();
            int selCount = (int)_selectedSkillIds.size();
            _lblSelected.setText(String("Selected Skills (") + std::to_string(selCount) + "/5)");
            _lblSelected.setColor(selCount >= 5 ? SDL_Color{255,210,180,255} : SDL_Color{230,230,240,255});
            _lblSelected.render();

            const float listY = areaTop + lm.h + 6.0f;
            const float listH = areaHeight - (lm.h + 6.0f);
            _allSkillsList.setPosition(leftX, listY);
            _allSkillsList.setSize(colW, listH);
            _allSkillsList.render();
            _selectedSkillsList.setPosition(rightX, listY);
            _selectedSkillsList.setSize(colW, listH);
            _selectedSkillsList.render();

            // Add/Remove buttons stacked in between
            const float midX = cr.x + colW + colGap;
            const float midW = midGap;
            const float midBtnW = midW * 0.9f;
            const float midBtnX = midX + (midW - midBtnW) * 0.5f;
            const float addY = listY + 8.0f;
            const float remY = addY + btnH + 8.0f;
            _btnAdd.setPosition(midBtnX, addY);
            _btnAdd.setSize(midBtnW, btnH);
            _btnRemove.setPosition(midBtnX, remY);
            _btnRemove.setSize(midBtnW, btnH);
            // Enable/disable based on state
            bool canAdd = selCount < 5 && _allSkillsList.selectedIndex() >= 0;
            bool canRemove = _selectedSkillsList.selectedIndex() >= 0 && selCount > 0;
            _btnAdd.setEnabled(canAdd);
            _btnRemove.setEnabled(canRemove);
            _btnAdd.render();
            _btnRemove.render();
        } else {
            // Archetype picker via ComboBox + preview + Edit
            const float comboW = cr.w * 0.5f;
            _comboArchetype.setPosition(cr.x, areaTop);
            _comboArchetype.setSize(comboW, lm.h + 10.0f);
            _comboArchetype.render();
            // Edit button to the right
            const float rightX = cr.x + comboW + 12.0f;
            _btnEditArchetype.setPosition(rightX, areaTop);
            _btnEditArchetype.setSize(cr.w - rightX + cr.x, btnH);
            _btnEditArchetype.render();
            // Preview selected skills from combo in the lower half when dropdown is not open (avoid overlap)
            if (!_comboArchetype.isOpen()) {
                const float previewTop = areaTop + lm.h + 16.0f;
                _lblSelected.setPosition(cr.x, previewTop);
                int selCount = (int)_selectedSkillIds.size();
                _lblSelected.setText(String("Selected Skills (") + std::to_string(selCount) + "/5)");
                _lblSelected.setColor(selCount >= 5 ? SDL_Color{255,210,180,255} : SDL_Color{230,230,240,255});
                _lblSelected.render();
                const float pvListY = previewTop + lm.h + 6.0f;
                const float pvListH = std::max(40.0f, (cr.y + cr.h - (btnH + 8.0f)) - pvListY - 8.0f);
                _selectedSkillsList.setPosition(cr.x, pvListY);
                _selectedSkillsList.setSize(cr.w, pvListH);
                _selectedSkillsList.render();
            }
        }
    } else if (_step == CharCreateStep::Summary) {
        // Summary view: show name, path, archetype (if any), attributes, and selected skills
        const float areaTop = inputY + (lm.h + 10.0f) + 8.0f;
        // Header
        Text::draw(_font, "Summary", cr.x, areaTop, SDL_Color{235,235,235,255});
        // Draw portrait above the left column details
        const float portraitPx = 96.0f;
        // Render using globally selected portrait
        Portraits::Render(GetCurrentCharacter(),
                          Vector2{ cr.x + portraitPx * 0.5f, areaTop + lm.h + 10.0f + portraitPx * 0.5f },
                          Vector2{ portraitPx / 128.0f, portraitPx / 128.0f },
                          0.0f,
                          Renderer::Color{255,255,255,255});
        float y = areaTop + lm.h + 10.0f + portraitPx + 8.0f;
    // Name
        String nm = _nameInput.text().empty() ? String("Hero") : _nameInput.text();
        Text::draw(_font, (String("Name: ") + nm).c_str(), cr.x, y, SDL_Color{220,220,220,255});
        y += lm.h + 4.0f;
    // Gender
    Text::draw(_font, (String("Gender: ") + (_isMale ? "Male" : "Female")).c_str(), cr.x, y, SDL_Color{220,220,220,255});
    y += lm.h + 4.0f;
        // Path
        const bool isArc = (_mode == CharCreateMode::Archetype);
        String pathStr = isArc ? String("Path: Archetype") : String("Path: Custom");
        Text::draw(_font, pathStr.c_str(), cr.x, y, SDL_Color{220,220,220,255});
        y += lm.h + 4.0f;
        // Archetype name if applicable
        if (isArc) {
            int idx = _comboArchetype.selectedIndex();
            const auto& arcs = getAllArchetypes();
            if (idx >= 0 && idx < (int)arcs.size()) {
                Text::draw(_font, (String("Archetype: ") + arcs[idx].name).c_str(), cr.x, y, SDL_Color{200,220,255,255});
                y += lm.h + 4.0f;
            }
        }
        // Attributes
        Text::draw(_font, "Attributes:", cr.x, y, SDL_Color{235,235,235,255});
        y += lm.h + 4.0f;
        for (const auto& row : _attrRows) {
            int v = 10; auto it = _attrAlloc.find((int)row.type); if (it != _attrAlloc.end()) v = it->second;
            String line = String(attributeTypeToString(row.type)) + ": " + std::to_string(v);
            Text::draw(_font, line.c_str(), cr.x + 12.0f, y, SDL_Color{220,220,220,255});
            y += lm.h + 2.0f;
        }
        // Skills list on the right
        const float rightX = cr.x + cr.w * 0.5f;
        _lblSelected.setPosition(rightX, areaTop);
        _lblSelected.setText("Selected Skills");
        _lblSelected.render();
        const float listY = areaTop + lm.h + 6.0f;
        const float listH = std::max(40.0f, (cr.y + cr.h - (btnH + 8.0f)) - listY - 8.0f);
        _selectedSkillsList.setPosition(rightX, listY);
        _selectedSkillsList.setSize(cr.w - (rightX - cr.x), listH);
        _selectedSkillsList.render();
    }

    // Buttons row at the bottom
    // Compute button layout based on which buttons are shown this step
    const bool showBack = (_step == CharCreateStep::Stats || _step == CharCreateStep::Choose || _step == CharCreateStep::Summary);
    const bool showNext = (_step == CharCreateStep::NamePath || _step == CharCreateStep::Stats || _step == CharCreateStep::Choose);
    const bool showConfirm = (_step == CharCreateStep::Summary);
    int slots = (showBack ? 1 : 0) + (showNext ? 1 : 0) + (showConfirm ? 1 : 0) + 1; // +1 for Cancel
    slots = std::max(2, slots);
    const float btnW = (cr.w - btnGap * (slots - 1)) / slots;
    const float btnY = cr.y + cr.h - btnH - 8.0f;
    float x = cr.x;
    if (showBack) {
        _btnBack.setPosition(x, btnY);
        _btnBack.setSize(btnW, btnH);
        _btnBack.render();
        x += btnW + btnGap;
    }
    if (showNext) {
        _btnNext.setPosition(x, btnY);
        _btnNext.setSize(btnW, btnH);
        _btnNext.render();
        x += btnW + btnGap;
    }
    if (showConfirm) {
        _btnConfirm.setPosition(x, btnY);
        _btnConfirm.setSize(btnW, btnH);
        _btnConfirm.render();
        x += btnW + btnGap;
    }
    _btnCancel.setPosition(x, btnY);
    _btnCancel.setSize(btnW, btnH);
    _btnCancel.render();

}

void GSCharCreation::adjustAttribute(AttributeType t, int delta) {
    int& v = _attrAlloc[(int)t];
    if (delta > 0) {
        if (_attrPoints <= 0 || v >= 18) return;
        v += 1; _attrPoints -= 1;
    } else if (delta < 0) {
        if (v <= 8) return;
        v -= 1; _attrPoints += 1;
    } else { return; }
    // Update the row value label for this attribute
    for (auto& row : _attrRows) {
        if (row.type == t) { row.value.setText(std::to_string(v)); break; }
    }
    syncRemainingLabel();
}

void GSCharCreation::syncRemainingLabel() {
    _lblRemaining.setText(String("Points remaining: ") + std::to_string(_attrPoints));
}

void GSCharCreation::rebuildSkillLists() {
    // Build set of selected ids for quick lookup
    UMap<int, bool> sel;
    for (int id : _selectedSkillIds) sel[id] = true;
    // Left: all non-selected skills
    Vector<String> left; left.reserve(skillTypeCount());
    for (SkillType s : allSkillTypes()) {
        int id = static_cast<int>(s);
        if (sel.find(id) == sel.end()) left.push_back(skillTypeToString(s));
    }
    _allSkillsList.setItems(std::move(left));
    // Right: selected skills in chosen order
    Vector<String> right; right.reserve(_selectedSkillIds.size());
    for (int id : _selectedSkillIds) {
        right.push_back(skillTypeToString(static_cast<SkillType>(id)));
    }
    _selectedSkillsList.setItems(std::move(right));

}
