#pragma once

#include "Common.h"
#include "AvatarQuestGameState.h"
#include "Text.h"
#include "UIControls.h"
#include "UIViewController.h"
#include "AvatarQuestRP.h" // AttributeType and helpers
#include "AvatarQuestPortraitManager.h"

namespace AvatarQuest {

// Expanded flow: Name/Path -> Stats -> Class/Skills -> Summary
enum class CharCreateStep { NamePath = 0, Stats, Choose, Summary };
enum class CharCreateMode { Custom = 0, Archetype = 1 };

class GSCharCreation : public IGameState {
public:
    GSCharCreation() = default;
    ~GSCharCreation() override;

    void onEnter() override;
    AQStateId handleEvents(float delta, Game::GameEvents& events) override;
    void update(float delta) override;
    void render(float delta) override;
    const char* name() const override { return "CharacterCreation"; }

private:
    Text::Font* _font = nullptr;
    // UI demo: window + prompt + name input
    UI::UIWindow _window;
    UI::UILabel _prompt;
    UI::UITextInput _nameInput;
    // Skills selection UI
    UI::UILabel _lblAll;
    UI::UILabel _lblSelected;
    UI::UIListBox _allSkillsList;
    UI::UIListBox _selectedSkillsList;
    UI::UIButton _btnAdd;
    UI::UIButton _btnRemove;
    // Store selected skills as enum ints
    Vector<int> _selectedSkillIds;
    void rebuildSkillLists();
    // Archetype path (keep as combo box)
    UI::UIComboBox _comboPath; // Custom or Archetype
    // Gender segmented control (Male/Female)
    UI::UISegmentedControl _genderSeg;
    bool _isMale = true;
    // Portrait viewer components
    UI::UIImage _portraitView;
    UI::UIButton _btnPrevPortrait;
    UI::UIButton _btnNextPortrait;
    UI::UILabel _lblPortraitIndex;
    int _portraitIndex = 0; // 0..15
    AvatarQuest::PortraitManager _pmMale;
    AvatarQuest::PortraitManager _pmFemale;
    UI::UIListBox _archetypeList;
    UI::UIButton _btnEditArchetype;
    UI::UIComboBox _comboArchetype; // Combo-based class selection
    // Stats distribution step
    struct AttrRow {
        AttributeType type;
        UI::UILabel label;   // attribute name
        UI::UIButton btnMinus;
        UI::UIButton btnPlus;
        UI::UILabel value;   // numeric value
    };
    UI::UILabel _lblStats;
    UI::UILabel _lblRemaining;
    Vector<AttrRow> _attrRows;
    int _attrPoints = 10;                // points available to distribute
    UMap<int,int> _attrAlloc;            // key=(int)AttributeType, value=current score
    // Press-and-hold repeat state for +/- buttons
    Vector<bool> _prevPlusPressed;
    Vector<bool> _prevMinusPressed;
    Vector<float> _accPlus;
    Vector<float> _accMinus;
    // Press duration trackers to ensure single-click = single step (apply on release)
    Vector<float> _holdPlus;
    Vector<float> _holdMinus;
    // Track if any repeat occurred during hold to avoid double-stepping on release
    Vector<bool> _didRepeatPlus;
    Vector<bool> _didRepeatMinus;
    // Faster auto-repeat for snappier +/- in Stats
    float _initialDelay = 0.30f;      // longer delay before auto-repeat to avoid accidental runs
    float _repeatInterval = 0.08f;    // slower repeat to prevent overshooting
    void adjustAttribute(AttributeType t, int delta);
    void syncRemainingLabel();
    // Flow control
    UI::UIButton _btnNext;
    UI::UIButton _btnBack;
    CharCreateStep _step = CharCreateStep::NamePath;
    CharCreateMode _mode = CharCreateMode::Custom;
    UI::UIButton _btnConfirm;
    UI::UIButton _btnCancel;
    AQStateId _next = AQStateId::None;
    // View controller to centralize focus and event routing per step
    UI::UIViewController _vc;
};

}
