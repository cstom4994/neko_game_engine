

#include "engine/imgui.hpp"

namespace Neko::imgui {

#define X(prefix, name) {#name, prefix##_##name}

static util::TableInteger WindowFlags[] = {
        X(ImGuiWindowFlags, None),
        X(ImGuiWindowFlags, NoTitleBar),
        X(ImGuiWindowFlags, NoResize),
        X(ImGuiWindowFlags, NoMove),
        X(ImGuiWindowFlags, NoScrollbar),
        X(ImGuiWindowFlags, NoScrollWithMouse),
        X(ImGuiWindowFlags, NoCollapse),
        X(ImGuiWindowFlags, AlwaysAutoResize),
        X(ImGuiWindowFlags, NoBackground),
        X(ImGuiWindowFlags, NoSavedSettings),
        X(ImGuiWindowFlags, NoMouseInputs),
        X(ImGuiWindowFlags, MenuBar),
        X(ImGuiWindowFlags, HorizontalScrollbar),
        X(ImGuiWindowFlags, NoFocusOnAppearing),
        X(ImGuiWindowFlags, NoBringToFrontOnFocus),
        X(ImGuiWindowFlags, AlwaysVerticalScrollbar),
        X(ImGuiWindowFlags, AlwaysHorizontalScrollbar),
        X(ImGuiWindowFlags, NoNavInputs),
        X(ImGuiWindowFlags, NoNavFocus),
        X(ImGuiWindowFlags, UnsavedDocument),
        X(ImGuiWindowFlags, NoDocking),
        X(ImGuiWindowFlags, NoNav),
        X(ImGuiWindowFlags, NoDecoration),
        X(ImGuiWindowFlags, NoInputs),
};

static util::TableInteger ChildFlags[] = {
        X(ImGuiChildFlags, None),        X(ImGuiChildFlags, Border),      X(ImGuiChildFlags, AlwaysUseWindowPadding), X(ImGuiChildFlags, ResizeX),    X(ImGuiChildFlags, ResizeY),
        X(ImGuiChildFlags, AutoResizeX), X(ImGuiChildFlags, AutoResizeY), X(ImGuiChildFlags, AlwaysAutoResize),       X(ImGuiChildFlags, FrameStyle), X(ImGuiChildFlags, NavFlattened),
};

static util::TableInteger ItemFlags[] = {
        X(ImGuiItemFlags, None), X(ImGuiItemFlags, NoTabStop), X(ImGuiItemFlags, NoNav), X(ImGuiItemFlags, NoNavDefaultFocus), X(ImGuiItemFlags, ButtonRepeat), X(ImGuiItemFlags, AutoClosePopups),
};

static util::TableInteger InputTextFlags[] = {
        X(ImGuiInputTextFlags, None),
        X(ImGuiInputTextFlags, CharsDecimal),
        X(ImGuiInputTextFlags, CharsHexadecimal),
        X(ImGuiInputTextFlags, CharsScientific),
        X(ImGuiInputTextFlags, CharsUppercase),
        X(ImGuiInputTextFlags, CharsNoBlank),
        X(ImGuiInputTextFlags, AllowTabInput),
        X(ImGuiInputTextFlags, EnterReturnsTrue),
        X(ImGuiInputTextFlags, EscapeClearsAll),
        X(ImGuiInputTextFlags, CtrlEnterForNewLine),
        X(ImGuiInputTextFlags, ReadOnly),
        X(ImGuiInputTextFlags, Password),
        X(ImGuiInputTextFlags, AlwaysOverwrite),
        X(ImGuiInputTextFlags, AutoSelectAll),
        X(ImGuiInputTextFlags, ParseEmptyRefVal),
        X(ImGuiInputTextFlags, DisplayEmptyRefVal),
        X(ImGuiInputTextFlags, NoHorizontalScroll),
        X(ImGuiInputTextFlags, NoUndoRedo),
        X(ImGuiInputTextFlags, CallbackCompletion),
        X(ImGuiInputTextFlags, CallbackHistory),
        X(ImGuiInputTextFlags, CallbackAlways),
        X(ImGuiInputTextFlags, CallbackCharFilter),
        X(ImGuiInputTextFlags, CallbackResize),
        X(ImGuiInputTextFlags, CallbackEdit),
};

static util::TableInteger TreeNodeFlags[] = {
        X(ImGuiTreeNodeFlags, None),
        X(ImGuiTreeNodeFlags, Selected),
        X(ImGuiTreeNodeFlags, Framed),
        X(ImGuiTreeNodeFlags, AllowOverlap),
        X(ImGuiTreeNodeFlags, NoTreePushOnOpen),
        X(ImGuiTreeNodeFlags, NoAutoOpenOnLog),
        X(ImGuiTreeNodeFlags, DefaultOpen),
        X(ImGuiTreeNodeFlags, OpenOnDoubleClick),
        X(ImGuiTreeNodeFlags, OpenOnArrow),
        X(ImGuiTreeNodeFlags, Leaf),
        X(ImGuiTreeNodeFlags, Bullet),
        X(ImGuiTreeNodeFlags, FramePadding),
        X(ImGuiTreeNodeFlags, SpanAvailWidth),
        X(ImGuiTreeNodeFlags, SpanFullWidth),
        X(ImGuiTreeNodeFlags, SpanTextWidth),
        X(ImGuiTreeNodeFlags, SpanAllColumns),
        X(ImGuiTreeNodeFlags, NavLeftJumpsBackHere),
        X(ImGuiTreeNodeFlags, CollapsingHeader),
};

static util::TableInteger PopupFlags[] = {
        X(ImGuiPopupFlags, None),
        X(ImGuiPopupFlags, MouseButtonLeft),
        X(ImGuiPopupFlags, MouseButtonRight),
        X(ImGuiPopupFlags, MouseButtonMiddle),
        X(ImGuiPopupFlags, NoReopen),
        X(ImGuiPopupFlags, NoOpenOverExistingPopup),
        X(ImGuiPopupFlags, NoOpenOverItems),
        X(ImGuiPopupFlags, AnyPopupId),
        X(ImGuiPopupFlags, AnyPopupLevel),
        X(ImGuiPopupFlags, AnyPopup),
};

static util::TableInteger SelectableFlags[] = {
        X(ImGuiSelectableFlags, None),     X(ImGuiSelectableFlags, NoAutoClosePopups), X(ImGuiSelectableFlags, SpanAllColumns), X(ImGuiSelectableFlags, AllowDoubleClick),
        X(ImGuiSelectableFlags, Disabled), X(ImGuiSelectableFlags, AllowOverlap),      X(ImGuiSelectableFlags, Highlight),
};

static util::TableInteger ComboFlags[] = {
        X(ImGuiComboFlags, None),          X(ImGuiComboFlags, PopupAlignLeft), X(ImGuiComboFlags, HeightSmall), X(ImGuiComboFlags, HeightRegular),   X(ImGuiComboFlags, HeightLarge),
        X(ImGuiComboFlags, HeightLargest), X(ImGuiComboFlags, NoArrowButton),  X(ImGuiComboFlags, NoPreview),   X(ImGuiComboFlags, WidthFitPreview),
};

static util::TableInteger TabBarFlags[] = {
        X(ImGuiTabBarFlags, None),
        X(ImGuiTabBarFlags, Reorderable),
        X(ImGuiTabBarFlags, AutoSelectNewTabs),
        X(ImGuiTabBarFlags, TabListPopupButton),
        X(ImGuiTabBarFlags, NoCloseWithMiddleMouseButton),
        X(ImGuiTabBarFlags, NoTabListScrollingButtons),
        X(ImGuiTabBarFlags, NoTooltip),
        X(ImGuiTabBarFlags, DrawSelectedOverline),
        X(ImGuiTabBarFlags, FittingPolicyResizeDown),
        X(ImGuiTabBarFlags, FittingPolicyScroll),
};

static util::TableInteger TabItemFlags[] = {
        X(ImGuiTabItemFlags, None),     X(ImGuiTabItemFlags, UnsavedDocument),  X(ImGuiTabItemFlags, SetSelected), X(ImGuiTabItemFlags, NoCloseWithMiddleMouseButton),
        X(ImGuiTabItemFlags, NoPushId), X(ImGuiTabItemFlags, NoTooltip),        X(ImGuiTabItemFlags, NoReorder),   X(ImGuiTabItemFlags, Leading),
        X(ImGuiTabItemFlags, Trailing), X(ImGuiTabItemFlags, NoAssumedClosure),
};

static util::TableInteger FocusedFlags[] = {
        X(ImGuiFocusedFlags, None),          X(ImGuiFocusedFlags, ChildWindows),        X(ImGuiFocusedFlags, RootWindow), X(ImGuiFocusedFlags, AnyWindow), X(ImGuiFocusedFlags, NoPopupHierarchy),
        X(ImGuiFocusedFlags, DockHierarchy), X(ImGuiFocusedFlags, RootAndChildWindows),
};

static util::TableInteger HoveredFlags[] = {
        X(ImGuiHoveredFlags, None),
        X(ImGuiHoveredFlags, ChildWindows),
        X(ImGuiHoveredFlags, RootWindow),
        X(ImGuiHoveredFlags, AnyWindow),
        X(ImGuiHoveredFlags, NoPopupHierarchy),
        X(ImGuiHoveredFlags, DockHierarchy),
        X(ImGuiHoveredFlags, AllowWhenBlockedByPopup),
        X(ImGuiHoveredFlags, AllowWhenBlockedByActiveItem),
        X(ImGuiHoveredFlags, AllowWhenOverlappedByItem),
        X(ImGuiHoveredFlags, AllowWhenOverlappedByWindow),
        X(ImGuiHoveredFlags, AllowWhenDisabled),
        X(ImGuiHoveredFlags, NoNavOverride),
        X(ImGuiHoveredFlags, AllowWhenOverlapped),
        X(ImGuiHoveredFlags, RectOnly),
        X(ImGuiHoveredFlags, RootAndChildWindows),
        X(ImGuiHoveredFlags, ForTooltip),
        X(ImGuiHoveredFlags, Stationary),
        X(ImGuiHoveredFlags, DelayNone),
        X(ImGuiHoveredFlags, DelayShort),
        X(ImGuiHoveredFlags, DelayNormal),
        X(ImGuiHoveredFlags, NoSharedDelay),
};

static util::TableInteger DockNodeFlags[] = {
        X(ImGuiDockNodeFlags, None),           X(ImGuiDockNodeFlags, KeepAliveOnly), X(ImGuiDockNodeFlags, NoDockingOverCentralNode), X(ImGuiDockNodeFlags, PassthruCentralNode),
        X(ImGuiDockNodeFlags, NoDockingSplit), X(ImGuiDockNodeFlags, NoResize),      X(ImGuiDockNodeFlags, AutoHideTabBar),           X(ImGuiDockNodeFlags, NoUndocking),
};

static util::TableInteger DragDropFlags[] = {
        X(ImGuiDragDropFlags, None),
        X(ImGuiDragDropFlags, SourceNoPreviewTooltip),
        X(ImGuiDragDropFlags, SourceNoDisableHover),
        X(ImGuiDragDropFlags, SourceNoHoldToOpenOthers),
        X(ImGuiDragDropFlags, SourceAllowNullID),
        X(ImGuiDragDropFlags, SourceExtern),
        X(ImGuiDragDropFlags, PayloadAutoExpire),
        X(ImGuiDragDropFlags, PayloadNoCrossContext),
        X(ImGuiDragDropFlags, PayloadNoCrossProcess),
        X(ImGuiDragDropFlags, AcceptBeforeDelivery),
        X(ImGuiDragDropFlags, AcceptNoDrawDefaultRect),
        X(ImGuiDragDropFlags, AcceptNoPreviewTooltip),
        X(ImGuiDragDropFlags, AcceptPeekOnly),
};

static util::TableInteger InputFlags[] = {
        X(ImGuiInputFlags, None),        X(ImGuiInputFlags, Repeat),           X(ImGuiInputFlags, RouteActive),     X(ImGuiInputFlags, RouteFocused),         X(ImGuiInputFlags, RouteGlobal),
        X(ImGuiInputFlags, RouteAlways), X(ImGuiInputFlags, RouteOverFocused), X(ImGuiInputFlags, RouteOverActive), X(ImGuiInputFlags, RouteUnlessBgFocused), X(ImGuiInputFlags, RouteFromRootWindow),
        X(ImGuiInputFlags, Tooltip),
};

static util::TableInteger ConfigFlags[] = {
        X(ImGuiConfigFlags, None),
        X(ImGuiConfigFlags, NavEnableKeyboard),
        X(ImGuiConfigFlags, NavEnableGamepad),
        X(ImGuiConfigFlags, NavEnableSetMousePos),
        X(ImGuiConfigFlags, NavNoCaptureKeyboard),
        X(ImGuiConfigFlags, NoMouse),
        X(ImGuiConfigFlags, NoMouseCursorChange),
        X(ImGuiConfigFlags, NoKeyboard),
        X(ImGuiConfigFlags, DockingEnable),
        X(ImGuiConfigFlags, ViewportsEnable),
        X(ImGuiConfigFlags, DpiEnableScaleViewports),
        X(ImGuiConfigFlags, DpiEnableScaleFonts),
        X(ImGuiConfigFlags, IsSRGB),
        X(ImGuiConfigFlags, IsTouchScreen),
};

static util::TableInteger BackendFlags[] = {
        X(ImGuiBackendFlags, None),
        X(ImGuiBackendFlags, HasGamepad),
        X(ImGuiBackendFlags, HasMouseCursors),
        X(ImGuiBackendFlags, HasSetMousePos),
        X(ImGuiBackendFlags, RendererHasVtxOffset),
        X(ImGuiBackendFlags, PlatformHasViewports),
        X(ImGuiBackendFlags, HasMouseHoveredViewport),
        X(ImGuiBackendFlags, RendererHasViewports),
};

static util::TableInteger ButtonFlags[] = {
        X(ImGuiButtonFlags, None),
        X(ImGuiButtonFlags, MouseButtonLeft),
        X(ImGuiButtonFlags, MouseButtonRight),
        X(ImGuiButtonFlags, MouseButtonMiddle),
};

static util::TableInteger ColorEditFlags[] = {
        X(ImGuiColorEditFlags, None),           X(ImGuiColorEditFlags, NoAlpha),          X(ImGuiColorEditFlags, NoPicker),  X(ImGuiColorEditFlags, NoOptions),
        X(ImGuiColorEditFlags, NoSmallPreview), X(ImGuiColorEditFlags, NoInputs),         X(ImGuiColorEditFlags, NoTooltip), X(ImGuiColorEditFlags, NoLabel),
        X(ImGuiColorEditFlags, NoSidePreview),  X(ImGuiColorEditFlags, NoDragDrop),       X(ImGuiColorEditFlags, NoBorder),  X(ImGuiColorEditFlags, AlphaBar),
        X(ImGuiColorEditFlags, AlphaPreview),   X(ImGuiColorEditFlags, AlphaPreviewHalf), X(ImGuiColorEditFlags, HDR),       X(ImGuiColorEditFlags, DisplayRGB),
        X(ImGuiColorEditFlags, DisplayHSV),     X(ImGuiColorEditFlags, DisplayHex),       X(ImGuiColorEditFlags, Uint8),     X(ImGuiColorEditFlags, Float),
        X(ImGuiColorEditFlags, PickerHueBar),   X(ImGuiColorEditFlags, PickerHueWheel),   X(ImGuiColorEditFlags, InputRGB),  X(ImGuiColorEditFlags, InputHSV),
};

static util::TableInteger SliderFlags[] = {
        X(ImGuiSliderFlags, None),    X(ImGuiSliderFlags, AlwaysClamp), X(ImGuiSliderFlags, Logarithmic), X(ImGuiSliderFlags, NoRoundToFormat),
        X(ImGuiSliderFlags, NoInput), X(ImGuiSliderFlags, WrapAround),
};

static util::TableInteger TableFlags[] = {
        X(ImGuiTableFlags, None),
        X(ImGuiTableFlags, Resizable),
        X(ImGuiTableFlags, Reorderable),
        X(ImGuiTableFlags, Hideable),
        X(ImGuiTableFlags, Sortable),
        X(ImGuiTableFlags, NoSavedSettings),
        X(ImGuiTableFlags, ContextMenuInBody),
        X(ImGuiTableFlags, RowBg),
        X(ImGuiTableFlags, BordersInnerH),
        X(ImGuiTableFlags, BordersOuterH),
        X(ImGuiTableFlags, BordersInnerV),
        X(ImGuiTableFlags, BordersOuterV),
        X(ImGuiTableFlags, BordersH),
        X(ImGuiTableFlags, BordersV),
        X(ImGuiTableFlags, BordersInner),
        X(ImGuiTableFlags, BordersOuter),
        X(ImGuiTableFlags, Borders),
        X(ImGuiTableFlags, NoBordersInBody),
        X(ImGuiTableFlags, NoBordersInBodyUntilResize),
        X(ImGuiTableFlags, SizingFixedFit),
        X(ImGuiTableFlags, SizingFixedSame),
        X(ImGuiTableFlags, SizingStretchProp),
        X(ImGuiTableFlags, SizingStretchSame),
        X(ImGuiTableFlags, NoHostExtendX),
        X(ImGuiTableFlags, NoHostExtendY),
        X(ImGuiTableFlags, NoKeepColumnsVisible),
        X(ImGuiTableFlags, PreciseWidths),
        X(ImGuiTableFlags, NoClip),
        X(ImGuiTableFlags, PadOuterX),
        X(ImGuiTableFlags, NoPadOuterX),
        X(ImGuiTableFlags, NoPadInnerX),
        X(ImGuiTableFlags, ScrollX),
        X(ImGuiTableFlags, ScrollY),
        X(ImGuiTableFlags, SortMulti),
        X(ImGuiTableFlags, SortTristate),
        X(ImGuiTableFlags, HighlightHoveredColumn),
};

static util::TableInteger TableColumnFlags[] = {
        X(ImGuiTableColumnFlags, None),
        X(ImGuiTableColumnFlags, Disabled),
        X(ImGuiTableColumnFlags, DefaultHide),
        X(ImGuiTableColumnFlags, DefaultSort),
        X(ImGuiTableColumnFlags, WidthStretch),
        X(ImGuiTableColumnFlags, WidthFixed),
        X(ImGuiTableColumnFlags, NoResize),
        X(ImGuiTableColumnFlags, NoReorder),
        X(ImGuiTableColumnFlags, NoHide),
        X(ImGuiTableColumnFlags, NoClip),
        X(ImGuiTableColumnFlags, NoSort),
        X(ImGuiTableColumnFlags, NoSortAscending),
        X(ImGuiTableColumnFlags, NoSortDescending),
        X(ImGuiTableColumnFlags, NoHeaderLabel),
        X(ImGuiTableColumnFlags, NoHeaderWidth),
        X(ImGuiTableColumnFlags, PreferSortAscending),
        X(ImGuiTableColumnFlags, PreferSortDescending),
        X(ImGuiTableColumnFlags, IndentEnable),
        X(ImGuiTableColumnFlags, IndentDisable),
        X(ImGuiTableColumnFlags, AngledHeader),
        X(ImGuiTableColumnFlags, IsEnabled),
        X(ImGuiTableColumnFlags, IsVisible),
        X(ImGuiTableColumnFlags, IsSorted),
        X(ImGuiTableColumnFlags, IsHovered),
};

static util::TableInteger TableRowFlags[] = {
        X(ImGuiTableRowFlags, None),
        X(ImGuiTableRowFlags, Headers),
};

static util::TableInteger MultiSelectFlags[] = {
        X(ImGuiMultiSelectFlags, None),         X(ImGuiMultiSelectFlags, SingleSelect),      X(ImGuiMultiSelectFlags, NoSelectAll),           X(ImGuiMultiSelectFlags, NoRangeSelect),
        X(ImGuiMultiSelectFlags, NoAutoSelect), X(ImGuiMultiSelectFlags, NoAutoClear),       X(ImGuiMultiSelectFlags, NoAutoClearOnReselect), X(ImGuiMultiSelectFlags, BoxSelect1d),
        X(ImGuiMultiSelectFlags, BoxSelect2d),  X(ImGuiMultiSelectFlags, BoxSelectNoScroll), X(ImGuiMultiSelectFlags, ClearOnEscape),         X(ImGuiMultiSelectFlags, ClearOnClickVoid),
        X(ImGuiMultiSelectFlags, ScopeWindow),  X(ImGuiMultiSelectFlags, ScopeRect),         X(ImGuiMultiSelectFlags, SelectOnClick),         X(ImGuiMultiSelectFlags, SelectOnClickRelease),
        X(ImGuiMultiSelectFlags, NavWrapX),
};

static util::TableInteger DrawFlags[] = {
        X(ImDrawFlags, None),
        X(ImDrawFlags, Closed),
        X(ImDrawFlags, RoundCornersTopLeft),
        X(ImDrawFlags, RoundCornersTopRight),
        X(ImDrawFlags, RoundCornersBottomLeft),
        X(ImDrawFlags, RoundCornersBottomRight),
        X(ImDrawFlags, RoundCornersNone),
        X(ImDrawFlags, RoundCornersTop),
        X(ImDrawFlags, RoundCornersBottom),
        X(ImDrawFlags, RoundCornersLeft),
        X(ImDrawFlags, RoundCornersRight),
        X(ImDrawFlags, RoundCornersAll),
};

static util::TableInteger DrawListFlags[] = {
        X(ImDrawListFlags, None), X(ImDrawListFlags, AntiAliasedLines), X(ImDrawListFlags, AntiAliasedLinesUseTex), X(ImDrawListFlags, AntiAliasedFill), X(ImDrawListFlags, AllowVtxOffset),
};

static util::TableInteger FontAtlasFlags[] = {
        X(ImFontAtlasFlags, None),
        X(ImFontAtlasFlags, NoPowerOfTwoHeight),
        X(ImFontAtlasFlags, NoMouseCursors),
        X(ImFontAtlasFlags, NoBakedLines),
};

static util::TableInteger ViewportFlags[] = {
        X(ImGuiViewportFlags, None),
        X(ImGuiViewportFlags, IsPlatformWindow),
        X(ImGuiViewportFlags, IsPlatformMonitor),
        X(ImGuiViewportFlags, OwnedByApp),
        X(ImGuiViewportFlags, NoDecoration),
        X(ImGuiViewportFlags, NoTaskBarIcon),
        X(ImGuiViewportFlags, NoFocusOnAppearing),
        X(ImGuiViewportFlags, NoFocusOnClick),
        X(ImGuiViewportFlags, NoInputs),
        X(ImGuiViewportFlags, NoRendererClear),
        X(ImGuiViewportFlags, NoAutoMerge),
        X(ImGuiViewportFlags, TopMost),
        X(ImGuiViewportFlags, CanHostOtherWindows),
        X(ImGuiViewportFlags, IsMinimized),
        X(ImGuiViewportFlags, IsFocused),
};

static util::TableInteger DataType[] = {
        X(ImGuiDataType, S8),  X(ImGuiDataType, U8),  X(ImGuiDataType, S16),   X(ImGuiDataType, U16),    X(ImGuiDataType, S32),  X(ImGuiDataType, U32),
        X(ImGuiDataType, S64), X(ImGuiDataType, U64), X(ImGuiDataType, Float), X(ImGuiDataType, Double), X(ImGuiDataType, Bool),
};

static util::TableInteger Dir[] = {
        X(ImGuiDir, None), X(ImGuiDir, Left), X(ImGuiDir, Right), X(ImGuiDir, Up), X(ImGuiDir, Down),
};

static util::TableInteger SortDirection[] = {
        X(ImGuiSortDirection, None),
        X(ImGuiSortDirection, Ascending),
        X(ImGuiSortDirection, Descending),
};

static util::TableInteger Key[] = {
        X(ImGuiKey, None),
        X(ImGuiKey, Tab),
        X(ImGuiKey, LeftArrow),
        X(ImGuiKey, RightArrow),
        X(ImGuiKey, UpArrow),
        X(ImGuiKey, DownArrow),
        X(ImGuiKey, PageUp),
        X(ImGuiKey, PageDown),
        X(ImGuiKey, Home),
        X(ImGuiKey, End),
        X(ImGuiKey, Insert),
        X(ImGuiKey, Delete),
        X(ImGuiKey, Backspace),
        X(ImGuiKey, Space),
        X(ImGuiKey, Enter),
        X(ImGuiKey, Escape),
        X(ImGuiKey, LeftCtrl),
        X(ImGuiKey, LeftShift),
        X(ImGuiKey, LeftAlt),
        X(ImGuiKey, LeftSuper),
        X(ImGuiKey, RightCtrl),
        X(ImGuiKey, RightShift),
        X(ImGuiKey, RightAlt),
        X(ImGuiKey, RightSuper),
        X(ImGuiKey, Menu),
        X(ImGuiKey, 0),
        X(ImGuiKey, 1),
        X(ImGuiKey, 2),
        X(ImGuiKey, 3),
        X(ImGuiKey, 4),
        X(ImGuiKey, 5),
        X(ImGuiKey, 6),
        X(ImGuiKey, 7),
        X(ImGuiKey, 8),
        X(ImGuiKey, 9),
        X(ImGuiKey, A),
        X(ImGuiKey, B),
        X(ImGuiKey, C),
        X(ImGuiKey, D),
        X(ImGuiKey, E),
        X(ImGuiKey, F),
        X(ImGuiKey, G),
        X(ImGuiKey, H),
        X(ImGuiKey, I),
        X(ImGuiKey, J),
        X(ImGuiKey, K),
        X(ImGuiKey, L),
        X(ImGuiKey, M),
        X(ImGuiKey, N),
        X(ImGuiKey, O),
        X(ImGuiKey, P),
        X(ImGuiKey, Q),
        X(ImGuiKey, R),
        X(ImGuiKey, S),
        X(ImGuiKey, T),
        X(ImGuiKey, U),
        X(ImGuiKey, V),
        X(ImGuiKey, W),
        X(ImGuiKey, X),
        X(ImGuiKey, Y),
        X(ImGuiKey, Z),
        X(ImGuiKey, F1),
        X(ImGuiKey, F2),
        X(ImGuiKey, F3),
        X(ImGuiKey, F4),
        X(ImGuiKey, F5),
        X(ImGuiKey, F6),
        X(ImGuiKey, F7),
        X(ImGuiKey, F8),
        X(ImGuiKey, F9),
        X(ImGuiKey, F10),
        X(ImGuiKey, F11),
        X(ImGuiKey, F12),
        X(ImGuiKey, F13),
        X(ImGuiKey, F14),
        X(ImGuiKey, F15),
        X(ImGuiKey, F16),
        X(ImGuiKey, F17),
        X(ImGuiKey, F18),
        X(ImGuiKey, F19),
        X(ImGuiKey, F20),
        X(ImGuiKey, F21),
        X(ImGuiKey, F22),
        X(ImGuiKey, F23),
        X(ImGuiKey, F24),
        X(ImGuiKey, Apostrophe),
        X(ImGuiKey, Comma),
        X(ImGuiKey, Minus),
        X(ImGuiKey, Period),
        X(ImGuiKey, Slash),
        X(ImGuiKey, Semicolon),
        X(ImGuiKey, Equal),
        X(ImGuiKey, LeftBracket),
        X(ImGuiKey, Backslash),
        X(ImGuiKey, RightBracket),
        X(ImGuiKey, GraveAccent),
        X(ImGuiKey, CapsLock),
        X(ImGuiKey, ScrollLock),
        X(ImGuiKey, NumLock),
        X(ImGuiKey, PrintScreen),
        X(ImGuiKey, Pause),
        X(ImGuiKey, Keypad0),
        X(ImGuiKey, Keypad1),
        X(ImGuiKey, Keypad2),
        X(ImGuiKey, Keypad3),
        X(ImGuiKey, Keypad4),
        X(ImGuiKey, Keypad5),
        X(ImGuiKey, Keypad6),
        X(ImGuiKey, Keypad7),
        X(ImGuiKey, Keypad8),
        X(ImGuiKey, Keypad9),
        X(ImGuiKey, KeypadDecimal),
        X(ImGuiKey, KeypadDivide),
        X(ImGuiKey, KeypadMultiply),
        X(ImGuiKey, KeypadSubtract),
        X(ImGuiKey, KeypadAdd),
        X(ImGuiKey, KeypadEnter),
        X(ImGuiKey, KeypadEqual),
        X(ImGuiKey, AppBack),
        X(ImGuiKey, AppForward),
        X(ImGuiKey, GamepadStart),
        X(ImGuiKey, GamepadBack),
        X(ImGuiKey, GamepadFaceLeft),
        X(ImGuiKey, GamepadFaceRight),
        X(ImGuiKey, GamepadFaceUp),
        X(ImGuiKey, GamepadFaceDown),
        X(ImGuiKey, GamepadDpadLeft),
        X(ImGuiKey, GamepadDpadRight),
        X(ImGuiKey, GamepadDpadUp),
        X(ImGuiKey, GamepadDpadDown),
        X(ImGuiKey, GamepadL1),
        X(ImGuiKey, GamepadR1),
        X(ImGuiKey, GamepadL2),
        X(ImGuiKey, GamepadR2),
        X(ImGuiKey, GamepadL3),
        X(ImGuiKey, GamepadR3),
        X(ImGuiKey, GamepadLStickLeft),
        X(ImGuiKey, GamepadLStickRight),
        X(ImGuiKey, GamepadLStickUp),
        X(ImGuiKey, GamepadLStickDown),
        X(ImGuiKey, GamepadRStickLeft),
        X(ImGuiKey, GamepadRStickRight),
        X(ImGuiKey, GamepadRStickUp),
        X(ImGuiKey, GamepadRStickDown),
        X(ImGuiKey, MouseLeft),
        X(ImGuiKey, MouseRight),
        X(ImGuiKey, MouseMiddle),
        X(ImGuiKey, MouseX1),
        X(ImGuiKey, MouseX2),
        X(ImGuiKey, MouseWheelX),
        X(ImGuiKey, MouseWheelY),
};

static util::TableInteger Mod[] = {
        X(ImGuiMod, None), X(ImGuiMod, Ctrl), X(ImGuiMod, Shift), X(ImGuiMod, Alt), X(ImGuiMod, Super),
};

static util::TableInteger Col[] = {
        X(ImGuiCol, Text),
        X(ImGuiCol, TextDisabled),
        X(ImGuiCol, WindowBg),
        X(ImGuiCol, ChildBg),
        X(ImGuiCol, PopupBg),
        X(ImGuiCol, Border),
        X(ImGuiCol, BorderShadow),
        X(ImGuiCol, FrameBg),
        X(ImGuiCol, FrameBgHovered),
        X(ImGuiCol, FrameBgActive),
        X(ImGuiCol, TitleBg),
        X(ImGuiCol, TitleBgActive),
        X(ImGuiCol, TitleBgCollapsed),
        X(ImGuiCol, MenuBarBg),
        X(ImGuiCol, ScrollbarBg),
        X(ImGuiCol, ScrollbarGrab),
        X(ImGuiCol, ScrollbarGrabHovered),
        X(ImGuiCol, ScrollbarGrabActive),
        X(ImGuiCol, CheckMark),
        X(ImGuiCol, SliderGrab),
        X(ImGuiCol, SliderGrabActive),
        X(ImGuiCol, Button),
        X(ImGuiCol, ButtonHovered),
        X(ImGuiCol, ButtonActive),
        X(ImGuiCol, Header),
        X(ImGuiCol, HeaderHovered),
        X(ImGuiCol, HeaderActive),
        X(ImGuiCol, Separator),
        X(ImGuiCol, SeparatorHovered),
        X(ImGuiCol, SeparatorActive),
        X(ImGuiCol, ResizeGrip),
        X(ImGuiCol, ResizeGripHovered),
        X(ImGuiCol, ResizeGripActive),
        X(ImGuiCol, TabHovered),
        X(ImGuiCol, Tab),
        X(ImGuiCol, TabSelected),
        X(ImGuiCol, TabSelectedOverline),
        X(ImGuiCol, TabDimmed),
        X(ImGuiCol, TabDimmedSelected),
        X(ImGuiCol, TabDimmedSelectedOverline),
        X(ImGuiCol, DockingPreview),
        X(ImGuiCol, DockingEmptyBg),
        X(ImGuiCol, PlotLines),
        X(ImGuiCol, PlotLinesHovered),
        X(ImGuiCol, PlotHistogram),
        X(ImGuiCol, PlotHistogramHovered),
        X(ImGuiCol, TableHeaderBg),
        X(ImGuiCol, TableBorderStrong),
        X(ImGuiCol, TableBorderLight),
        X(ImGuiCol, TableRowBg),
        X(ImGuiCol, TableRowBgAlt),
        X(ImGuiCol, TextLink),
        X(ImGuiCol, TextSelectedBg),
        X(ImGuiCol, DragDropTarget),
        X(ImGuiCol, NavHighlight),
        X(ImGuiCol, NavWindowingHighlight),
        X(ImGuiCol, NavWindowingDimBg),
        X(ImGuiCol, ModalWindowDimBg),
};

static util::TableInteger StyleVar[] = {
        X(ImGuiStyleVar, Alpha),
        X(ImGuiStyleVar, DisabledAlpha),
        X(ImGuiStyleVar, WindowPadding),
        X(ImGuiStyleVar, WindowRounding),
        X(ImGuiStyleVar, WindowBorderSize),
        X(ImGuiStyleVar, WindowMinSize),
        X(ImGuiStyleVar, WindowTitleAlign),
        X(ImGuiStyleVar, ChildRounding),
        X(ImGuiStyleVar, ChildBorderSize),
        X(ImGuiStyleVar, PopupRounding),
        X(ImGuiStyleVar, PopupBorderSize),
        X(ImGuiStyleVar, FramePadding),
        X(ImGuiStyleVar, FrameRounding),
        X(ImGuiStyleVar, FrameBorderSize),
        X(ImGuiStyleVar, ItemSpacing),
        X(ImGuiStyleVar, ItemInnerSpacing),
        X(ImGuiStyleVar, IndentSpacing),
        X(ImGuiStyleVar, CellPadding),
        X(ImGuiStyleVar, ScrollbarSize),
        X(ImGuiStyleVar, ScrollbarRounding),
        X(ImGuiStyleVar, GrabMinSize),
        X(ImGuiStyleVar, GrabRounding),
        X(ImGuiStyleVar, TabRounding),
        X(ImGuiStyleVar, TabBorderSize),
        X(ImGuiStyleVar, TabBarBorderSize),
        X(ImGuiStyleVar, TabBarOverlineSize),
        X(ImGuiStyleVar, TableAngledHeadersAngle),
        X(ImGuiStyleVar, TableAngledHeadersTextAlign),
        X(ImGuiStyleVar, ButtonTextAlign),
        X(ImGuiStyleVar, SelectableTextAlign),
        X(ImGuiStyleVar, SeparatorTextBorderSize),
        X(ImGuiStyleVar, SeparatorTextAlign),
        X(ImGuiStyleVar, SeparatorTextPadding),
        X(ImGuiStyleVar, DockingSeparatorSize),
};

static util::TableInteger MouseButton[] = {
        X(ImGuiMouseButton, Left),
        X(ImGuiMouseButton, Right),
        X(ImGuiMouseButton, Middle),
};

static util::TableInteger MouseCursor[] = {
        X(ImGuiMouseCursor, None),     X(ImGuiMouseCursor, Arrow),      X(ImGuiMouseCursor, TextInput),  X(ImGuiMouseCursor, ResizeAll), X(ImGuiMouseCursor, ResizeNS),
        X(ImGuiMouseCursor, ResizeEW), X(ImGuiMouseCursor, ResizeNESW), X(ImGuiMouseCursor, ResizeNWSE), X(ImGuiMouseCursor, Hand),      X(ImGuiMouseCursor, NotAllowed),
};

static util::TableInteger MouseSource[] = {
        X(ImGuiMouseSource, Mouse),
        X(ImGuiMouseSource, TouchScreen),
        X(ImGuiMouseSource, Pen),
};

static util::TableInteger Cond[] = {
        X(ImGuiCond, None), X(ImGuiCond, Always), X(ImGuiCond, Once), X(ImGuiCond, FirstUseEver), X(ImGuiCond, Appearing),
};

static util::TableInteger TableBgTarget[] = {
        X(ImGuiTableBgTarget, None),
        X(ImGuiTableBgTarget, RowBg0),
        X(ImGuiTableBgTarget, RowBg1),
        X(ImGuiTableBgTarget, CellBg),
};

static util::TableInteger SelectionRequestType[] = {
        X(ImGuiSelectionRequestType, None),
        X(ImGuiSelectionRequestType, SetAll),
        X(ImGuiSelectionRequestType, SetRange),
};

#undef X

namespace wrap_ImGuiContext {
void pointer(lua_State* L, ImGuiContext& v);
}
namespace wrap_ImGuiIO {
void pointer(lua_State* L, ImGuiIO& v);
}
namespace wrap_ImGuiInputTextCallbackData {
void pointer(lua_State* L, ImGuiInputTextCallbackData& v);
}
namespace wrap_ImGuiWindowClass {
void pointer(lua_State* L, ImGuiWindowClass& v);
}
namespace wrap_ImGuiStorage {
void pointer(lua_State* L, ImGuiStorage& v);
}
namespace wrap_ImGuiMultiSelectIO {
void pointer(lua_State* L, ImGuiMultiSelectIO& v);
}
namespace wrap_ImGuiSelectionRequest {
void pointer(lua_State* L, ImGuiSelectionRequest& v);
}
namespace wrap_ImGuiSelectionExternalStorage {
void pointer(lua_State* L, ImGuiSelectionExternalStorage& v);
}
namespace wrap_ImFontConfig {
void pointer(lua_State* L, ImFontConfig& v);
}
namespace wrap_ImFontAtlas {
void pointer(lua_State* L, ImFontAtlas& v);
}
namespace wrap_ImGuiViewport {
void const_pointer(lua_State* L, ImGuiViewport& v);
}

static int IO(lua_State* L) {
    auto _retval = (ImGuiIO*)lua_newuserdatauv(L, sizeof(ImGuiIO), 0);
    new (_retval) ImGuiIO;
    wrap_ImGuiIO::pointer(L, *_retval);
    return 2;
}

static int InputTextCallbackData(lua_State* L) {
    auto _retval = (ImGuiInputTextCallbackData*)lua_newuserdatauv(L, sizeof(ImGuiInputTextCallbackData), 0);
    new (_retval) ImGuiInputTextCallbackData;
    wrap_ImGuiInputTextCallbackData::pointer(L, *_retval);
    return 2;
}

static int WindowClass(lua_State* L) {
    auto _retval = (ImGuiWindowClass*)lua_newuserdatauv(L, sizeof(ImGuiWindowClass), 0);
    new (_retval) ImGuiWindowClass;
    wrap_ImGuiWindowClass::pointer(L, *_retval);
    return 2;
}

static int Storage(lua_State* L) {
    auto _retval = (ImGuiStorage*)lua_newuserdatauv(L, sizeof(ImGuiStorage), 0);
    new (_retval) ImGuiStorage;
    wrap_ImGuiStorage::pointer(L, *_retval);
    return 2;
}

static int MultiSelectIO(lua_State* L) {
    auto _retval = (ImGuiMultiSelectIO*)lua_newuserdatauv(L, sizeof(ImGuiMultiSelectIO), 0);
    new (_retval) ImGuiMultiSelectIO;
    wrap_ImGuiMultiSelectIO::pointer(L, *_retval);
    return 2;
}

static int SelectionRequest(lua_State* L) {
    auto _retval = (ImGuiSelectionRequest*)lua_newuserdatauv(L, sizeof(ImGuiSelectionRequest), 0);
    new (_retval) ImGuiSelectionRequest;
    wrap_ImGuiSelectionRequest::pointer(L, *_retval);
    return 2;
}

static int SelectionExternalStorage(lua_State* L) {
    auto _retval = (ImGuiSelectionExternalStorage*)lua_newuserdatauv(L, sizeof(ImGuiSelectionExternalStorage), 0);
    new (_retval) ImGuiSelectionExternalStorage;
    wrap_ImGuiSelectionExternalStorage::pointer(L, *_retval);
    return 2;
}

static int FontConfig(lua_State* L) {
    auto _retval = (ImFontConfig*)lua_newuserdatauv(L, sizeof(ImFontConfig), 0);
    new (_retval) ImFontConfig;
    wrap_ImFontConfig::pointer(L, *_retval);
    return 2;
}

static int FontAtlas(lua_State* L) {
    auto _retval = (ImFontAtlas*)lua_newuserdatauv(L, sizeof(ImFontAtlas), 0);
    new (_retval) ImFontAtlas;
    wrap_ImFontAtlas::pointer(L, *_retval);
    return 2;
}

static int Viewport(lua_State* L) {
    auto _retval = (ImGuiViewport*)lua_newuserdatauv(L, sizeof(ImGuiViewport), 0);
    new (_retval) ImGuiViewport;
    wrap_ImGuiViewport::const_pointer(L, *_retval);
    return 2;
}

static int StringBuf(lua_State* L) {
    util::strbuf_create(L, 1);
    return 1;
}

static int CreateContext(lua_State* L) {
    auto shared_font_atlas = lua_isnoneornil(L, 1) ? NULL : *(ImFontAtlas**)lua_touserdata(L, 1);
    auto&& _retval = ImGui::CreateContext(shared_font_atlas);
    if (_retval != NULL) {
        wrap_ImGuiContext::pointer(L, *_retval);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int DestroyContext(lua_State* L) {
    auto ctx = lua_isnoneornil(L, 1) ? NULL : *(ImGuiContext**)lua_touserdata(L, 1);
    ImGui::DestroyContext(ctx);
    return 0;
}

static int GetCurrentContext(lua_State* L) {
    auto&& _retval = ImGui::GetCurrentContext();
    if (_retval != NULL) {
        wrap_ImGuiContext::pointer(L, *_retval);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int SetCurrentContext(lua_State* L) {
    auto ctx = *(ImGuiContext**)lua_touserdata(L, 1);
    ImGui::SetCurrentContext(ctx);
    return 0;
}

static int GetIO(lua_State* L) {
    auto&& _retval = ImGui::GetIO();
    wrap_ImGuiIO::pointer(L, _retval);
    return 1;
}

static int NewFrame(lua_State* L) {
    ImGui::NewFrame();
    return 0;
}

static int EndFrame(lua_State* L) {
    ImGui::EndFrame();
    return 0;
}

static int Render(lua_State* L) {
    ImGui::Render();
    return 0;
}

static int GetVersion(lua_State* L) {
    auto&& _retval = ImGui::GetVersion();
    lua_pushstring(L, _retval);
    return 1;
}

static int Begin(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    bool has_p_open = !lua_isnil(L, 2);
    bool p_open = true;
    auto flags = (ImGuiWindowFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiWindowFlags_None));
    auto&& _retval = ImGui::Begin(name, (has_p_open ? &p_open : NULL), flags);
    lua_pushboolean(L, _retval);
    if (has_p_open) {
        lua_pushboolean(L, p_open);
    } else {
        lua_pushnil(L);
    }
    return 2;
}

static int End(lua_State* L) {
    ImGui::End();
    return 0;
}

static int BeginChild(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_optnumber(L, 2, 0),
            (float)luaL_optnumber(L, 3, 0),
    };
    auto child_flags = (ImGuiChildFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiChildFlags_None));
    auto window_flags = (ImGuiWindowFlags)luaL_optinteger(L, 5, lua_Integer(ImGuiWindowFlags_None));
    auto&& _retval = ImGui::BeginChild(str_id, size, child_flags, window_flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginChildID(lua_State* L) {
    auto id = (ImGuiID)luaL_checkinteger(L, 1);
    auto size = ImVec2{
            (float)luaL_optnumber(L, 2, 0),
            (float)luaL_optnumber(L, 3, 0),
    };
    auto child_flags = (ImGuiChildFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiChildFlags_None));
    auto window_flags = (ImGuiWindowFlags)luaL_optinteger(L, 5, lua_Integer(ImGuiWindowFlags_None));
    auto&& _retval = ImGui::BeginChild(id, size, child_flags, window_flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndChild(lua_State* L) {
    ImGui::EndChild();
    return 0;
}

static int IsWindowAppearing(lua_State* L) {
    auto&& _retval = ImGui::IsWindowAppearing();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsWindowCollapsed(lua_State* L) {
    auto&& _retval = ImGui::IsWindowCollapsed();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsWindowFocused(lua_State* L) {
    auto flags = (ImGuiFocusedFlags)luaL_optinteger(L, 1, lua_Integer(ImGuiFocusedFlags_None));
    auto&& _retval = ImGui::IsWindowFocused(flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsWindowHovered(lua_State* L) {
    auto flags = (ImGuiHoveredFlags)luaL_optinteger(L, 1, lua_Integer(ImGuiHoveredFlags_None));
    auto&& _retval = ImGui::IsWindowHovered(flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int GetWindowDpiScale(lua_State* L) {
    auto&& _retval = ImGui::GetWindowDpiScale();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetWindowPos(lua_State* L) {
    auto&& _retval = ImGui::GetWindowPos();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetWindowSize(lua_State* L) {
    auto&& _retval = ImGui::GetWindowSize();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetWindowWidth(lua_State* L) {
    auto&& _retval = ImGui::GetWindowWidth();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetWindowHeight(lua_State* L) {
    auto&& _retval = ImGui::GetWindowHeight();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetWindowViewport(lua_State* L) {
    auto&& _retval = ImGui::GetWindowViewport();
    wrap_ImGuiViewport::const_pointer(L, *_retval);
    return 1;
}

static int SetNextWindowPos(lua_State* L) {
    auto pos = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto cond = (ImGuiCond)luaL_optinteger(L, 3, lua_Integer(ImGuiCond_None));
    ImGui::SetNextWindowPos(pos, cond);
    return 0;
}

static int SetNextWindowPosEx(lua_State* L) {
    auto pos = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto cond = (ImGuiCond)luaL_optinteger(L, 3, lua_Integer(ImGuiCond_None));
    auto pivot = ImVec2{
            (float)luaL_optnumber(L, 4, 0),
            (float)luaL_optnumber(L, 5, 0),
    };
    ImGui::SetNextWindowPos(pos, cond, pivot);
    return 0;
}

static int SetNextWindowSize(lua_State* L) {
    auto size = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto cond = (ImGuiCond)luaL_optinteger(L, 3, lua_Integer(ImGuiCond_None));
    ImGui::SetNextWindowSize(size, cond);
    return 0;
}

static int SetNextWindowContentSize(lua_State* L) {
    auto size = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    ImGui::SetNextWindowContentSize(size);
    return 0;
}

static int SetNextWindowCollapsed(lua_State* L) {
    auto collapsed = !!lua_toboolean(L, 1);
    auto cond = (ImGuiCond)luaL_optinteger(L, 2, lua_Integer(ImGuiCond_None));
    ImGui::SetNextWindowCollapsed(collapsed, cond);
    return 0;
}

static int SetNextWindowFocus(lua_State* L) {
    ImGui::SetNextWindowFocus();
    return 0;
}

static int SetNextWindowScroll(lua_State* L) {
    auto scroll = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    ImGui::SetNextWindowScroll(scroll);
    return 0;
}

static int SetNextWindowBgAlpha(lua_State* L) {
    auto alpha = (float)luaL_checknumber(L, 1);
    ImGui::SetNextWindowBgAlpha(alpha);
    return 0;
}

static int SetNextWindowViewport(lua_State* L) {
    auto viewport_id = (ImGuiID)luaL_checkinteger(L, 1);
    ImGui::SetNextWindowViewport(viewport_id);
    return 0;
}

static int SetWindowPos(lua_State* L) {
    auto pos = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto cond = (ImGuiCond)luaL_optinteger(L, 3, lua_Integer(ImGuiCond_None));
    ImGui::SetWindowPos(pos, cond);
    return 0;
}

static int SetWindowSize(lua_State* L) {
    auto size = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto cond = (ImGuiCond)luaL_optinteger(L, 3, lua_Integer(ImGuiCond_None));
    ImGui::SetWindowSize(size, cond);
    return 0;
}

static int SetWindowCollapsed(lua_State* L) {
    auto collapsed = !!lua_toboolean(L, 1);
    auto cond = (ImGuiCond)luaL_optinteger(L, 2, lua_Integer(ImGuiCond_None));
    ImGui::SetWindowCollapsed(collapsed, cond);
    return 0;
}

static int SetWindowFocus(lua_State* L) {
    ImGui::SetWindowFocus();
    return 0;
}

static int SetWindowFontScale(lua_State* L) {
    auto scale = (float)luaL_checknumber(L, 1);
    ImGui::SetWindowFontScale(scale);
    return 0;
}

static int SetWindowPosStr(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    auto pos = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    auto cond = (ImGuiCond)luaL_optinteger(L, 4, lua_Integer(ImGuiCond_None));
    ImGui::SetWindowPos(name, pos, cond);
    return 0;
}

static int SetWindowSizeStr(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    auto cond = (ImGuiCond)luaL_optinteger(L, 4, lua_Integer(ImGuiCond_None));
    ImGui::SetWindowSize(name, size, cond);
    return 0;
}

static int SetWindowCollapsedStr(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    auto collapsed = !!lua_toboolean(L, 2);
    auto cond = (ImGuiCond)luaL_optinteger(L, 3, lua_Integer(ImGuiCond_None));
    ImGui::SetWindowCollapsed(name, collapsed, cond);
    return 0;
}

static int SetWindowFocusStr(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    ImGui::SetWindowFocus(name);
    return 0;
}

static int GetScrollX(lua_State* L) {
    auto&& _retval = ImGui::GetScrollX();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetScrollY(lua_State* L) {
    auto&& _retval = ImGui::GetScrollY();
    lua_pushnumber(L, _retval);
    return 1;
}

static int SetScrollX(lua_State* L) {
    auto scroll_x = (float)luaL_checknumber(L, 1);
    ImGui::SetScrollX(scroll_x);
    return 0;
}

static int SetScrollY(lua_State* L) {
    auto scroll_y = (float)luaL_checknumber(L, 1);
    ImGui::SetScrollY(scroll_y);
    return 0;
}

static int GetScrollMaxX(lua_State* L) {
    auto&& _retval = ImGui::GetScrollMaxX();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetScrollMaxY(lua_State* L) {
    auto&& _retval = ImGui::GetScrollMaxY();
    lua_pushnumber(L, _retval);
    return 1;
}

static int SetScrollHereX(lua_State* L) {
    auto center_x_ratio = (float)luaL_optnumber(L, 1, 0.5f);
    ImGui::SetScrollHereX(center_x_ratio);
    return 0;
}

static int SetScrollHereY(lua_State* L) {
    auto center_y_ratio = (float)luaL_optnumber(L, 1, 0.5f);
    ImGui::SetScrollHereY(center_y_ratio);
    return 0;
}

static int SetScrollFromPosX(lua_State* L) {
    auto local_x = (float)luaL_checknumber(L, 1);
    auto center_x_ratio = (float)luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollFromPosX(local_x, center_x_ratio);
    return 0;
}

static int SetScrollFromPosY(lua_State* L) {
    auto local_y = (float)luaL_checknumber(L, 1);
    auto center_y_ratio = (float)luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollFromPosY(local_y, center_y_ratio);
    return 0;
}

static int PushFont(lua_State* L) {
    auto font = (ImFont*)lua_touserdata(L, 1);
    ImGui::PushFont(font);
    return 0;
}

static int PopFont(lua_State* L) {
    ImGui::PopFont();
    return 0;
}

static int PushStyleColor(lua_State* L) {
    auto idx = (ImGuiCol)luaL_checkinteger(L, 1);
    auto col = (ImU32)luaL_checkinteger(L, 2);
    ImGui::PushStyleColor(idx, col);
    return 0;
}

static int PushStyleColorImVec4(lua_State* L) {
    auto idx = (ImGuiCol)luaL_checkinteger(L, 1);
    auto col = ImVec4{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
            (float)luaL_checknumber(L, 5),
    };
    ImGui::PushStyleColor(idx, col);
    return 0;
}

static int PopStyleColor(lua_State* L) {
    ImGui::PopStyleColor();
    return 0;
}

static int PopStyleColorEx(lua_State* L) {
    auto count = (int)luaL_optinteger(L, 1, 1);
    ImGui::PopStyleColor(count);
    return 0;
}

static int PushStyleVar(lua_State* L) {
    auto idx = (ImGuiStyleVar)luaL_checkinteger(L, 1);
    auto val = (float)luaL_checknumber(L, 2);
    ImGui::PushStyleVar(idx, val);
    return 0;
}

static int PushStyleVarImVec2(lua_State* L) {
    auto idx = (ImGuiStyleVar)luaL_checkinteger(L, 1);
    auto val = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    ImGui::PushStyleVar(idx, val);
    return 0;
}

static int PopStyleVar(lua_State* L) {
    ImGui::PopStyleVar();
    return 0;
}

static int PopStyleVarEx(lua_State* L) {
    auto count = (int)luaL_optinteger(L, 1, 1);
    ImGui::PopStyleVar(count);
    return 0;
}

static int PushItemFlag(lua_State* L) {
    auto option = (ImGuiItemFlags)luaL_checkinteger(L, 1);
    auto enabled = !!lua_toboolean(L, 2);
    ImGui::PushItemFlag(option, enabled);
    return 0;
}

static int PopItemFlag(lua_State* L) {
    ImGui::PopItemFlag();
    return 0;
}

static int PushItemWidth(lua_State* L) {
    auto item_width = (float)luaL_checknumber(L, 1);
    ImGui::PushItemWidth(item_width);
    return 0;
}

static int PopItemWidth(lua_State* L) {
    ImGui::PopItemWidth();
    return 0;
}

static int SetNextItemWidth(lua_State* L) {
    auto item_width = (float)luaL_checknumber(L, 1);
    ImGui::SetNextItemWidth(item_width);
    return 0;
}

static int CalcItemWidth(lua_State* L) {
    auto&& _retval = ImGui::CalcItemWidth();
    lua_pushnumber(L, _retval);
    return 1;
}

static int PushTextWrapPos(lua_State* L) {
    auto wrap_local_pos_x = (float)luaL_optnumber(L, 1, 0.0f);
    ImGui::PushTextWrapPos(wrap_local_pos_x);
    return 0;
}

static int PopTextWrapPos(lua_State* L) {
    ImGui::PopTextWrapPos();
    return 0;
}

static int GetFont(lua_State* L) {
    auto&& _retval = ImGui::GetFont();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetFontSize(lua_State* L) {
    auto&& _retval = ImGui::GetFontSize();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetFontTexUvWhitePixel(lua_State* L) {
    auto&& _retval = ImGui::GetFontTexUvWhitePixel();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetColorU32(lua_State* L) {
    auto idx = (ImGuiCol)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::GetColorU32(idx);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetColorU32Ex(lua_State* L) {
    auto idx = (ImGuiCol)luaL_checkinteger(L, 1);
    auto alpha_mul = (float)luaL_optnumber(L, 2, 1.0f);
    auto&& _retval = ImGui::GetColorU32(idx, alpha_mul);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetColorU32ImVec4(lua_State* L) {
    auto col = ImVec4{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto&& _retval = ImGui::GetColorU32(col);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetColorU32ImU32(lua_State* L) {
    auto col = (ImU32)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::GetColorU32(col);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetColorU32ImU32Ex(lua_State* L) {
    auto col = (ImU32)luaL_checkinteger(L, 1);
    auto alpha_mul = (float)luaL_optnumber(L, 2, 1.0f);
    auto&& _retval = ImGui::GetColorU32(col, alpha_mul);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetStyleColorVec4(lua_State* L) {
    auto idx = (ImGuiCol)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::GetStyleColorVec4(idx);
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    lua_pushnumber(L, _retval.z);
    lua_pushnumber(L, _retval.w);
    return 4;
}

static int GetCursorScreenPos(lua_State* L) {
    auto&& _retval = ImGui::GetCursorScreenPos();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int SetCursorScreenPos(lua_State* L) {
    auto pos = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    ImGui::SetCursorScreenPos(pos);
    return 0;
}

static int GetContentRegionAvail(lua_State* L) {
    auto&& _retval = ImGui::GetContentRegionAvail();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetCursorPos(lua_State* L) {
    auto&& _retval = ImGui::GetCursorPos();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetCursorPosX(lua_State* L) {
    auto&& _retval = ImGui::GetCursorPosX();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetCursorPosY(lua_State* L) {
    auto&& _retval = ImGui::GetCursorPosY();
    lua_pushnumber(L, _retval);
    return 1;
}

static int SetCursorPos(lua_State* L) {
    auto local_pos = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    ImGui::SetCursorPos(local_pos);
    return 0;
}

static int SetCursorPosX(lua_State* L) {
    auto local_x = (float)luaL_checknumber(L, 1);
    ImGui::SetCursorPosX(local_x);
    return 0;
}

static int SetCursorPosY(lua_State* L) {
    auto local_y = (float)luaL_checknumber(L, 1);
    ImGui::SetCursorPosY(local_y);
    return 0;
}

static int GetCursorStartPos(lua_State* L) {
    auto&& _retval = ImGui::GetCursorStartPos();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int Separator(lua_State* L) {
    ImGui::Separator();
    return 0;
}

static int SameLine(lua_State* L) {
    ImGui::SameLine();
    return 0;
}

static int SameLineEx(lua_State* L) {
    auto offset_from_start_x = (float)luaL_optnumber(L, 1, 0.0f);
    auto spacing = (float)luaL_optnumber(L, 2, -1.0f);
    ImGui::SameLine(offset_from_start_x, spacing);
    return 0;
}

static int NewLine(lua_State* L) {
    ImGui::NewLine();
    return 0;
}

static int Spacing(lua_State* L) {
    ImGui::Spacing();
    return 0;
}

static int Dummy(lua_State* L) {
    auto size = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    ImGui::Dummy(size);
    return 0;
}

static int Indent(lua_State* L) {
    ImGui::Indent();
    return 0;
}

static int IndentEx(lua_State* L) {
    auto indent_w = (float)luaL_optnumber(L, 1, 0.0f);
    ImGui::Indent(indent_w);
    return 0;
}

static int Unindent(lua_State* L) {
    ImGui::Unindent();
    return 0;
}

static int UnindentEx(lua_State* L) {
    auto indent_w = (float)luaL_optnumber(L, 1, 0.0f);
    ImGui::Unindent(indent_w);
    return 0;
}

static int BeginGroup(lua_State* L) {
    ImGui::BeginGroup();
    return 0;
}

static int EndGroup(lua_State* L) {
    ImGui::EndGroup();
    return 0;
}

static int AlignTextToFramePadding(lua_State* L) {
    ImGui::AlignTextToFramePadding();
    return 0;
}

static int GetTextLineHeight(lua_State* L) {
    auto&& _retval = ImGui::GetTextLineHeight();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetTextLineHeightWithSpacing(lua_State* L) {
    auto&& _retval = ImGui::GetTextLineHeightWithSpacing();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetFrameHeight(lua_State* L) {
    auto&& _retval = ImGui::GetFrameHeight();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetFrameHeightWithSpacing(lua_State* L) {
    auto&& _retval = ImGui::GetFrameHeightWithSpacing();
    lua_pushnumber(L, _retval);
    return 1;
}

static int PushID(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    ImGui::PushID(str_id);
    return 0;
}

static int PushIDStr(lua_State* L) {
    auto str_id_begin = luaL_checkstring(L, 1);
    auto str_id_end = luaL_checkstring(L, 2);
    ImGui::PushID(str_id_begin, str_id_end);
    return 0;
}

static int PushIDPtr(lua_State* L) {
    auto ptr_id = lua_touserdata(L, 1);
    ImGui::PushID(ptr_id);
    return 0;
}

static int PushIDInt(lua_State* L) {
    auto int_id = (int)luaL_checkinteger(L, 1);
    ImGui::PushID(int_id);
    return 0;
}

static int PopID(lua_State* L) {
    ImGui::PopID();
    return 0;
}

static int GetID(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::GetID(str_id);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetIDStr(lua_State* L) {
    auto str_id_begin = luaL_checkstring(L, 1);
    auto str_id_end = luaL_checkstring(L, 2);
    auto&& _retval = ImGui::GetID(str_id_begin, str_id_end);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetIDPtr(lua_State* L) {
    auto ptr_id = lua_touserdata(L, 1);
    auto&& _retval = ImGui::GetID(ptr_id);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetIDInt(lua_State* L) {
    auto int_id = (int)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::GetID(int_id);
    lua_pushinteger(L, _retval);
    return 1;
}

static int Text(lua_State* L) {
    const char* fmt = util::format(L, 1);
    ImGui::Text("%s", fmt);
    return 0;
}

static int TextColored(lua_State* L) {
    auto col = ImVec4{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    const char* fmt = util::format(L, 5);
    ImGui::TextColored(col, "%s", fmt);
    return 0;
}

static int TextDisabled(lua_State* L) {
    const char* fmt = util::format(L, 1);
    ImGui::TextDisabled("%s", fmt);
    return 0;
}

static int TextWrapped(lua_State* L) {
    const char* fmt = util::format(L, 1);
    ImGui::TextWrapped("%s", fmt);
    return 0;
}

static int LabelText(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    const char* fmt = util::format(L, 2);
    ImGui::LabelText(label, "%s", fmt);
    return 0;
}

static int BulletText(lua_State* L) {
    const char* fmt = util::format(L, 1);
    ImGui::BulletText("%s", fmt);
    return 0;
}

static int SeparatorText(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    ImGui::SeparatorText(label);
    return 0;
}

static int Button(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::Button(label);
    lua_pushboolean(L, _retval);
    return 1;
}

static int ButtonEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_optnumber(L, 2, 0),
            (float)luaL_optnumber(L, 3, 0),
    };
    auto&& _retval = ImGui::Button(label, size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SmallButton(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::SmallButton(label);
    lua_pushboolean(L, _retval);
    return 1;
}

static int InvisibleButton(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    auto flags = (ImGuiButtonFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiButtonFlags_None));
    auto&& _retval = ImGui::InvisibleButton(str_id, size, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int ArrowButton(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto dir = (ImGuiDir)luaL_checkinteger(L, 2);
    auto&& _retval = ImGui::ArrowButton(str_id, dir);
    lua_pushboolean(L, _retval);
    return 1;
}

static int Checkbox(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    bool v[] = {
            util::field_toboolean(L, 2, 1),
    };
    auto&& _retval = ImGui::Checkbox(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushboolean(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int CheckboxFlagsIntPtr(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _flags_index = 2;
    int flags[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto flags_value = (int)luaL_checkinteger(L, 3);
    auto&& _retval = ImGui::CheckboxFlags(label, flags, flags_value);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, flags[0]);
        lua_seti(L, _flags_index, 1);
    };
    return 1;
}

static int CheckboxFlagsUintPtr(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _flags_index = 2;
    unsigned int flags[] = {
            (unsigned int)util::field_tointeger(L, 2, 1),
    };
    auto flags_value = (unsigned int)luaL_checkinteger(L, 3);
    auto&& _retval = ImGui::CheckboxFlags(label, flags, flags_value);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, flags[0]);
        lua_seti(L, _flags_index, 1);
    };
    return 1;
}

static int RadioButton(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto active = !!lua_toboolean(L, 2);
    auto&& _retval = ImGui::RadioButton(label, active);
    lua_pushboolean(L, _retval);
    return 1;
}

static int RadioButtonIntPtr(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto v_button = (int)luaL_checkinteger(L, 3);
    auto&& _retval = ImGui::RadioButton(label, v, v_button);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int ProgressBar(lua_State* L) {
    auto fraction = (float)luaL_checknumber(L, 1);
    auto size_arg = ImVec2{
            (float)luaL_optnumber(L, 2, -FLT_MIN),
            (float)luaL_optnumber(L, 3, 0),
    };
    auto overlay = luaL_optstring(L, 4, NULL);
    ImGui::ProgressBar(fraction, size_arg, overlay);
    return 0;
}

static int Bullet(lua_State* L) {
    ImGui::Bullet();
    return 0;
}

static int TextLink(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::TextLink(label);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TextLinkOpenURL(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    ImGui::TextLinkOpenURL(label);
    return 0;
}

static int TextLinkOpenURLEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto url = luaL_optstring(L, 2, NULL);
    ImGui::TextLinkOpenURL(label, url);
    return 0;
}

static int Image(lua_State* L) {
    auto user_texture_id = util::get_texture_id(L, 1);
    auto image_size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    ImGui::Image(user_texture_id, image_size);
    return 0;
}

static int ImageEx(lua_State* L) {
    auto user_texture_id = util::get_texture_id(L, 1);
    auto image_size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    auto uv0 = ImVec2{
            (float)luaL_optnumber(L, 4, 0),
            (float)luaL_optnumber(L, 5, 0),
    };
    auto uv1 = ImVec2{
            (float)luaL_optnumber(L, 6, 1),
            (float)luaL_optnumber(L, 7, 1),
    };
    auto tint_col = ImVec4{
            (float)luaL_optnumber(L, 8, 1),
            (float)luaL_optnumber(L, 9, 1),
            (float)luaL_optnumber(L, 10, 1),
            (float)luaL_optnumber(L, 11, 1),
    };
    auto border_col = ImVec4{
            (float)luaL_optnumber(L, 12, 0),
            (float)luaL_optnumber(L, 13, 0),
            (float)luaL_optnumber(L, 14, 0),
            (float)luaL_optnumber(L, 15, 0),
    };
    ImGui::Image(user_texture_id, image_size, uv0, uv1, tint_col, border_col);
    return 0;
}

static int ImageButton(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto user_texture_id = util::get_texture_id(L, 2);
    auto image_size = ImVec2{
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto&& _retval = ImGui::ImageButton(str_id, user_texture_id, image_size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int ImageButtonEx(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto user_texture_id = util::get_texture_id(L, 2);
    auto image_size = ImVec2{
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto uv0 = ImVec2{
            (float)luaL_optnumber(L, 5, 0),
            (float)luaL_optnumber(L, 6, 0),
    };
    auto uv1 = ImVec2{
            (float)luaL_optnumber(L, 7, 1),
            (float)luaL_optnumber(L, 8, 1),
    };
    auto bg_col = ImVec4{
            (float)luaL_optnumber(L, 9, 0),
            (float)luaL_optnumber(L, 10, 0),
            (float)luaL_optnumber(L, 11, 0),
            (float)luaL_optnumber(L, 12, 0),
    };
    auto tint_col = ImVec4{
            (float)luaL_optnumber(L, 13, 1),
            (float)luaL_optnumber(L, 14, 1),
            (float)luaL_optnumber(L, 15, 1),
            (float)luaL_optnumber(L, 16, 1),
    };
    auto&& _retval = ImGui::ImageButton(str_id, user_texture_id, image_size, uv0, uv1, bg_col, tint_col);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginCombo(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto preview_value = luaL_checkstring(L, 2);
    auto flags = (ImGuiComboFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiComboFlags_None));
    auto&& _retval = ImGui::BeginCombo(label, preview_value, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndCombo(lua_State* L) {
    ImGui::EndCombo();
    return 0;
}

static int Combo(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _current_item_index = 2;
    int current_item[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto items_separated_by_zeros = luaL_checkstring(L, 3);
    auto&& _retval = ImGui::Combo(label, current_item, items_separated_by_zeros);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, current_item[0]);
        lua_seti(L, _current_item_index, 1);
    };
    return 1;
}

static int ComboEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _current_item_index = 2;
    int current_item[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto items_separated_by_zeros = luaL_checkstring(L, 3);
    auto popup_max_height_in_items = (int)luaL_optinteger(L, 4, -1);
    auto&& _retval = ImGui::Combo(label, current_item, items_separated_by_zeros, popup_max_height_in_items);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, current_item[0]);
        lua_seti(L, _current_item_index, 1);
    };
    return 1;
}

static int DragFloat(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto&& _retval = ImGui::DragFloat(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int DragFloatEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (float)luaL_optnumber(L, 4, 0.0f);
    auto v_max = (float)luaL_optnumber(L, 5, 0.0f);
    auto format = luaL_optstring(L, 6, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int DragFloat2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
    };
    auto&& _retval = ImGui::DragFloat2(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int DragFloat2Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (float)luaL_optnumber(L, 4, 0.0f);
    auto v_max = (float)luaL_optnumber(L, 5, 0.0f);
    auto format = luaL_optstring(L, 6, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragFloat2(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int DragFloat3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto&& _retval = ImGui::DragFloat3(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int DragFloat3Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (float)luaL_optnumber(L, 4, 0.0f);
    auto v_max = (float)luaL_optnumber(L, 5, 0.0f);
    auto format = luaL_optstring(L, 6, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int DragFloat4(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
            (float)util::field_tonumber(L, 2, 4),
    };
    auto&& _retval = ImGui::DragFloat4(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushnumber(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int DragFloat4Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
            (float)util::field_tonumber(L, 2, 4),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (float)luaL_optnumber(L, 4, 0.0f);
    auto v_max = (float)luaL_optnumber(L, 5, 0.0f);
    auto format = luaL_optstring(L, 6, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragFloat4(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushnumber(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int DragFloatRange2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_current_min_index = 2;
    float v_current_min[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    luaL_checktype(L, 3, LUA_TTABLE);
    int _v_current_max_index = 3;
    float v_current_max[] = {
            (float)util::field_tonumber(L, 3, 1),
    };
    auto&& _retval = ImGui::DragFloatRange2(label, v_current_min, v_current_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v_current_min[0]);
        lua_seti(L, _v_current_min_index, 1);
    };
    if (_retval) {
        lua_pushnumber(L, v_current_max[0]);
        lua_seti(L, _v_current_max_index, 1);
    };
    return 1;
}

static int DragFloatRange2Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_current_min_index = 2;
    float v_current_min[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    luaL_checktype(L, 3, LUA_TTABLE);
    int _v_current_max_index = 3;
    float v_current_max[] = {
            (float)util::field_tonumber(L, 3, 1),
    };
    auto v_speed = (float)luaL_optnumber(L, 4, 1.0f);
    auto v_min = (float)luaL_optnumber(L, 5, 0.0f);
    auto v_max = (float)luaL_optnumber(L, 6, 0.0f);
    auto format = luaL_optstring(L, 7, "%.3f");
    auto format_max = luaL_optstring(L, 8, NULL);
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 9, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v_current_min[0]);
        lua_seti(L, _v_current_min_index, 1);
    };
    if (_retval) {
        lua_pushnumber(L, v_current_max[0]);
        lua_seti(L, _v_current_max_index, 1);
    };
    return 1;
}

static int DragInt(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto&& _retval = ImGui::DragInt(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int DragIntEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (int)luaL_optinteger(L, 4, 0);
    auto v_max = (int)luaL_optinteger(L, 5, 0);
    auto format = luaL_optstring(L, 6, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragInt(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int DragInt2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
    };
    auto&& _retval = ImGui::DragInt2(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int DragInt2Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (int)luaL_optinteger(L, 4, 0);
    auto v_max = (int)luaL_optinteger(L, 5, 0);
    auto format = luaL_optstring(L, 6, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragInt2(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int DragInt3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
    };
    auto&& _retval = ImGui::DragInt3(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int DragInt3Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (int)luaL_optinteger(L, 4, 0);
    auto v_max = (int)luaL_optinteger(L, 5, 0);
    auto format = luaL_optstring(L, 6, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragInt3(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int DragInt4(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
            (int)util::field_tointeger(L, 2, 4),
    };
    auto&& _retval = ImGui::DragInt4(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushinteger(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int DragInt4Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
            (int)util::field_tointeger(L, 2, 4),
    };
    auto v_speed = (float)luaL_optnumber(L, 3, 1.0f);
    auto v_min = (int)luaL_optinteger(L, 4, 0);
    auto v_max = (int)luaL_optinteger(L, 5, 0);
    auto format = luaL_optstring(L, 6, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 7, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragInt4(label, v, v_speed, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushinteger(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int DragIntRange2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_current_min_index = 2;
    int v_current_min[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    luaL_checktype(L, 3, LUA_TTABLE);
    int _v_current_max_index = 3;
    int v_current_max[] = {
            (int)util::field_tointeger(L, 3, 1),
    };
    auto&& _retval = ImGui::DragIntRange2(label, v_current_min, v_current_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v_current_min[0]);
        lua_seti(L, _v_current_min_index, 1);
    };
    if (_retval) {
        lua_pushinteger(L, v_current_max[0]);
        lua_seti(L, _v_current_max_index, 1);
    };
    return 1;
}

static int DragIntRange2Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_current_min_index = 2;
    int v_current_min[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    luaL_checktype(L, 3, LUA_TTABLE);
    int _v_current_max_index = 3;
    int v_current_max[] = {
            (int)util::field_tointeger(L, 3, 1),
    };
    auto v_speed = (float)luaL_optnumber(L, 4, 1.0f);
    auto v_min = (int)luaL_optinteger(L, 5, 0);
    auto v_max = (int)luaL_optinteger(L, 6, 0);
    auto format = luaL_optstring(L, 7, "%d");
    auto format_max = luaL_optstring(L, 8, NULL);
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 9, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::DragIntRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v_current_min[0]);
        lua_seti(L, _v_current_min_index, 1);
    };
    if (_retval) {
        lua_pushinteger(L, v_current_max[0]);
        lua_seti(L, _v_current_max_index, 1);
    };
    return 1;
}

static int SliderFloat(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto&& _retval = ImGui::SliderFloat(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int SliderFloatEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto format = luaL_optstring(L, 5, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int SliderFloat2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto&& _retval = ImGui::SliderFloat2(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int SliderFloat2Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto format = luaL_optstring(L, 5, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderFloat2(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int SliderFloat3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto&& _retval = ImGui::SliderFloat3(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int SliderFloat3Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto format = luaL_optstring(L, 5, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderFloat3(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int SliderFloat4(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
            (float)util::field_tonumber(L, 2, 4),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto&& _retval = ImGui::SliderFloat4(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushnumber(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int SliderFloat4Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
            (float)util::field_tonumber(L, 2, 4),
    };
    auto v_min = (float)luaL_checknumber(L, 3);
    auto v_max = (float)luaL_checknumber(L, 4);
    auto format = luaL_optstring(L, 5, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderFloat4(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushnumber(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int SliderAngle(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_rad_index = 2;
    float v_rad[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto&& _retval = ImGui::SliderAngle(label, v_rad);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v_rad[0]);
        lua_seti(L, _v_rad_index, 1);
    };
    return 1;
}

static int SliderAngleEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_rad_index = 2;
    float v_rad[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto v_degrees_min = (float)luaL_optnumber(L, 3, -360.0f);
    auto v_degrees_max = (float)luaL_optnumber(L, 4, +360.0f);
    auto format = luaL_optstring(L, 5, "%.0f deg");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v_rad[0]);
        lua_seti(L, _v_rad_index, 1);
    };
    return 1;
}

static int SliderInt(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto&& _retval = ImGui::SliderInt(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int SliderIntEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto format = luaL_optstring(L, 5, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderInt(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int SliderInt2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto&& _retval = ImGui::SliderInt2(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int SliderInt2Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto format = luaL_optstring(L, 5, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderInt2(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int SliderInt3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto&& _retval = ImGui::SliderInt3(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int SliderInt3Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto format = luaL_optstring(L, 5, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderInt3(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int SliderInt4(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
            (int)util::field_tointeger(L, 2, 4),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto&& _retval = ImGui::SliderInt4(label, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushinteger(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int SliderInt4Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
            (int)util::field_tointeger(L, 2, 4),
    };
    auto v_min = (int)luaL_checkinteger(L, 3);
    auto v_max = (int)luaL_checkinteger(L, 4);
    auto format = luaL_optstring(L, 5, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::SliderInt4(label, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushinteger(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int VSliderFloat(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    luaL_checktype(L, 4, LUA_TTABLE);
    int _v_index = 4;
    float v[] = {
            (float)util::field_tonumber(L, 4, 1),
    };
    auto v_min = (float)luaL_checknumber(L, 5);
    auto v_max = (float)luaL_checknumber(L, 6);
    auto&& _retval = ImGui::VSliderFloat(label, size, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int VSliderFloatEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    luaL_checktype(L, 4, LUA_TTABLE);
    int _v_index = 4;
    float v[] = {
            (float)util::field_tonumber(L, 4, 1),
    };
    auto v_min = (float)luaL_checknumber(L, 5);
    auto v_max = (float)luaL_checknumber(L, 6);
    auto format = luaL_optstring(L, 7, "%.3f");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 8, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::VSliderFloat(label, size, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int VSliderInt(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    luaL_checktype(L, 4, LUA_TTABLE);
    int _v_index = 4;
    int v[] = {
            (int)util::field_tointeger(L, 4, 1),
    };
    auto v_min = (int)luaL_checkinteger(L, 5);
    auto v_max = (int)luaL_checkinteger(L, 6);
    auto&& _retval = ImGui::VSliderInt(label, size, v, v_min, v_max);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int VSliderIntEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
    };
    luaL_checktype(L, 4, LUA_TTABLE);
    int _v_index = 4;
    int v[] = {
            (int)util::field_tointeger(L, 4, 1),
    };
    auto v_min = (int)luaL_checkinteger(L, 5);
    auto v_max = (int)luaL_checkinteger(L, 6);
    auto format = luaL_optstring(L, 7, "%d");
    auto flags = (ImGuiSliderFlags)luaL_optinteger(L, 8, lua_Integer(ImGuiSliderFlags_None));
    auto&& _retval = ImGui::VSliderInt(label, size, v, v_min, v_max, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int InputText(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputText(label, _strbuf->data, _strbuf->size, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int InputTextEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiInputTextFlags_None));
    util::input_context _ctx{L, 4};
    auto _top = lua_gettop(L);
    auto&& _retval = ImGui::InputText(label, _strbuf->data, _strbuf->size, flags, util::input_callback, &_ctx);
    lua_pushboolean(L, _retval);
    if (lua_gettop(L) != _top + 1) {
        lua_pop(L, 1);
        lua_error(L);
    }
    return 1;
}

static int InputTextMultiline(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto&& _retval = ImGui::InputTextMultiline(label, _strbuf->data, _strbuf->size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int InputTextMultilineEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto _strbuf = util::strbuf_get(L, 2);
    auto size = ImVec2{
            (float)luaL_optnumber(L, 3, 0),
            (float)luaL_optnumber(L, 4, 0),
    };
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 5, lua_Integer(ImGuiInputTextFlags_None));
    util::input_context _ctx{L, 6};
    auto _top = lua_gettop(L);
    auto&& _retval = ImGui::InputTextMultiline(label, _strbuf->data, _strbuf->size, size, flags, util::input_callback, &_ctx);
    lua_pushboolean(L, _retval);
    if (lua_gettop(L) != _top + 1) {
        lua_pop(L, 1);
        lua_error(L);
    }
    return 1;
}

static int InputTextWithHint(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto hint = luaL_checkstring(L, 2);
    auto _strbuf = util::strbuf_get(L, 3);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputTextWithHint(label, hint, _strbuf->data, _strbuf->size, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int InputTextWithHintEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto hint = luaL_checkstring(L, 2);
    auto _strbuf = util::strbuf_get(L, 3);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiInputTextFlags_None));
    util::input_context _ctx{L, 5};
    auto _top = lua_gettop(L);
    auto&& _retval = ImGui::InputTextWithHint(label, hint, _strbuf->data, _strbuf->size, flags, util::input_callback, &_ctx);
    lua_pushboolean(L, _retval);
    if (lua_gettop(L) != _top + 1) {
        lua_pop(L, 1);
        lua_error(L);
    }
    return 1;
}

static int InputFloat(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto&& _retval = ImGui::InputFloat(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int InputFloatEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
    };
    auto step = (float)luaL_optnumber(L, 3, 0.0f);
    auto step_fast = (float)luaL_optnumber(L, 4, 0.0f);
    auto format = luaL_optstring(L, 5, "%.3f");
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputFloat(label, v, step, step_fast, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int InputFloat2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
    };
    auto&& _retval = ImGui::InputFloat2(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int InputFloat2Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
    };
    auto format = luaL_optstring(L, 3, "%.3f");
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputFloat2(label, v, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int InputFloat3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto&& _retval = ImGui::InputFloat3(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int InputFloat3Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto format = luaL_optstring(L, 3, "%.3f");
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputFloat3(label, v, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int InputFloat4(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
            (float)util::field_tonumber(L, 2, 4),
    };
    auto&& _retval = ImGui::InputFloat4(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushnumber(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int InputFloat4Ex(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    float v[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
            (float)util::field_tonumber(L, 2, 4),
    };
    auto format = luaL_optstring(L, 3, "%.3f");
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputFloat4(label, v, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushnumber(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushnumber(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushnumber(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int InputInt(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto&& _retval = ImGui::InputInt(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int InputIntEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
    };
    auto step = (int)luaL_optinteger(L, 3, 1);
    auto step_fast = (int)luaL_optinteger(L, 4, 100);
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 5, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputInt(label, v, step, step_fast, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int InputInt2(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
    };
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputInt2(label, v, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
    };
    return 1;
}

static int InputInt3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
    };
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputInt3(label, v, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
    };
    return 1;
}

static int InputInt4(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    int v[] = {
            (int)util::field_tointeger(L, 2, 1),
            (int)util::field_tointeger(L, 2, 2),
            (int)util::field_tointeger(L, 2, 3),
            (int)util::field_tointeger(L, 2, 4),
    };
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputInt4(label, v, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushinteger(L, v[0]);
        lua_seti(L, _v_index, 1);
        lua_pushinteger(L, v[1]);
        lua_seti(L, _v_index, 2);
        lua_pushinteger(L, v[2]);
        lua_seti(L, _v_index, 3);
        lua_pushinteger(L, v[3]);
        lua_seti(L, _v_index, 4);
    };
    return 1;
}

static int InputDouble(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    double v[] = {
            (double)util::field_tonumber(L, 2, 1),
    };
    auto&& _retval = ImGui::InputDouble(label, v);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int InputDoubleEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _v_index = 2;
    double v[] = {
            (double)util::field_tonumber(L, 2, 1),
    };
    auto step = (double)luaL_optnumber(L, 3, 0.0);
    auto step_fast = (double)luaL_optnumber(L, 4, 0.0);
    auto format = luaL_optstring(L, 5, "%.6f");
    auto flags = (ImGuiInputTextFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiInputTextFlags_None));
    auto&& _retval = ImGui::InputDouble(label, v, step, step_fast, format, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, v[0]);
        lua_seti(L, _v_index, 1);
    };
    return 1;
}

static int ColorEdit3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _col_index = 2;
    float col[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto flags = (ImGuiColorEditFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiColorEditFlags_None));
    auto&& _retval = ImGui::ColorEdit3(label, col, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, col[0]);
        lua_seti(L, _col_index, 1);
        lua_pushnumber(L, col[1]);
        lua_seti(L, _col_index, 2);
        lua_pushnumber(L, col[2]);
        lua_seti(L, _col_index, 3);
    };
    return 1;
}

static int ColorEdit4(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _col_index = 2;
    float col[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
            (float)util::field_tonumber(L, 2, 4),
    };
    auto flags = (ImGuiColorEditFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiColorEditFlags_None));
    auto&& _retval = ImGui::ColorEdit4(label, col, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, col[0]);
        lua_seti(L, _col_index, 1);
        lua_pushnumber(L, col[1]);
        lua_seti(L, _col_index, 2);
        lua_pushnumber(L, col[2]);
        lua_seti(L, _col_index, 3);
        lua_pushnumber(L, col[3]);
        lua_seti(L, _col_index, 4);
    };
    return 1;
}

static int ColorPicker3(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _col_index = 2;
    float col[] = {
            (float)util::field_tonumber(L, 2, 1),
            (float)util::field_tonumber(L, 2, 2),
            (float)util::field_tonumber(L, 2, 3),
    };
    auto flags = (ImGuiColorEditFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiColorEditFlags_None));
    auto&& _retval = ImGui::ColorPicker3(label, col, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushnumber(L, col[0]);
        lua_seti(L, _col_index, 1);
        lua_pushnumber(L, col[1]);
        lua_seti(L, _col_index, 2);
        lua_pushnumber(L, col[2]);
        lua_seti(L, _col_index, 3);
    };
    return 1;
}

static int ColorButton(lua_State* L) {
    auto desc_id = luaL_checkstring(L, 1);
    auto col = ImVec4{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
            (float)luaL_checknumber(L, 5),
    };
    auto flags = (ImGuiColorEditFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiColorEditFlags_None));
    auto&& _retval = ImGui::ColorButton(desc_id, col, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int ColorButtonEx(lua_State* L) {
    auto desc_id = luaL_checkstring(L, 1);
    auto col = ImVec4{
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
            (float)luaL_checknumber(L, 5),
    };
    auto flags = (ImGuiColorEditFlags)luaL_optinteger(L, 6, lua_Integer(ImGuiColorEditFlags_None));
    auto size = ImVec2{
            (float)luaL_optnumber(L, 7, 0),
            (float)luaL_optnumber(L, 8, 0),
    };
    auto&& _retval = ImGui::ColorButton(desc_id, col, flags, size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SetColorEditOptions(lua_State* L) {
    auto flags = (ImGuiColorEditFlags)luaL_checkinteger(L, 1);
    ImGui::SetColorEditOptions(flags);
    return 0;
}

static int TreeNode(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::TreeNode(label);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TreeNodeStr(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    const char* fmt = util::format(L, 2);
    auto&& _retval = ImGui::TreeNode(str_id, "%s", fmt);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TreeNodePtr(lua_State* L) {
    auto ptr_id = lua_touserdata(L, 1);
    const char* fmt = util::format(L, 2);
    auto&& _retval = ImGui::TreeNode(ptr_id, "%s", fmt);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TreeNodeEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto flags = (ImGuiTreeNodeFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiTreeNodeFlags_None));
    auto&& _retval = ImGui::TreeNodeEx(label, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TreeNodeExStr(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
    const char* fmt = util::format(L, 3);
    auto&& _retval = ImGui::TreeNodeEx(str_id, flags, "%s", fmt);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TreeNodeExPtr(lua_State* L) {
    auto ptr_id = lua_touserdata(L, 1);
    auto flags = (ImGuiTreeNodeFlags)luaL_checkinteger(L, 2);
    const char* fmt = util::format(L, 3);
    auto&& _retval = ImGui::TreeNodeEx(ptr_id, flags, "%s", fmt);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TreePush(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    ImGui::TreePush(str_id);
    return 0;
}

static int TreePushPtr(lua_State* L) {
    auto ptr_id = lua_touserdata(L, 1);
    ImGui::TreePush(ptr_id);
    return 0;
}

static int TreePop(lua_State* L) {
    ImGui::TreePop();
    return 0;
}

static int GetTreeNodeToLabelSpacing(lua_State* L) {
    auto&& _retval = ImGui::GetTreeNodeToLabelSpacing();
    lua_pushnumber(L, _retval);
    return 1;
}

static int CollapsingHeader(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto flags = (ImGuiTreeNodeFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiTreeNodeFlags_None));
    auto&& _retval = ImGui::CollapsingHeader(label, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int CollapsingHeaderBoolPtr(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _p_visible_index = 2;
    bool p_visible[] = {
            util::field_toboolean(L, 2, 1),
    };
    auto flags = (ImGuiTreeNodeFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiTreeNodeFlags_None));
    auto&& _retval = ImGui::CollapsingHeader(label, p_visible, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushboolean(L, p_visible[0]);
        lua_seti(L, _p_visible_index, 1);
    };
    return 1;
}

static int SetNextItemOpen(lua_State* L) {
    auto is_open = !!lua_toboolean(L, 1);
    auto cond = (ImGuiCond)luaL_optinteger(L, 2, lua_Integer(ImGuiCond_None));
    ImGui::SetNextItemOpen(is_open, cond);
    return 0;
}

static int SetNextItemStorageID(lua_State* L) {
    auto storage_id = (ImGuiID)luaL_checkinteger(L, 1);
    ImGui::SetNextItemStorageID(storage_id);
    return 0;
}

static int Selectable(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::Selectable(label);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SelectableEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto selected = lua_isnoneornil(L, 2) ? false : !!lua_toboolean(L, 2);
    auto flags = (ImGuiSelectableFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiSelectableFlags_None));
    auto size = ImVec2{
            (float)luaL_optnumber(L, 4, 0),
            (float)luaL_optnumber(L, 5, 0),
    };
    auto&& _retval = ImGui::Selectable(label, selected, flags, size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SelectableBoolPtr(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _p_selected_index = 2;
    bool p_selected[] = {
            util::field_toboolean(L, 2, 1),
    };
    auto flags = (ImGuiSelectableFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiSelectableFlags_None));
    auto&& _retval = ImGui::Selectable(label, p_selected, flags);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushboolean(L, p_selected[0]);
        lua_seti(L, _p_selected_index, 1);
    };
    return 1;
}

static int SelectableBoolPtrEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int _p_selected_index = 2;
    bool p_selected[] = {
            util::field_toboolean(L, 2, 1),
    };
    auto flags = (ImGuiSelectableFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiSelectableFlags_None));
    auto size = ImVec2{
            (float)luaL_optnumber(L, 4, 0),
            (float)luaL_optnumber(L, 5, 0),
    };
    auto&& _retval = ImGui::Selectable(label, p_selected, flags, size);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushboolean(L, p_selected[0]);
        lua_seti(L, _p_selected_index, 1);
    };
    return 1;
}

static int BeginMultiSelect(lua_State* L) {
    auto flags = (ImGuiMultiSelectFlags)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::BeginMultiSelect(flags);
    if (_retval != NULL) {
        wrap_ImGuiMultiSelectIO::pointer(L, *_retval);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int BeginMultiSelectEx(lua_State* L) {
    auto flags = (ImGuiMultiSelectFlags)luaL_checkinteger(L, 1);
    auto selection_size = (int)luaL_optinteger(L, 2, -1);
    auto items_count = (int)luaL_optinteger(L, 3, -1);
    auto&& _retval = ImGui::BeginMultiSelect(flags, selection_size, items_count);
    if (_retval != NULL) {
        wrap_ImGuiMultiSelectIO::pointer(L, *_retval);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int EndMultiSelect(lua_State* L) {
    auto&& _retval = ImGui::EndMultiSelect();
    if (_retval != NULL) {
        wrap_ImGuiMultiSelectIO::pointer(L, *_retval);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int IsItemToggledSelection(lua_State* L) {
    auto&& _retval = ImGui::IsItemToggledSelection();
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginListBox(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto size = ImVec2{
            (float)luaL_optnumber(L, 2, 0),
            (float)luaL_optnumber(L, 3, 0),
    };
    auto&& _retval = ImGui::BeginListBox(label, size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndListBox(lua_State* L) {
    ImGui::EndListBox();
    return 0;
}

static int BeginMenuBar(lua_State* L) {
    auto&& _retval = ImGui::BeginMenuBar();
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndMenuBar(lua_State* L) {
    ImGui::EndMenuBar();
    return 0;
}

static int BeginMainMenuBar(lua_State* L) {
    auto&& _retval = ImGui::BeginMainMenuBar();
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndMainMenuBar(lua_State* L) {
    ImGui::EndMainMenuBar();
    return 0;
}

static int BeginMenu(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::BeginMenu(label);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginMenuEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto enabled = lua_isnoneornil(L, 2) ? true : !!lua_toboolean(L, 2);
    auto&& _retval = ImGui::BeginMenu(label, enabled);
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndMenu(lua_State* L) {
    ImGui::EndMenu();
    return 0;
}

static int MenuItem(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::MenuItem(label);
    lua_pushboolean(L, _retval);
    return 1;
}

static int MenuItemEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto shortcut = luaL_optstring(L, 2, NULL);
    auto selected = lua_isnoneornil(L, 3) ? false : !!lua_toboolean(L, 3);
    auto enabled = lua_isnoneornil(L, 4) ? true : !!lua_toboolean(L, 4);
    auto&& _retval = ImGui::MenuItem(label, shortcut, selected, enabled);
    lua_pushboolean(L, _retval);
    return 1;
}

static int MenuItemBoolPtr(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto shortcut = luaL_checkstring(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);
    int _p_selected_index = 3;
    bool p_selected[] = {
            util::field_toboolean(L, 3, 1),
    };
    auto enabled = lua_isnoneornil(L, 4) ? true : !!lua_toboolean(L, 4);
    auto&& _retval = ImGui::MenuItem(label, shortcut, p_selected, enabled);
    lua_pushboolean(L, _retval);
    if (_retval) {
        lua_pushboolean(L, p_selected[0]);
        lua_seti(L, _p_selected_index, 1);
    };
    return 1;
}

static int BeginTooltip(lua_State* L) {
    auto&& _retval = ImGui::BeginTooltip();
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndTooltip(lua_State* L) {
    ImGui::EndTooltip();
    return 0;
}

static int SetTooltip(lua_State* L) {
    const char* fmt = util::format(L, 1);
    ImGui::SetTooltip("%s", fmt);
    return 0;
}

static int BeginItemTooltip(lua_State* L) {
    auto&& _retval = ImGui::BeginItemTooltip();
    lua_pushboolean(L, _retval);
    return 1;
}

static int SetItemTooltip(lua_State* L) {
    const char* fmt = util::format(L, 1);
    ImGui::SetItemTooltip("%s", fmt);
    return 0;
}

static int BeginPopup(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto flags = (ImGuiWindowFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiWindowFlags_None));
    auto&& _retval = ImGui::BeginPopup(str_id, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginPopupModal(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    bool has_p_open = !lua_isnil(L, 2);
    bool p_open = true;
    auto flags = (ImGuiWindowFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiWindowFlags_None));
    auto&& _retval = ImGui::BeginPopupModal(name, (has_p_open ? &p_open : NULL), flags);
    lua_pushboolean(L, _retval);
    if (has_p_open) {
        lua_pushboolean(L, p_open);
    } else {
        lua_pushnil(L);
    }
    return 2;
}

static int EndPopup(lua_State* L) {
    ImGui::EndPopup();
    return 0;
}

static int OpenPopup(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto popup_flags = (ImGuiPopupFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiPopupFlags_None));
    ImGui::OpenPopup(str_id, popup_flags);
    return 0;
}

static int OpenPopupID(lua_State* L) {
    auto id = (ImGuiID)luaL_checkinteger(L, 1);
    auto popup_flags = (ImGuiPopupFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiPopupFlags_None));
    ImGui::OpenPopup(id, popup_flags);
    return 0;
}

static int OpenPopupOnItemClick(lua_State* L) {
    auto str_id = luaL_optstring(L, 1, NULL);
    auto popup_flags = (ImGuiPopupFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiPopupFlags_MouseButtonRight));
    ImGui::OpenPopupOnItemClick(str_id, popup_flags);
    return 0;
}

static int CloseCurrentPopup(lua_State* L) {
    ImGui::CloseCurrentPopup();
    return 0;
}

static int BeginPopupContextItem(lua_State* L) {
    auto&& _retval = ImGui::BeginPopupContextItem();
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginPopupContextItemEx(lua_State* L) {
    auto str_id = luaL_optstring(L, 1, NULL);
    auto popup_flags = (ImGuiPopupFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiPopupFlags_MouseButtonRight));
    auto&& _retval = ImGui::BeginPopupContextItem(str_id, popup_flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginPopupContextWindow(lua_State* L) {
    auto&& _retval = ImGui::BeginPopupContextWindow();
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginPopupContextWindowEx(lua_State* L) {
    auto str_id = luaL_optstring(L, 1, NULL);
    auto popup_flags = (ImGuiPopupFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiPopupFlags_MouseButtonRight));
    auto&& _retval = ImGui::BeginPopupContextWindow(str_id, popup_flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginPopupContextVoid(lua_State* L) {
    auto&& _retval = ImGui::BeginPopupContextVoid();
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginPopupContextVoidEx(lua_State* L) {
    auto str_id = luaL_optstring(L, 1, NULL);
    auto popup_flags = (ImGuiPopupFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiPopupFlags_MouseButtonRight));
    auto&& _retval = ImGui::BeginPopupContextVoid(str_id, popup_flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsPopupOpen(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto flags = (ImGuiPopupFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiPopupFlags_None));
    auto&& _retval = ImGui::IsPopupOpen(str_id, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginTable(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto columns = (int)luaL_checkinteger(L, 2);
    auto flags = (ImGuiTableFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiTableFlags_None));
    auto&& _retval = ImGui::BeginTable(str_id, columns, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginTableEx(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto columns = (int)luaL_checkinteger(L, 2);
    auto flags = (ImGuiTableFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiTableFlags_None));
    auto outer_size = ImVec2{
            (float)luaL_optnumber(L, 4, 0.0f),
            (float)luaL_optnumber(L, 5, 0.0f),
    };
    auto inner_width = (float)luaL_optnumber(L, 6, 0.0f);
    auto&& _retval = ImGui::BeginTable(str_id, columns, flags, outer_size, inner_width);
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndTable(lua_State* L) {
    ImGui::EndTable();
    return 0;
}

static int TableNextRow(lua_State* L) {
    ImGui::TableNextRow();
    return 0;
}

static int TableNextRowEx(lua_State* L) {
    auto row_flags = (ImGuiTableRowFlags)luaL_optinteger(L, 1, lua_Integer(ImGuiTableRowFlags_None));
    auto min_row_height = (float)luaL_optnumber(L, 2, 0.0f);
    ImGui::TableNextRow(row_flags, min_row_height);
    return 0;
}

static int TableNextColumn(lua_State* L) {
    auto&& _retval = ImGui::TableNextColumn();
    lua_pushboolean(L, _retval);
    return 1;
}

static int TableSetColumnIndex(lua_State* L) {
    auto column_n = (int)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::TableSetColumnIndex(column_n);
    lua_pushboolean(L, _retval);
    return 1;
}

static int TableSetupColumn(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto flags = (ImGuiTableColumnFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiTableColumnFlags_None));
    ImGui::TableSetupColumn(label, flags);
    return 0;
}

static int TableSetupColumnEx(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto flags = (ImGuiTableColumnFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiTableColumnFlags_None));
    auto init_width_or_weight = (float)luaL_optnumber(L, 3, 0.0f);
    auto user_id = (ImGuiID)luaL_optinteger(L, 4, 0);
    ImGui::TableSetupColumn(label, flags, init_width_or_weight, user_id);
    return 0;
}

static int TableSetupScrollFreeze(lua_State* L) {
    auto cols = (int)luaL_checkinteger(L, 1);
    auto rows = (int)luaL_checkinteger(L, 2);
    ImGui::TableSetupScrollFreeze(cols, rows);
    return 0;
}

static int TableHeader(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    ImGui::TableHeader(label);
    return 0;
}

static int TableHeadersRow(lua_State* L) {
    ImGui::TableHeadersRow();
    return 0;
}

static int TableAngledHeadersRow(lua_State* L) {
    ImGui::TableAngledHeadersRow();
    return 0;
}

static int TableGetColumnCount(lua_State* L) {
    auto&& _retval = ImGui::TableGetColumnCount();
    lua_pushinteger(L, _retval);
    return 1;
}

static int TableGetColumnIndex(lua_State* L) {
    auto&& _retval = ImGui::TableGetColumnIndex();
    lua_pushinteger(L, _retval);
    return 1;
}

static int TableGetRowIndex(lua_State* L) {
    auto&& _retval = ImGui::TableGetRowIndex();
    lua_pushinteger(L, _retval);
    return 1;
}

static int TableGetColumnName(lua_State* L) {
    auto column_n = (int)luaL_optinteger(L, 1, -1);
    auto&& _retval = ImGui::TableGetColumnName(column_n);
    lua_pushstring(L, _retval);
    return 1;
}

static int TableGetColumnFlags(lua_State* L) {
    auto column_n = (int)luaL_optinteger(L, 1, -1);
    auto&& _retval = ImGui::TableGetColumnFlags(column_n);
    lua_pushinteger(L, _retval);
    return 1;
}

static int TableSetColumnEnabled(lua_State* L) {
    auto column_n = (int)luaL_checkinteger(L, 1);
    auto v = !!lua_toboolean(L, 2);
    ImGui::TableSetColumnEnabled(column_n, v);
    return 0;
}

static int TableGetHoveredColumn(lua_State* L) {
    auto&& _retval = ImGui::TableGetHoveredColumn();
    lua_pushinteger(L, _retval);
    return 1;
}

static int TableSetBgColor(lua_State* L) {
    auto target = (ImGuiTableBgTarget)luaL_checkinteger(L, 1);
    auto color = (ImU32)luaL_checkinteger(L, 2);
    auto column_n = (int)luaL_optinteger(L, 3, -1);
    ImGui::TableSetBgColor(target, color, column_n);
    return 0;
}

static int BeginTabBar(lua_State* L) {
    auto str_id = luaL_checkstring(L, 1);
    auto flags = (ImGuiTabBarFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiTabBarFlags_None));
    auto&& _retval = ImGui::BeginTabBar(str_id, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndTabBar(lua_State* L) {
    ImGui::EndTabBar();
    return 0;
}

static int BeginTabItem(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    bool has_p_open = !lua_isnil(L, 2);
    bool p_open = true;
    auto flags = (ImGuiTabItemFlags)luaL_optinteger(L, 3, lua_Integer(ImGuiTabItemFlags_None));
    auto&& _retval = ImGui::BeginTabItem(label, (has_p_open ? &p_open : NULL), flags);
    lua_pushboolean(L, _retval);
    if (has_p_open) {
        lua_pushboolean(L, p_open);
    } else {
        lua_pushnil(L);
    }
    return 2;
}

static int EndTabItem(lua_State* L) {
    ImGui::EndTabItem();
    return 0;
}

static int TabItemButton(lua_State* L) {
    auto label = luaL_checkstring(L, 1);
    auto flags = (ImGuiTabItemFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiTabItemFlags_None));
    auto&& _retval = ImGui::TabItemButton(label, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SetTabItemClosed(lua_State* L) {
    auto tab_or_docked_window_label = luaL_checkstring(L, 1);
    ImGui::SetTabItemClosed(tab_or_docked_window_label);
    return 0;
}

static int DockSpace(lua_State* L) {
    auto dockspace_id = (ImGuiID)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::DockSpace(dockspace_id);
    lua_pushinteger(L, _retval);
    return 1;
}

static int DockSpaceEx(lua_State* L) {
    auto dockspace_id = (ImGuiID)luaL_checkinteger(L, 1);
    auto size = ImVec2{
            (float)luaL_optnumber(L, 2, 0),
            (float)luaL_optnumber(L, 3, 0),
    };
    auto flags = (ImGuiDockNodeFlags)luaL_optinteger(L, 4, lua_Integer(ImGuiDockNodeFlags_None));
    auto window_class = lua_isnoneornil(L, 5) ? NULL : *(const ImGuiWindowClass**)lua_touserdata(L, 5);
    auto&& _retval = ImGui::DockSpace(dockspace_id, size, flags, window_class);
    lua_pushinteger(L, _retval);
    return 1;
}

static int DockSpaceOverViewport(lua_State* L) {
    auto&& _retval = ImGui::DockSpaceOverViewport();
    lua_pushinteger(L, _retval);
    return 1;
}

static int SetNextWindowDockID(lua_State* L) {
    auto dock_id = (ImGuiID)luaL_checkinteger(L, 1);
    auto cond = (ImGuiCond)luaL_optinteger(L, 2, lua_Integer(ImGuiCond_None));
    ImGui::SetNextWindowDockID(dock_id, cond);
    return 0;
}

static int SetNextWindowClass(lua_State* L) {
    auto window_class = *(const ImGuiWindowClass**)lua_touserdata(L, 1);
    ImGui::SetNextWindowClass(window_class);
    return 0;
}

static int GetWindowDockID(lua_State* L) {
    auto&& _retval = ImGui::GetWindowDockID();
    lua_pushinteger(L, _retval);
    return 1;
}

static int IsWindowDocked(lua_State* L) {
    auto&& _retval = ImGui::IsWindowDocked();
    lua_pushboolean(L, _retval);
    return 1;
}

static int BeginDragDropSource(lua_State* L) {
    auto flags = (ImGuiDragDropFlags)luaL_optinteger(L, 1, lua_Integer(ImGuiDragDropFlags_None));
    auto&& _retval = ImGui::BeginDragDropSource(flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SetDragDropPayload(lua_State* L) {
    auto type = luaL_checkstring(L, 1);
    size_t sz = 0;
    auto data = luaL_checklstring(L, 2, &sz);
    auto cond = (ImGuiCond)luaL_optinteger(L, 3, lua_Integer(ImGuiCond_None));
    auto&& _retval = ImGui::SetDragDropPayload(type, data, sz, cond);
    lua_pushboolean(L, _retval);
    return 1;
}

static int EndDragDropSource(lua_State* L) {
    ImGui::EndDragDropSource();
    return 0;
}

static int BeginDragDropTarget(lua_State* L) {
    auto&& _retval = ImGui::BeginDragDropTarget();
    lua_pushboolean(L, _retval);
    return 1;
}

static int AcceptDragDropPayload(lua_State* L) {
    auto type = luaL_checkstring(L, 1);
    auto flags = (ImGuiDragDropFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiDragDropFlags_None));
    auto&& _retval = ImGui::AcceptDragDropPayload(type, flags);
    if (_retval != NULL) {
        lua_pushlstring(L, (const char*)_retval->Data, _retval->DataSize);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int EndDragDropTarget(lua_State* L) {
    ImGui::EndDragDropTarget();
    return 0;
}

static int GetDragDropPayload(lua_State* L) {
    auto&& _retval = ImGui::GetDragDropPayload();
    if (_retval != NULL) {
        lua_pushlstring(L, (const char*)_retval->Data, _retval->DataSize);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int BeginDisabled(lua_State* L) {
    auto disabled = lua_isnoneornil(L, 1) ? true : !!lua_toboolean(L, 1);
    ImGui::BeginDisabled(disabled);
    return 0;
}

static int EndDisabled(lua_State* L) {
    ImGui::EndDisabled();
    return 0;
}

static int PushClipRect(lua_State* L) {
    auto clip_rect_min = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto clip_rect_max = ImVec2{
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto intersect_with_current_clip_rect = !!lua_toboolean(L, 5);
    ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}

static int PopClipRect(lua_State* L) {
    ImGui::PopClipRect();
    return 0;
}

static int SetItemDefaultFocus(lua_State* L) {
    ImGui::SetItemDefaultFocus();
    return 0;
}

static int SetKeyboardFocusHere(lua_State* L) {
    ImGui::SetKeyboardFocusHere();
    return 0;
}

static int SetKeyboardFocusHereEx(lua_State* L) {
    auto offset = (int)luaL_optinteger(L, 1, 0);
    ImGui::SetKeyboardFocusHere(offset);
    return 0;
}

static int SetNextItemAllowOverlap(lua_State* L) {
    ImGui::SetNextItemAllowOverlap();
    return 0;
}

static int IsItemHovered(lua_State* L) {
    auto flags = (ImGuiHoveredFlags)luaL_optinteger(L, 1, lua_Integer(ImGuiHoveredFlags_None));
    auto&& _retval = ImGui::IsItemHovered(flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemActive(lua_State* L) {
    auto&& _retval = ImGui::IsItemActive();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemFocused(lua_State* L) {
    auto&& _retval = ImGui::IsItemFocused();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemClicked(lua_State* L) {
    auto&& _retval = ImGui::IsItemClicked();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemClickedEx(lua_State* L) {
    auto mouse_button = (ImGuiMouseButton)luaL_optinteger(L, 1, lua_Integer(ImGuiMouseButton_Left));
    auto&& _retval = ImGui::IsItemClicked(mouse_button);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemVisible(lua_State* L) {
    auto&& _retval = ImGui::IsItemVisible();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemEdited(lua_State* L) {
    auto&& _retval = ImGui::IsItemEdited();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemActivated(lua_State* L) {
    auto&& _retval = ImGui::IsItemActivated();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemDeactivated(lua_State* L) {
    auto&& _retval = ImGui::IsItemDeactivated();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemDeactivatedAfterEdit(lua_State* L) {
    auto&& _retval = ImGui::IsItemDeactivatedAfterEdit();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsItemToggledOpen(lua_State* L) {
    auto&& _retval = ImGui::IsItemToggledOpen();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsAnyItemHovered(lua_State* L) {
    auto&& _retval = ImGui::IsAnyItemHovered();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsAnyItemActive(lua_State* L) {
    auto&& _retval = ImGui::IsAnyItemActive();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsAnyItemFocused(lua_State* L) {
    auto&& _retval = ImGui::IsAnyItemFocused();
    lua_pushboolean(L, _retval);
    return 1;
}

static int GetItemID(lua_State* L) {
    auto&& _retval = ImGui::GetItemID();
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetItemRectMin(lua_State* L) {
    auto&& _retval = ImGui::GetItemRectMin();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetItemRectMax(lua_State* L) {
    auto&& _retval = ImGui::GetItemRectMax();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetItemRectSize(lua_State* L) {
    auto&& _retval = ImGui::GetItemRectSize();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetMainViewport(lua_State* L) {
    auto&& _retval = ImGui::GetMainViewport();
    wrap_ImGuiViewport::const_pointer(L, *_retval);
    return 1;
}

static int IsRectVisibleBySize(lua_State* L) {
    auto size = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto&& _retval = ImGui::IsRectVisible(size);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsRectVisible(lua_State* L) {
    auto rect_min = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto rect_max = ImVec2{
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto&& _retval = ImGui::IsRectVisible(rect_min, rect_max);
    lua_pushboolean(L, _retval);
    return 1;
}

static int GetTime(lua_State* L) {
    auto&& _retval = ImGui::GetTime();
    lua_pushnumber(L, _retval);
    return 1;
}

static int GetFrameCount(lua_State* L) {
    auto&& _retval = ImGui::GetFrameCount();
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetStyleColorName(lua_State* L) {
    auto idx = (ImGuiCol)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::GetStyleColorName(idx);
    lua_pushstring(L, _retval);
    return 1;
}

static int CalcTextSize(lua_State* L) {
    auto text = luaL_checkstring(L, 1);
    auto&& _retval = ImGui::CalcTextSize(text);
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int CalcTextSizeEx(lua_State* L) {
    auto text = luaL_checkstring(L, 1);
    auto text_end = luaL_optstring(L, 2, NULL);
    auto hide_text_after_double_hash = lua_isnoneornil(L, 3) ? false : !!lua_toboolean(L, 3);
    auto wrap_width = (float)luaL_optnumber(L, 4, -1.0f);
    auto&& _retval = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int ColorConvertU32ToFloat4(lua_State* L) {
    auto in = (ImU32)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::ColorConvertU32ToFloat4(in);
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    lua_pushnumber(L, _retval.z);
    lua_pushnumber(L, _retval.w);
    return 4;
}

static int ColorConvertFloat4ToU32(lua_State* L) {
    auto in = ImVec4{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto&& _retval = ImGui::ColorConvertFloat4ToU32(in);
    lua_pushinteger(L, _retval);
    return 1;
}

static int IsKeyDown(lua_State* L) {
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsKeyDown(key);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsKeyPressed(lua_State* L) {
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsKeyPressed(key);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsKeyPressedEx(lua_State* L) {
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto repeat = lua_isnoneornil(L, 2) ? true : !!lua_toboolean(L, 2);
    auto&& _retval = ImGui::IsKeyPressed(key, repeat);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsKeyReleased(lua_State* L) {
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsKeyReleased(key);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsKeyChordPressed(lua_State* L) {
    auto key_chord = (ImGuiKeyChord)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsKeyChordPressed(key_chord);
    lua_pushboolean(L, _retval);
    return 1;
}

static int GetKeyPressedAmount(lua_State* L) {
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto repeat_delay = (float)luaL_checknumber(L, 2);
    auto rate = (float)luaL_checknumber(L, 3);
    auto&& _retval = ImGui::GetKeyPressedAmount(key, repeat_delay, rate);
    lua_pushinteger(L, _retval);
    return 1;
}

static int GetKeyName(lua_State* L) {
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::GetKeyName(key);
    lua_pushstring(L, _retval);
    return 1;
}

static int SetNextFrameWantCaptureKeyboard(lua_State* L) {
    auto want_capture_keyboard = !!lua_toboolean(L, 1);
    ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard);
    return 0;
}

static int Shortcut(lua_State* L) {
    auto key_chord = (ImGuiKeyChord)luaL_checkinteger(L, 1);
    auto flags = (ImGuiInputFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiInputFlags_None));
    auto&& _retval = ImGui::Shortcut(key_chord, flags);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SetNextItemShortcut(lua_State* L) {
    auto key_chord = (ImGuiKeyChord)luaL_checkinteger(L, 1);
    auto flags = (ImGuiInputFlags)luaL_optinteger(L, 2, lua_Integer(ImGuiInputFlags_None));
    ImGui::SetNextItemShortcut(key_chord, flags);
    return 0;
}

static int SetItemKeyOwner(lua_State* L) {
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    ImGui::SetItemKeyOwner(key);
    return 0;
}

static int IsMouseDown(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsMouseDown(button);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsMouseClicked(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsMouseClicked(button);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsMouseClickedEx(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    auto repeat = lua_isnoneornil(L, 2) ? false : !!lua_toboolean(L, 2);
    auto&& _retval = ImGui::IsMouseClicked(button, repeat);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsMouseReleased(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsMouseReleased(button);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsMouseDoubleClicked(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::IsMouseDoubleClicked(button);
    lua_pushboolean(L, _retval);
    return 1;
}

static int GetMouseClickedCount(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::GetMouseClickedCount(button);
    lua_pushinteger(L, _retval);
    return 1;
}

static int IsMouseHoveringRect(lua_State* L) {
    auto r_min = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto r_max = ImVec2{
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto&& _retval = ImGui::IsMouseHoveringRect(r_min, r_max);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsMouseHoveringRectEx(lua_State* L) {
    auto r_min = ImVec2{
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
    };
    auto r_max = ImVec2{
            (float)luaL_checknumber(L, 3),
            (float)luaL_checknumber(L, 4),
    };
    auto clip = lua_isnoneornil(L, 5) ? true : !!lua_toboolean(L, 5);
    auto&& _retval = ImGui::IsMouseHoveringRect(r_min, r_max, clip);
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsAnyMouseDown(lua_State* L) {
    auto&& _retval = ImGui::IsAnyMouseDown();
    lua_pushboolean(L, _retval);
    return 1;
}

static int GetMousePos(lua_State* L) {
    auto&& _retval = ImGui::GetMousePos();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetMousePosOnOpeningCurrentPopup(lua_State* L) {
    auto&& _retval = ImGui::GetMousePosOnOpeningCurrentPopup();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int IsMouseDragging(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_checkinteger(L, 1);
    auto lock_threshold = (float)luaL_optnumber(L, 2, -1.0f);
    auto&& _retval = ImGui::IsMouseDragging(button, lock_threshold);
    lua_pushboolean(L, _retval);
    return 1;
}

static int GetMouseDragDelta(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_optinteger(L, 1, lua_Integer(ImGuiMouseButton_Left));
    auto lock_threshold = (float)luaL_optnumber(L, 2, -1.0f);
    auto&& _retval = ImGui::GetMouseDragDelta(button, lock_threshold);
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int ResetMouseDragDelta(lua_State* L) {
    ImGui::ResetMouseDragDelta();
    return 0;
}

static int ResetMouseDragDeltaEx(lua_State* L) {
    auto button = (ImGuiMouseButton)luaL_optinteger(L, 1, lua_Integer(ImGuiMouseButton_Left));
    ImGui::ResetMouseDragDelta(button);
    return 0;
}

static int GetMouseCursor(lua_State* L) {
    auto&& _retval = ImGui::GetMouseCursor();
    lua_pushinteger(L, _retval);
    return 1;
}

static int SetMouseCursor(lua_State* L) {
    auto cursor_type = (ImGuiMouseCursor)luaL_checkinteger(L, 1);
    ImGui::SetMouseCursor(cursor_type);
    return 0;
}

static int SetNextFrameWantCaptureMouse(lua_State* L) {
    auto want_capture_mouse = !!lua_toboolean(L, 1);
    ImGui::SetNextFrameWantCaptureMouse(want_capture_mouse);
    return 0;
}

static int GetClipboardText(lua_State* L) {
    auto&& _retval = ImGui::GetClipboardText();
    lua_pushstring(L, _retval);
    return 1;
}

static int SetClipboardText(lua_State* L) {
    auto text = luaL_checkstring(L, 1);
    ImGui::SetClipboardText(text);
    return 0;
}

static int LoadIniSettingsFromDisk(lua_State* L) {
    auto ini_filename = luaL_checkstring(L, 1);
    ImGui::LoadIniSettingsFromDisk(ini_filename);
    return 0;
}

static int LoadIniSettingsFromMemory(lua_State* L) {
    size_t ini_size = 0;
    auto ini_data = luaL_checklstring(L, 1, &ini_size);
    ImGui::LoadIniSettingsFromMemory(ini_data, ini_size);
    return 0;
}

static int SaveIniSettingsToDisk(lua_State* L) {
    auto ini_filename = luaL_checkstring(L, 1);
    ImGui::SaveIniSettingsToDisk(ini_filename);
    return 0;
}

static int SaveIniSettingsToMemory(lua_State* L) {
    size_t out_ini_size = 0;
    auto&& _retval = ImGui::SaveIniSettingsToMemory(&out_ini_size);
    lua_pushlstring(L, _retval, out_ini_size);
    return 1;
}

static int UpdatePlatformWindows(lua_State* L) {
    ImGui::UpdatePlatformWindows();
    return 0;
}

static int RenderPlatformWindowsDefault(lua_State* L) {
    ImGui::RenderPlatformWindowsDefault();
    return 0;
}

static int RenderPlatformWindowsDefaultEx(lua_State* L) {
    auto platform_render_arg = lua_isnoneornil(L, 1) ? NULL : lua_touserdata(L, 1);
    auto renderer_render_arg = lua_isnoneornil(L, 2) ? NULL : lua_touserdata(L, 2);
    ImGui::RenderPlatformWindowsDefault(platform_render_arg, renderer_render_arg);
    return 0;
}

static int DestroyPlatformWindows(lua_State* L) {
    ImGui::DestroyPlatformWindows();
    return 0;
}

static int FindViewportByID(lua_State* L) {
    auto id = (ImGuiID)luaL_checkinteger(L, 1);
    auto&& _retval = ImGui::FindViewportByID(id);
    wrap_ImGuiViewport::const_pointer(L, *_retval);
    return 1;
}

static int FindViewportByPlatformHandle(lua_State* L) {
    auto platform_handle = lua_touserdata(L, 1);
    auto&& _retval = ImGui::FindViewportByPlatformHandle(platform_handle);
    wrap_ImGuiViewport::const_pointer(L, *_retval);
    return 1;
}

namespace wrap_ImGuiContext {

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiContext& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiContext**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiContext", {}, {}, {});
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiContext

namespace wrap_ImGuiIO {

static int AddKeyEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto down = !!lua_toboolean(L, 2);
    OBJ.AddKeyEvent(key, down);
    return 0;
}

static int AddKeyAnalogEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto down = !!lua_toboolean(L, 2);
    auto v = (float)luaL_checknumber(L, 3);
    OBJ.AddKeyAnalogEvent(key, down, v);
    return 0;
}

static int AddMousePosEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto x = (float)luaL_checknumber(L, 1);
    auto y = (float)luaL_checknumber(L, 2);
    OBJ.AddMousePosEvent(x, y);
    return 0;
}

static int AddMouseButtonEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto button = (int)luaL_checkinteger(L, 1);
    auto down = !!lua_toboolean(L, 2);
    OBJ.AddMouseButtonEvent(button, down);
    return 0;
}

static int AddMouseWheelEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto wheel_x = (float)luaL_checknumber(L, 1);
    auto wheel_y = (float)luaL_checknumber(L, 2);
    OBJ.AddMouseWheelEvent(wheel_x, wheel_y);
    return 0;
}

static int AddMouseSourceEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto source = (ImGuiMouseSource)luaL_checkinteger(L, 1);
    OBJ.AddMouseSourceEvent(source);
    return 0;
}

static int AddMouseViewportEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto id = (ImGuiID)luaL_checkinteger(L, 1);
    OBJ.AddMouseViewportEvent(id);
    return 0;
}

static int AddFocusEvent(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto focused = !!lua_toboolean(L, 1);
    OBJ.AddFocusEvent(focused);
    return 0;
}

static int AddInputCharacter(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto c = (unsigned int)luaL_checkinteger(L, 1);
    OBJ.AddInputCharacter(c);
    return 0;
}

static int AddInputCharacterUTF16(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto c = (ImWchar16)luaL_checkinteger(L, 1);
    OBJ.AddInputCharacterUTF16(c);
    return 0;
}

static int AddInputCharactersUTF8(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto str = luaL_checkstring(L, 1);
    OBJ.AddInputCharactersUTF8(str);
    return 0;
}

static int SetKeyEventNativeData(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto native_keycode = (int)luaL_checkinteger(L, 2);
    auto native_scancode = (int)luaL_checkinteger(L, 3);
    OBJ.SetKeyEventNativeData(key, native_keycode, native_scancode);
    return 0;
}

static int SetKeyEventNativeDataEx(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiKey)luaL_checkinteger(L, 1);
    auto native_keycode = (int)luaL_checkinteger(L, 2);
    auto native_scancode = (int)luaL_checkinteger(L, 3);
    auto native_legacy_index = (int)luaL_optinteger(L, 4, -1);
    OBJ.SetKeyEventNativeData(key, native_keycode, native_scancode, native_legacy_index);
    return 0;
}

static int SetAppAcceptingEvents(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    auto accepting_events = !!lua_toboolean(L, 1);
    OBJ.SetAppAcceptingEvents(accepting_events);
    return 0;
}

static int ClearEventsQueue(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearEventsQueue();
    return 0;
}

static int ClearInputKeys(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearInputKeys();
    return 0;
}

static int ClearInputMouse(lua_State* L) {
    auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearInputMouse();
    return 0;
}

struct ConfigFlags {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ConfigFlags);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigFlags = (ImGuiConfigFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BackendFlags {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.BackendFlags);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BackendFlags = (ImGuiBackendFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct DisplaySize {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.DisplaySize.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.DisplaySize.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct DeltaTime {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.DeltaTime);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.DeltaTime = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct IniSavingRate {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.IniSavingRate);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.IniSavingRate = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct UserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.UserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.UserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct Fonts {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        wrap_ImFontAtlas::pointer(L, *OBJ.Fonts);
        return 1;
    }
};

struct FontGlobalScale {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.FontGlobalScale);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.FontGlobalScale = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct FontAllowUserScaling {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.FontAllowUserScaling);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.FontAllowUserScaling = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct DisplayFramebufferScale {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.DisplayFramebufferScale.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.DisplayFramebufferScale.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct ConfigDockingNoSplit {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDockingNoSplit);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDockingNoSplit = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDockingWithShift {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDockingWithShift);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDockingWithShift = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDockingAlwaysTabBar {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDockingAlwaysTabBar);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDockingAlwaysTabBar = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDockingTransparentPayload {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDockingTransparentPayload);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDockingTransparentPayload = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigViewportsNoAutoMerge {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigViewportsNoAutoMerge);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigViewportsNoAutoMerge = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigViewportsNoTaskBarIcon {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigViewportsNoTaskBarIcon);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigViewportsNoTaskBarIcon = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigViewportsNoDecoration {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigViewportsNoDecoration);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigViewportsNoDecoration = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigViewportsNoDefaultParent {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigViewportsNoDefaultParent);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigViewportsNoDefaultParent = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct MouseDrawCursor {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.MouseDrawCursor);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseDrawCursor = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigMacOSXBehaviors {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigMacOSXBehaviors);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigMacOSXBehaviors = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigNavSwapGamepadButtons {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigNavSwapGamepadButtons);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigNavSwapGamepadButtons = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigInputTrickleEventQueue {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigInputTrickleEventQueue);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigInputTrickleEventQueue = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigInputTextCursorBlink {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigInputTextCursorBlink);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigInputTextCursorBlink = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigInputTextEnterKeepActive {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigInputTextEnterKeepActive);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigInputTextEnterKeepActive = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDragClickToInputText {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDragClickToInputText);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDragClickToInputText = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigWindowsResizeFromEdges {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigWindowsResizeFromEdges);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigWindowsResizeFromEdges = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigWindowsMoveFromTitleBarOnly {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigWindowsMoveFromTitleBarOnly);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigWindowsMoveFromTitleBarOnly = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigMemoryCompactTimer {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.ConfigMemoryCompactTimer);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigMemoryCompactTimer = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct MouseDoubleClickTime {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.MouseDoubleClickTime);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseDoubleClickTime = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct MouseDoubleClickMaxDist {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.MouseDoubleClickMaxDist);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseDoubleClickMaxDist = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct MouseDragThreshold {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.MouseDragThreshold);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseDragThreshold = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct KeyRepeatDelay {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.KeyRepeatDelay);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.KeyRepeatDelay = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct KeyRepeatRate {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.KeyRepeatRate);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.KeyRepeatRate = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct ConfigDebugIsDebuggerPresent {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDebugIsDebuggerPresent);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDebugIsDebuggerPresent = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDebugBeginReturnValueOnce {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDebugBeginReturnValueOnce);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDebugBeginReturnValueOnce = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDebugBeginReturnValueLoop {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDebugBeginReturnValueLoop);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDebugBeginReturnValueLoop = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDebugIgnoreFocusLoss {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDebugIgnoreFocusLoss);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDebugIgnoreFocusLoss = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ConfigDebugIniSettings {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.ConfigDebugIniSettings);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ConfigDebugIniSettings = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct BackendPlatformUserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.BackendPlatformUserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.BackendPlatformUserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct BackendRendererUserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.BackendRendererUserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.BackendRendererUserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct BackendLanguageUserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.BackendLanguageUserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.BackendLanguageUserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct ClipboardUserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.ClipboardUserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.ClipboardUserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

#if 0
struct PlatformOpenInShellUserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.PlatformOpenInShellUserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.PlatformOpenInShellUserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct PlatformLocaleDecimalPoint {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.PlatformLocaleDecimalPoint);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.PlatformLocaleDecimalPoint = (ImWchar)luaL_checkinteger(L, 1);
        return 0;
    }
};
#endif

struct WantCaptureMouse {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.WantCaptureMouse);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.WantCaptureMouse = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct WantCaptureKeyboard {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.WantCaptureKeyboard);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.WantCaptureKeyboard = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct WantTextInput {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.WantTextInput);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.WantTextInput = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct WantSetMousePos {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.WantSetMousePos);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.WantSetMousePos = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct WantSaveIniSettings {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.WantSaveIniSettings);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.WantSaveIniSettings = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct NavActive {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.NavActive);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.NavActive = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct NavVisible {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.NavVisible);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.NavVisible = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct Framerate {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.Framerate);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.Framerate = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct MetricsRenderVertices {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.MetricsRenderVertices);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MetricsRenderVertices = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct MetricsRenderIndices {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.MetricsRenderIndices);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MetricsRenderIndices = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct MetricsRenderWindows {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.MetricsRenderWindows);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MetricsRenderWindows = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct MetricsActiveWindows {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.MetricsActiveWindows);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MetricsActiveWindows = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct MouseDelta {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.MouseDelta.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.MouseDelta.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct Ctx {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        wrap_ImGuiContext::pointer(L, *OBJ.Ctx);
        return 1;
    }
};

struct MousePos {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.MousePos.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.MousePos.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct MouseWheel {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.MouseWheel);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseWheel = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct MouseWheelH {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.MouseWheelH);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseWheelH = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct MouseSource {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.MouseSource);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseSource = (ImGuiMouseSource)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct MouseHoveredViewport {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.MouseHoveredViewport);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseHoveredViewport = (ImGuiID)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct KeyCtrl {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.KeyCtrl);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.KeyCtrl = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct KeyShift {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.KeyShift);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.KeyShift = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct KeyAlt {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.KeyAlt);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.KeyAlt = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct KeySuper {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.KeySuper);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.KeySuper = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct KeyMods {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.KeyMods);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.KeyMods = (ImGuiKeyChord)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct WantCaptureMouseUnlessPopupClose {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.WantCaptureMouseUnlessPopupClose);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.WantCaptureMouseUnlessPopupClose = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct MousePosPrev {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.MousePosPrev.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.MousePosPrev.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct MouseWheelRequestAxisSwap {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.MouseWheelRequestAxisSwap);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseWheelRequestAxisSwap = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct MouseCtrlLeftAsRightClick {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.MouseCtrlLeftAsRightClick);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MouseCtrlLeftAsRightClick = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct PenPressure {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.PenPressure);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.PenPressure = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct AppFocusLost {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.AppFocusLost);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.AppFocusLost = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct AppAcceptingEvents {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.AppAcceptingEvents);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.AppAcceptingEvents = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

#if 0
struct BackendUsingLegacyKeyArrays {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.BackendUsingLegacyKeyArrays);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BackendUsingLegacyKeyArrays = (ImS8)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BackendUsingLegacyNavInputArray {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.BackendUsingLegacyNavInputArray);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BackendUsingLegacyNavInputArray = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};
#endif

struct InputQueueSurrogate {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.InputQueueSurrogate);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.InputQueueSurrogate = (ImWchar16)luaL_checkinteger(L, 1);
        return 0;
    }
};

static luaL_Reg funcs[] = {
        {"AddKeyEvent", AddKeyEvent},
        {"AddKeyAnalogEvent", AddKeyAnalogEvent},
        {"AddMousePosEvent", AddMousePosEvent},
        {"AddMouseButtonEvent", AddMouseButtonEvent},
        {"AddMouseWheelEvent", AddMouseWheelEvent},
        {"AddMouseSourceEvent", AddMouseSourceEvent},
        {"AddMouseViewportEvent", AddMouseViewportEvent},
        {"AddFocusEvent", AddFocusEvent},
        {"AddInputCharacter", AddInputCharacter},
        {"AddInputCharacterUTF16", AddInputCharacterUTF16},
        {"AddInputCharactersUTF8", AddInputCharactersUTF8},
        {"SetKeyEventNativeData", SetKeyEventNativeData},
        {"SetKeyEventNativeDataEx", SetKeyEventNativeDataEx},
        {"SetAppAcceptingEvents", SetAppAcceptingEvents},
        {"ClearEventsQueue", ClearEventsQueue},
        {"ClearInputKeys", ClearInputKeys},
        {"ClearInputMouse", ClearInputMouse},
};

static luaL_Reg setters[] = {
        {"ConfigFlags", ConfigFlags::setter},
        {"BackendFlags", BackendFlags::setter},
        {"DeltaTime", DeltaTime::setter},
        {"IniSavingRate", IniSavingRate::setter},
        {"UserData", UserData::setter},
        {"FontGlobalScale", FontGlobalScale::setter},
        {"FontAllowUserScaling", FontAllowUserScaling::setter},
        {"ConfigDockingNoSplit", ConfigDockingNoSplit::setter},
        {"ConfigDockingWithShift", ConfigDockingWithShift::setter},
        {"ConfigDockingAlwaysTabBar", ConfigDockingAlwaysTabBar::setter},
        {"ConfigDockingTransparentPayload", ConfigDockingTransparentPayload::setter},
        {"ConfigViewportsNoAutoMerge", ConfigViewportsNoAutoMerge::setter},
        {"ConfigViewportsNoTaskBarIcon", ConfigViewportsNoTaskBarIcon::setter},
        {"ConfigViewportsNoDecoration", ConfigViewportsNoDecoration::setter},
        {"ConfigViewportsNoDefaultParent", ConfigViewportsNoDefaultParent::setter},
        {"MouseDrawCursor", MouseDrawCursor::setter},
        {"ConfigMacOSXBehaviors", ConfigMacOSXBehaviors::setter},
        {"ConfigNavSwapGamepadButtons", ConfigNavSwapGamepadButtons::setter},
        {"ConfigInputTrickleEventQueue", ConfigInputTrickleEventQueue::setter},
        {"ConfigInputTextCursorBlink", ConfigInputTextCursorBlink::setter},
        {"ConfigInputTextEnterKeepActive", ConfigInputTextEnterKeepActive::setter},
        {"ConfigDragClickToInputText", ConfigDragClickToInputText::setter},
        {"ConfigWindowsResizeFromEdges", ConfigWindowsResizeFromEdges::setter},
        {"ConfigWindowsMoveFromTitleBarOnly", ConfigWindowsMoveFromTitleBarOnly::setter},
        {"ConfigMemoryCompactTimer", ConfigMemoryCompactTimer::setter},
        {"MouseDoubleClickTime", MouseDoubleClickTime::setter},
        {"MouseDoubleClickMaxDist", MouseDoubleClickMaxDist::setter},
        {"MouseDragThreshold", MouseDragThreshold::setter},
        {"KeyRepeatDelay", KeyRepeatDelay::setter},
        {"KeyRepeatRate", KeyRepeatRate::setter},
        {"ConfigDebugIsDebuggerPresent", ConfigDebugIsDebuggerPresent::setter},
        {"ConfigDebugBeginReturnValueOnce", ConfigDebugBeginReturnValueOnce::setter},
        {"ConfigDebugBeginReturnValueLoop", ConfigDebugBeginReturnValueLoop::setter},
        {"ConfigDebugIgnoreFocusLoss", ConfigDebugIgnoreFocusLoss::setter},
        {"ConfigDebugIniSettings", ConfigDebugIniSettings::setter},
        {"BackendPlatformUserData", BackendPlatformUserData::setter},
        {"BackendRendererUserData", BackendRendererUserData::setter},
        {"BackendLanguageUserData", BackendLanguageUserData::setter},
        {"ClipboardUserData", ClipboardUserData::setter},
        //{"PlatformOpenInShellUserData", PlatformOpenInShellUserData::setter},
        //{"PlatformLocaleDecimalPoint", PlatformLocaleDecimalPoint::setter},
        {"WantCaptureMouse", WantCaptureMouse::setter},
        {"WantCaptureKeyboard", WantCaptureKeyboard::setter},
        {"WantTextInput", WantTextInput::setter},
        {"WantSetMousePos", WantSetMousePos::setter},
        {"WantSaveIniSettings", WantSaveIniSettings::setter},
        {"NavActive", NavActive::setter},
        {"NavVisible", NavVisible::setter},
        {"Framerate", Framerate::setter},
        {"MetricsRenderVertices", MetricsRenderVertices::setter},
        {"MetricsRenderIndices", MetricsRenderIndices::setter},
        {"MetricsRenderWindows", MetricsRenderWindows::setter},
        {"MetricsActiveWindows", MetricsActiveWindows::setter},
        {"MouseWheel", MouseWheel::setter},
        {"MouseWheelH", MouseWheelH::setter},
        {"MouseSource", MouseSource::setter},
        {"MouseHoveredViewport", MouseHoveredViewport::setter},
        {"KeyCtrl", KeyCtrl::setter},
        {"KeyShift", KeyShift::setter},
        {"KeyAlt", KeyAlt::setter},
        {"KeySuper", KeySuper::setter},
        {"KeyMods", KeyMods::setter},
        {"WantCaptureMouseUnlessPopupClose", WantCaptureMouseUnlessPopupClose::setter},
        {"MouseWheelRequestAxisSwap", MouseWheelRequestAxisSwap::setter},
        {"MouseCtrlLeftAsRightClick", MouseCtrlLeftAsRightClick::setter},
        {"PenPressure", PenPressure::setter},
        {"AppFocusLost", AppFocusLost::setter},
        {"AppAcceptingEvents", AppAcceptingEvents::setter},
        //{"BackendUsingLegacyKeyArrays", BackendUsingLegacyKeyArrays::setter},
        //{"BackendUsingLegacyNavInputArray", BackendUsingLegacyNavInputArray::setter},
        {"InputQueueSurrogate", InputQueueSurrogate::setter},
};

static luaL_Reg getters[] = {
        {"ConfigFlags", ConfigFlags::getter},
        {"BackendFlags", BackendFlags::getter},
        {"DisplaySize", DisplaySize::getter},
        {"DeltaTime", DeltaTime::getter},
        {"IniSavingRate", IniSavingRate::getter},
        {"UserData", UserData::getter},
        {"Fonts", Fonts::getter},
        {"FontGlobalScale", FontGlobalScale::getter},
        {"FontAllowUserScaling", FontAllowUserScaling::getter},
        {"DisplayFramebufferScale", DisplayFramebufferScale::getter},
        {"ConfigDockingNoSplit", ConfigDockingNoSplit::getter},
        {"ConfigDockingWithShift", ConfigDockingWithShift::getter},
        {"ConfigDockingAlwaysTabBar", ConfigDockingAlwaysTabBar::getter},
        {"ConfigDockingTransparentPayload", ConfigDockingTransparentPayload::getter},
        {"ConfigViewportsNoAutoMerge", ConfigViewportsNoAutoMerge::getter},
        {"ConfigViewportsNoTaskBarIcon", ConfigViewportsNoTaskBarIcon::getter},
        {"ConfigViewportsNoDecoration", ConfigViewportsNoDecoration::getter},
        {"ConfigViewportsNoDefaultParent", ConfigViewportsNoDefaultParent::getter},
        {"MouseDrawCursor", MouseDrawCursor::getter},
        {"ConfigMacOSXBehaviors", ConfigMacOSXBehaviors::getter},
        {"ConfigNavSwapGamepadButtons", ConfigNavSwapGamepadButtons::getter},
        {"ConfigInputTrickleEventQueue", ConfigInputTrickleEventQueue::getter},
        {"ConfigInputTextCursorBlink", ConfigInputTextCursorBlink::getter},
        {"ConfigInputTextEnterKeepActive", ConfigInputTextEnterKeepActive::getter},
        {"ConfigDragClickToInputText", ConfigDragClickToInputText::getter},
        {"ConfigWindowsResizeFromEdges", ConfigWindowsResizeFromEdges::getter},
        {"ConfigWindowsMoveFromTitleBarOnly", ConfigWindowsMoveFromTitleBarOnly::getter},
        {"ConfigMemoryCompactTimer", ConfigMemoryCompactTimer::getter},
        {"MouseDoubleClickTime", MouseDoubleClickTime::getter},
        {"MouseDoubleClickMaxDist", MouseDoubleClickMaxDist::getter},
        {"MouseDragThreshold", MouseDragThreshold::getter},
        {"KeyRepeatDelay", KeyRepeatDelay::getter},
        {"KeyRepeatRate", KeyRepeatRate::getter},
        {"ConfigDebugIsDebuggerPresent", ConfigDebugIsDebuggerPresent::getter},
        {"ConfigDebugBeginReturnValueOnce", ConfigDebugBeginReturnValueOnce::getter},
        {"ConfigDebugBeginReturnValueLoop", ConfigDebugBeginReturnValueLoop::getter},
        {"ConfigDebugIgnoreFocusLoss", ConfigDebugIgnoreFocusLoss::getter},
        {"ConfigDebugIniSettings", ConfigDebugIniSettings::getter},
        {"BackendPlatformUserData", BackendPlatformUserData::getter},
        {"BackendRendererUserData", BackendRendererUserData::getter},
        {"BackendLanguageUserData", BackendLanguageUserData::getter},
        {"ClipboardUserData", ClipboardUserData::getter},
        //{"PlatformOpenInShellUserData", PlatformOpenInShellUserData::getter},
        //{"PlatformLocaleDecimalPoint", PlatformLocaleDecimalPoint::getter},
        {"WantCaptureMouse", WantCaptureMouse::getter},
        {"WantCaptureKeyboard", WantCaptureKeyboard::getter},
        {"WantTextInput", WantTextInput::getter},
        {"WantSetMousePos", WantSetMousePos::getter},
        {"WantSaveIniSettings", WantSaveIniSettings::getter},
        {"NavActive", NavActive::getter},
        {"NavVisible", NavVisible::getter},
        {"Framerate", Framerate::getter},
        {"MetricsRenderVertices", MetricsRenderVertices::getter},
        {"MetricsRenderIndices", MetricsRenderIndices::getter},
        {"MetricsRenderWindows", MetricsRenderWindows::getter},
        {"MetricsActiveWindows", MetricsActiveWindows::getter},
        {"MouseDelta", MouseDelta::getter},
        {"Ctx", Ctx::getter},
        {"MousePos", MousePos::getter},
        {"MouseWheel", MouseWheel::getter},
        {"MouseWheelH", MouseWheelH::getter},
        {"MouseSource", MouseSource::getter},
        {"MouseHoveredViewport", MouseHoveredViewport::getter},
        {"KeyCtrl", KeyCtrl::getter},
        {"KeyShift", KeyShift::getter},
        {"KeyAlt", KeyAlt::getter},
        {"KeySuper", KeySuper::getter},
        {"KeyMods", KeyMods::getter},
        {"WantCaptureMouseUnlessPopupClose", WantCaptureMouseUnlessPopupClose::getter},
        {"MousePosPrev", MousePosPrev::getter},
        {"MouseWheelRequestAxisSwap", MouseWheelRequestAxisSwap::getter},
        {"MouseCtrlLeftAsRightClick", MouseCtrlLeftAsRightClick::getter},
        {"PenPressure", PenPressure::getter},
        {"AppFocusLost", AppFocusLost::getter},
        {"AppAcceptingEvents", AppAcceptingEvents::getter},
        //{"BackendUsingLegacyKeyArrays", BackendUsingLegacyKeyArrays::getter},
        //{"BackendUsingLegacyNavInputArray", BackendUsingLegacyNavInputArray::getter},
        {"InputQueueSurrogate", InputQueueSurrogate::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiIO& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiIO**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiIO", funcs, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiIO

namespace wrap_ImGuiInputTextCallbackData {

static int DeleteChars(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    auto pos = (int)luaL_checkinteger(L, 1);
    auto bytes_count = (int)luaL_checkinteger(L, 2);
    OBJ.DeleteChars(pos, bytes_count);
    return 0;
}

static int InsertChars(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    auto pos = (int)luaL_checkinteger(L, 1);
    auto text = luaL_checkstring(L, 2);
    auto text_end = luaL_optstring(L, 3, NULL);
    OBJ.InsertChars(pos, text, text_end);
    return 0;
}

static int SelectAll(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.SelectAll();
    return 0;
}

static int ClearSelection(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearSelection();
    return 0;
}

static int HasSelection(lua_State* L) {
    auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.HasSelection();
    lua_pushboolean(L, _retval);
    return 1;
}

struct Ctx {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        wrap_ImGuiContext::pointer(L, *OBJ.Ctx);
        return 1;
    }
};

struct EventFlag {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.EventFlag);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.EventFlag = (ImGuiInputTextFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct Flags {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.Flags);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.Flags = (ImGuiInputTextFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct UserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.UserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.UserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct EventChar {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.EventChar);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.EventChar = (ImWchar)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct EventKey {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.EventKey);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.EventKey = (ImGuiKey)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BufTextLen {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.BufTextLen);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BufTextLen = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BufSize {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.BufSize);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BufSize = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct BufDirty {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.BufDirty);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.BufDirty = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct CursorPos {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.CursorPos);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.CursorPos = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct SelectionStart {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.SelectionStart);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.SelectionStart = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct SelectionEnd {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.SelectionEnd);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiInputTextCallbackData**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.SelectionEnd = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

static luaL_Reg funcs[] = {
        {"DeleteChars", DeleteChars}, {"InsertChars", InsertChars}, {"SelectAll", SelectAll}, {"ClearSelection", ClearSelection}, {"HasSelection", HasSelection},
};

static luaL_Reg setters[] = {
        {"EventFlag", EventFlag::setter},       {"Flags", Flags::setter},     {"UserData", UserData::setter}, {"EventChar", EventChar::setter}, {"EventKey", EventKey::setter},
        {"BufTextLen", BufTextLen::setter},     {"BufSize", BufSize::setter}, {"BufDirty", BufDirty::setter}, {"CursorPos", CursorPos::setter}, {"SelectionStart", SelectionStart::setter},
        {"SelectionEnd", SelectionEnd::setter},
};

static luaL_Reg getters[] = {
        {"Ctx", Ctx::getter},
        {"EventFlag", EventFlag::getter},
        {"Flags", Flags::getter},
        {"UserData", UserData::getter},
        {"EventChar", EventChar::getter},
        {"EventKey", EventKey::getter},
        {"BufTextLen", BufTextLen::getter},
        {"BufSize", BufSize::getter},
        {"BufDirty", BufDirty::getter},
        {"CursorPos", CursorPos::getter},
        {"SelectionStart", SelectionStart::getter},
        {"SelectionEnd", SelectionEnd::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiInputTextCallbackData& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiInputTextCallbackData**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiInputTextCallbackData", funcs, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiInputTextCallbackData

namespace wrap_ImGuiWindowClass {

struct ClassId {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ClassId);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ClassId = (ImGuiID)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct ParentViewportId {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ParentViewportId);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ParentViewportId = (ImGuiID)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct FocusRouteParentWindowId {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.FocusRouteParentWindowId);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.FocusRouteParentWindowId = (ImGuiID)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct ViewportFlagsOverrideSet {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ViewportFlagsOverrideSet);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ViewportFlagsOverrideSet = (ImGuiViewportFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct ViewportFlagsOverrideClear {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ViewportFlagsOverrideClear);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ViewportFlagsOverrideClear = (ImGuiViewportFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct TabItemFlagsOverrideSet {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.TabItemFlagsOverrideSet);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.TabItemFlagsOverrideSet = (ImGuiTabItemFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct DockNodeFlagsOverrideSet {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.DockNodeFlagsOverrideSet);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.DockNodeFlagsOverrideSet = (ImGuiDockNodeFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct DockingAlwaysTabBar {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.DockingAlwaysTabBar);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.DockingAlwaysTabBar = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct DockingAllowUnclassed {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.DockingAllowUnclassed);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiWindowClass**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.DockingAllowUnclassed = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

static luaL_Reg setters[] = {
        {"ClassId", ClassId::setter},
        {"ParentViewportId", ParentViewportId::setter},
        {"FocusRouteParentWindowId", FocusRouteParentWindowId::setter},
        {"ViewportFlagsOverrideSet", ViewportFlagsOverrideSet::setter},
        {"ViewportFlagsOverrideClear", ViewportFlagsOverrideClear::setter},
        {"TabItemFlagsOverrideSet", TabItemFlagsOverrideSet::setter},
        {"DockNodeFlagsOverrideSet", DockNodeFlagsOverrideSet::setter},
        {"DockingAlwaysTabBar", DockingAlwaysTabBar::setter},
        {"DockingAllowUnclassed", DockingAllowUnclassed::setter},
};

static luaL_Reg getters[] = {
        {"ClassId", ClassId::getter},
        {"ParentViewportId", ParentViewportId::getter},
        {"FocusRouteParentWindowId", FocusRouteParentWindowId::getter},
        {"ViewportFlagsOverrideSet", ViewportFlagsOverrideSet::getter},
        {"ViewportFlagsOverrideClear", ViewportFlagsOverrideClear::getter},
        {"TabItemFlagsOverrideSet", TabItemFlagsOverrideSet::getter},
        {"DockNodeFlagsOverrideSet", DockNodeFlagsOverrideSet::getter},
        {"DockingAlwaysTabBar", DockingAlwaysTabBar::getter},
        {"DockingAllowUnclassed", DockingAllowUnclassed::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiWindowClass& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiWindowClass**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiWindowClass", {}, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiWindowClass

namespace wrap_ImGuiStorage {

static int Clear(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.Clear();
    return 0;
}

static int GetInt(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiID)luaL_checkinteger(L, 1);
    auto default_val = (int)luaL_optinteger(L, 2, 0);
    auto&& _retval = OBJ.GetInt(key, default_val);
    lua_pushinteger(L, _retval);
    return 1;
}

static int SetInt(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiID)luaL_checkinteger(L, 1);
    auto val = (int)luaL_checkinteger(L, 2);
    OBJ.SetInt(key, val);
    return 0;
}

static int GetBool(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiID)luaL_checkinteger(L, 1);
    auto default_val = lua_isnoneornil(L, 2) ? false : !!lua_toboolean(L, 2);
    auto&& _retval = OBJ.GetBool(key, default_val);
    lua_pushboolean(L, _retval);
    return 1;
}

static int SetBool(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiID)luaL_checkinteger(L, 1);
    auto val = !!lua_toboolean(L, 2);
    OBJ.SetBool(key, val);
    return 0;
}

static int GetFloat(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiID)luaL_checkinteger(L, 1);
    auto default_val = (float)luaL_optnumber(L, 2, 0.0f);
    auto&& _retval = OBJ.GetFloat(key, default_val);
    lua_pushnumber(L, _retval);
    return 1;
}

static int SetFloat(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiID)luaL_checkinteger(L, 1);
    auto val = (float)luaL_checknumber(L, 2);
    OBJ.SetFloat(key, val);
    return 0;
}

static int SetVoidPtr(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto key = (ImGuiID)luaL_checkinteger(L, 1);
    auto val = lua_touserdata(L, 2);
    OBJ.SetVoidPtr(key, val);
    return 0;
}

static int BuildSortByKey(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.BuildSortByKey();
    return 0;
}

static int SetAllInt(lua_State* L) {
    auto& OBJ = **(ImGuiStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto val = (int)luaL_checkinteger(L, 1);
    OBJ.SetAllInt(val);
    return 0;
}

static luaL_Reg funcs[] = {
        {"Clear", Clear},         {"GetInt", GetInt},         {"SetInt", SetInt},
        {"GetBool", GetBool},     {"SetBool", SetBool},       {"GetFloat", GetFloat},
        {"SetFloat", SetFloat},   {"SetVoidPtr", SetVoidPtr}, {"BuildSortByKey", BuildSortByKey},
        {"SetAllInt", SetAllInt},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiStorage& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiStorage**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiStorage", funcs, {}, {});
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiStorage

namespace wrap_ImGuiMultiSelectIO {

struct RangeSrcItem {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.RangeSrcItem);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.RangeSrcItem = (ImGuiSelectionUserData)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct NavIdItem {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.NavIdItem);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.NavIdItem = (ImGuiSelectionUserData)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct NavIdSelected {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.NavIdSelected);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.NavIdSelected = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct RangeSrcReset {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.RangeSrcReset);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.RangeSrcReset = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct ItemsCount {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ItemsCount);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiMultiSelectIO**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.ItemsCount = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

static luaL_Reg setters[] = {
        {"RangeSrcItem", RangeSrcItem::setter},   {"NavIdItem", NavIdItem::setter},   {"NavIdSelected", NavIdSelected::setter},
        {"RangeSrcReset", RangeSrcReset::setter}, {"ItemsCount", ItemsCount::setter},
};

static luaL_Reg getters[] = {
        {"RangeSrcItem", RangeSrcItem::getter},   {"NavIdItem", NavIdItem::getter},   {"NavIdSelected", NavIdSelected::getter},
        {"RangeSrcReset", RangeSrcReset::getter}, {"ItemsCount", ItemsCount::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiMultiSelectIO& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiMultiSelectIO**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiMultiSelectIO", {}, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiMultiSelectIO

namespace wrap_ImGuiSelectionRequest {

struct Type {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.Type);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.Type = (ImGuiSelectionRequestType)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct Selected {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.Selected);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.Selected = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct RangeDirection {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.RangeDirection);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.RangeDirection = (ImS8)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct RangeFirstItem {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.RangeFirstItem);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.RangeFirstItem = (ImGuiSelectionUserData)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct RangeLastItem {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.RangeLastItem);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionRequest**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.RangeLastItem = (ImGuiSelectionUserData)luaL_checkinteger(L, 1);
        return 0;
    }
};

static luaL_Reg setters[] = {
        {"Type", Type::setter}, {"Selected", Selected::setter}, {"RangeDirection", RangeDirection::setter}, {"RangeFirstItem", RangeFirstItem::setter}, {"RangeLastItem", RangeLastItem::setter},
};

static luaL_Reg getters[] = {
        {"Type", Type::getter}, {"Selected", Selected::getter}, {"RangeDirection", RangeDirection::getter}, {"RangeFirstItem", RangeFirstItem::getter}, {"RangeLastItem", RangeLastItem::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiSelectionRequest& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiSelectionRequest**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiSelectionRequest", {}, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiSelectionRequest

namespace wrap_ImGuiSelectionExternalStorage {

static int ApplyRequests(lua_State* L) {
    auto& OBJ = **(ImGuiSelectionExternalStorage**)lua_touserdata(L, lua_upvalueindex(1));
    auto ms_io = *(ImGuiMultiSelectIO**)lua_touserdata(L, 1);
    OBJ.ApplyRequests(ms_io);
    return 0;
}

struct UserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionExternalStorage**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.UserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImGuiSelectionExternalStorage**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.UserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

static luaL_Reg funcs[] = {
        {"ApplyRequests", ApplyRequests},
};

static luaL_Reg setters[] = {
        {"UserData", UserData::setter},
};

static luaL_Reg getters[] = {
        {"UserData", UserData::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImGuiSelectionExternalStorage& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImGuiSelectionExternalStorage**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiSelectionExternalStorage", funcs, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImGuiSelectionExternalStorage

namespace wrap_ImFontConfig {

struct FontData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.FontData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.FontData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

struct FontDataSize {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.FontDataSize);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.FontDataSize = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct FontDataOwnedByAtlas {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.FontDataOwnedByAtlas);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.FontDataOwnedByAtlas = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct FontNo {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.FontNo);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.FontNo = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct SizePixels {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.SizePixels);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.SizePixels = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct OversampleH {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.OversampleH);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.OversampleH = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct OversampleV {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.OversampleV);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.OversampleV = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct PixelSnapH {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.PixelSnapH);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.PixelSnapH = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct GlyphExtraSpacing {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.GlyphExtraSpacing.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.GlyphExtraSpacing.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct GlyphOffset {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.GlyphOffset.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.GlyphOffset.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct GlyphRanges {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, (void*)OBJ.GlyphRanges);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.GlyphRanges = (const ImWchar*)lua_touserdata(L, 1);
        return 0;
    }
};

struct GlyphMinAdvanceX {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.GlyphMinAdvanceX);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.GlyphMinAdvanceX = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct GlyphMaxAdvanceX {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.GlyphMaxAdvanceX);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.GlyphMaxAdvanceX = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct MergeMode {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.MergeMode);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.MergeMode = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct FontBuilderFlags {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.FontBuilderFlags);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.FontBuilderFlags = (unsigned int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct RasterizerMultiply {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.RasterizerMultiply);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.RasterizerMultiply = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct RasterizerDensity {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.RasterizerDensity);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.RasterizerDensity = (float)luaL_checknumber(L, 1);
        return 0;
    }
};

struct EllipsisChar {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.EllipsisChar);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontConfig**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.EllipsisChar = (ImWchar)luaL_checkinteger(L, 1);
        return 0;
    }
};

static luaL_Reg setters[] = {
        {"FontData", FontData::setter},
        {"FontDataSize", FontDataSize::setter},
        {"FontDataOwnedByAtlas", FontDataOwnedByAtlas::setter},
        {"FontNo", FontNo::setter},
        {"SizePixels", SizePixels::setter},
        {"OversampleH", OversampleH::setter},
        {"OversampleV", OversampleV::setter},
        {"PixelSnapH", PixelSnapH::setter},
        {"GlyphRanges", GlyphRanges::setter},
        {"GlyphMinAdvanceX", GlyphMinAdvanceX::setter},
        {"GlyphMaxAdvanceX", GlyphMaxAdvanceX::setter},
        {"MergeMode", MergeMode::setter},
        {"FontBuilderFlags", FontBuilderFlags::setter},
        {"RasterizerMultiply", RasterizerMultiply::setter},
        {"RasterizerDensity", RasterizerDensity::setter},
        {"EllipsisChar", EllipsisChar::setter},
};

static luaL_Reg getters[] = {
        {"FontData", FontData::getter},
        {"FontDataSize", FontDataSize::getter},
        {"FontDataOwnedByAtlas", FontDataOwnedByAtlas::getter},
        {"FontNo", FontNo::getter},
        {"SizePixels", SizePixels::getter},
        {"OversampleH", OversampleH::getter},
        {"OversampleV", OversampleV::getter},
        {"PixelSnapH", PixelSnapH::getter},
        {"GlyphExtraSpacing", GlyphExtraSpacing::getter},
        {"GlyphOffset", GlyphOffset::getter},
        {"GlyphRanges", GlyphRanges::getter},
        {"GlyphMinAdvanceX", GlyphMinAdvanceX::getter},
        {"GlyphMaxAdvanceX", GlyphMaxAdvanceX::getter},
        {"MergeMode", MergeMode::getter},
        {"FontBuilderFlags", FontBuilderFlags::getter},
        {"RasterizerMultiply", RasterizerMultiply::getter},
        {"RasterizerDensity", RasterizerDensity::getter},
        {"EllipsisChar", EllipsisChar::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImFontConfig& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImFontConfig**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImFontConfig", {}, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImFontConfig

namespace wrap_ImFontAtlas {

static int AddFont(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto font_cfg = *(const ImFontConfig**)lua_touserdata(L, 1);
    auto&& _retval = OBJ.AddFont(font_cfg);
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int AddFontDefault(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto font_cfg = lua_isnoneornil(L, 1) ? NULL : *(const ImFontConfig**)lua_touserdata(L, 1);
    auto&& _retval = OBJ.AddFontDefault(font_cfg);
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int AddFontFromFileTTF(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto filename = luaL_checkstring(L, 1);
    auto size_pixels = (float)luaL_checknumber(L, 2);
    auto font_cfg = lua_isnoneornil(L, 3) ? NULL : *(const ImFontConfig**)lua_touserdata(L, 3);
    const ImWchar* glyph_ranges = NULL;
    switch (lua_type(L, 4)) {
        case LUA_TSTRING:
            glyph_ranges = (const ImWchar*)lua_tostring(L, 4);
            break;
        case LUA_TLIGHTUSERDATA:
            glyph_ranges = (const ImWchar*)lua_touserdata(L, 4);
            break;
        default:
            break;
    };
    auto&& _retval = OBJ.AddFontFromFileTTF(filename, size_pixels, font_cfg, glyph_ranges);
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int AddFontFromMemoryTTF(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto font_data = lua_touserdata(L, 1);
    auto font_data_size = (int)luaL_checkinteger(L, 2);
    auto size_pixels = (float)luaL_checknumber(L, 3);
    auto font_cfg = lua_isnoneornil(L, 4) ? NULL : *(const ImFontConfig**)lua_touserdata(L, 4);
    const ImWchar* glyph_ranges = NULL;
    switch (lua_type(L, 5)) {
        case LUA_TSTRING:
            glyph_ranges = (const ImWchar*)lua_tostring(L, 5);
            break;
        case LUA_TLIGHTUSERDATA:
            glyph_ranges = (const ImWchar*)lua_touserdata(L, 5);
            break;
        default:
            break;
    };
    auto&& _retval = OBJ.AddFontFromMemoryTTF(font_data, font_data_size, size_pixels, font_cfg, glyph_ranges);
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int AddFontFromMemoryCompressedTTF(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto compressed_font_data = lua_touserdata(L, 1);
    auto compressed_font_data_size = (int)luaL_checkinteger(L, 2);
    auto size_pixels = (float)luaL_checknumber(L, 3);
    auto font_cfg = lua_isnoneornil(L, 4) ? NULL : *(const ImFontConfig**)lua_touserdata(L, 4);
    const ImWchar* glyph_ranges = NULL;
    switch (lua_type(L, 5)) {
        case LUA_TSTRING:
            glyph_ranges = (const ImWchar*)lua_tostring(L, 5);
            break;
        case LUA_TLIGHTUSERDATA:
            glyph_ranges = (const ImWchar*)lua_touserdata(L, 5);
            break;
        default:
            break;
    };
    auto&& _retval = OBJ.AddFontFromMemoryCompressedTTF(compressed_font_data, compressed_font_data_size, size_pixels, font_cfg, glyph_ranges);
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int AddFontFromMemoryCompressedBase85TTF(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto compressed_font_data_base85 = luaL_checkstring(L, 1);
    auto size_pixels = (float)luaL_checknumber(L, 2);
    auto font_cfg = lua_isnoneornil(L, 3) ? NULL : *(const ImFontConfig**)lua_touserdata(L, 3);
    const ImWchar* glyph_ranges = NULL;
    switch (lua_type(L, 4)) {
        case LUA_TSTRING:
            glyph_ranges = (const ImWchar*)lua_tostring(L, 4);
            break;
        case LUA_TLIGHTUSERDATA:
            glyph_ranges = (const ImWchar*)lua_touserdata(L, 4);
            break;
        default:
            break;
    };
    auto&& _retval = OBJ.AddFontFromMemoryCompressedBase85TTF(compressed_font_data_base85, size_pixels, font_cfg, glyph_ranges);
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int ClearInputData(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearInputData();
    return 0;
}

static int ClearTexData(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearTexData();
    return 0;
}

static int ClearFonts(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.ClearFonts();
    return 0;
}

static int Clear(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    OBJ.Clear();
    return 0;
}

static int Build(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.Build();
    lua_pushboolean(L, _retval);
    return 1;
}

static int IsBuilt(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.IsBuilt();
    lua_pushboolean(L, _retval);
    return 1;
}

static int SetTexID(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto id = util::get_texture_id(L, 1);
    OBJ.SetTexID(id);
    return 0;
}

static int GetGlyphRangesDefault(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesDefault();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesGreek(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesGreek();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesKorean(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesKorean();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesJapanese(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesJapanese();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesChineseFull(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesChineseFull();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesChineseSimplifiedCommon(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesChineseSimplifiedCommon();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesCyrillic(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesCyrillic();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesThai(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesThai();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int GetGlyphRangesVietnamese(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetGlyphRangesVietnamese();
    lua_pushlightuserdata(L, (void*)_retval);
    return 1;
}

static int AddCustomRectRegular(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto width = (int)luaL_checkinteger(L, 1);
    auto height = (int)luaL_checkinteger(L, 2);
    auto&& _retval = OBJ.AddCustomRectRegular(width, height);
    lua_pushinteger(L, _retval);
    return 1;
}

static int AddCustomRectFontGlyph(lua_State* L) {
    auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
    auto font = (ImFont*)lua_touserdata(L, 1);
    auto id = (ImWchar)luaL_checkinteger(L, 2);
    auto width = (int)luaL_checkinteger(L, 3);
    auto height = (int)luaL_checkinteger(L, 4);
    auto advance_x = (float)luaL_checknumber(L, 5);
    auto offset = ImVec2{
            (float)luaL_optnumber(L, 6, 0),
            (float)luaL_optnumber(L, 7, 0),
    };
    auto&& _retval = OBJ.AddCustomRectFontGlyph(font, id, width, height, advance_x, offset);
    lua_pushinteger(L, _retval);
    return 1;
}

struct Flags {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.Flags);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.Flags = (ImFontAtlasFlags)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct TexDesiredWidth {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.TexDesiredWidth);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.TexDesiredWidth = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct TexGlyphPadding {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.TexGlyphPadding);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.TexGlyphPadding = (int)luaL_checkinteger(L, 1);
        return 0;
    }
};

struct Locked {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.Locked);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        OBJ.Locked = (bool)!!lua_toboolean(L, 1);
        return 0;
    }
};

struct UserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.UserData);
        return 1;
    }

    static int setter(lua_State* L) {
        auto& OBJ = **(ImFontAtlas**)lua_touserdata(L, lua_upvalueindex(1));
        luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
        OBJ.UserData = (void*)lua_touserdata(L, 1);
        return 0;
    }
};

static luaL_Reg funcs[] = {
        {"AddFont", AddFont},
        {"AddFontDefault", AddFontDefault},
        {"AddFontFromFileTTF", AddFontFromFileTTF},
        {"AddFontFromMemoryTTF", AddFontFromMemoryTTF},
        {"AddFontFromMemoryCompressedTTF", AddFontFromMemoryCompressedTTF},
        {"AddFontFromMemoryCompressedBase85TTF", AddFontFromMemoryCompressedBase85TTF},
        {"ClearInputData", ClearInputData},
        {"ClearTexData", ClearTexData},
        {"ClearFonts", ClearFonts},
        {"Clear", Clear},
        {"Build", Build},
        {"IsBuilt", IsBuilt},
        {"SetTexID", SetTexID},
        {"GetGlyphRangesDefault", GetGlyphRangesDefault},
        {"GetGlyphRangesGreek", GetGlyphRangesGreek},
        {"GetGlyphRangesKorean", GetGlyphRangesKorean},
        {"GetGlyphRangesJapanese", GetGlyphRangesJapanese},
        {"GetGlyphRangesChineseFull", GetGlyphRangesChineseFull},
        {"GetGlyphRangesChineseSimplifiedCommon", GetGlyphRangesChineseSimplifiedCommon},
        {"GetGlyphRangesCyrillic", GetGlyphRangesCyrillic},
        {"GetGlyphRangesThai", GetGlyphRangesThai},
        {"GetGlyphRangesVietnamese", GetGlyphRangesVietnamese},
        {"AddCustomRectRegular", AddCustomRectRegular},
        {"AddCustomRectFontGlyph", AddCustomRectFontGlyph},
};

static luaL_Reg setters[] = {
        {"Flags", Flags::setter}, {"TexDesiredWidth", TexDesiredWidth::setter}, {"TexGlyphPadding", TexGlyphPadding::setter}, {"Locked", Locked::setter}, {"UserData", UserData::setter},
};

static luaL_Reg getters[] = {
        {"Flags", Flags::getter}, {"TexDesiredWidth", TexDesiredWidth::getter}, {"TexGlyphPadding", TexGlyphPadding::getter}, {"Locked", Locked::getter}, {"UserData", UserData::getter},
};

static int tag_pointer = 0;

void pointer(lua_State* L, ImFontAtlas& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_pointer);
    auto** ptr = (ImFontAtlas**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImFontAtlas", funcs, setters, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_pointer);
}

}  // namespace wrap_ImFontAtlas

namespace wrap_ImGuiViewport {

static int GetCenter(lua_State* L) {
    auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetCenter();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

static int GetWorkCenter(lua_State* L) {
    auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
    auto&& _retval = OBJ.GetWorkCenter();
    lua_pushnumber(L, _retval.x);
    lua_pushnumber(L, _retval.y);
    return 2;
}

struct ID {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ID);
        return 1;
    }
};

struct Flags {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.Flags);
        return 1;
    }
};

struct Pos {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.Pos.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.Pos.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct Size {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.Size.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.Size.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct WorkPos {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.WorkPos.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.WorkPos.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct WorkSize {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_createtable(L, 0, 2);
        lua_pushnumber(L, OBJ.WorkSize.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, OBJ.WorkSize.y);
        lua_setfield(L, -2, "y");
        return 1;
    }
};

struct DpiScale {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushnumber(L, OBJ.DpiScale);
        return 1;
    }
};

struct ParentViewportId {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushinteger(L, OBJ.ParentViewportId);
        return 1;
    }
};

struct RendererUserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.RendererUserData);
        return 1;
    }
};

struct PlatformUserData {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.PlatformUserData);
        return 1;
    }
};

struct PlatformHandle {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.PlatformHandle);
        return 1;
    }
};

struct PlatformHandleRaw {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushlightuserdata(L, OBJ.PlatformHandleRaw);
        return 1;
    }
};

struct PlatformWindowCreated {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.PlatformWindowCreated);
        return 1;
    }
};

struct PlatformRequestMove {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.PlatformRequestMove);
        return 1;
    }
};

struct PlatformRequestResize {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.PlatformRequestResize);
        return 1;
    }
};

struct PlatformRequestClose {
    static int getter(lua_State* L) {
        auto& OBJ = **(ImGuiViewport**)lua_touserdata(L, lua_upvalueindex(1));
        lua_pushboolean(L, OBJ.PlatformRequestClose);
        return 1;
    }
};

static luaL_Reg funcs[] = {
        {"GetCenter", GetCenter},
        {"GetWorkCenter", GetWorkCenter},
};

static luaL_Reg getters[] = {
        {"ID", ID::getter},
        {"Flags", Flags::getter},
        {"Pos", Pos::getter},
        {"Size", Size::getter},
        {"WorkPos", WorkPos::getter},
        {"WorkSize", WorkSize::getter},
        {"DpiScale", DpiScale::getter},
        {"ParentViewportId", ParentViewportId::getter},
        {"RendererUserData", RendererUserData::getter},
        {"PlatformUserData", PlatformUserData::getter},
        {"PlatformHandle", PlatformHandle::getter},
        {"PlatformHandleRaw", PlatformHandleRaw::getter},
        {"PlatformWindowCreated", PlatformWindowCreated::getter},
        {"PlatformRequestMove", PlatformRequestMove::getter},
        {"PlatformRequestResize", PlatformRequestResize::getter},
        {"PlatformRequestClose", PlatformRequestClose::getter},
};

static int tag_const_pointer = 0;

void const_pointer(lua_State* L, ImGuiViewport& v) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, &tag_const_pointer);
    auto** ptr = (ImGuiViewport**)lua_touserdata(L, -1);
    *ptr = &v;
}

static void init(lua_State* L) {
    util::struct_gen(L, "ImGuiViewport", funcs, {}, getters);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &tag_const_pointer);
}

}  // namespace wrap_ImGuiViewport

static void init(lua_State* L) {
    static luaL_Reg funcs[] = {
            {"IO", IO},
            {"InputTextCallbackData", InputTextCallbackData},
            {"WindowClass", WindowClass},
            {"Storage", Storage},
            {"MultiSelectIO", MultiSelectIO},
            {"SelectionRequest", SelectionRequest},
            {"SelectionExternalStorage", SelectionExternalStorage},
            {"FontConfig", FontConfig},
            {"FontAtlas", FontAtlas},
            {"Viewport", Viewport},
            {"StringBuf", StringBuf},
            {"CreateContext", CreateContext},
            {"DestroyContext", DestroyContext},
            {"GetCurrentContext", GetCurrentContext},
            {"SetCurrentContext", SetCurrentContext},
            {"GetIO", GetIO},
            {"NewFrame", NewFrame},
            {"EndFrame", EndFrame},
            {"Render", Render},
            {"GetVersion", GetVersion},
            {"Begin", Begin},
            {"End", End},
            {"BeginChild", BeginChild},
            {"BeginChildID", BeginChildID},
            {"EndChild", EndChild},
            {"IsWindowAppearing", IsWindowAppearing},
            {"IsWindowCollapsed", IsWindowCollapsed},
            {"IsWindowFocused", IsWindowFocused},
            {"IsWindowHovered", IsWindowHovered},
            {"GetWindowDpiScale", GetWindowDpiScale},
            {"GetWindowPos", GetWindowPos},
            {"GetWindowSize", GetWindowSize},
            {"GetWindowWidth", GetWindowWidth},
            {"GetWindowHeight", GetWindowHeight},
            {"GetWindowViewport", GetWindowViewport},
            {"SetNextWindowPos", SetNextWindowPos},
            {"SetNextWindowPosEx", SetNextWindowPosEx},
            {"SetNextWindowSize", SetNextWindowSize},
            {"SetNextWindowContentSize", SetNextWindowContentSize},
            {"SetNextWindowCollapsed", SetNextWindowCollapsed},
            {"SetNextWindowFocus", SetNextWindowFocus},
            {"SetNextWindowScroll", SetNextWindowScroll},
            {"SetNextWindowBgAlpha", SetNextWindowBgAlpha},
            {"SetNextWindowViewport", SetNextWindowViewport},
            {"SetWindowPos", SetWindowPos},
            {"SetWindowSize", SetWindowSize},
            {"SetWindowCollapsed", SetWindowCollapsed},
            {"SetWindowFocus", SetWindowFocus},
            {"SetWindowFontScale", SetWindowFontScale},
            {"SetWindowPosStr", SetWindowPosStr},
            {"SetWindowSizeStr", SetWindowSizeStr},
            {"SetWindowCollapsedStr", SetWindowCollapsedStr},
            {"SetWindowFocusStr", SetWindowFocusStr},
            {"GetScrollX", GetScrollX},
            {"GetScrollY", GetScrollY},
            {"SetScrollX", SetScrollX},
            {"SetScrollY", SetScrollY},
            {"GetScrollMaxX", GetScrollMaxX},
            {"GetScrollMaxY", GetScrollMaxY},
            {"SetScrollHereX", SetScrollHereX},
            {"SetScrollHereY", SetScrollHereY},
            {"SetScrollFromPosX", SetScrollFromPosX},
            {"SetScrollFromPosY", SetScrollFromPosY},
            {"PushFont", PushFont},
            {"PopFont", PopFont},
            {"PushStyleColor", PushStyleColor},
            {"PushStyleColorImVec4", PushStyleColorImVec4},
            {"PopStyleColor", PopStyleColor},
            {"PopStyleColorEx", PopStyleColorEx},
            {"PushStyleVar", PushStyleVar},
            {"PushStyleVarImVec2", PushStyleVarImVec2},
            {"PopStyleVar", PopStyleVar},
            {"PopStyleVarEx", PopStyleVarEx},
            {"PushItemFlag", PushItemFlag},
            {"PopItemFlag", PopItemFlag},
            {"PushItemWidth", PushItemWidth},
            {"PopItemWidth", PopItemWidth},
            {"SetNextItemWidth", SetNextItemWidth},
            {"CalcItemWidth", CalcItemWidth},
            {"PushTextWrapPos", PushTextWrapPos},
            {"PopTextWrapPos", PopTextWrapPos},
            {"GetFont", GetFont},
            {"GetFontSize", GetFontSize},
            {"GetFontTexUvWhitePixel", GetFontTexUvWhitePixel},
            {"GetColorU32", GetColorU32},
            {"GetColorU32Ex", GetColorU32Ex},
            {"GetColorU32ImVec4", GetColorU32ImVec4},
            {"GetColorU32ImU32", GetColorU32ImU32},
            {"GetColorU32ImU32Ex", GetColorU32ImU32Ex},
            {"GetStyleColorVec4", GetStyleColorVec4},
            {"GetCursorScreenPos", GetCursorScreenPos},
            {"SetCursorScreenPos", SetCursorScreenPos},
            {"GetContentRegionAvail", GetContentRegionAvail},
            {"GetCursorPos", GetCursorPos},
            {"GetCursorPosX", GetCursorPosX},
            {"GetCursorPosY", GetCursorPosY},
            {"SetCursorPos", SetCursorPos},
            {"SetCursorPosX", SetCursorPosX},
            {"SetCursorPosY", SetCursorPosY},
            {"GetCursorStartPos", GetCursorStartPos},
            {"Separator", Separator},
            {"SameLine", SameLine},
            {"SameLineEx", SameLineEx},
            {"NewLine", NewLine},
            {"Spacing", Spacing},
            {"Dummy", Dummy},
            {"Indent", Indent},
            {"IndentEx", IndentEx},
            {"Unindent", Unindent},
            {"UnindentEx", UnindentEx},
            {"BeginGroup", BeginGroup},
            {"EndGroup", EndGroup},
            {"AlignTextToFramePadding", AlignTextToFramePadding},
            {"GetTextLineHeight", GetTextLineHeight},
            {"GetTextLineHeightWithSpacing", GetTextLineHeightWithSpacing},
            {"GetFrameHeight", GetFrameHeight},
            {"GetFrameHeightWithSpacing", GetFrameHeightWithSpacing},
            {"PushID", PushID},
            {"PushIDStr", PushIDStr},
            {"PushIDPtr", PushIDPtr},
            {"PushIDInt", PushIDInt},
            {"PopID", PopID},
            {"GetID", GetID},
            {"GetIDStr", GetIDStr},
            {"GetIDPtr", GetIDPtr},
            {"GetIDInt", GetIDInt},
            {"Text", Text},
            {"TextColored", TextColored},
            {"TextDisabled", TextDisabled},
            {"TextWrapped", TextWrapped},
            {"LabelText", LabelText},
            {"BulletText", BulletText},
            {"SeparatorText", SeparatorText},
            {"Button", Button},
            {"ButtonEx", ButtonEx},
            {"SmallButton", SmallButton},
            {"InvisibleButton", InvisibleButton},
            {"ArrowButton", ArrowButton},
            {"Checkbox", Checkbox},
            {"CheckboxFlagsIntPtr", CheckboxFlagsIntPtr},
            {"CheckboxFlagsUintPtr", CheckboxFlagsUintPtr},
            {"RadioButton", RadioButton},
            {"RadioButtonIntPtr", RadioButtonIntPtr},
            {"ProgressBar", ProgressBar},
            {"Bullet", Bullet},
            {"TextLink", TextLink},
            {"TextLinkOpenURL", TextLinkOpenURL},
            {"TextLinkOpenURLEx", TextLinkOpenURLEx},
            {"Image", Image},
            {"ImageEx", ImageEx},
            {"ImageButton", ImageButton},
            {"ImageButtonEx", ImageButtonEx},
            {"BeginCombo", BeginCombo},
            {"EndCombo", EndCombo},
            {"Combo", Combo},
            {"ComboEx", ComboEx},
            {"DragFloat", DragFloat},
            {"DragFloatEx", DragFloatEx},
            {"DragFloat2", DragFloat2},
            {"DragFloat2Ex", DragFloat2Ex},
            {"DragFloat3", DragFloat3},
            {"DragFloat3Ex", DragFloat3Ex},
            {"DragFloat4", DragFloat4},
            {"DragFloat4Ex", DragFloat4Ex},
            {"DragFloatRange2", DragFloatRange2},
            {"DragFloatRange2Ex", DragFloatRange2Ex},
            {"DragInt", DragInt},
            {"DragIntEx", DragIntEx},
            {"DragInt2", DragInt2},
            {"DragInt2Ex", DragInt2Ex},
            {"DragInt3", DragInt3},
            {"DragInt3Ex", DragInt3Ex},
            {"DragInt4", DragInt4},
            {"DragInt4Ex", DragInt4Ex},
            {"DragIntRange2", DragIntRange2},
            {"DragIntRange2Ex", DragIntRange2Ex},
            {"SliderFloat", SliderFloat},
            {"SliderFloatEx", SliderFloatEx},
            {"SliderFloat2", SliderFloat2},
            {"SliderFloat2Ex", SliderFloat2Ex},
            {"SliderFloat3", SliderFloat3},
            {"SliderFloat3Ex", SliderFloat3Ex},
            {"SliderFloat4", SliderFloat4},
            {"SliderFloat4Ex", SliderFloat4Ex},
            {"SliderAngle", SliderAngle},
            {"SliderAngleEx", SliderAngleEx},
            {"SliderInt", SliderInt},
            {"SliderIntEx", SliderIntEx},
            {"SliderInt2", SliderInt2},
            {"SliderInt2Ex", SliderInt2Ex},
            {"SliderInt3", SliderInt3},
            {"SliderInt3Ex", SliderInt3Ex},
            {"SliderInt4", SliderInt4},
            {"SliderInt4Ex", SliderInt4Ex},
            {"VSliderFloat", VSliderFloat},
            {"VSliderFloatEx", VSliderFloatEx},
            {"VSliderInt", VSliderInt},
            {"VSliderIntEx", VSliderIntEx},
            {"InputText", InputText},
            {"InputTextEx", InputTextEx},
            {"InputTextMultiline", InputTextMultiline},
            {"InputTextMultilineEx", InputTextMultilineEx},
            {"InputTextWithHint", InputTextWithHint},
            {"InputTextWithHintEx", InputTextWithHintEx},
            {"InputFloat", InputFloat},
            {"InputFloatEx", InputFloatEx},
            {"InputFloat2", InputFloat2},
            {"InputFloat2Ex", InputFloat2Ex},
            {"InputFloat3", InputFloat3},
            {"InputFloat3Ex", InputFloat3Ex},
            {"InputFloat4", InputFloat4},
            {"InputFloat4Ex", InputFloat4Ex},
            {"InputInt", InputInt},
            {"InputIntEx", InputIntEx},
            {"InputInt2", InputInt2},
            {"InputInt3", InputInt3},
            {"InputInt4", InputInt4},
            {"InputDouble", InputDouble},
            {"InputDoubleEx", InputDoubleEx},
            {"ColorEdit3", ColorEdit3},
            {"ColorEdit4", ColorEdit4},
            {"ColorPicker3", ColorPicker3},
            {"ColorButton", ColorButton},
            {"ColorButtonEx", ColorButtonEx},
            {"SetColorEditOptions", SetColorEditOptions},
            {"TreeNode", TreeNode},
            {"TreeNodeStr", TreeNodeStr},
            {"TreeNodePtr", TreeNodePtr},
            {"TreeNodeEx", TreeNodeEx},
            {"TreeNodeExStr", TreeNodeExStr},
            {"TreeNodeExPtr", TreeNodeExPtr},
            {"TreePush", TreePush},
            {"TreePushPtr", TreePushPtr},
            {"TreePop", TreePop},
            {"GetTreeNodeToLabelSpacing", GetTreeNodeToLabelSpacing},
            {"CollapsingHeader", CollapsingHeader},
            {"CollapsingHeaderBoolPtr", CollapsingHeaderBoolPtr},
            {"SetNextItemOpen", SetNextItemOpen},
            {"SetNextItemStorageID", SetNextItemStorageID},
            {"Selectable", Selectable},
            {"SelectableEx", SelectableEx},
            {"SelectableBoolPtr", SelectableBoolPtr},
            {"SelectableBoolPtrEx", SelectableBoolPtrEx},
            {"BeginMultiSelect", BeginMultiSelect},
            {"BeginMultiSelectEx", BeginMultiSelectEx},
            {"EndMultiSelect", EndMultiSelect},
            {"IsItemToggledSelection", IsItemToggledSelection},
            {"BeginListBox", BeginListBox},
            {"EndListBox", EndListBox},
            {"BeginMenuBar", BeginMenuBar},
            {"EndMenuBar", EndMenuBar},
            {"BeginMainMenuBar", BeginMainMenuBar},
            {"EndMainMenuBar", EndMainMenuBar},
            {"BeginMenu", BeginMenu},
            {"BeginMenuEx", BeginMenuEx},
            {"EndMenu", EndMenu},
            {"MenuItem", MenuItem},
            {"MenuItemEx", MenuItemEx},
            {"MenuItemBoolPtr", MenuItemBoolPtr},
            {"BeginTooltip", BeginTooltip},
            {"EndTooltip", EndTooltip},
            {"SetTooltip", SetTooltip},
            {"BeginItemTooltip", BeginItemTooltip},
            {"SetItemTooltip", SetItemTooltip},
            {"BeginPopup", BeginPopup},
            {"BeginPopupModal", BeginPopupModal},
            {"EndPopup", EndPopup},
            {"OpenPopup", OpenPopup},
            {"OpenPopupID", OpenPopupID},
            {"OpenPopupOnItemClick", OpenPopupOnItemClick},
            {"CloseCurrentPopup", CloseCurrentPopup},
            {"BeginPopupContextItem", BeginPopupContextItem},
            {"BeginPopupContextItemEx", BeginPopupContextItemEx},
            {"BeginPopupContextWindow", BeginPopupContextWindow},
            {"BeginPopupContextWindowEx", BeginPopupContextWindowEx},
            {"BeginPopupContextVoid", BeginPopupContextVoid},
            {"BeginPopupContextVoidEx", BeginPopupContextVoidEx},
            {"IsPopupOpen", IsPopupOpen},
            {"BeginTable", BeginTable},
            {"BeginTableEx", BeginTableEx},
            {"EndTable", EndTable},
            {"TableNextRow", TableNextRow},
            {"TableNextRowEx", TableNextRowEx},
            {"TableNextColumn", TableNextColumn},
            {"TableSetColumnIndex", TableSetColumnIndex},
            {"TableSetupColumn", TableSetupColumn},
            {"TableSetupColumnEx", TableSetupColumnEx},
            {"TableSetupScrollFreeze", TableSetupScrollFreeze},
            {"TableHeader", TableHeader},
            {"TableHeadersRow", TableHeadersRow},
            {"TableAngledHeadersRow", TableAngledHeadersRow},
            {"TableGetColumnCount", TableGetColumnCount},
            {"TableGetColumnIndex", TableGetColumnIndex},
            {"TableGetRowIndex", TableGetRowIndex},
            {"TableGetColumnName", TableGetColumnName},
            {"TableGetColumnFlags", TableGetColumnFlags},
            {"TableSetColumnEnabled", TableSetColumnEnabled},
            {"TableGetHoveredColumn", TableGetHoveredColumn},
            {"TableSetBgColor", TableSetBgColor},
            {"BeginTabBar", BeginTabBar},
            {"EndTabBar", EndTabBar},
            {"BeginTabItem", BeginTabItem},
            {"EndTabItem", EndTabItem},
            {"TabItemButton", TabItemButton},
            {"SetTabItemClosed", SetTabItemClosed},
            {"DockSpace", DockSpace},
            {"DockSpaceEx", DockSpaceEx},
            {"DockSpaceOverViewport", DockSpaceOverViewport},
            {"SetNextWindowDockID", SetNextWindowDockID},
            {"SetNextWindowClass", SetNextWindowClass},
            {"GetWindowDockID", GetWindowDockID},
            {"IsWindowDocked", IsWindowDocked},
            {"BeginDragDropSource", BeginDragDropSource},
            {"SetDragDropPayload", SetDragDropPayload},
            {"EndDragDropSource", EndDragDropSource},
            {"BeginDragDropTarget", BeginDragDropTarget},
            {"AcceptDragDropPayload", AcceptDragDropPayload},
            {"EndDragDropTarget", EndDragDropTarget},
            {"GetDragDropPayload", GetDragDropPayload},
            {"BeginDisabled", BeginDisabled},
            {"EndDisabled", EndDisabled},
            {"PushClipRect", PushClipRect},
            {"PopClipRect", PopClipRect},
            {"SetItemDefaultFocus", SetItemDefaultFocus},
            {"SetKeyboardFocusHere", SetKeyboardFocusHere},
            {"SetKeyboardFocusHereEx", SetKeyboardFocusHereEx},
            {"SetNextItemAllowOverlap", SetNextItemAllowOverlap},
            {"IsItemHovered", IsItemHovered},
            {"IsItemActive", IsItemActive},
            {"IsItemFocused", IsItemFocused},
            {"IsItemClicked", IsItemClicked},
            {"IsItemClickedEx", IsItemClickedEx},
            {"IsItemVisible", IsItemVisible},
            {"IsItemEdited", IsItemEdited},
            {"IsItemActivated", IsItemActivated},
            {"IsItemDeactivated", IsItemDeactivated},
            {"IsItemDeactivatedAfterEdit", IsItemDeactivatedAfterEdit},
            {"IsItemToggledOpen", IsItemToggledOpen},
            {"IsAnyItemHovered", IsAnyItemHovered},
            {"IsAnyItemActive", IsAnyItemActive},
            {"IsAnyItemFocused", IsAnyItemFocused},
            {"GetItemID", GetItemID},
            {"GetItemRectMin", GetItemRectMin},
            {"GetItemRectMax", GetItemRectMax},
            {"GetItemRectSize", GetItemRectSize},
            {"GetMainViewport", GetMainViewport},
            {"IsRectVisibleBySize", IsRectVisibleBySize},
            {"IsRectVisible", IsRectVisible},
            {"GetTime", GetTime},
            {"GetFrameCount", GetFrameCount},
            {"GetStyleColorName", GetStyleColorName},
            {"CalcTextSize", CalcTextSize},
            {"CalcTextSizeEx", CalcTextSizeEx},
            {"ColorConvertU32ToFloat4", ColorConvertU32ToFloat4},
            {"ColorConvertFloat4ToU32", ColorConvertFloat4ToU32},
            {"IsKeyDown", IsKeyDown},
            {"IsKeyPressed", IsKeyPressed},
            {"IsKeyPressedEx", IsKeyPressedEx},
            {"IsKeyReleased", IsKeyReleased},
            {"IsKeyChordPressed", IsKeyChordPressed},
            {"GetKeyPressedAmount", GetKeyPressedAmount},
            {"GetKeyName", GetKeyName},
            {"SetNextFrameWantCaptureKeyboard", SetNextFrameWantCaptureKeyboard},
            {"Shortcut", Shortcut},
            {"SetNextItemShortcut", SetNextItemShortcut},
            {"SetItemKeyOwner", SetItemKeyOwner},
            {"IsMouseDown", IsMouseDown},
            {"IsMouseClicked", IsMouseClicked},
            {"IsMouseClickedEx", IsMouseClickedEx},
            {"IsMouseReleased", IsMouseReleased},
            {"IsMouseDoubleClicked", IsMouseDoubleClicked},
            {"GetMouseClickedCount", GetMouseClickedCount},
            {"IsMouseHoveringRect", IsMouseHoveringRect},
            {"IsMouseHoveringRectEx", IsMouseHoveringRectEx},
            {"IsAnyMouseDown", IsAnyMouseDown},
            {"GetMousePos", GetMousePos},
            {"GetMousePosOnOpeningCurrentPopup", GetMousePosOnOpeningCurrentPopup},
            {"IsMouseDragging", IsMouseDragging},
            {"GetMouseDragDelta", GetMouseDragDelta},
            {"ResetMouseDragDelta", ResetMouseDragDelta},
            {"ResetMouseDragDeltaEx", ResetMouseDragDeltaEx},
            {"GetMouseCursor", GetMouseCursor},
            {"SetMouseCursor", SetMouseCursor},
            {"SetNextFrameWantCaptureMouse", SetNextFrameWantCaptureMouse},
            {"GetClipboardText", GetClipboardText},
            {"SetClipboardText", SetClipboardText},
            {"LoadIniSettingsFromDisk", LoadIniSettingsFromDisk},
            {"LoadIniSettingsFromMemory", LoadIniSettingsFromMemory},
            {"SaveIniSettingsToDisk", SaveIniSettingsToDisk},
            {"SaveIniSettingsToMemory", SaveIniSettingsToMemory},
            {"UpdatePlatformWindows", UpdatePlatformWindows},
            {"RenderPlatformWindowsDefault", RenderPlatformWindowsDefault},
            {"RenderPlatformWindowsDefaultEx", RenderPlatformWindowsDefaultEx},
            {"DestroyPlatformWindows", DestroyPlatformWindows},
            {"FindViewportByID", FindViewportByID},
            {"FindViewportByPlatformHandle", FindViewportByPlatformHandle},
            {NULL, NULL},
    };

#define GEN_FLAGS(name)                  \
    {                                    \
        #name, +[](lua_State* L) {       \
            util::create_table(L, name); \
            util::flags_gen(L, #name);   \
        }                                \
    }

    static util::TableAny flags[] = {
            GEN_FLAGS(WindowFlags),     GEN_FLAGS(ChildFlags),    GEN_FLAGS(ItemFlags),      GEN_FLAGS(InputTextFlags),   GEN_FLAGS(TreeNodeFlags), GEN_FLAGS(PopupFlags),
            GEN_FLAGS(SelectableFlags), GEN_FLAGS(ComboFlags),    GEN_FLAGS(TabBarFlags),    GEN_FLAGS(TabItemFlags),     GEN_FLAGS(FocusedFlags),  GEN_FLAGS(HoveredFlags),
            GEN_FLAGS(DockNodeFlags),   GEN_FLAGS(DragDropFlags), GEN_FLAGS(InputFlags),     GEN_FLAGS(ConfigFlags),      GEN_FLAGS(BackendFlags),  GEN_FLAGS(ButtonFlags),
            GEN_FLAGS(ColorEditFlags),  GEN_FLAGS(SliderFlags),   GEN_FLAGS(TableFlags),     GEN_FLAGS(TableColumnFlags), GEN_FLAGS(TableRowFlags), GEN_FLAGS(MultiSelectFlags),
            GEN_FLAGS(DrawFlags),       GEN_FLAGS(DrawListFlags), GEN_FLAGS(FontAtlasFlags), GEN_FLAGS(ViewportFlags),
    };
#undef GEN_FLAGS

#define GEN_ENUM(name)                                            \
    {                                                             \
        #name, +[](lua_State* L) { util::create_table(L, name); } \
    }

    static util::TableAny enums[] = {
            GEN_ENUM(DataType), GEN_ENUM(Dir),           GEN_ENUM(SortDirection),        GEN_ENUM(Key),         GEN_ENUM(Mod),
            GEN_ENUM(Col),      GEN_ENUM(StyleVar),      GEN_ENUM(MouseButton),          GEN_ENUM(MouseCursor), GEN_ENUM(MouseSource),
            GEN_ENUM(Cond),     GEN_ENUM(TableBgTarget), GEN_ENUM(SelectionRequestType),
    };
#undef GEN_ENUM

    util::init(L);
    lua_createtable(L, 0, sizeof(funcs) / sizeof(funcs[0]) - 1 + sizeof(flags) / sizeof(flags[0]) + sizeof(enums) / sizeof(enums[0]));
    luaL_setfuncs(L, funcs, 0);
    util::set_table(L, flags);
    util::set_table(L, enums);
    wrap_ImGuiContext::init(L);
    wrap_ImGuiIO::init(L);
    wrap_ImGuiInputTextCallbackData::init(L);
    wrap_ImGuiWindowClass::init(L);
    wrap_ImGuiStorage::init(L);
    wrap_ImGuiMultiSelectIO::init(L);
    wrap_ImGuiSelectionRequest::init(L);
    wrap_ImGuiSelectionExternalStorage::init(L);
    wrap_ImFontConfig::init(L);
    wrap_ImFontAtlas::init(L);
    wrap_ImGuiViewport::init(L);
}
}  // namespace Neko::imgui

int open_imgui(lua_State* L) {
    Neko::imgui::init(L);
    return 1;
}