-- Dear Imgui version: 1.88
local ffi = FFI

ffi.cdef [[
typedef struct FILE FILE;
typedef struct ImDrawChannel ImDrawChannel;
typedef struct ImDrawCmd ImDrawCmd;
typedef struct ImDrawData ImDrawData;
typedef struct ImDrawList ImDrawList;
typedef struct ImDrawListSharedData ImDrawListSharedData;
typedef struct ImDrawListSplitter ImDrawListSplitter;
typedef struct ImDrawVert ImDrawVert;
typedef struct ImFont ImFont;
typedef struct ImFontAtlas ImFontAtlas;
typedef struct ImFontBuilderIO ImFontBuilderIO;
typedef struct ImFontConfig ImFontConfig;
typedef struct ImFontGlyph ImFontGlyph;
typedef struct ImFontGlyphRangesBuilder ImFontGlyphRangesBuilder;
typedef struct ImColor ImColor;
typedef struct ImGuiContext ImGuiContext;
typedef struct ImGuiIO ImGuiIO;
typedef struct ImGuiInputTextCallbackData ImGuiInputTextCallbackData;
typedef struct ImGuiKeyData ImGuiKeyData;
typedef struct ImGuiListClipper ImGuiListClipper;
typedef struct ImGuiMultiSelectIO ImGuiMultiSelectIO;
typedef struct ImGuiOnceUponAFrame ImGuiOnceUponAFrame;
typedef struct ImGuiPayload ImGuiPayload;
typedef struct ImGuiPlatformIO ImGuiPlatformIO;
typedef struct ImGuiPlatformMonitor ImGuiPlatformMonitor;
typedef struct ImGuiPlatformImeData ImGuiPlatformImeData;
typedef struct ImGuiSelectionBasicStorage ImGuiSelectionBasicStorage;
typedef struct ImGuiSelectionExternalStorage ImGuiSelectionExternalStorage;
typedef struct ImGuiSelectionRequest ImGuiSelectionRequest;
typedef struct ImGuiSizeCallbackData ImGuiSizeCallbackData;
typedef struct ImGuiStorage ImGuiStorage;
typedef struct ImGuiStoragePair ImGuiStoragePair;
typedef struct ImGuiStyle ImGuiStyle;
typedef struct ImGuiTableSortSpecs ImGuiTableSortSpecs;
typedef struct ImGuiTableColumnSortSpecs ImGuiTableColumnSortSpecs;
typedef struct ImGuiTextBuffer ImGuiTextBuffer;
typedef struct ImGuiTextFilter ImGuiTextFilter;
typedef struct ImGuiViewport ImGuiViewport;
typedef struct ImGuiWindowClass ImGuiWindowClass;
typedef struct ImBitVector ImBitVector;
typedef struct ImRect ImRect;
typedef struct ImDrawDataBuilder ImDrawDataBuilder;
typedef struct ImGuiBoxSelectState ImGuiBoxSelectState;
typedef struct ImGuiColorMod ImGuiColorMod;
typedef struct ImGuiContextHook ImGuiContextHook;
typedef struct ImGuiDataVarInfo ImGuiDataVarInfo;
typedef struct ImGuiDataTypeInfo ImGuiDataTypeInfo;
typedef struct ImGuiDockContext ImGuiDockContext;
typedef struct ImGuiDockRequest ImGuiDockRequest;
typedef struct ImGuiDockNode ImGuiDockNode;
typedef struct ImGuiDockNodeSettings ImGuiDockNodeSettings;
typedef struct ImGuiGroupData ImGuiGroupData;
typedef struct ImGuiInputTextState ImGuiInputTextState;
typedef struct ImGuiInputTextDeactivateData ImGuiInputTextDeactivateData;
typedef struct ImGuiLastItemData ImGuiLastItemData;
typedef struct ImGuiLocEntry ImGuiLocEntry;
typedef struct ImGuiMenuColumns ImGuiMenuColumns;
typedef struct ImGuiMultiSelectState ImGuiMultiSelectState;
typedef struct ImGuiMultiSelectTempData ImGuiMultiSelectTempData;
typedef struct ImGuiNavItemData ImGuiNavItemData;
typedef struct ImGuiMetricsConfig ImGuiMetricsConfig;
typedef struct ImGuiNextWindowData ImGuiNextWindowData;
typedef struct ImGuiNextItemData ImGuiNextItemData;
typedef struct ImGuiOldColumnData ImGuiOldColumnData;
typedef struct ImGuiOldColumns ImGuiOldColumns;
typedef struct ImGuiPopupData ImGuiPopupData;
typedef struct ImGuiSettingsHandler ImGuiSettingsHandler;
typedef struct ImGuiStackSizes ImGuiStackSizes;
typedef struct ImGuiStyleMod ImGuiStyleMod;
typedef struct ImGuiTabBar ImGuiTabBar;
typedef struct ImGuiTabItem ImGuiTabItem;
typedef struct ImGuiTable ImGuiTable;
typedef struct ImGuiTableHeaderData ImGuiTableHeaderData;
typedef struct ImGuiTableColumn ImGuiTableColumn;
typedef struct ImGuiTableInstanceData ImGuiTableInstanceData;
typedef struct ImGuiTableTempData ImGuiTableTempData;
typedef struct ImGuiTableSettings ImGuiTableSettings;
typedef struct ImGuiTableColumnsSettings ImGuiTableColumnsSettings;
typedef struct ImGuiTreeNodeStackData ImGuiTreeNodeStackData;
typedef struct ImGuiTypingSelectState ImGuiTypingSelectState;
typedef struct ImGuiTypingSelectRequest ImGuiTypingSelectRequest;
typedef struct ImGuiWindow ImGuiWindow;
typedef struct ImGuiWindowDockStyle ImGuiWindowDockStyle;
typedef struct ImGuiWindowTempData ImGuiWindowTempData;
typedef struct ImGuiWindowSettings ImGuiWindowSettings;
typedef struct ImVector_const_charPtr {
    int Size;
    int Capacity;
    const char** Data;
} ImVector_const_charPtr;
typedef unsigned int ImGuiID;
typedef signed char ImS8;
typedef unsigned char ImU8;
typedef signed short ImS16;
typedef unsigned short ImU16;
typedef signed int ImS32;
typedef unsigned int ImU32;
typedef signed long long ImS64;
typedef unsigned long long ImU64;
struct ImDrawChannel;
struct ImDrawCmd;
struct ImDrawData;
struct ImDrawList;
struct ImDrawListSharedData;
struct ImDrawListSplitter;
struct ImDrawVert;
struct ImFont;
struct ImFontAtlas;
struct ImFontBuilderIO;
struct ImFontConfig;
struct ImFontGlyph;
struct ImFontGlyphRangesBuilder;
struct ImColor;
struct ImGuiContext;
struct ImGuiIO;
struct ImGuiInputTextCallbackData;
struct ImGuiKeyData;
struct ImGuiListClipper;
struct ImGuiMultiSelectIO;
struct ImGuiOnceUponAFrame;
struct ImGuiPayload;
struct ImGuiPlatformIO;
struct ImGuiPlatformMonitor;
struct ImGuiPlatformImeData;
struct ImGuiSelectionBasicStorage;
struct ImGuiSelectionExternalStorage;
struct ImGuiSelectionRequest;
struct ImGuiSizeCallbackData;
struct ImGuiStorage;
struct ImGuiStoragePair;
struct ImGuiStyle;
struct ImGuiTableSortSpecs;
struct ImGuiTableColumnSortSpecs;
struct ImGuiTextBuffer;
struct ImGuiTextFilter;
struct ImGuiViewport;
struct ImGuiWindowClass;
typedef int ImGuiCol;
typedef int ImGuiCond;
typedef int ImGuiDataType;
typedef int ImGuiMouseButton;
typedef int ImGuiMouseCursor;
typedef int ImGuiStyleVar;
typedef int ImGuiTableBgTarget;
typedef int ImDrawFlags;
typedef int ImDrawListFlags;
typedef int ImFontAtlasFlags;
typedef int ImGuiBackendFlags;
typedef int ImGuiButtonFlags;
typedef int ImGuiChildFlags;
typedef int ImGuiColorEditFlags;
typedef int ImGuiConfigFlags;
typedef int ImGuiComboFlags;
typedef int ImGuiDockNodeFlags;
typedef int ImGuiDragDropFlags;
typedef int ImGuiFocusedFlags;
typedef int ImGuiHoveredFlags;
typedef int ImGuiInputFlags;
typedef int ImGuiInputTextFlags;
typedef int ImGuiItemFlags;
typedef int ImGuiKeyChord;
typedef int ImGuiPopupFlags;
typedef int ImGuiMultiSelectFlags;
typedef int ImGuiSelectableFlags;
typedef int ImGuiSliderFlags;
typedef int ImGuiTabBarFlags;
typedef int ImGuiTabItemFlags;
typedef int ImGuiTableFlags;
typedef int ImGuiTableColumnFlags;
typedef int ImGuiTableRowFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImGuiViewportFlags;
typedef int ImGuiWindowFlags;
typedef void* ImTextureID;
typedef unsigned short ImDrawIdx;
typedef unsigned int ImWchar32;
typedef unsigned short ImWchar16;
typedef ImWchar16 ImWchar;
typedef ImS64 ImGuiSelectionUserData;
typedef int (*ImGuiInputTextCallback)(ImGuiInputTextCallbackData* data);
typedef void (*ImGuiSizeCallback)(ImGuiSizeCallbackData* data);
typedef void* (*ImGuiMemAllocFunc)(size_t sz, void* user_data);
typedef void (*ImGuiMemFreeFunc)(void* ptr, void* user_data);
typedef struct ImVec2 ImVec2;
struct ImVec2 {
    float x, y;
};
typedef struct ImVec4 ImVec4;
struct ImVec4 {
    float x, y, z, w;
};
typedef enum {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoTitleBar = 1 << 0,
    ImGuiWindowFlags_NoResize = 1 << 1,
    ImGuiWindowFlags_NoMove = 1 << 2,
    ImGuiWindowFlags_NoScrollbar = 1 << 3,
    ImGuiWindowFlags_NoScrollWithMouse = 1 << 4,
    ImGuiWindowFlags_NoCollapse = 1 << 5,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,
    ImGuiWindowFlags_NoBackground = 1 << 7,
    ImGuiWindowFlags_NoSavedSettings = 1 << 8,
    ImGuiWindowFlags_NoMouseInputs = 1 << 9,
    ImGuiWindowFlags_MenuBar = 1 << 10,
    ImGuiWindowFlags_HorizontalScrollbar = 1 << 11,
    ImGuiWindowFlags_NoFocusOnAppearing = 1 << 12,
    ImGuiWindowFlags_NoBringToFrontOnFocus = 1 << 13,
    ImGuiWindowFlags_AlwaysVerticalScrollbar = 1 << 14,
    ImGuiWindowFlags_AlwaysHorizontalScrollbar = 1 << 15,
    ImGuiWindowFlags_NoNavInputs = 1 << 16,
    ImGuiWindowFlags_NoNavFocus = 1 << 17,
    ImGuiWindowFlags_UnsavedDocument = 1 << 18,
    ImGuiWindowFlags_NoDocking = 1 << 19,
    ImGuiWindowFlags_NoNav = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
    ImGuiWindowFlags_NoDecoration = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse,
    ImGuiWindowFlags_NoInputs = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
    ImGuiWindowFlags_ChildWindow = 1 << 24,
    ImGuiWindowFlags_Tooltip = 1 << 25,
    ImGuiWindowFlags_Popup = 1 << 26,
    ImGuiWindowFlags_Modal = 1 << 27,
    ImGuiWindowFlags_ChildMenu = 1 << 28,
    ImGuiWindowFlags_DockNodeHost = 1 << 29,
} ImGuiWindowFlags_;
typedef enum {
    ImGuiChildFlags_None = 0,
    ImGuiChildFlags_Border = 1 << 0,
    ImGuiChildFlags_AlwaysUseWindowPadding = 1 << 1,
    ImGuiChildFlags_ResizeX = 1 << 2,
    ImGuiChildFlags_ResizeY = 1 << 3,
    ImGuiChildFlags_AutoResizeX = 1 << 4,
    ImGuiChildFlags_AutoResizeY = 1 << 5,
    ImGuiChildFlags_AlwaysAutoResize = 1 << 6,
    ImGuiChildFlags_FrameStyle = 1 << 7,
    ImGuiChildFlags_NavFlattened = 1 << 8,
} ImGuiChildFlags_;
typedef enum {
    ImGuiItemFlags_None = 0,
    ImGuiItemFlags_NoTabStop = 1 << 0,
    ImGuiItemFlags_NoNav = 1 << 1,
    ImGuiItemFlags_NoNavDefaultFocus = 1 << 2,
    ImGuiItemFlags_ButtonRepeat = 1 << 3,
    ImGuiItemFlags_AutoClosePopups = 1 << 4,
} ImGuiItemFlags_;
typedef enum {
    ImGuiInputTextFlags_None = 0,
    ImGuiInputTextFlags_CharsDecimal = 1 << 0,
    ImGuiInputTextFlags_CharsHexadecimal = 1 << 1,
    ImGuiInputTextFlags_CharsScientific = 1 << 2,
    ImGuiInputTextFlags_CharsUppercase = 1 << 3,
    ImGuiInputTextFlags_CharsNoBlank = 1 << 4,
    ImGuiInputTextFlags_AllowTabInput = 1 << 5,
    ImGuiInputTextFlags_EnterReturnsTrue = 1 << 6,
    ImGuiInputTextFlags_EscapeClearsAll = 1 << 7,
    ImGuiInputTextFlags_CtrlEnterForNewLine = 1 << 8,
    ImGuiInputTextFlags_ReadOnly = 1 << 9,
    ImGuiInputTextFlags_Password = 1 << 10,
    ImGuiInputTextFlags_AlwaysOverwrite = 1 << 11,
    ImGuiInputTextFlags_AutoSelectAll = 1 << 12,
    ImGuiInputTextFlags_ParseEmptyRefVal = 1 << 13,
    ImGuiInputTextFlags_DisplayEmptyRefVal = 1 << 14,
    ImGuiInputTextFlags_NoHorizontalScroll = 1 << 15,
    ImGuiInputTextFlags_NoUndoRedo = 1 << 16,
    ImGuiInputTextFlags_CallbackCompletion = 1 << 17,
    ImGuiInputTextFlags_CallbackHistory = 1 << 18,
    ImGuiInputTextFlags_CallbackAlways = 1 << 19,
    ImGuiInputTextFlags_CallbackCharFilter = 1 << 20,
    ImGuiInputTextFlags_CallbackResize = 1 << 21,
    ImGuiInputTextFlags_CallbackEdit = 1 << 22,
} ImGuiInputTextFlags_;
typedef enum {
    ImGuiTreeNodeFlags_None = 0,
    ImGuiTreeNodeFlags_Selected = 1 << 0,
    ImGuiTreeNodeFlags_Framed = 1 << 1,
    ImGuiTreeNodeFlags_AllowOverlap = 1 << 2,
    ImGuiTreeNodeFlags_NoTreePushOnOpen = 1 << 3,
    ImGuiTreeNodeFlags_NoAutoOpenOnLog = 1 << 4,
    ImGuiTreeNodeFlags_DefaultOpen = 1 << 5,
    ImGuiTreeNodeFlags_OpenOnDoubleClick = 1 << 6,
    ImGuiTreeNodeFlags_OpenOnArrow = 1 << 7,
    ImGuiTreeNodeFlags_Leaf = 1 << 8,
    ImGuiTreeNodeFlags_Bullet = 1 << 9,
    ImGuiTreeNodeFlags_FramePadding = 1 << 10,
    ImGuiTreeNodeFlags_SpanAvailWidth = 1 << 11,
    ImGuiTreeNodeFlags_SpanFullWidth = 1 << 12,
    ImGuiTreeNodeFlags_SpanTextWidth = 1 << 13,
    ImGuiTreeNodeFlags_SpanAllColumns = 1 << 14,
    ImGuiTreeNodeFlags_NavLeftJumpsBackHere = 1 << 15,
    ImGuiTreeNodeFlags_CollapsingHeader = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_NoAutoOpenOnLog,
} ImGuiTreeNodeFlags_;
typedef enum {
    ImGuiPopupFlags_None = 0,
    ImGuiPopupFlags_MouseButtonLeft = 0,
    ImGuiPopupFlags_MouseButtonRight = 1,
    ImGuiPopupFlags_MouseButtonMiddle = 2,
    ImGuiPopupFlags_MouseButtonMask_ = 0x1F,
    ImGuiPopupFlags_MouseButtonDefault_ = 1,
    ImGuiPopupFlags_NoReopen = 1 << 5,
    ImGuiPopupFlags_NoOpenOverExistingPopup = 1 << 7,
    ImGuiPopupFlags_NoOpenOverItems = 1 << 8,
    ImGuiPopupFlags_AnyPopupId = 1 << 10,
    ImGuiPopupFlags_AnyPopupLevel = 1 << 11,
    ImGuiPopupFlags_AnyPopup = ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel,
} ImGuiPopupFlags_;
typedef enum {
    ImGuiSelectableFlags_None = 0,
    ImGuiSelectableFlags_NoAutoClosePopups = 1 << 0,
    ImGuiSelectableFlags_SpanAllColumns = 1 << 1,
    ImGuiSelectableFlags_AllowDoubleClick = 1 << 2,
    ImGuiSelectableFlags_Disabled = 1 << 3,
    ImGuiSelectableFlags_AllowOverlap = 1 << 4,
    ImGuiSelectableFlags_Highlight = 1 << 5,
} ImGuiSelectableFlags_;
typedef enum {
    ImGuiComboFlags_None = 0,
    ImGuiComboFlags_PopupAlignLeft = 1 << 0,
    ImGuiComboFlags_HeightSmall = 1 << 1,
    ImGuiComboFlags_HeightRegular = 1 << 2,
    ImGuiComboFlags_HeightLarge = 1 << 3,
    ImGuiComboFlags_HeightLargest = 1 << 4,
    ImGuiComboFlags_NoArrowButton = 1 << 5,
    ImGuiComboFlags_NoPreview = 1 << 6,
    ImGuiComboFlags_WidthFitPreview = 1 << 7,
    ImGuiComboFlags_HeightMask_ = ImGuiComboFlags_HeightSmall | ImGuiComboFlags_HeightRegular | ImGuiComboFlags_HeightLarge | ImGuiComboFlags_HeightLargest,
} ImGuiComboFlags_;
typedef enum {
    ImGuiTabBarFlags_None = 0,
    ImGuiTabBarFlags_Reorderable = 1 << 0,
    ImGuiTabBarFlags_AutoSelectNewTabs = 1 << 1,
    ImGuiTabBarFlags_TabListPopupButton = 1 << 2,
    ImGuiTabBarFlags_NoCloseWithMiddleMouseButton = 1 << 3,
    ImGuiTabBarFlags_NoTabListScrollingButtons = 1 << 4,
    ImGuiTabBarFlags_NoTooltip = 1 << 5,
    ImGuiTabBarFlags_DrawSelectedOverline = 1 << 6,
    ImGuiTabBarFlags_FittingPolicyResizeDown = 1 << 7,
    ImGuiTabBarFlags_FittingPolicyScroll = 1 << 8,
    ImGuiTabBarFlags_FittingPolicyMask_ = ImGuiTabBarFlags_FittingPolicyResizeDown | ImGuiTabBarFlags_FittingPolicyScroll,
    ImGuiTabBarFlags_FittingPolicyDefault_ = ImGuiTabBarFlags_FittingPolicyResizeDown,
} ImGuiTabBarFlags_;
typedef enum {
    ImGuiTabItemFlags_None = 0,
    ImGuiTabItemFlags_UnsavedDocument = 1 << 0,
    ImGuiTabItemFlags_SetSelected = 1 << 1,
    ImGuiTabItemFlags_NoCloseWithMiddleMouseButton = 1 << 2,
    ImGuiTabItemFlags_NoPushId = 1 << 3,
    ImGuiTabItemFlags_NoTooltip = 1 << 4,
    ImGuiTabItemFlags_NoReorder = 1 << 5,
    ImGuiTabItemFlags_Leading = 1 << 6,
    ImGuiTabItemFlags_Trailing = 1 << 7,
    ImGuiTabItemFlags_NoAssumedClosure = 1 << 8,
} ImGuiTabItemFlags_;
typedef enum {
    ImGuiFocusedFlags_None = 0,
    ImGuiFocusedFlags_ChildWindows = 1 << 0,
    ImGuiFocusedFlags_RootWindow = 1 << 1,
    ImGuiFocusedFlags_AnyWindow = 1 << 2,
    ImGuiFocusedFlags_NoPopupHierarchy = 1 << 3,
    ImGuiFocusedFlags_DockHierarchy = 1 << 4,
    ImGuiFocusedFlags_RootAndChildWindows = ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows,
} ImGuiFocusedFlags_;
typedef enum {
    ImGuiHoveredFlags_None = 0,
    ImGuiHoveredFlags_ChildWindows = 1 << 0,
    ImGuiHoveredFlags_RootWindow = 1 << 1,
    ImGuiHoveredFlags_AnyWindow = 1 << 2,
    ImGuiHoveredFlags_NoPopupHierarchy = 1 << 3,
    ImGuiHoveredFlags_DockHierarchy = 1 << 4,
    ImGuiHoveredFlags_AllowWhenBlockedByPopup = 1 << 5,
    ImGuiHoveredFlags_AllowWhenBlockedByActiveItem = 1 << 7,
    ImGuiHoveredFlags_AllowWhenOverlappedByItem = 1 << 8,
    ImGuiHoveredFlags_AllowWhenOverlappedByWindow = 1 << 9,
    ImGuiHoveredFlags_AllowWhenDisabled = 1 << 10,
    ImGuiHoveredFlags_NoNavOverride = 1 << 11,
    ImGuiHoveredFlags_AllowWhenOverlapped = ImGuiHoveredFlags_AllowWhenOverlappedByItem | ImGuiHoveredFlags_AllowWhenOverlappedByWindow,
    ImGuiHoveredFlags_RectOnly = ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped,
    ImGuiHoveredFlags_RootAndChildWindows = ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows,
    ImGuiHoveredFlags_ForTooltip = 1 << 12,
    ImGuiHoveredFlags_Stationary = 1 << 13,
    ImGuiHoveredFlags_DelayNone = 1 << 14,
    ImGuiHoveredFlags_DelayShort = 1 << 15,
    ImGuiHoveredFlags_DelayNormal = 1 << 16,
    ImGuiHoveredFlags_NoSharedDelay = 1 << 17,
} ImGuiHoveredFlags_;
typedef enum {
    ImGuiDockNodeFlags_None = 0,
    ImGuiDockNodeFlags_KeepAliveOnly = 1 << 0,
    ImGuiDockNodeFlags_NoDockingOverCentralNode = 1 << 2,
    ImGuiDockNodeFlags_PassthruCentralNode = 1 << 3,
    ImGuiDockNodeFlags_NoDockingSplit = 1 << 4,
    ImGuiDockNodeFlags_NoResize = 1 << 5,
    ImGuiDockNodeFlags_AutoHideTabBar = 1 << 6,
    ImGuiDockNodeFlags_NoUndocking = 1 << 7,
} ImGuiDockNodeFlags_;
typedef enum {
    ImGuiDragDropFlags_None = 0,
    ImGuiDragDropFlags_SourceNoPreviewTooltip = 1 << 0,
    ImGuiDragDropFlags_SourceNoDisableHover = 1 << 1,
    ImGuiDragDropFlags_SourceNoHoldToOpenOthers = 1 << 2,
    ImGuiDragDropFlags_SourceAllowNullID = 1 << 3,
    ImGuiDragDropFlags_SourceExtern = 1 << 4,
    ImGuiDragDropFlags_PayloadAutoExpire = 1 << 5,
    ImGuiDragDropFlags_PayloadNoCrossContext = 1 << 6,
    ImGuiDragDropFlags_PayloadNoCrossProcess = 1 << 7,
    ImGuiDragDropFlags_AcceptBeforeDelivery = 1 << 10,
    ImGuiDragDropFlags_AcceptNoDrawDefaultRect = 1 << 11,
    ImGuiDragDropFlags_AcceptNoPreviewTooltip = 1 << 12,
    ImGuiDragDropFlags_AcceptPeekOnly = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect,
} ImGuiDragDropFlags_;
typedef enum {
    ImGuiDataType_S8,
    ImGuiDataType_U8,
    ImGuiDataType_S16,
    ImGuiDataType_U16,
    ImGuiDataType_S32,
    ImGuiDataType_U32,
    ImGuiDataType_S64,
    ImGuiDataType_U64,
    ImGuiDataType_Float,
    ImGuiDataType_Double,
    ImGuiDataType_Bool,
    ImGuiDataType_COUNT
} ImGuiDataType_;
typedef enum {
    ImGuiDir_None = -1,
    ImGuiDir_Left = 0,
    ImGuiDir_Right = 1,
    ImGuiDir_Up = 2,
    ImGuiDir_Down = 3,
    ImGuiDir_COUNT = 4,
} ImGuiDir;
typedef enum {
    ImGuiSortDirection_None = 0,
    ImGuiSortDirection_Ascending = 1,
    ImGuiSortDirection_Descending = 2,
} ImGuiSortDirection;
typedef enum {
    ImGuiKey_None = 0,
    ImGuiKey_Tab = 512,
    ImGuiKey_LeftArrow = 513,
    ImGuiKey_RightArrow = 514,
    ImGuiKey_UpArrow = 515,
    ImGuiKey_DownArrow = 516,
    ImGuiKey_PageUp = 517,
    ImGuiKey_PageDown = 518,
    ImGuiKey_Home = 519,
    ImGuiKey_End = 520,
    ImGuiKey_Insert = 521,
    ImGuiKey_Delete = 522,
    ImGuiKey_Backspace = 523,
    ImGuiKey_Space = 524,
    ImGuiKey_Enter = 525,
    ImGuiKey_Escape = 526,
    ImGuiKey_LeftCtrl = 527,
    ImGuiKey_LeftShift = 528,
    ImGuiKey_LeftAlt = 529,
    ImGuiKey_LeftSuper = 530,
    ImGuiKey_RightCtrl = 531,
    ImGuiKey_RightShift = 532,
    ImGuiKey_RightAlt = 533,
    ImGuiKey_RightSuper = 534,
    ImGuiKey_Menu = 535,
    ImGuiKey_0 = 536,
    ImGuiKey_1 = 537,
    ImGuiKey_2 = 538,
    ImGuiKey_3 = 539,
    ImGuiKey_4 = 540,
    ImGuiKey_5 = 541,
    ImGuiKey_6 = 542,
    ImGuiKey_7 = 543,
    ImGuiKey_8 = 544,
    ImGuiKey_9 = 545,
    ImGuiKey_A = 546,
    ImGuiKey_B = 547,
    ImGuiKey_C = 548,
    ImGuiKey_D = 549,
    ImGuiKey_E = 550,
    ImGuiKey_F = 551,
    ImGuiKey_G = 552,
    ImGuiKey_H = 553,
    ImGuiKey_I = 554,
    ImGuiKey_J = 555,
    ImGuiKey_K = 556,
    ImGuiKey_L = 557,
    ImGuiKey_M = 558,
    ImGuiKey_N = 559,
    ImGuiKey_O = 560,
    ImGuiKey_P = 561,
    ImGuiKey_Q = 562,
    ImGuiKey_R = 563,
    ImGuiKey_S = 564,
    ImGuiKey_T = 565,
    ImGuiKey_U = 566,
    ImGuiKey_V = 567,
    ImGuiKey_W = 568,
    ImGuiKey_X = 569,
    ImGuiKey_Y = 570,
    ImGuiKey_Z = 571,
    ImGuiKey_F1 = 572,
    ImGuiKey_F2 = 573,
    ImGuiKey_F3 = 574,
    ImGuiKey_F4 = 575,
    ImGuiKey_F5 = 576,
    ImGuiKey_F6 = 577,
    ImGuiKey_F7 = 578,
    ImGuiKey_F8 = 579,
    ImGuiKey_F9 = 580,
    ImGuiKey_F10 = 581,
    ImGuiKey_F11 = 582,
    ImGuiKey_F12 = 583,
    ImGuiKey_F13 = 584,
    ImGuiKey_F14 = 585,
    ImGuiKey_F15 = 586,
    ImGuiKey_F16 = 587,
    ImGuiKey_F17 = 588,
    ImGuiKey_F18 = 589,
    ImGuiKey_F19 = 590,
    ImGuiKey_F20 = 591,
    ImGuiKey_F21 = 592,
    ImGuiKey_F22 = 593,
    ImGuiKey_F23 = 594,
    ImGuiKey_F24 = 595,
    ImGuiKey_Apostrophe = 596,
    ImGuiKey_Comma = 597,
    ImGuiKey_Minus = 598,
    ImGuiKey_Period = 599,
    ImGuiKey_Slash = 600,
    ImGuiKey_Semicolon = 601,
    ImGuiKey_Equal = 602,
    ImGuiKey_LeftBracket = 603,
    ImGuiKey_Backslash = 604,
    ImGuiKey_RightBracket = 605,
    ImGuiKey_GraveAccent = 606,
    ImGuiKey_CapsLock = 607,
    ImGuiKey_ScrollLock = 608,
    ImGuiKey_NumLock = 609,
    ImGuiKey_PrintScreen = 610,
    ImGuiKey_Pause = 611,
    ImGuiKey_Keypad0 = 612,
    ImGuiKey_Keypad1 = 613,
    ImGuiKey_Keypad2 = 614,
    ImGuiKey_Keypad3 = 615,
    ImGuiKey_Keypad4 = 616,
    ImGuiKey_Keypad5 = 617,
    ImGuiKey_Keypad6 = 618,
    ImGuiKey_Keypad7 = 619,
    ImGuiKey_Keypad8 = 620,
    ImGuiKey_Keypad9 = 621,
    ImGuiKey_KeypadDecimal = 622,
    ImGuiKey_KeypadDivide = 623,
    ImGuiKey_KeypadMultiply = 624,
    ImGuiKey_KeypadSubtract = 625,
    ImGuiKey_KeypadAdd = 626,
    ImGuiKey_KeypadEnter = 627,
    ImGuiKey_KeypadEqual = 628,
    ImGuiKey_AppBack = 629,
    ImGuiKey_AppForward = 630,
    ImGuiKey_GamepadStart = 631,
    ImGuiKey_GamepadBack = 632,
    ImGuiKey_GamepadFaceLeft = 633,
    ImGuiKey_GamepadFaceRight = 634,
    ImGuiKey_GamepadFaceUp = 635,
    ImGuiKey_GamepadFaceDown = 636,
    ImGuiKey_GamepadDpadLeft = 637,
    ImGuiKey_GamepadDpadRight = 638,
    ImGuiKey_GamepadDpadUp = 639,
    ImGuiKey_GamepadDpadDown = 640,
    ImGuiKey_GamepadL1 = 641,
    ImGuiKey_GamepadR1 = 642,
    ImGuiKey_GamepadL2 = 643,
    ImGuiKey_GamepadR2 = 644,
    ImGuiKey_GamepadL3 = 645,
    ImGuiKey_GamepadR3 = 646,
    ImGuiKey_GamepadLStickLeft = 647,
    ImGuiKey_GamepadLStickRight = 648,
    ImGuiKey_GamepadLStickUp = 649,
    ImGuiKey_GamepadLStickDown = 650,
    ImGuiKey_GamepadRStickLeft = 651,
    ImGuiKey_GamepadRStickRight = 652,
    ImGuiKey_GamepadRStickUp = 653,
    ImGuiKey_GamepadRStickDown = 654,
    ImGuiKey_MouseLeft = 655,
    ImGuiKey_MouseRight = 656,
    ImGuiKey_MouseMiddle = 657,
    ImGuiKey_MouseX1 = 658,
    ImGuiKey_MouseX2 = 659,
    ImGuiKey_MouseWheelX = 660,
    ImGuiKey_MouseWheelY = 661,
    ImGuiKey_ReservedForModCtrl = 662,
    ImGuiKey_ReservedForModShift = 663,
    ImGuiKey_ReservedForModAlt = 664,
    ImGuiKey_ReservedForModSuper = 665,
    ImGuiKey_COUNT = 666,
    ImGuiMod_None = 0,
    ImGuiMod_Ctrl = 1 << 12,
    ImGuiMod_Shift = 1 << 13,
    ImGuiMod_Alt = 1 << 14,
    ImGuiMod_Super = 1 << 15,
    ImGuiMod_Mask_ = 0xF000,
    ImGuiKey_NamedKey_BEGIN = 512,
    ImGuiKey_NamedKey_END = ImGuiKey_COUNT,
    ImGuiKey_NamedKey_COUNT = ImGuiKey_NamedKey_END - ImGuiKey_NamedKey_BEGIN,
    ImGuiKey_KeysData_SIZE = ImGuiKey_NamedKey_COUNT,
    ImGuiKey_KeysData_OFFSET = ImGuiKey_NamedKey_BEGIN,
} ImGuiKey;
typedef enum {
    ImGuiInputFlags_None = 0,
    ImGuiInputFlags_Repeat = 1 << 0,
    ImGuiInputFlags_RouteActive = 1 << 10,
    ImGuiInputFlags_RouteFocused = 1 << 11,
    ImGuiInputFlags_RouteGlobal = 1 << 12,
    ImGuiInputFlags_RouteAlways = 1 << 13,
    ImGuiInputFlags_RouteOverFocused = 1 << 14,
    ImGuiInputFlags_RouteOverActive = 1 << 15,
    ImGuiInputFlags_RouteUnlessBgFocused = 1 << 16,
    ImGuiInputFlags_RouteFromRootWindow = 1 << 17,
    ImGuiInputFlags_Tooltip = 1 << 18,
} ImGuiInputFlags_;
typedef enum {
    ImGuiConfigFlags_None = 0,
    ImGuiConfigFlags_NavEnableKeyboard = 1 << 0,
    ImGuiConfigFlags_NavEnableGamepad = 1 << 1,
    ImGuiConfigFlags_NavEnableSetMousePos = 1 << 2,
    ImGuiConfigFlags_NavNoCaptureKeyboard = 1 << 3,
    ImGuiConfigFlags_NoMouse = 1 << 4,
    ImGuiConfigFlags_NoMouseCursorChange = 1 << 5,
    ImGuiConfigFlags_NoKeyboard = 1 << 6,
    ImGuiConfigFlags_DockingEnable = 1 << 7,
    ImGuiConfigFlags_ViewportsEnable = 1 << 10,
    ImGuiConfigFlags_DpiEnableScaleViewports = 1 << 14,
    ImGuiConfigFlags_DpiEnableScaleFonts = 1 << 15,
    ImGuiConfigFlags_IsSRGB = 1 << 20,
    ImGuiConfigFlags_IsTouchScreen = 1 << 21,
} ImGuiConfigFlags_;
typedef enum {
    ImGuiBackendFlags_None = 0,
    ImGuiBackendFlags_HasGamepad = 1 << 0,
    ImGuiBackendFlags_HasMouseCursors = 1 << 1,
    ImGuiBackendFlags_HasSetMousePos = 1 << 2,
    ImGuiBackendFlags_RendererHasVtxOffset = 1 << 3,
    ImGuiBackendFlags_PlatformHasViewports = 1 << 10,
    ImGuiBackendFlags_HasMouseHoveredViewport = 1 << 11,
    ImGuiBackendFlags_RendererHasViewports = 1 << 12,
} ImGuiBackendFlags_;
typedef enum {
    ImGuiCol_Text,
    ImGuiCol_TextDisabled,
    ImGuiCol_WindowBg,
    ImGuiCol_ChildBg,
    ImGuiCol_PopupBg,
    ImGuiCol_Border,
    ImGuiCol_BorderShadow,
    ImGuiCol_FrameBg,
    ImGuiCol_FrameBgHovered,
    ImGuiCol_FrameBgActive,
    ImGuiCol_TitleBg,
    ImGuiCol_TitleBgActive,
    ImGuiCol_TitleBgCollapsed,
    ImGuiCol_MenuBarBg,
    ImGuiCol_ScrollbarBg,
    ImGuiCol_ScrollbarGrab,
    ImGuiCol_ScrollbarGrabHovered,
    ImGuiCol_ScrollbarGrabActive,
    ImGuiCol_CheckMark,
    ImGuiCol_SliderGrab,
    ImGuiCol_SliderGrabActive,
    ImGuiCol_Button,
    ImGuiCol_ButtonHovered,
    ImGuiCol_ButtonActive,
    ImGuiCol_Header,
    ImGuiCol_HeaderHovered,
    ImGuiCol_HeaderActive,
    ImGuiCol_Separator,
    ImGuiCol_SeparatorHovered,
    ImGuiCol_SeparatorActive,
    ImGuiCol_ResizeGrip,
    ImGuiCol_ResizeGripHovered,
    ImGuiCol_ResizeGripActive,
    ImGuiCol_TabHovered,
    ImGuiCol_Tab,
    ImGuiCol_TabSelected,
    ImGuiCol_TabSelectedOverline,
    ImGuiCol_TabDimmed,
    ImGuiCol_TabDimmedSelected,
    ImGuiCol_TabDimmedSelectedOverline,
    ImGuiCol_DockingPreview,
    ImGuiCol_DockingEmptyBg,
    ImGuiCol_PlotLines,
    ImGuiCol_PlotLinesHovered,
    ImGuiCol_PlotHistogram,
    ImGuiCol_PlotHistogramHovered,
    ImGuiCol_TableHeaderBg,
    ImGuiCol_TableBorderStrong,
    ImGuiCol_TableBorderLight,
    ImGuiCol_TableRowBg,
    ImGuiCol_TableRowBgAlt,
    ImGuiCol_TextLink,
    ImGuiCol_TextSelectedBg,
    ImGuiCol_DragDropTarget,
    ImGuiCol_NavHighlight,
    ImGuiCol_NavWindowingHighlight,
    ImGuiCol_NavWindowingDimBg,
    ImGuiCol_ModalWindowDimBg,
    ImGuiCol_COUNT,
} ImGuiCol_;
typedef enum {
    ImGuiStyleVar_Alpha,
    ImGuiStyleVar_DisabledAlpha,
    ImGuiStyleVar_WindowPadding,
    ImGuiStyleVar_WindowRounding,
    ImGuiStyleVar_WindowBorderSize,
    ImGuiStyleVar_WindowMinSize,
    ImGuiStyleVar_WindowTitleAlign,
    ImGuiStyleVar_ChildRounding,
    ImGuiStyleVar_ChildBorderSize,
    ImGuiStyleVar_PopupRounding,
    ImGuiStyleVar_PopupBorderSize,
    ImGuiStyleVar_FramePadding,
    ImGuiStyleVar_FrameRounding,
    ImGuiStyleVar_FrameBorderSize,
    ImGuiStyleVar_ItemSpacing,
    ImGuiStyleVar_ItemInnerSpacing,
    ImGuiStyleVar_IndentSpacing,
    ImGuiStyleVar_CellPadding,
    ImGuiStyleVar_ScrollbarSize,
    ImGuiStyleVar_ScrollbarRounding,
    ImGuiStyleVar_GrabMinSize,
    ImGuiStyleVar_GrabRounding,
    ImGuiStyleVar_TabRounding,
    ImGuiStyleVar_TabBorderSize,
    ImGuiStyleVar_TabBarBorderSize,
    ImGuiStyleVar_TabBarOverlineSize,
    ImGuiStyleVar_TableAngledHeadersAngle,
    ImGuiStyleVar_TableAngledHeadersTextAlign,
    ImGuiStyleVar_ButtonTextAlign,
    ImGuiStyleVar_SelectableTextAlign,
    ImGuiStyleVar_SeparatorTextBorderSize,
    ImGuiStyleVar_SeparatorTextAlign,
    ImGuiStyleVar_SeparatorTextPadding,
    ImGuiStyleVar_DockingSeparatorSize,
    ImGuiStyleVar_COUNT
} ImGuiStyleVar_;
typedef enum {
    ImGuiButtonFlags_None = 0,
    ImGuiButtonFlags_MouseButtonLeft = 1 << 0,
    ImGuiButtonFlags_MouseButtonRight = 1 << 1,
    ImGuiButtonFlags_MouseButtonMiddle = 1 << 2,
    ImGuiButtonFlags_MouseButtonMask_ = ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle,
} ImGuiButtonFlags_;
typedef enum {
    ImGuiColorEditFlags_None = 0,
    ImGuiColorEditFlags_NoAlpha = 1 << 1,
    ImGuiColorEditFlags_NoPicker = 1 << 2,
    ImGuiColorEditFlags_NoOptions = 1 << 3,
    ImGuiColorEditFlags_NoSmallPreview = 1 << 4,
    ImGuiColorEditFlags_NoInputs = 1 << 5,
    ImGuiColorEditFlags_NoTooltip = 1 << 6,
    ImGuiColorEditFlags_NoLabel = 1 << 7,
    ImGuiColorEditFlags_NoSidePreview = 1 << 8,
    ImGuiColorEditFlags_NoDragDrop = 1 << 9,
    ImGuiColorEditFlags_NoBorder = 1 << 10,
    ImGuiColorEditFlags_AlphaBar = 1 << 16,
    ImGuiColorEditFlags_AlphaPreview = 1 << 17,
    ImGuiColorEditFlags_AlphaPreviewHalf = 1 << 18,
    ImGuiColorEditFlags_HDR = 1 << 19,
    ImGuiColorEditFlags_DisplayRGB = 1 << 20,
    ImGuiColorEditFlags_DisplayHSV = 1 << 21,
    ImGuiColorEditFlags_DisplayHex = 1 << 22,
    ImGuiColorEditFlags_Uint8 = 1 << 23,
    ImGuiColorEditFlags_Float = 1 << 24,
    ImGuiColorEditFlags_PickerHueBar = 1 << 25,
    ImGuiColorEditFlags_PickerHueWheel = 1 << 26,
    ImGuiColorEditFlags_InputRGB = 1 << 27,
    ImGuiColorEditFlags_InputHSV = 1 << 28,
    ImGuiColorEditFlags_DefaultOptions_ = ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueBar,
    ImGuiColorEditFlags_DisplayMask_ = ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_DisplayHex,
    ImGuiColorEditFlags_DataTypeMask_ = ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_Float,
    ImGuiColorEditFlags_PickerMask_ = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_PickerHueBar,
    ImGuiColorEditFlags_InputMask_ = ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_InputHSV,
} ImGuiColorEditFlags_;
typedef enum {
    ImGuiSliderFlags_None = 0,
    ImGuiSliderFlags_AlwaysClamp = 1 << 4,
    ImGuiSliderFlags_Logarithmic = 1 << 5,
    ImGuiSliderFlags_NoRoundToFormat = 1 << 6,
    ImGuiSliderFlags_NoInput = 1 << 7,
    ImGuiSliderFlags_WrapAround = 1 << 8,
    ImGuiSliderFlags_InvalidMask_ = 0x7000000F,
} ImGuiSliderFlags_;
typedef enum { ImGuiMouseButton_Left = 0, ImGuiMouseButton_Right = 1, ImGuiMouseButton_Middle = 2, ImGuiMouseButton_COUNT = 5 } ImGuiMouseButton_;
typedef enum {
    ImGuiMouseCursor_None = -1,
    ImGuiMouseCursor_Arrow = 0,
    ImGuiMouseCursor_TextInput,
    ImGuiMouseCursor_ResizeAll,
    ImGuiMouseCursor_ResizeNS,
    ImGuiMouseCursor_ResizeEW,
    ImGuiMouseCursor_ResizeNESW,
    ImGuiMouseCursor_ResizeNWSE,
    ImGuiMouseCursor_Hand,
    ImGuiMouseCursor_NotAllowed,
    ImGuiMouseCursor_COUNT
} ImGuiMouseCursor_;
typedef enum {
    ImGuiMouseSource_Mouse = 0,
    ImGuiMouseSource_TouchScreen = 1,
    ImGuiMouseSource_Pen = 2,
    ImGuiMouseSource_COUNT = 3,
} ImGuiMouseSource;
typedef enum {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3,
} ImGuiCond_;
typedef enum {
    ImGuiTableFlags_None = 0,
    ImGuiTableFlags_Resizable = 1 << 0,
    ImGuiTableFlags_Reorderable = 1 << 1,
    ImGuiTableFlags_Hideable = 1 << 2,
    ImGuiTableFlags_Sortable = 1 << 3,
    ImGuiTableFlags_NoSavedSettings = 1 << 4,
    ImGuiTableFlags_ContextMenuInBody = 1 << 5,
    ImGuiTableFlags_RowBg = 1 << 6,
    ImGuiTableFlags_BordersInnerH = 1 << 7,
    ImGuiTableFlags_BordersOuterH = 1 << 8,
    ImGuiTableFlags_BordersInnerV = 1 << 9,
    ImGuiTableFlags_BordersOuterV = 1 << 10,
    ImGuiTableFlags_BordersH = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuterH,
    ImGuiTableFlags_BordersV = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV,
    ImGuiTableFlags_BordersInner = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH,
    ImGuiTableFlags_BordersOuter = ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH,
    ImGuiTableFlags_Borders = ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter,
    ImGuiTableFlags_NoBordersInBody = 1 << 11,
    ImGuiTableFlags_NoBordersInBodyUntilResize = 1 << 12,
    ImGuiTableFlags_SizingFixedFit = 1 << 13,
    ImGuiTableFlags_SizingFixedSame = 2 << 13,
    ImGuiTableFlags_SizingStretchProp = 3 << 13,
    ImGuiTableFlags_SizingStretchSame = 4 << 13,
    ImGuiTableFlags_NoHostExtendX = 1 << 16,
    ImGuiTableFlags_NoHostExtendY = 1 << 17,
    ImGuiTableFlags_NoKeepColumnsVisible = 1 << 18,
    ImGuiTableFlags_PreciseWidths = 1 << 19,
    ImGuiTableFlags_NoClip = 1 << 20,
    ImGuiTableFlags_PadOuterX = 1 << 21,
    ImGuiTableFlags_NoPadOuterX = 1 << 22,
    ImGuiTableFlags_NoPadInnerX = 1 << 23,
    ImGuiTableFlags_ScrollX = 1 << 24,
    ImGuiTableFlags_ScrollY = 1 << 25,
    ImGuiTableFlags_SortMulti = 1 << 26,
    ImGuiTableFlags_SortTristate = 1 << 27,
    ImGuiTableFlags_HighlightHoveredColumn = 1 << 28,
    ImGuiTableFlags_SizingMask_ = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingStretchSame,
} ImGuiTableFlags_;
typedef enum {
    ImGuiTableColumnFlags_None = 0,
    ImGuiTableColumnFlags_Disabled = 1 << 0,
    ImGuiTableColumnFlags_DefaultHide = 1 << 1,
    ImGuiTableColumnFlags_DefaultSort = 1 << 2,
    ImGuiTableColumnFlags_WidthStretch = 1 << 3,
    ImGuiTableColumnFlags_WidthFixed = 1 << 4,
    ImGuiTableColumnFlags_NoResize = 1 << 5,
    ImGuiTableColumnFlags_NoReorder = 1 << 6,
    ImGuiTableColumnFlags_NoHide = 1 << 7,
    ImGuiTableColumnFlags_NoClip = 1 << 8,
    ImGuiTableColumnFlags_NoSort = 1 << 9,
    ImGuiTableColumnFlags_NoSortAscending = 1 << 10,
    ImGuiTableColumnFlags_NoSortDescending = 1 << 11,
    ImGuiTableColumnFlags_NoHeaderLabel = 1 << 12,
    ImGuiTableColumnFlags_NoHeaderWidth = 1 << 13,
    ImGuiTableColumnFlags_PreferSortAscending = 1 << 14,
    ImGuiTableColumnFlags_PreferSortDescending = 1 << 15,
    ImGuiTableColumnFlags_IndentEnable = 1 << 16,
    ImGuiTableColumnFlags_IndentDisable = 1 << 17,
    ImGuiTableColumnFlags_AngledHeader = 1 << 18,
    ImGuiTableColumnFlags_IsEnabled = 1 << 24,
    ImGuiTableColumnFlags_IsVisible = 1 << 25,
    ImGuiTableColumnFlags_IsSorted = 1 << 26,
    ImGuiTableColumnFlags_IsHovered = 1 << 27,
    ImGuiTableColumnFlags_WidthMask_ = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_WidthFixed,
    ImGuiTableColumnFlags_IndentMask_ = ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_IndentDisable,
    ImGuiTableColumnFlags_StatusMask_ = ImGuiTableColumnFlags_IsEnabled | ImGuiTableColumnFlags_IsVisible | ImGuiTableColumnFlags_IsSorted | ImGuiTableColumnFlags_IsHovered,
    ImGuiTableColumnFlags_NoDirectResize_ = 1 << 30,
} ImGuiTableColumnFlags_;
typedef enum {
    ImGuiTableRowFlags_None = 0,
    ImGuiTableRowFlags_Headers = 1 << 0,
} ImGuiTableRowFlags_;
typedef enum {
    ImGuiTableBgTarget_None = 0,
    ImGuiTableBgTarget_RowBg0 = 1,
    ImGuiTableBgTarget_RowBg1 = 2,
    ImGuiTableBgTarget_CellBg = 3,
} ImGuiTableBgTarget_;
struct ImGuiTableSortSpecs {
    const ImGuiTableColumnSortSpecs* Specs;
    int SpecsCount;
    _Bool SpecsDirty;
};
struct ImGuiTableColumnSortSpecs {
    ImGuiID ColumnUserID;
    ImS16 ColumnIndex;
    ImS16 SortOrder;
    ImGuiSortDirection SortDirection;
};
struct ImGuiStyle {
    float Alpha;
    float DisabledAlpha;
    ImVec2 WindowPadding;
    float WindowRounding;
    float WindowBorderSize;
    ImVec2 WindowMinSize;
    ImVec2 WindowTitleAlign;
    ImGuiDir WindowMenuButtonPosition;
    float ChildRounding;
    float ChildBorderSize;
    float PopupRounding;
    float PopupBorderSize;
    ImVec2 FramePadding;
    float FrameRounding;
    float FrameBorderSize;
    ImVec2 ItemSpacing;
    ImVec2 ItemInnerSpacing;
    ImVec2 CellPadding;
    ImVec2 TouchExtraPadding;
    float IndentSpacing;
    float ColumnsMinSpacing;
    float ScrollbarSize;
    float ScrollbarRounding;
    float GrabMinSize;
    float GrabRounding;
    float LogSliderDeadzone;
    float TabRounding;
    float TabBorderSize;
    float TabMinWidthForCloseButton;
    float TabBarBorderSize;
    float TabBarOverlineSize;
    float TableAngledHeadersAngle;
    ImVec2 TableAngledHeadersTextAlign;
    ImGuiDir ColorButtonPosition;
    ImVec2 ButtonTextAlign;
    ImVec2 SelectableTextAlign;
    float SeparatorTextBorderSize;
    ImVec2 SeparatorTextAlign;
    ImVec2 SeparatorTextPadding;
    ImVec2 DisplayWindowPadding;
    ImVec2 DisplaySafeAreaPadding;
    float DockingSeparatorSize;
    float MouseCursorScale;
    _Bool AntiAliasedLines;
    _Bool AntiAliasedLinesUseTex;
    _Bool AntiAliasedFill;
    float CurveTessellationTol;
    float CircleTessellationMaxError;
    ImVec4 Colors[ImGuiCol_COUNT];
    float HoverStationaryDelay;
    float HoverDelayShort;
    float HoverDelayNormal;
    ImGuiHoveredFlags HoverFlagsForTooltipMouse;
    ImGuiHoveredFlags HoverFlagsForTooltipNav;
};
struct ImGuiKeyData {
    _Bool Down;
    float DownDuration;
    float DownDurationPrev;
    float AnalogValue;
};
typedef struct ImVector_ImWchar {
    int Size;
    int Capacity;
    ImWchar* Data;
} ImVector_ImWchar;
struct ImGuiIO {
    ImGuiConfigFlags ConfigFlags;
    ImGuiBackendFlags BackendFlags;
    ImVec2 DisplaySize;
    float DeltaTime;
    float IniSavingRate;
    const char* IniFilename;
    const char* LogFilename;
    void* UserData;
    ImFontAtlas* Fonts;
    float FontGlobalScale;
    _Bool FontAllowUserScaling;
    ImFont* FontDefault;
    ImVec2 DisplayFramebufferScale;
    _Bool ConfigDockingNoSplit;
    _Bool ConfigDockingWithShift;
    _Bool ConfigDockingAlwaysTabBar;
    _Bool ConfigDockingTransparentPayload;
    _Bool ConfigViewportsNoAutoMerge;
    _Bool ConfigViewportsNoTaskBarIcon;
    _Bool ConfigViewportsNoDecoration;
    _Bool ConfigViewportsNoDefaultParent;
    _Bool MouseDrawCursor;
    _Bool ConfigMacOSXBehaviors;
    _Bool ConfigNavSwapGamepadButtons;
    _Bool ConfigInputTrickleEventQueue;
    _Bool ConfigInputTextCursorBlink;
    _Bool ConfigInputTextEnterKeepActive;
    _Bool ConfigDragClickToInputText;
    _Bool ConfigWindowsResizeFromEdges;
    _Bool ConfigWindowsMoveFromTitleBarOnly;
    float ConfigMemoryCompactTimer;
    float MouseDoubleClickTime;
    float MouseDoubleClickMaxDist;
    float MouseDragThreshold;
    float KeyRepeatDelay;
    float KeyRepeatRate;
    _Bool ConfigDebugIsDebuggerPresent;
    _Bool ConfigDebugBeginReturnValueOnce;
    _Bool ConfigDebugBeginReturnValueLoop;
    _Bool ConfigDebugIgnoreFocusLoss;
    _Bool ConfigDebugIniSettings;
    const char* BackendPlatformName;
    const char* BackendRendererName;
    void* BackendPlatformUserData;
    void* BackendRendererUserData;
    void* BackendLanguageUserData;
    const char* (*GetClipboardTextFn)(void* user_data);
    void (*SetClipboardTextFn)(void* user_data, const char* text);
    void* ClipboardUserData;
    _Bool (*PlatformOpenInShellFn)(ImGuiContext* ctx, const char* path);
    void* PlatformOpenInShellUserData;
    void (*PlatformSetImeDataFn)(ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data);
    ImWchar PlatformLocaleDecimalPoint;
    _Bool WantCaptureMouse;
    _Bool WantCaptureKeyboard;
    _Bool WantTextInput;
    _Bool WantSetMousePos;
    _Bool WantSaveIniSettings;
    _Bool NavActive;
    _Bool NavVisible;
    float Framerate;
    int MetricsRenderVertices;
    int MetricsRenderIndices;
    int MetricsRenderWindows;
    int MetricsActiveWindows;
    ImVec2 MouseDelta;
    ImGuiContext* Ctx;
    ImVec2 MousePos;
    _Bool MouseDown[5];
    float MouseWheel;
    float MouseWheelH;
    ImGuiMouseSource MouseSource;
    ImGuiID MouseHoveredViewport;
    _Bool KeyCtrl;
    _Bool KeyShift;
    _Bool KeyAlt;
    _Bool KeySuper;
    ImGuiKeyChord KeyMods;
    ImGuiKeyData KeysData[ImGuiKey_KeysData_SIZE];
    _Bool WantCaptureMouseUnlessPopupClose;
    ImVec2 MousePosPrev;
    ImVec2 MouseClickedPos[5];
    double MouseClickedTime[5];
    _Bool MouseClicked[5];
    _Bool MouseDoubleClicked[5];
    ImU16 MouseClickedCount[5];
    ImU16 MouseClickedLastCount[5];
    _Bool MouseReleased[5];
    _Bool MouseDownOwned[5];
    _Bool MouseDownOwnedUnlessPopupClose[5];
    _Bool MouseWheelRequestAxisSwap;
    _Bool MouseCtrlLeftAsRightClick;
    float MouseDownDuration[5];
    float MouseDownDurationPrev[5];
    ImVec2 MouseDragMaxDistanceAbs[5];
    float MouseDragMaxDistanceSqr[5];
    float PenPressure;
    _Bool AppFocusLost;
    _Bool AppAcceptingEvents;
    ImS8 BackendUsingLegacyKeyArrays;
    _Bool BackendUsingLegacyNavInputArray;
    ImWchar16 InputQueueSurrogate;
    ImVector_ImWchar InputQueueCharacters;
};
struct ImGuiInputTextCallbackData {
    ImGuiContext* Ctx;
    ImGuiInputTextFlags EventFlag;
    ImGuiInputTextFlags Flags;
    void* UserData;
    ImWchar EventChar;
    ImGuiKey EventKey;
    char* Buf;
    int BufTextLen;
    int BufSize;
    _Bool BufDirty;
    int CursorPos;
    int SelectionStart;
    int SelectionEnd;
};
struct ImGuiSizeCallbackData {
    void* UserData;
    ImVec2 Pos;
    ImVec2 CurrentSize;
    ImVec2 DesiredSize;
};
struct ImGuiWindowClass {
    ImGuiID ClassId;
    ImGuiID ParentViewportId;
    ImGuiID FocusRouteParentWindowId;
    ImGuiViewportFlags ViewportFlagsOverrideSet;
    ImGuiViewportFlags ViewportFlagsOverrideClear;
    ImGuiTabItemFlags TabItemFlagsOverrideSet;
    ImGuiDockNodeFlags DockNodeFlagsOverrideSet;
    _Bool DockingAlwaysTabBar;
    _Bool DockingAllowUnclassed;
};
struct ImGuiPayload {
    void* Data;
    int DataSize;
    ImGuiID SourceId;
    ImGuiID SourceParentId;
    int DataFrameCount;
    char DataType[32 + 1];
    _Bool Preview;
    _Bool Delivery;
};
struct ImGuiOnceUponAFrame {
    int RefFrame;
};
struct ImGuiTextRange {
    const char* b;
    const char* e;
};
typedef struct ImGuiTextRange ImGuiTextRange;
typedef struct ImVector_ImGuiTextRange {
    int Size;
    int Capacity;
    ImGuiTextRange* Data;
} ImVector_ImGuiTextRange;
struct ImGuiTextFilter {
    char InputBuf[256];
    ImVector_ImGuiTextRange Filters;
    int CountGrep;
};
typedef struct ImGuiTextRange ImGuiTextRange;
typedef struct ImVector_char {
    int Size;
    int Capacity;
    char* Data;
} ImVector_char;
struct ImGuiTextBuffer {
    ImVector_char Buf;
};
struct ImGuiStoragePair {
    ImGuiID key;
    union {
        int val_i;
        float val_f;
        void* val_p;
    };
};
typedef struct ImVector_ImGuiStoragePair {
    int Size;
    int Capacity;
    ImGuiStoragePair* Data;
} ImVector_ImGuiStoragePair;
struct ImGuiStorage {
    ImVector_ImGuiStoragePair Data;
};
struct ImGuiListClipper {
    ImGuiContext* Ctx;
    int DisplayStart;
    int DisplayEnd;
    int ItemsCount;
    float ItemsHeight;
    float StartPosY;
    double StartSeekOffsetY;
    void* TempData;
};
struct ImColor {
    ImVec4 Value;
};
typedef enum {
    ImGuiMultiSelectFlags_None = 0,
    ImGuiMultiSelectFlags_SingleSelect = 1 << 0,
    ImGuiMultiSelectFlags_NoSelectAll = 1 << 1,
    ImGuiMultiSelectFlags_NoRangeSelect = 1 << 2,
    ImGuiMultiSelectFlags_NoAutoSelect = 1 << 3,
    ImGuiMultiSelectFlags_NoAutoClear = 1 << 4,
    ImGuiMultiSelectFlags_NoAutoClearOnReselect = 1 << 5,
    ImGuiMultiSelectFlags_BoxSelect1d = 1 << 6,
    ImGuiMultiSelectFlags_BoxSelect2d = 1 << 7,
    ImGuiMultiSelectFlags_BoxSelectNoScroll = 1 << 8,
    ImGuiMultiSelectFlags_ClearOnEscape = 1 << 9,
    ImGuiMultiSelectFlags_ClearOnClickVoid = 1 << 10,
    ImGuiMultiSelectFlags_ScopeWindow = 1 << 11,
    ImGuiMultiSelectFlags_ScopeRect = 1 << 12,
    ImGuiMultiSelectFlags_SelectOnClick = 1 << 13,
    ImGuiMultiSelectFlags_SelectOnClickRelease = 1 << 14,
    ImGuiMultiSelectFlags_NavWrapX = 1 << 16,
} ImGuiMultiSelectFlags_;
typedef struct ImVector_ImGuiSelectionRequest {
    int Size;
    int Capacity;
    ImGuiSelectionRequest* Data;
} ImVector_ImGuiSelectionRequest;
struct ImGuiMultiSelectIO {
    ImVector_ImGuiSelectionRequest Requests;
    ImGuiSelectionUserData RangeSrcItem;
    ImGuiSelectionUserData NavIdItem;
    _Bool NavIdSelected;
    _Bool RangeSrcReset;
    int ItemsCount;
};
typedef enum {
    ImGuiSelectionRequestType_None = 0,
    ImGuiSelectionRequestType_SetAll,
    ImGuiSelectionRequestType_SetRange,
} ImGuiSelectionRequestType;
struct ImGuiSelectionRequest {
    ImGuiSelectionRequestType Type;
    _Bool Selected;
    ImS8 RangeDirection;
    ImGuiSelectionUserData RangeFirstItem;
    ImGuiSelectionUserData RangeLastItem;
};
struct ImGuiSelectionBasicStorage {
    int Size;
    _Bool PreserveOrder;
    void* UserData;
    ImGuiID (*AdapterIndexToStorageId)(ImGuiSelectionBasicStorage* self, int idx);
    int _SelectionOrder;
    ImGuiStorage _Storage;
};
struct ImGuiSelectionExternalStorage {
    void* UserData;
    void (*AdapterSetItemSelected)(ImGuiSelectionExternalStorage* self, int idx, _Bool selected);
};
typedef void (*ImDrawCallback)(const ImDrawList* parent_list, const ImDrawCmd* cmd);
struct ImDrawCmd {
    ImVec4 ClipRect;
    ImTextureID TextureId;
    unsigned int VtxOffset;
    unsigned int IdxOffset;
    unsigned int ElemCount;
    ImDrawCallback UserCallback;
    void* UserCallbackData;
};
struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    ImU32 col;
};
typedef struct ImDrawCmdHeader ImDrawCmdHeader;
struct ImDrawCmdHeader {
    ImVec4 ClipRect;
    ImTextureID TextureId;
    unsigned int VtxOffset;
};
typedef struct ImVector_ImDrawCmd {
    int Size;
    int Capacity;
    ImDrawCmd* Data;
} ImVector_ImDrawCmd;
typedef struct ImVector_ImDrawIdx {
    int Size;
    int Capacity;
    ImDrawIdx* Data;
} ImVector_ImDrawIdx;
struct ImDrawChannel {
    ImVector_ImDrawCmd _CmdBuffer;
    ImVector_ImDrawIdx _IdxBuffer;
};
typedef struct ImVector_ImDrawChannel {
    int Size;
    int Capacity;
    ImDrawChannel* Data;
} ImVector_ImDrawChannel;
struct ImDrawListSplitter {
    int _Current;
    int _Count;
    ImVector_ImDrawChannel _Channels;
};
typedef enum {
    ImDrawFlags_None = 0,
    ImDrawFlags_Closed = 1 << 0,
    ImDrawFlags_RoundCornersTopLeft = 1 << 4,
    ImDrawFlags_RoundCornersTopRight = 1 << 5,
    ImDrawFlags_RoundCornersBottomLeft = 1 << 6,
    ImDrawFlags_RoundCornersBottomRight = 1 << 7,
    ImDrawFlags_RoundCornersNone = 1 << 8,
    ImDrawFlags_RoundCornersTop = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersBottom = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
    ImDrawFlags_RoundCornersLeft = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft,
    ImDrawFlags_RoundCornersRight = ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight,
    ImDrawFlags_RoundCornersAll = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
    ImDrawFlags_RoundCornersDefault_ = ImDrawFlags_RoundCornersAll,
    ImDrawFlags_RoundCornersMask_ = ImDrawFlags_RoundCornersAll | ImDrawFlags_RoundCornersNone,
} ImDrawFlags_;
typedef enum {
    ImDrawListFlags_None = 0,
    ImDrawListFlags_AntiAliasedLines = 1 << 0,
    ImDrawListFlags_AntiAliasedLinesUseTex = 1 << 1,
    ImDrawListFlags_AntiAliasedFill = 1 << 2,
    ImDrawListFlags_AllowVtxOffset = 1 << 3,
} ImDrawListFlags_;
typedef struct ImVector_ImDrawVert {
    int Size;
    int Capacity;
    ImDrawVert* Data;
} ImVector_ImDrawVert;
typedef struct ImVector_ImVec2 {
    int Size;
    int Capacity;
    ImVec2* Data;
} ImVector_ImVec2;
typedef struct ImVector_ImVec4 {
    int Size;
    int Capacity;
    ImVec4* Data;
} ImVector_ImVec4;
typedef struct ImVector_ImTextureID {
    int Size;
    int Capacity;
    ImTextureID* Data;
} ImVector_ImTextureID;
struct ImDrawList {
    ImVector_ImDrawCmd CmdBuffer;
    ImVector_ImDrawIdx IdxBuffer;
    ImVector_ImDrawVert VtxBuffer;
    ImDrawListFlags Flags;
    unsigned int _VtxCurrentIdx;
    ImDrawListSharedData* _Data;
    ImDrawVert* _VtxWritePtr;
    ImDrawIdx* _IdxWritePtr;
    ImVector_ImVec2 _Path;
    ImDrawCmdHeader _CmdHeader;
    ImDrawListSplitter _Splitter;
    ImVector_ImVec4 _ClipRectStack;
    ImVector_ImTextureID _TextureIdStack;
    float _FringeScale;
    const char* _OwnerName;
};
typedef struct ImVector_ImDrawListPtr {
    int Size;
    int Capacity;
    ImDrawList** Data;
} ImVector_ImDrawListPtr;
struct ImDrawData {
    _Bool Valid;
    int CmdListsCount;
    int TotalIdxCount;
    int TotalVtxCount;
    ImVector_ImDrawListPtr CmdLists;
    ImVec2 DisplayPos;
    ImVec2 DisplaySize;
    ImVec2 FramebufferScale;
    ImGuiViewport* OwnerViewport;
};
struct ImFontConfig {
    void* FontData;
    int FontDataSize;
    _Bool FontDataOwnedByAtlas;
    int FontNo;
    float SizePixels;
    int OversampleH;
    int OversampleV;
    _Bool PixelSnapH;
    ImVec2 GlyphExtraSpacing;
    ImVec2 GlyphOffset;
    const ImWchar* GlyphRanges;
    float GlyphMinAdvanceX;
    float GlyphMaxAdvanceX;
    _Bool MergeMode;
    unsigned int FontBuilderFlags;
    float RasterizerMultiply;
    float RasterizerDensity;
    ImWchar EllipsisChar;
    char Name[40];
    ImFont* DstFont;
};
struct ImFontGlyph {
    unsigned int Colored : 1;
    unsigned int Visible : 1;
    unsigned int Codepoint : 30;
    float AdvanceX;
    float X0, Y0, X1, Y1;
    float U0, V0, U1, V1;
};
typedef struct ImVector_ImU32 {
    int Size;
    int Capacity;
    ImU32* Data;
} ImVector_ImU32;
struct ImFontGlyphRangesBuilder {
    ImVector_ImU32 UsedChars;
};
typedef struct ImFontAtlasCustomRect ImFontAtlasCustomRect;
struct ImFontAtlasCustomRect {
    unsigned short Width, Height;
    unsigned short X, Y;
    unsigned int GlyphID;
    float GlyphAdvanceX;
    ImVec2 GlyphOffset;
    ImFont* Font;
};
typedef enum {
    ImFontAtlasFlags_None = 0,
    ImFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,
    ImFontAtlasFlags_NoMouseCursors = 1 << 1,
    ImFontAtlasFlags_NoBakedLines = 1 << 2,
} ImFontAtlasFlags_;
typedef struct ImVector_ImFontPtr {
    int Size;
    int Capacity;
    ImFont** Data;
} ImVector_ImFontPtr;
typedef struct ImVector_ImFontAtlasCustomRect {
    int Size;
    int Capacity;
    ImFontAtlasCustomRect* Data;
} ImVector_ImFontAtlasCustomRect;
typedef struct ImVector_ImFontConfig {
    int Size;
    int Capacity;
    ImFontConfig* Data;
} ImVector_ImFontConfig;
struct ImFontAtlas {
    ImFontAtlasFlags Flags;
    ImTextureID TexID;
    int TexDesiredWidth;
    int TexGlyphPadding;
    _Bool Locked;
    void* UserData;
    _Bool TexReady;
    _Bool TexPixelsUseColors;
    unsigned char* TexPixelsAlpha8;
    unsigned int* TexPixelsRGBA32;
    int TexWidth;
    int TexHeight;
    ImVec2 TexUvScale;
    ImVec2 TexUvWhitePixel;
    ImVector_ImFontPtr Fonts;
    ImVector_ImFontAtlasCustomRect CustomRects;
    ImVector_ImFontConfig ConfigData;
    ImVec4 TexUvLines[(63) + 1];
    const ImFontBuilderIO* FontBuilderIO;
    unsigned int FontBuilderFlags;
    int PackIdMouseCursors;
    int PackIdLines;
};
typedef struct ImVector_float {
    int Size;
    int Capacity;
    float* Data;
} ImVector_float;
typedef struct ImVector_ImFontGlyph {
    int Size;
    int Capacity;
    ImFontGlyph* Data;
} ImVector_ImFontGlyph;
struct ImFont {
    ImVector_float IndexAdvanceX;
    float FallbackAdvanceX;
    float FontSize;
    ImVector_ImWchar IndexLookup;
    ImVector_ImFontGlyph Glyphs;
    const ImFontGlyph* FallbackGlyph;
    ImFontAtlas* ContainerAtlas;
    const ImFontConfig* ConfigData;
    short ConfigDataCount;
    ImWchar FallbackChar;
    ImWchar EllipsisChar;
    short EllipsisCharCount;
    float EllipsisWidth;
    float EllipsisCharStep;
    _Bool DirtyLookupTables;
    float Scale;
    float Ascent, Descent;
    int MetricsTotalSurface;
    ImU8 Used4kPagesMap[(0xFFFF + 1) / 4096 / 8];
};
typedef enum {
    ImGuiViewportFlags_None = 0,
    ImGuiViewportFlags_IsPlatformWindow = 1 << 0,
    ImGuiViewportFlags_IsPlatformMonitor = 1 << 1,
    ImGuiViewportFlags_OwnedByApp = 1 << 2,
    ImGuiViewportFlags_NoDecoration = 1 << 3,
    ImGuiViewportFlags_NoTaskBarIcon = 1 << 4,
    ImGuiViewportFlags_NoFocusOnAppearing = 1 << 5,
    ImGuiViewportFlags_NoFocusOnClick = 1 << 6,
    ImGuiViewportFlags_NoInputs = 1 << 7,
    ImGuiViewportFlags_NoRendererClear = 1 << 8,
    ImGuiViewportFlags_NoAutoMerge = 1 << 9,
    ImGuiViewportFlags_TopMost = 1 << 10,
    ImGuiViewportFlags_CanHostOtherWindows = 1 << 11,
    ImGuiViewportFlags_IsMinimized = 1 << 12,
    ImGuiViewportFlags_IsFocused = 1 << 13,
} ImGuiViewportFlags_;
struct ImGuiViewport {
    ImGuiID ID;
    ImGuiViewportFlags Flags;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 WorkPos;
    ImVec2 WorkSize;
    float DpiScale;
    ImGuiID ParentViewportId;
    ImDrawData* DrawData;
    void* RendererUserData;
    void* PlatformUserData;
    void* PlatformHandle;
    void* PlatformHandleRaw;
    _Bool PlatformWindowCreated;
    _Bool PlatformRequestMove;
    _Bool PlatformRequestResize;
    _Bool PlatformRequestClose;
};
typedef struct ImVector_ImGuiPlatformMonitor {
    int Size;
    int Capacity;
    ImGuiPlatformMonitor* Data;
} ImVector_ImGuiPlatformMonitor;
typedef struct ImVector_ImGuiViewportPtr {
    int Size;
    int Capacity;
    ImGuiViewport** Data;
} ImVector_ImGuiViewportPtr;
struct ImGuiPlatformIO {
    void (*Platform_CreateWindow)(ImGuiViewport* vp);
    void (*Platform_DestroyWindow)(ImGuiViewport* vp);
    void (*Platform_ShowWindow)(ImGuiViewport* vp);
    void (*Platform_SetWindowPos)(ImGuiViewport* vp, ImVec2 pos);
    ImVec2 (*Platform_GetWindowPos)(ImGuiViewport* vp);
    void (*Platform_SetWindowSize)(ImGuiViewport* vp, ImVec2 size);
    ImVec2 (*Platform_GetWindowSize)(ImGuiViewport* vp);
    void (*Platform_SetWindowFocus)(ImGuiViewport* vp);
    _Bool (*Platform_GetWindowFocus)(ImGuiViewport* vp);
    _Bool (*Platform_GetWindowMinimized)(ImGuiViewport* vp);
    void (*Platform_SetWindowTitle)(ImGuiViewport* vp, const char* str);
    void (*Platform_SetWindowAlpha)(ImGuiViewport* vp, float alpha);
    void (*Platform_UpdateWindow)(ImGuiViewport* vp);
    void (*Platform_RenderWindow)(ImGuiViewport* vp, void* render_arg);
    void (*Platform_SwapBuffers)(ImGuiViewport* vp, void* render_arg);
    float (*Platform_GetWindowDpiScale)(ImGuiViewport* vp);
    void (*Platform_OnChangedViewport)(ImGuiViewport* vp);
    int (*Platform_CreateVkSurface)(ImGuiViewport* vp, ImU64 vk_inst, const void* vk_allocators, ImU64* out_vk_surface);
    void (*Renderer_CreateWindow)(ImGuiViewport* vp);
    void (*Renderer_DestroyWindow)(ImGuiViewport* vp);
    void (*Renderer_SetWindowSize)(ImGuiViewport* vp, ImVec2 size);
    void (*Renderer_RenderWindow)(ImGuiViewport* vp, void* render_arg);
    void (*Renderer_SwapBuffers)(ImGuiViewport* vp, void* render_arg);
    ImVector_ImGuiPlatformMonitor Monitors;
    ImVector_ImGuiViewportPtr Viewports;
};
struct ImGuiPlatformMonitor {
    ImVec2 MainPos, MainSize;
    ImVec2 WorkPos, WorkSize;
    float DpiScale;
    void* PlatformHandle;
};
struct ImGuiPlatformImeData {
    _Bool WantVisible;
    ImVec2 InputPos;
    float InputLineHeight;
};
struct ImBitVector;
struct ImRect;
struct ImDrawDataBuilder;
struct ImDrawListSharedData;
struct ImGuiBoxSelectState;
struct ImGuiColorMod;
struct ImGuiContext;
struct ImGuiContextHook;
struct ImGuiDataVarInfo;
struct ImGuiDataTypeInfo;
struct ImGuiDockContext;
struct ImGuiDockRequest;
struct ImGuiDockNode;
struct ImGuiDockNodeSettings;
struct ImGuiGroupData;
struct ImGuiInputTextState;
struct ImGuiInputTextDeactivateData;
struct ImGuiLastItemData;
struct ImGuiLocEntry;
struct ImGuiMenuColumns;
struct ImGuiMultiSelectState;
struct ImGuiMultiSelectTempData;
struct ImGuiNavItemData;
struct ImGuiMetricsConfig;
struct ImGuiNextWindowData;
struct ImGuiNextItemData;
struct ImGuiOldColumnData;
struct ImGuiOldColumns;
struct ImGuiPopupData;
struct ImGuiSettingsHandler;
struct ImGuiStackSizes;
struct ImGuiStyleMod;
struct ImGuiTabBar;
struct ImGuiTabItem;
struct ImGuiTable;
struct ImGuiTableHeaderData;
struct ImGuiTableColumn;
struct ImGuiTableInstanceData;
struct ImGuiTableTempData;
struct ImGuiTableSettings;
struct ImGuiTableColumnsSettings;
struct ImGuiTreeNodeStackData;
struct ImGuiTypingSelectState;
struct ImGuiTypingSelectRequest;
struct ImGuiWindow;
struct ImGuiWindowDockStyle;
struct ImGuiWindowTempData;
struct ImGuiWindowSettings;
typedef int ImGuiDataAuthority;
typedef int ImGuiLayoutType;
typedef int ImGuiActivateFlags;
typedef int ImGuiDebugLogFlags;
typedef int ImGuiFocusRequestFlags;
typedef int ImGuiItemStatusFlags;
typedef int ImGuiOldColumnFlags;
typedef int ImGuiNavHighlightFlags;
typedef int ImGuiNavMoveFlags;
typedef int ImGuiNextItemDataFlags;
typedef int ImGuiNextWindowDataFlags;
typedef int ImGuiScrollFlags;
typedef int ImGuiSeparatorFlags;
typedef int ImGuiTextFlags;
typedef int ImGuiTooltipFlags;
typedef int ImGuiTypingSelectFlags;
typedef int ImGuiWindowRefreshFlags;
typedef void (*ImGuiErrorLogCallback)(void* user_data, const char* fmt, ...);
extern ImGuiContext* GImGui;
typedef struct StbUndoRecord StbUndoRecord;
struct StbUndoRecord {
    int where;
    int insert_length;
    int delete_length;
    int char_storage;
};
typedef struct StbUndoState StbUndoState;
struct StbUndoState {
    StbUndoRecord undo_rec[99];
    ImWchar undo_char[999];
    short undo_point, redo_point;
    int undo_char_point, redo_char_point;
};
typedef struct STB_TexteditState STB_TexteditState;
struct STB_TexteditState {
    int cursor;
    int select_start;
    int select_end;
    unsigned char insert_mode;
    int row_count_per_page;
    unsigned char cursor_at_end_of_line;
    unsigned char initialized;
    unsigned char has_preferred_x;
    unsigned char single_line;
    unsigned char padding1, padding2, padding3;
    float preferred_x;
    StbUndoState undostate;
};
typedef struct StbTexteditRow StbTexteditRow;
struct StbTexteditRow {
    float x0, x1;
    float baseline_y_delta;
    float ymin, ymax;
    int num_chars;
};
typedef FILE* ImFileHandle;
typedef struct ImVec1 ImVec1;
struct ImVec1 {
    float x;
};
typedef struct ImVec2ih ImVec2ih;
struct ImVec2ih {
    short x, y;
};
struct ImRect {
    ImVec2 Min;
    ImVec2 Max;
};
typedef ImU32* ImBitArrayPtr;
struct ImBitVector {
    ImVector_ImU32 Storage;
};
typedef int ImPoolIdx;
typedef struct ImGuiTextIndex ImGuiTextIndex;
typedef struct ImVector_int {
    int Size;
    int Capacity;
    int* Data;
} ImVector_int;
struct ImGuiTextIndex {
    ImVector_int LineOffsets;
    int EndOffset;
};
struct ImDrawListSharedData {
    ImVec2 TexUvWhitePixel;
    ImFont* Font;
    float FontSize;
    float FontScale;
    float CurveTessellationTol;
    float CircleSegmentMaxError;
    ImVec4 ClipRectFullscreen;
    ImDrawListFlags InitialFlags;
    ImVector_ImVec2 TempBuffer;
    ImVec2 ArcFastVtx[48];
    float ArcFastRadiusCutoff;
    ImU8 CircleSegmentCounts[64];
    const ImVec4* TexUvLines;
};
struct ImDrawDataBuilder {
    ImVector_ImDrawListPtr* Layers[2];
    ImVector_ImDrawListPtr LayerData1;
};
struct ImGuiDataVarInfo {
    ImGuiDataType Type;
    ImU32 Count;
    ImU32 Offset;
};
typedef struct ImGuiDataTypeStorage ImGuiDataTypeStorage;
struct ImGuiDataTypeStorage {
    ImU8 Data[8];
};
struct ImGuiDataTypeInfo {
    size_t Size;
    const char* Name;
    const char* PrintFmt;
    const char* ScanFmt;
};
typedef enum {
    ImGuiDataType_String = ImGuiDataType_COUNT + 1,
    ImGuiDataType_Pointer,
    ImGuiDataType_ID,
} ImGuiDataTypePrivate_;
typedef enum {
    ImGuiItemFlags_Disabled = 1 << 10,
    ImGuiItemFlags_ReadOnly = 1 << 11,
    ImGuiItemFlags_MixedValue = 1 << 12,
    ImGuiItemFlags_NoWindowHoverableCheck = 1 << 13,
    ImGuiItemFlags_AllowOverlap = 1 << 14,
    ImGuiItemFlags_Inputable = 1 << 20,
    ImGuiItemFlags_HasSelectionUserData = 1 << 21,
    ImGuiItemFlags_IsMultiSelect = 1 << 22,
    ImGuiItemFlags_Default_ = ImGuiItemFlags_AutoClosePopups,
} ImGuiItemFlagsPrivate_;
typedef enum {
    ImGuiItemStatusFlags_None = 0,
    ImGuiItemStatusFlags_HoveredRect = 1 << 0,
    ImGuiItemStatusFlags_HasDisplayRect = 1 << 1,
    ImGuiItemStatusFlags_Edited = 1 << 2,
    ImGuiItemStatusFlags_ToggledSelection = 1 << 3,
    ImGuiItemStatusFlags_ToggledOpen = 1 << 4,
    ImGuiItemStatusFlags_HasDeactivated = 1 << 5,
    ImGuiItemStatusFlags_Deactivated = 1 << 6,
    ImGuiItemStatusFlags_HoveredWindow = 1 << 7,
    ImGuiItemStatusFlags_Visible = 1 << 8,
    ImGuiItemStatusFlags_HasClipRect = 1 << 9,
    ImGuiItemStatusFlags_HasShortcut = 1 << 10,
} ImGuiItemStatusFlags_;
typedef enum {
    ImGuiHoveredFlags_DelayMask_ = ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay,
    ImGuiHoveredFlags_AllowedMaskForIsWindowHovered = ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_NoPopupHierarchy |
                                                      ImGuiHoveredFlags_DockHierarchy | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem |
                                                      ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_Stationary,
    ImGuiHoveredFlags_AllowedMaskForIsItemHovered = ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped |
                                                    ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_NoNavOverride | ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_Stationary |
                                                    ImGuiHoveredFlags_DelayMask_,
} ImGuiHoveredFlagsPrivate_;
typedef enum {
    ImGuiInputTextFlags_Multiline = 1 << 26,
    ImGuiInputTextFlags_NoMarkEdited = 1 << 27,
    ImGuiInputTextFlags_MergedItem = 1 << 28,
    ImGuiInputTextFlags_LocalizeDecimalPoint = 1 << 29,
} ImGuiInputTextFlagsPrivate_;
typedef enum {
    ImGuiButtonFlags_PressedOnClick = 1 << 4,
    ImGuiButtonFlags_PressedOnClickRelease = 1 << 5,
    ImGuiButtonFlags_PressedOnClickReleaseAnywhere = 1 << 6,
    ImGuiButtonFlags_PressedOnRelease = 1 << 7,
    ImGuiButtonFlags_PressedOnDoubleClick = 1 << 8,
    ImGuiButtonFlags_PressedOnDragDropHold = 1 << 9,
    ImGuiButtonFlags_Repeat = 1 << 10,
    ImGuiButtonFlags_FlattenChildren = 1 << 11,
    ImGuiButtonFlags_AllowOverlap = 1 << 12,
    ImGuiButtonFlags_DontClosePopups = 1 << 13,
    ImGuiButtonFlags_AlignTextBaseLine = 1 << 15,
    ImGuiButtonFlags_NoKeyModifiers = 1 << 16,
    ImGuiButtonFlags_NoHoldingActiveId = 1 << 17,
    ImGuiButtonFlags_NoNavFocus = 1 << 18,
    ImGuiButtonFlags_NoHoveredOnFocus = 1 << 19,
    ImGuiButtonFlags_NoSetKeyOwner = 1 << 20,
    ImGuiButtonFlags_NoTestKeyOwner = 1 << 21,
    ImGuiButtonFlags_PressedOnMask_ = ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnClickReleaseAnywhere | ImGuiButtonFlags_PressedOnRelease |
                                      ImGuiButtonFlags_PressedOnDoubleClick | ImGuiButtonFlags_PressedOnDragDropHold,
    ImGuiButtonFlags_PressedOnDefault_ = ImGuiButtonFlags_PressedOnClickRelease,
} ImGuiButtonFlagsPrivate_;
typedef enum {
    ImGuiComboFlags_CustomPreview = 1 << 20,
} ImGuiComboFlagsPrivate_;
typedef enum {
    ImGuiSliderFlags_Vertical = 1 << 20,
    ImGuiSliderFlags_ReadOnly = 1 << 21,
} ImGuiSliderFlagsPrivate_;
typedef enum {
    ImGuiSelectableFlags_NoHoldingActiveID = 1 << 20,
    ImGuiSelectableFlags_SelectOnNav = 1 << 21,
    ImGuiSelectableFlags_SelectOnClick = 1 << 22,
    ImGuiSelectableFlags_SelectOnRelease = 1 << 23,
    ImGuiSelectableFlags_SpanAvailWidth = 1 << 24,
    ImGuiSelectableFlags_SetNavIdOnHover = 1 << 25,
    ImGuiSelectableFlags_NoPadWithHalfSpacing = 1 << 26,
    ImGuiSelectableFlags_NoSetKeyOwner = 1 << 27,
} ImGuiSelectableFlagsPrivate_;
typedef enum {
    ImGuiTreeNodeFlags_ClipLabelForTrailingButton = 1 << 28,
    ImGuiTreeNodeFlags_UpsideDownArrow = 1 << 29,
} ImGuiTreeNodeFlagsPrivate_;
typedef enum {
    ImGuiSeparatorFlags_None = 0,
    ImGuiSeparatorFlags_Horizontal = 1 << 0,
    ImGuiSeparatorFlags_Vertical = 1 << 1,
    ImGuiSeparatorFlags_SpanAllColumns = 1 << 2,
} ImGuiSeparatorFlags_;
typedef enum {
    ImGuiFocusRequestFlags_None = 0,
    ImGuiFocusRequestFlags_RestoreFocusedChild = 1 << 0,
    ImGuiFocusRequestFlags_UnlessBelowModal = 1 << 1,
} ImGuiFocusRequestFlags_;
typedef enum {
    ImGuiTextFlags_None = 0,
    ImGuiTextFlags_NoWidthForLargeClippedText = 1 << 0,
} ImGuiTextFlags_;
typedef enum {
    ImGuiTooltipFlags_None = 0,
    ImGuiTooltipFlags_OverridePrevious = 1 << 1,
} ImGuiTooltipFlags_;
typedef enum { ImGuiLayoutType_Horizontal = 0, ImGuiLayoutType_Vertical = 1 } ImGuiLayoutType_;
typedef enum {
    ImGuiLogType_None = 0,
    ImGuiLogType_TTY,
    ImGuiLogType_File,
    ImGuiLogType_Buffer,
    ImGuiLogType_Clipboard,
} ImGuiLogType;
typedef enum { ImGuiAxis_None = -1, ImGuiAxis_X = 0, ImGuiAxis_Y = 1 } ImGuiAxis;
typedef enum {
    ImGuiPlotType_Lines,
    ImGuiPlotType_Histogram,
} ImGuiPlotType;
struct ImGuiColorMod {
    ImGuiCol Col;
    ImVec4 BackupValue;
};
struct ImGuiStyleMod {
    ImGuiStyleVar VarIdx;
    union {
        int BackupInt[2];
        float BackupFloat[2];
    };
};
typedef struct ImGuiComboPreviewData ImGuiComboPreviewData;
struct ImGuiComboPreviewData {
    ImRect PreviewRect;
    ImVec2 BackupCursorPos;
    ImVec2 BackupCursorMaxPos;
    ImVec2 BackupCursorPosPrevLine;
    float BackupPrevLineTextBaseOffset;
    ImGuiLayoutType BackupLayout;
};
struct ImGuiGroupData {
    ImGuiID WindowID;
    ImVec2 BackupCursorPos;
    ImVec2 BackupCursorMaxPos;
    ImVec2 BackupCursorPosPrevLine;
    ImVec1 BackupIndent;
    ImVec1 BackupGroupOffset;
    ImVec2 BackupCurrLineSize;
    float BackupCurrLineTextBaseOffset;
    ImGuiID BackupActiveIdIsAlive;
    _Bool BackupActiveIdPreviousFrameIsAlive;
    _Bool BackupHoveredIdIsAlive;
    _Bool BackupIsSameLine;
    _Bool EmitItem;
};
struct ImGuiMenuColumns {
    ImU32 TotalWidth;
    ImU32 NextTotalWidth;
    ImU16 Spacing;
    ImU16 OffsetIcon;
    ImU16 OffsetLabel;
    ImU16 OffsetShortcut;
    ImU16 OffsetMark;
    ImU16 Widths[4];
};
typedef struct ImGuiInputTextDeactivatedState ImGuiInputTextDeactivatedState;
struct ImGuiInputTextDeactivatedState {
    ImGuiID ID;
    ImVector_char TextA;
};
struct ImGuiInputTextState {
    ImGuiContext* Ctx;
    ImGuiID ID;
    int CurLenW, CurLenA;
    ImVector_ImWchar TextW;
    ImVector_char TextA;
    ImVector_char InitialTextA;
    _Bool TextAIsValid;
    int BufCapacityA;
    float ScrollX;
    STB_TexteditState Stb;
    float CursorAnim;
    _Bool CursorFollow;
    _Bool SelectedAllMouseLock;
    _Bool Edited;
    ImGuiInputTextFlags Flags;
    _Bool ReloadUserBuf;
    int ReloadSelectionStart;
    int ReloadSelectionEnd;
};
typedef enum {
    ImGuiWindowRefreshFlags_None = 0,
    ImGuiWindowRefreshFlags_TryToAvoidRefresh = 1 << 0,
    ImGuiWindowRefreshFlags_RefreshOnHover = 1 << 1,
    ImGuiWindowRefreshFlags_RefreshOnFocus = 1 << 2,
} ImGuiWindowRefreshFlags_;
typedef enum {
    ImGuiNextWindowDataFlags_None = 0,
    ImGuiNextWindowDataFlags_HasPos = 1 << 0,
    ImGuiNextWindowDataFlags_HasSize = 1 << 1,
    ImGuiNextWindowDataFlags_HasContentSize = 1 << 2,
    ImGuiNextWindowDataFlags_HasCollapsed = 1 << 3,
    ImGuiNextWindowDataFlags_HasSizeConstraint = 1 << 4,
    ImGuiNextWindowDataFlags_HasFocus = 1 << 5,
    ImGuiNextWindowDataFlags_HasBgAlpha = 1 << 6,
    ImGuiNextWindowDataFlags_HasScroll = 1 << 7,
    ImGuiNextWindowDataFlags_HasChildFlags = 1 << 8,
    ImGuiNextWindowDataFlags_HasRefreshPolicy = 1 << 9,
    ImGuiNextWindowDataFlags_HasViewport = 1 << 10,
    ImGuiNextWindowDataFlags_HasDock = 1 << 11,
    ImGuiNextWindowDataFlags_HasWindowClass = 1 << 12,
} ImGuiNextWindowDataFlags_;
struct ImGuiNextWindowData {
    ImGuiNextWindowDataFlags Flags;
    ImGuiCond PosCond;
    ImGuiCond SizeCond;
    ImGuiCond CollapsedCond;
    ImGuiCond DockCond;
    ImVec2 PosVal;
    ImVec2 PosPivotVal;
    ImVec2 SizeVal;
    ImVec2 ContentSizeVal;
    ImVec2 ScrollVal;
    ImGuiChildFlags ChildFlags;
    _Bool PosUndock;
    _Bool CollapsedVal;
    ImRect SizeConstraintRect;
    ImGuiSizeCallback SizeCallback;
    void* SizeCallbackUserData;
    float BgAlphaVal;
    ImGuiID ViewportId;
    ImGuiID DockId;
    ImGuiWindowClass WindowClass;
    ImVec2 MenuBarOffsetMinVal;
    ImGuiWindowRefreshFlags RefreshFlagsVal;
};
typedef enum {
    ImGuiNextItemDataFlags_None = 0,
    ImGuiNextItemDataFlags_HasWidth = 1 << 0,
    ImGuiNextItemDataFlags_HasOpen = 1 << 1,
    ImGuiNextItemDataFlags_HasShortcut = 1 << 2,
    ImGuiNextItemDataFlags_HasRefVal = 1 << 3,
    ImGuiNextItemDataFlags_HasStorageID = 1 << 4,
} ImGuiNextItemDataFlags_;
struct ImGuiNextItemData {
    ImGuiNextItemDataFlags Flags;
    ImGuiItemFlags ItemFlags;
    ImGuiID FocusScopeId;
    ImGuiSelectionUserData SelectionUserData;
    float Width;
    ImGuiKeyChord Shortcut;
    ImGuiInputFlags ShortcutFlags;
    _Bool OpenVal;
    ImU8 OpenCond;
    ImGuiDataTypeStorage RefVal;
    ImGuiID StorageId;
};
struct ImGuiLastItemData {
    ImGuiID ID;
    ImGuiItemFlags InFlags;
    ImGuiItemStatusFlags StatusFlags;
    ImRect Rect;
    ImRect NavRect;
    ImRect DisplayRect;
    ImRect ClipRect;
    ImGuiKeyChord Shortcut;
};
struct ImGuiTreeNodeStackData {
    ImGuiID ID;
    ImGuiTreeNodeFlags TreeFlags;
    ImGuiItemFlags InFlags;
    ImRect NavRect;
};
struct ImGuiStackSizes {
    short SizeOfIDStack;
    short SizeOfColorStack;
    short SizeOfStyleVarStack;
    short SizeOfFontStack;
    short SizeOfFocusScopeStack;
    short SizeOfGroupStack;
    short SizeOfItemFlagsStack;
    short SizeOfBeginPopupStack;
    short SizeOfDisabledStack;
};
typedef struct ImGuiWindowStackData ImGuiWindowStackData;
struct ImGuiWindowStackData {
    ImGuiWindow* Window;
    ImGuiLastItemData ParentLastItemDataBackup;
    ImGuiStackSizes StackSizesOnBegin;
    _Bool DisabledOverrideReenable;
};
typedef struct ImGuiShrinkWidthItem ImGuiShrinkWidthItem;
struct ImGuiShrinkWidthItem {
    int Index;
    float Width;
    float InitialWidth;
};
typedef struct ImGuiPtrOrIndex ImGuiPtrOrIndex;
struct ImGuiPtrOrIndex {
    void* Ptr;
    int Index;
};
typedef enum {
    ImGuiPopupPositionPolicy_Default,
    ImGuiPopupPositionPolicy_ComboBox,
    ImGuiPopupPositionPolicy_Tooltip,
} ImGuiPopupPositionPolicy;
struct ImGuiPopupData {
    ImGuiID PopupId;
    ImGuiWindow* Window;
    ImGuiWindow* RestoreNavWindow;
    int ParentNavLayer;
    int OpenFrameCount;
    ImGuiID OpenParentId;
    ImVec2 OpenPopupPos;
    ImVec2 OpenMousePos;
};
typedef struct ImBitArray_ImGuiKey_NamedKey_COUNT__lessImGuiKey_NamedKey_BEGIN {
    ImU32 Storage[(ImGuiKey_NamedKey_COUNT + 31) >> 5];
} ImBitArray_ImGuiKey_NamedKey_COUNT__lessImGuiKey_NamedKey_BEGIN;
typedef ImBitArray_ImGuiKey_NamedKey_COUNT__lessImGuiKey_NamedKey_BEGIN ImBitArrayForNamedKeys;
typedef enum {
    ImGuiInputEventType_None = 0,
    ImGuiInputEventType_MousePos,
    ImGuiInputEventType_MouseWheel,
    ImGuiInputEventType_MouseButton,
    ImGuiInputEventType_MouseViewport,
    ImGuiInputEventType_Key,
    ImGuiInputEventType_Text,
    ImGuiInputEventType_Focus,
    ImGuiInputEventType_COUNT
} ImGuiInputEventType;
typedef enum { ImGuiInputSource_None = 0, ImGuiInputSource_Mouse, ImGuiInputSource_Keyboard, ImGuiInputSource_Gamepad, ImGuiInputSource_COUNT } ImGuiInputSource;
typedef struct ImGuiInputEventMousePos ImGuiInputEventMousePos;
struct ImGuiInputEventMousePos {
    float PosX, PosY;
    ImGuiMouseSource MouseSource;
};
typedef struct ImGuiInputEventMouseWheel ImGuiInputEventMouseWheel;
struct ImGuiInputEventMouseWheel {
    float WheelX, WheelY;
    ImGuiMouseSource MouseSource;
};
typedef struct ImGuiInputEventMouseButton ImGuiInputEventMouseButton;
struct ImGuiInputEventMouseButton {
    int Button;
    _Bool Down;
    ImGuiMouseSource MouseSource;
};
typedef struct ImGuiInputEventMouseViewport ImGuiInputEventMouseViewport;
struct ImGuiInputEventMouseViewport {
    ImGuiID HoveredViewportID;
};
typedef struct ImGuiInputEventKey ImGuiInputEventKey;
struct ImGuiInputEventKey {
    ImGuiKey Key;
    _Bool Down;
    float AnalogValue;
};
typedef struct ImGuiInputEventText ImGuiInputEventText;
struct ImGuiInputEventText {
    unsigned int Char;
};
typedef struct ImGuiInputEventAppFocused ImGuiInputEventAppFocused;
struct ImGuiInputEventAppFocused {
    _Bool Focused;
};
typedef struct ImGuiInputEvent ImGuiInputEvent;
struct ImGuiInputEvent {
    ImGuiInputEventType Type;
    ImGuiInputSource Source;
    ImU32 EventId;
    union {
        ImGuiInputEventMousePos MousePos;
        ImGuiInputEventMouseWheel MouseWheel;
        ImGuiInputEventMouseButton MouseButton;
        ImGuiInputEventMouseViewport MouseViewport;
        ImGuiInputEventKey Key;
        ImGuiInputEventText Text;
        ImGuiInputEventAppFocused AppFocused;
    };
    _Bool AddedByTestEngine;
};
typedef ImS16 ImGuiKeyRoutingIndex;
typedef struct ImGuiKeyRoutingData ImGuiKeyRoutingData;
struct ImGuiKeyRoutingData {
    ImGuiKeyRoutingIndex NextEntryIndex;
    ImU16 Mods;
    ImU8 RoutingCurrScore;
    ImU8 RoutingNextScore;
    ImGuiID RoutingCurr;
    ImGuiID RoutingNext;
};
typedef struct ImGuiKeyRoutingTable ImGuiKeyRoutingTable;
typedef struct ImVector_ImGuiKeyRoutingData {
    int Size;
    int Capacity;
    ImGuiKeyRoutingData* Data;
} ImVector_ImGuiKeyRoutingData;
struct ImGuiKeyRoutingTable {
    ImGuiKeyRoutingIndex Index[ImGuiKey_NamedKey_COUNT];
    ImVector_ImGuiKeyRoutingData Entries;
    ImVector_ImGuiKeyRoutingData EntriesNext;
};
typedef struct ImGuiKeyOwnerData ImGuiKeyOwnerData;
struct ImGuiKeyOwnerData {
    ImGuiID OwnerCurr;
    ImGuiID OwnerNext;
    _Bool LockThisFrame;
    _Bool LockUntilRelease;
};
typedef enum {
    ImGuiInputFlags_RepeatRateDefault = 1 << 1,
    ImGuiInputFlags_RepeatRateNavMove = 1 << 2,
    ImGuiInputFlags_RepeatRateNavTweak = 1 << 3,
    ImGuiInputFlags_RepeatUntilRelease = 1 << 4,
    ImGuiInputFlags_RepeatUntilKeyModsChange = 1 << 5,
    ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone = 1 << 6,
    ImGuiInputFlags_RepeatUntilOtherKeyPress = 1 << 7,
    ImGuiInputFlags_LockThisFrame = 1 << 20,
    ImGuiInputFlags_LockUntilRelease = 1 << 21,
    ImGuiInputFlags_CondHovered = 1 << 22,
    ImGuiInputFlags_CondActive = 1 << 23,
    ImGuiInputFlags_CondDefault_ = ImGuiInputFlags_CondHovered | ImGuiInputFlags_CondActive,
    ImGuiInputFlags_RepeatRateMask_ = ImGuiInputFlags_RepeatRateDefault | ImGuiInputFlags_RepeatRateNavMove | ImGuiInputFlags_RepeatRateNavTweak,
    ImGuiInputFlags_RepeatUntilMask_ =
            ImGuiInputFlags_RepeatUntilRelease | ImGuiInputFlags_RepeatUntilKeyModsChange | ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone | ImGuiInputFlags_RepeatUntilOtherKeyPress,
    ImGuiInputFlags_RepeatMask_ = ImGuiInputFlags_Repeat | ImGuiInputFlags_RepeatRateMask_ | ImGuiInputFlags_RepeatUntilMask_,
    ImGuiInputFlags_CondMask_ = ImGuiInputFlags_CondHovered | ImGuiInputFlags_CondActive,
    ImGuiInputFlags_RouteTypeMask_ = ImGuiInputFlags_RouteActive | ImGuiInputFlags_RouteFocused | ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteAlways,
    ImGuiInputFlags_RouteOptionsMask_ = ImGuiInputFlags_RouteOverFocused | ImGuiInputFlags_RouteOverActive | ImGuiInputFlags_RouteUnlessBgFocused | ImGuiInputFlags_RouteFromRootWindow,
    ImGuiInputFlags_SupportedByIsKeyPressed = ImGuiInputFlags_RepeatMask_,
    ImGuiInputFlags_SupportedByIsMouseClicked = ImGuiInputFlags_Repeat,
    ImGuiInputFlags_SupportedByShortcut = ImGuiInputFlags_RepeatMask_ | ImGuiInputFlags_RouteTypeMask_ | ImGuiInputFlags_RouteOptionsMask_,
    ImGuiInputFlags_SupportedBySetNextItemShortcut = ImGuiInputFlags_RepeatMask_ | ImGuiInputFlags_RouteTypeMask_ | ImGuiInputFlags_RouteOptionsMask_ | ImGuiInputFlags_Tooltip,
    ImGuiInputFlags_SupportedBySetKeyOwner = ImGuiInputFlags_LockThisFrame | ImGuiInputFlags_LockUntilRelease,
    ImGuiInputFlags_SupportedBySetItemKeyOwner = ImGuiInputFlags_SupportedBySetKeyOwner | ImGuiInputFlags_CondMask_,
} ImGuiInputFlagsPrivate_;
typedef struct ImGuiListClipperRange ImGuiListClipperRange;
struct ImGuiListClipperRange {
    int Min;
    int Max;
    _Bool PosToIndexConvert;
    ImS8 PosToIndexOffsetMin;
    ImS8 PosToIndexOffsetMax;
};
typedef struct ImGuiListClipperData ImGuiListClipperData;
typedef struct ImVector_ImGuiListClipperRange {
    int Size;
    int Capacity;
    ImGuiListClipperRange* Data;
} ImVector_ImGuiListClipperRange;
struct ImGuiListClipperData {
    ImGuiListClipper* ListClipper;
    float LossynessOffset;
    int StepNo;
    int ItemsFrozen;
    ImVector_ImGuiListClipperRange Ranges;
};
typedef enum {
    ImGuiActivateFlags_None = 0,
    ImGuiActivateFlags_PreferInput = 1 << 0,
    ImGuiActivateFlags_PreferTweak = 1 << 1,
    ImGuiActivateFlags_TryToPreserveState = 1 << 2,
    ImGuiActivateFlags_FromTabbing = 1 << 3,
    ImGuiActivateFlags_FromShortcut = 1 << 4,
} ImGuiActivateFlags_;
typedef enum {
    ImGuiScrollFlags_None = 0,
    ImGuiScrollFlags_KeepVisibleEdgeX = 1 << 0,
    ImGuiScrollFlags_KeepVisibleEdgeY = 1 << 1,
    ImGuiScrollFlags_KeepVisibleCenterX = 1 << 2,
    ImGuiScrollFlags_KeepVisibleCenterY = 1 << 3,
    ImGuiScrollFlags_AlwaysCenterX = 1 << 4,
    ImGuiScrollFlags_AlwaysCenterY = 1 << 5,
    ImGuiScrollFlags_NoScrollParent = 1 << 6,
    ImGuiScrollFlags_MaskX_ = ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleCenterX | ImGuiScrollFlags_AlwaysCenterX,
    ImGuiScrollFlags_MaskY_ = ImGuiScrollFlags_KeepVisibleEdgeY | ImGuiScrollFlags_KeepVisibleCenterY | ImGuiScrollFlags_AlwaysCenterY,
} ImGuiScrollFlags_;
typedef enum {
    ImGuiNavHighlightFlags_None = 0,
    ImGuiNavHighlightFlags_Compact = 1 << 1,
    ImGuiNavHighlightFlags_AlwaysDraw = 1 << 2,
    ImGuiNavHighlightFlags_NoRounding = 1 << 3,
} ImGuiNavHighlightFlags_;
typedef enum {
    ImGuiNavMoveFlags_None = 0,
    ImGuiNavMoveFlags_LoopX = 1 << 0,
    ImGuiNavMoveFlags_LoopY = 1 << 1,
    ImGuiNavMoveFlags_WrapX = 1 << 2,
    ImGuiNavMoveFlags_WrapY = 1 << 3,
    ImGuiNavMoveFlags_WrapMask_ = ImGuiNavMoveFlags_LoopX | ImGuiNavMoveFlags_LoopY | ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_WrapY,
    ImGuiNavMoveFlags_AllowCurrentNavId = 1 << 4,
    ImGuiNavMoveFlags_AlsoScoreVisibleSet = 1 << 5,
    ImGuiNavMoveFlags_ScrollToEdgeY = 1 << 6,
    ImGuiNavMoveFlags_Forwarded = 1 << 7,
    ImGuiNavMoveFlags_DebugNoResult = 1 << 8,
    ImGuiNavMoveFlags_FocusApi = 1 << 9,
    ImGuiNavMoveFlags_IsTabbing = 1 << 10,
    ImGuiNavMoveFlags_IsPageMove = 1 << 11,
    ImGuiNavMoveFlags_Activate = 1 << 12,
    ImGuiNavMoveFlags_NoSelect = 1 << 13,
    ImGuiNavMoveFlags_NoSetNavHighlight = 1 << 14,
    ImGuiNavMoveFlags_NoClearActiveId = 1 << 15,
} ImGuiNavMoveFlags_;
typedef enum { ImGuiNavLayer_Main = 0, ImGuiNavLayer_Menu = 1, ImGuiNavLayer_COUNT } ImGuiNavLayer;
struct ImGuiNavItemData {
    ImGuiWindow* Window;
    ImGuiID ID;
    ImGuiID FocusScopeId;
    ImRect RectRel;
    ImGuiItemFlags InFlags;
    float DistBox;
    float DistCenter;
    float DistAxial;
    ImGuiSelectionUserData SelectionUserData;
};
typedef struct ImGuiFocusScopeData ImGuiFocusScopeData;
struct ImGuiFocusScopeData {
    ImGuiID ID;
    ImGuiID WindowID;
};
typedef enum {
    ImGuiTypingSelectFlags_None = 0,
    ImGuiTypingSelectFlags_AllowBackspace = 1 << 0,
    ImGuiTypingSelectFlags_AllowSingleCharMode = 1 << 1,
} ImGuiTypingSelectFlags_;
struct ImGuiTypingSelectRequest {
    ImGuiTypingSelectFlags Flags;
    int SearchBufferLen;
    const char* SearchBuffer;
    _Bool SelectRequest;
    _Bool SingleCharMode;
    ImS8 SingleCharSize;
};
struct ImGuiTypingSelectState {
    ImGuiTypingSelectRequest Request;
    char SearchBuffer[64];
    ImGuiID FocusScope;
    int LastRequestFrame;
    float LastRequestTime;
    _Bool SingleCharModeLock;
};
typedef enum {
    ImGuiOldColumnFlags_None = 0,
    ImGuiOldColumnFlags_NoBorder = 1 << 0,
    ImGuiOldColumnFlags_NoResize = 1 << 1,
    ImGuiOldColumnFlags_NoPreserveWidths = 1 << 2,
    ImGuiOldColumnFlags_NoForceWithinWindow = 1 << 3,
    ImGuiOldColumnFlags_GrowParentContentsSize = 1 << 4,
} ImGuiOldColumnFlags_;
struct ImGuiOldColumnData {
    float OffsetNorm;
    float OffsetNormBeforeResize;
    ImGuiOldColumnFlags Flags;
    ImRect ClipRect;
};
typedef struct ImVector_ImGuiOldColumnData {
    int Size;
    int Capacity;
    ImGuiOldColumnData* Data;
} ImVector_ImGuiOldColumnData;
struct ImGuiOldColumns {
    ImGuiID ID;
    ImGuiOldColumnFlags Flags;
    _Bool IsFirstFrame;
    _Bool IsBeingResized;
    int Current;
    int Count;
    float OffMinX, OffMaxX;
    float LineMinY, LineMaxY;
    float HostCursorPosY;
    float HostCursorMaxPosX;
    ImRect HostInitialClipRect;
    ImRect HostBackupClipRect;
    ImRect HostBackupParentWorkRect;
    ImVector_ImGuiOldColumnData Columns;
    ImDrawListSplitter Splitter;
};
struct ImGuiBoxSelectState {
    ImGuiID ID;
    _Bool IsActive;
    _Bool IsStarting;
    _Bool IsStartedFromVoid;
    _Bool IsStartedSetNavIdOnce;
    _Bool RequestClear;
    ImGuiKeyChord KeyMods : 16;
    ImVec2 StartPosRel;
    ImVec2 EndPosRel;
    ImVec2 ScrollAccum;
    ImGuiWindow* Window;
    _Bool UnclipMode;
    ImRect UnclipRect;
    ImRect BoxSelectRectPrev;
    ImRect BoxSelectRectCurr;
};
struct ImGuiMultiSelectTempData {
    ImGuiMultiSelectIO IO;
    ImGuiMultiSelectState* Storage;
    ImGuiID FocusScopeId;
    ImGuiMultiSelectFlags Flags;
    ImVec2 ScopeRectMin;
    ImVec2 BackupCursorMaxPos;
    ImGuiSelectionUserData LastSubmittedItem;
    ImGuiID BoxSelectId;
    ImGuiKeyChord KeyMods;
    ImS8 LoopRequestSetAll;
    _Bool IsEndIO;
    _Bool IsFocused;
    _Bool IsKeyboardSetRange;
    _Bool NavIdPassedBy;
    _Bool RangeSrcPassedBy;
    _Bool RangeDstPassedBy;
};
struct ImGuiMultiSelectState {
    ImGuiWindow* Window;
    ImGuiID ID;
    int LastFrameActive;
    int LastSelectionSize;
    ImS8 RangeSelected;
    ImS8 NavIdSelected;
    ImGuiSelectionUserData RangeSrcItem;
    ImGuiSelectionUserData NavIdItem;
};
typedef enum {
    ImGuiDockNodeFlags_DockSpace = 1 << 10,
    ImGuiDockNodeFlags_CentralNode = 1 << 11,
    ImGuiDockNodeFlags_NoTabBar = 1 << 12,
    ImGuiDockNodeFlags_HiddenTabBar = 1 << 13,
    ImGuiDockNodeFlags_NoWindowMenuButton = 1 << 14,
    ImGuiDockNodeFlags_NoCloseButton = 1 << 15,
    ImGuiDockNodeFlags_NoResizeX = 1 << 16,
    ImGuiDockNodeFlags_NoResizeY = 1 << 17,
    ImGuiDockNodeFlags_DockedWindowsInFocusRoute = 1 << 18,
    ImGuiDockNodeFlags_NoDockingSplitOther = 1 << 19,
    ImGuiDockNodeFlags_NoDockingOverMe = 1 << 20,
    ImGuiDockNodeFlags_NoDockingOverOther = 1 << 21,
    ImGuiDockNodeFlags_NoDockingOverEmpty = 1 << 22,
    ImGuiDockNodeFlags_NoDocking = ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoDockingOverOther | ImGuiDockNodeFlags_NoDockingOverEmpty | ImGuiDockNodeFlags_NoDockingSplit |
                                   ImGuiDockNodeFlags_NoDockingSplitOther,
    ImGuiDockNodeFlags_SharedFlagsInheritMask_ = ~0,
    ImGuiDockNodeFlags_NoResizeFlagsMask_ = ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoResizeX | ImGuiDockNodeFlags_NoResizeY,
    ImGuiDockNodeFlags_LocalFlagsTransferMask_ = ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoResizeFlagsMask_ | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_CentralNode |
                                                 ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton,
    ImGuiDockNodeFlags_SavedFlagsMask_ = ImGuiDockNodeFlags_NoResizeFlagsMask_ | ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_CentralNode | ImGuiDockNodeFlags_NoTabBar |
                                         ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton,
} ImGuiDockNodeFlagsPrivate_;
typedef enum {
    ImGuiDataAuthority_Auto,
    ImGuiDataAuthority_DockNode,
    ImGuiDataAuthority_Window,
} ImGuiDataAuthority_;
typedef enum {
    ImGuiDockNodeState_Unknown,
    ImGuiDockNodeState_HostWindowHiddenBecauseSingleWindow,
    ImGuiDockNodeState_HostWindowHiddenBecauseWindowsAreResizing,
    ImGuiDockNodeState_HostWindowVisible,
} ImGuiDockNodeState;
typedef struct ImVector_ImGuiWindowPtr {
    int Size;
    int Capacity;
    ImGuiWindow** Data;
} ImVector_ImGuiWindowPtr;
struct ImGuiDockNode {
    ImGuiID ID;
    ImGuiDockNodeFlags SharedFlags;
    ImGuiDockNodeFlags LocalFlags;
    ImGuiDockNodeFlags LocalFlagsInWindows;
    ImGuiDockNodeFlags MergedFlags;
    ImGuiDockNodeState State;
    ImGuiDockNode* ParentNode;
    ImGuiDockNode* ChildNodes[2];
    ImVector_ImGuiWindowPtr Windows;
    ImGuiTabBar* TabBar;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 SizeRef;
    ImGuiAxis SplitAxis;
    ImGuiWindowClass WindowClass;
    ImU32 LastBgColor;
    ImGuiWindow* HostWindow;
    ImGuiWindow* VisibleWindow;
    ImGuiDockNode* CentralNode;
    ImGuiDockNode* OnlyNodeWithWindows;
    int CountNodeWithWindows;
    int LastFrameAlive;
    int LastFrameActive;
    int LastFrameFocused;
    ImGuiID LastFocusedNodeId;
    ImGuiID SelectedTabId;
    ImGuiID WantCloseTabId;
    ImGuiID RefViewportId;
    ImGuiDataAuthority AuthorityForPos : 3;
    ImGuiDataAuthority AuthorityForSize : 3;
    ImGuiDataAuthority AuthorityForViewport : 3;
    _Bool IsVisible : 1;
    _Bool IsFocused : 1;
    _Bool IsBgDrawnThisFrame : 1;
    _Bool HasCloseButton : 1;
    _Bool HasWindowMenuButton : 1;
    _Bool HasCentralNodeChild : 1;
    _Bool WantCloseAll : 1;
    _Bool WantLockSizeOnce : 1;
    _Bool WantMouseMove : 1;
    _Bool WantHiddenTabBarUpdate : 1;
    _Bool WantHiddenTabBarToggle : 1;
};
typedef enum {
    ImGuiWindowDockStyleCol_Text,
    ImGuiWindowDockStyleCol_TabHovered,
    ImGuiWindowDockStyleCol_TabFocused,
    ImGuiWindowDockStyleCol_TabSelected,
    ImGuiWindowDockStyleCol_TabSelectedOverline,
    ImGuiWindowDockStyleCol_TabDimmed,
    ImGuiWindowDockStyleCol_TabDimmedSelected,
    ImGuiWindowDockStyleCol_TabDimmedSelectedOverline,
    ImGuiWindowDockStyleCol_COUNT
} ImGuiWindowDockStyleCol;
struct ImGuiWindowDockStyle {
    ImU32 Colors[ImGuiWindowDockStyleCol_COUNT];
};
typedef struct ImVector_ImGuiDockRequest {
    int Size;
    int Capacity;
    ImGuiDockRequest* Data;
} ImVector_ImGuiDockRequest;
typedef struct ImVector_ImGuiDockNodeSettings {
    int Size;
    int Capacity;
    ImGuiDockNodeSettings* Data;
} ImVector_ImGuiDockNodeSettings;
struct ImGuiDockContext {
    ImGuiStorage Nodes;
    ImVector_ImGuiDockRequest Requests;
    ImVector_ImGuiDockNodeSettings NodesSettings;
    _Bool WantFullRebuild;
};
typedef struct ImGuiViewportP ImGuiViewportP;
struct ImGuiViewportP {
    ImGuiViewport _ImGuiViewport;
    ImGuiWindow* Window;
    int Idx;
    int LastFrameActive;
    int LastFocusedStampCount;
    ImGuiID LastNameHash;
    ImVec2 LastPos;
    float Alpha;
    float LastAlpha;
    _Bool LastFocusedHadNavWindow;
    short PlatformMonitor;
    int BgFgDrawListsLastFrame[2];
    ImDrawList* BgFgDrawLists[2];
    ImDrawData DrawDataP;
    ImDrawDataBuilder DrawDataBuilder;
    ImVec2 LastPlatformPos;
    ImVec2 LastPlatformSize;
    ImVec2 LastRendererSize;
    ImVec2 WorkOffsetMin;
    ImVec2 WorkOffsetMax;
    ImVec2 BuildWorkOffsetMin;
    ImVec2 BuildWorkOffsetMax;
};
struct ImGuiWindowSettings {
    ImGuiID ID;
    ImVec2ih Pos;
    ImVec2ih Size;
    ImVec2ih ViewportPos;
    ImGuiID ViewportId;
    ImGuiID DockId;
    ImGuiID ClassId;
    short DockOrder;
    _Bool Collapsed;
    _Bool IsChild;
    _Bool WantApply;
    _Bool WantDelete;
};
struct ImGuiSettingsHandler {
    const char* TypeName;
    ImGuiID TypeHash;
    void (*ClearAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    void (*ReadInitFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    void* (*ReadOpenFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);
    void (*ReadLineFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line);
    void (*ApplyAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    void (*WriteAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf);
    void* UserData;
};
typedef enum {
    ImGuiLocKey_VersionStr = 0,
    ImGuiLocKey_TableSizeOne = 1,
    ImGuiLocKey_TableSizeAllFit = 2,
    ImGuiLocKey_TableSizeAllDefault = 3,
    ImGuiLocKey_TableResetOrder = 4,
    ImGuiLocKey_WindowingMainMenuBar = 5,
    ImGuiLocKey_WindowingPopup = 6,
    ImGuiLocKey_WindowingUntitled = 7,
    ImGuiLocKey_CopyLink = 8,
    ImGuiLocKey_DockingHideTabBar = 9,
    ImGuiLocKey_DockingHoldShiftToDock = 10,
    ImGuiLocKey_DockingDragToUndockOrMoveNode = 11,
    ImGuiLocKey_COUNT = 12,
} ImGuiLocKey;
struct ImGuiLocEntry {
    ImGuiLocKey Key;
    const char* Text;
};
typedef enum {
    ImGuiDebugLogFlags_None = 0,
    ImGuiDebugLogFlags_EventActiveId = 1 << 0,
    ImGuiDebugLogFlags_EventFocus = 1 << 1,
    ImGuiDebugLogFlags_EventPopup = 1 << 2,
    ImGuiDebugLogFlags_EventNav = 1 << 3,
    ImGuiDebugLogFlags_EventClipper = 1 << 4,
    ImGuiDebugLogFlags_EventSelection = 1 << 5,
    ImGuiDebugLogFlags_EventIO = 1 << 6,
    ImGuiDebugLogFlags_EventInputRouting = 1 << 7,
    ImGuiDebugLogFlags_EventDocking = 1 << 8,
    ImGuiDebugLogFlags_EventViewport = 1 << 9,
    ImGuiDebugLogFlags_EventMask_ = ImGuiDebugLogFlags_EventActiveId | ImGuiDebugLogFlags_EventFocus | ImGuiDebugLogFlags_EventPopup | ImGuiDebugLogFlags_EventNav | ImGuiDebugLogFlags_EventClipper |
                                    ImGuiDebugLogFlags_EventSelection | ImGuiDebugLogFlags_EventIO | ImGuiDebugLogFlags_EventInputRouting | ImGuiDebugLogFlags_EventDocking |
                                    ImGuiDebugLogFlags_EventViewport,
    ImGuiDebugLogFlags_OutputToTTY = 1 << 20,
    ImGuiDebugLogFlags_OutputToTestEngine = 1 << 21,
} ImGuiDebugLogFlags_;
typedef struct ImGuiDebugAllocEntry ImGuiDebugAllocEntry;
struct ImGuiDebugAllocEntry {
    int FrameCount;
    ImS16 AllocCount;
    ImS16 FreeCount;
};
typedef struct ImGuiDebugAllocInfo ImGuiDebugAllocInfo;
struct ImGuiDebugAllocInfo {
    int TotalAllocCount;
    int TotalFreeCount;
    ImS16 LastEntriesIdx;
    ImGuiDebugAllocEntry LastEntriesBuf[6];
};
struct ImGuiMetricsConfig {
    _Bool ShowDebugLog;
    _Bool ShowIDStackTool;
    _Bool ShowWindowsRects;
    _Bool ShowWindowsBeginOrder;
    _Bool ShowTablesRects;
    _Bool ShowDrawCmdMesh;
    _Bool ShowDrawCmdBoundingBoxes;
    _Bool ShowTextEncodingViewer;
    _Bool ShowAtlasTintedWithTextColor;
    _Bool ShowDockingNodes;
    int ShowWindowsRectsType;
    int ShowTablesRectsType;
    int HighlightMonitorIdx;
    ImGuiID HighlightViewportID;
};
typedef struct ImGuiStackLevelInfo ImGuiStackLevelInfo;
struct ImGuiStackLevelInfo {
    ImGuiID ID;
    ImS8 QueryFrameCount;
    _Bool QuerySuccess;
    ImGuiDataType DataType : 8;
    char Desc[57];
};
typedef struct ImGuiIDStackTool ImGuiIDStackTool;
typedef struct ImVector_ImGuiStackLevelInfo {
    int Size;
    int Capacity;
    ImGuiStackLevelInfo* Data;
} ImVector_ImGuiStackLevelInfo;
struct ImGuiIDStackTool {
    int LastActiveFrame;
    int StackLevel;
    ImGuiID QueryId;
    ImVector_ImGuiStackLevelInfo Results;
    _Bool CopyToClipboardOnCtrlC;
    float CopyToClipboardLastTime;
};
typedef void (*ImGuiContextHookCallback)(ImGuiContext* ctx, ImGuiContextHook* hook);
typedef enum {
    ImGuiContextHookType_NewFramePre,
    ImGuiContextHookType_NewFramePost,
    ImGuiContextHookType_EndFramePre,
    ImGuiContextHookType_EndFramePost,
    ImGuiContextHookType_RenderPre,
    ImGuiContextHookType_RenderPost,
    ImGuiContextHookType_Shutdown,
    ImGuiContextHookType_PendingRemoval_
} ImGuiContextHookType;
struct ImGuiContextHook {
    ImGuiID HookId;
    ImGuiContextHookType Type;
    ImGuiID Owner;
    ImGuiContextHookCallback Callback;
    void* UserData;
};
typedef struct ImVector_ImGuiInputEvent {
    int Size;
    int Capacity;
    ImGuiInputEvent* Data;
} ImVector_ImGuiInputEvent;
typedef struct ImVector_ImGuiWindowStackData {
    int Size;
    int Capacity;
    ImGuiWindowStackData* Data;
} ImVector_ImGuiWindowStackData;
typedef struct ImVector_ImGuiColorMod {
    int Size;
    int Capacity;
    ImGuiColorMod* Data;
} ImVector_ImGuiColorMod;
typedef struct ImVector_ImGuiStyleMod {
    int Size;
    int Capacity;
    ImGuiStyleMod* Data;
} ImVector_ImGuiStyleMod;
typedef struct ImVector_ImGuiFocusScopeData {
    int Size;
    int Capacity;
    ImGuiFocusScopeData* Data;
} ImVector_ImGuiFocusScopeData;
typedef struct ImVector_ImGuiItemFlags {
    int Size;
    int Capacity;
    ImGuiItemFlags* Data;
} ImVector_ImGuiItemFlags;
typedef struct ImVector_ImGuiGroupData {
    int Size;
    int Capacity;
    ImGuiGroupData* Data;
} ImVector_ImGuiGroupData;
typedef struct ImVector_ImGuiPopupData {
    int Size;
    int Capacity;
    ImGuiPopupData* Data;
} ImVector_ImGuiPopupData;
typedef struct ImVector_ImGuiTreeNodeStackData {
    int Size;
    int Capacity;
    ImGuiTreeNodeStackData* Data;
} ImVector_ImGuiTreeNodeStackData;
typedef struct ImVector_ImGuiViewportPPtr {
    int Size;
    int Capacity;
    ImGuiViewportP** Data;
} ImVector_ImGuiViewportPPtr;
typedef struct ImVector_unsigned_char {
    int Size;
    int Capacity;
    unsigned char* Data;
} ImVector_unsigned_char;
typedef struct ImVector_ImGuiListClipperData {
    int Size;
    int Capacity;
    ImGuiListClipperData* Data;
} ImVector_ImGuiListClipperData;
typedef struct ImVector_ImGuiTableTempData {
    int Size;
    int Capacity;
    ImGuiTableTempData* Data;
} ImVector_ImGuiTableTempData;
typedef struct ImVector_ImGuiTable {
    int Size;
    int Capacity;
    ImGuiTable* Data;
} ImVector_ImGuiTable;
typedef struct ImPool_ImGuiTable {
    ImVector_ImGuiTable Buf;
    ImGuiStorage Map;
    ImPoolIdx FreeIdx;
    ImPoolIdx AliveCount;
} ImPool_ImGuiTable;
typedef struct ImVector_ImGuiTabBar {
    int Size;
    int Capacity;
    ImGuiTabBar* Data;
} ImVector_ImGuiTabBar;
typedef struct ImPool_ImGuiTabBar {
    ImVector_ImGuiTabBar Buf;
    ImGuiStorage Map;
    ImPoolIdx FreeIdx;
    ImPoolIdx AliveCount;
} ImPool_ImGuiTabBar;
typedef struct ImVector_ImGuiPtrOrIndex {
    int Size;
    int Capacity;
    ImGuiPtrOrIndex* Data;
} ImVector_ImGuiPtrOrIndex;
typedef struct ImVector_ImGuiShrinkWidthItem {
    int Size;
    int Capacity;
    ImGuiShrinkWidthItem* Data;
} ImVector_ImGuiShrinkWidthItem;
typedef struct ImVector_ImGuiMultiSelectTempData {
    int Size;
    int Capacity;
    ImGuiMultiSelectTempData* Data;
} ImVector_ImGuiMultiSelectTempData;
typedef struct ImVector_ImGuiMultiSelectState {
    int Size;
    int Capacity;
    ImGuiMultiSelectState* Data;
} ImVector_ImGuiMultiSelectState;
typedef struct ImPool_ImGuiMultiSelectState {
    ImVector_ImGuiMultiSelectState Buf;
    ImGuiStorage Map;
    ImPoolIdx FreeIdx;
    ImPoolIdx AliveCount;
} ImPool_ImGuiMultiSelectState;
typedef struct ImVector_ImGuiID {
    int Size;
    int Capacity;
    ImGuiID* Data;
} ImVector_ImGuiID;
typedef struct ImVector_ImGuiSettingsHandler {
    int Size;
    int Capacity;
    ImGuiSettingsHandler* Data;
} ImVector_ImGuiSettingsHandler;
typedef struct ImChunkStream_ImGuiWindowSettings {
    ImVector_char Buf;
} ImChunkStream_ImGuiWindowSettings;
typedef struct ImChunkStream_ImGuiTableSettings {
    ImVector_char Buf;
} ImChunkStream_ImGuiTableSettings;
typedef struct ImVector_ImGuiContextHook {
    int Size;
    int Capacity;
    ImGuiContextHook* Data;
} ImVector_ImGuiContextHook;
struct ImGuiContext {
    _Bool Initialized;
    _Bool FontAtlasOwnedByContext;
    ImGuiIO IO;
    ImGuiPlatformIO PlatformIO;
    ImGuiStyle Style;
    ImGuiConfigFlags ConfigFlagsCurrFrame;
    ImGuiConfigFlags ConfigFlagsLastFrame;
    ImFont* Font;
    float FontSize;
    float FontBaseSize;
    float FontScale;
    float CurrentDpiScale;
    ImDrawListSharedData DrawListSharedData;
    double Time;
    int FrameCount;
    int FrameCountEnded;
    int FrameCountPlatformEnded;
    int FrameCountRendered;
    _Bool WithinFrameScope;
    _Bool WithinFrameScopeWithImplicitWindow;
    _Bool WithinEndChild;
    _Bool GcCompactAll;
    _Bool TestEngineHookItems;
    void* TestEngine;
    char ContextName[16];
    ImVector_ImGuiInputEvent InputEventsQueue;
    ImVector_ImGuiInputEvent InputEventsTrail;
    ImGuiMouseSource InputEventsNextMouseSource;
    ImU32 InputEventsNextEventId;
    ImVector_ImGuiWindowPtr Windows;
    ImVector_ImGuiWindowPtr WindowsFocusOrder;
    ImVector_ImGuiWindowPtr WindowsTempSortBuffer;
    ImVector_ImGuiWindowStackData CurrentWindowStack;
    ImGuiStorage WindowsById;
    int WindowsActiveCount;
    ImVec2 WindowsHoverPadding;
    ImGuiID DebugBreakInWindow;
    ImGuiWindow* CurrentWindow;
    ImGuiWindow* HoveredWindow;
    ImGuiWindow* HoveredWindowUnderMovingWindow;
    ImGuiWindow* HoveredWindowBeforeClear;
    ImGuiWindow* MovingWindow;
    ImGuiWindow* WheelingWindow;
    ImVec2 WheelingWindowRefMousePos;
    int WheelingWindowStartFrame;
    int WheelingWindowScrolledFrame;
    float WheelingWindowReleaseTimer;
    ImVec2 WheelingWindowWheelRemainder;
    ImVec2 WheelingAxisAvg;
    ImGuiID DebugHookIdInfo;
    ImGuiID HoveredId;
    ImGuiID HoveredIdPreviousFrame;
    float HoveredIdTimer;
    float HoveredIdNotActiveTimer;
    _Bool HoveredIdAllowOverlap;
    _Bool HoveredIdIsDisabled;
    _Bool ItemUnclipByLog;
    ImGuiID ActiveId;
    ImGuiID ActiveIdIsAlive;
    float ActiveIdTimer;
    _Bool ActiveIdIsJustActivated;
    _Bool ActiveIdAllowOverlap;
    _Bool ActiveIdNoClearOnFocusLoss;
    _Bool ActiveIdHasBeenPressedBefore;
    _Bool ActiveIdHasBeenEditedBefore;
    _Bool ActiveIdHasBeenEditedThisFrame;
    _Bool ActiveIdFromShortcut;
    int ActiveIdMouseButton : 8;
    ImVec2 ActiveIdClickOffset;
    ImGuiWindow* ActiveIdWindow;
    ImGuiInputSource ActiveIdSource;
    ImGuiID ActiveIdPreviousFrame;
    _Bool ActiveIdPreviousFrameIsAlive;
    _Bool ActiveIdPreviousFrameHasBeenEditedBefore;
    ImGuiWindow* ActiveIdPreviousFrameWindow;
    ImGuiID LastActiveId;
    float LastActiveIdTimer;
    double LastKeyModsChangeTime;
    double LastKeyModsChangeFromNoneTime;
    double LastKeyboardKeyPressTime;
    ImBitArrayForNamedKeys KeysMayBeCharInput;
    ImGuiKeyOwnerData KeysOwnerData[ImGuiKey_NamedKey_COUNT];
    ImGuiKeyRoutingTable KeysRoutingTable;
    ImU32 ActiveIdUsingNavDirMask;
    _Bool ActiveIdUsingAllKeyboardKeys;
    ImGuiKeyChord DebugBreakInShortcutRouting;
    ImGuiID CurrentFocusScopeId;
    ImGuiItemFlags CurrentItemFlags;
    ImGuiID DebugLocateId;
    ImGuiNextItemData NextItemData;
    ImGuiLastItemData LastItemData;
    ImGuiNextWindowData NextWindowData;
    _Bool DebugShowGroupRects;
    ImGuiCol DebugFlashStyleColorIdx;
    ImVector_ImGuiColorMod ColorStack;
    ImVector_ImGuiStyleMod StyleVarStack;
    ImVector_ImFontPtr FontStack;
    ImVector_ImGuiFocusScopeData FocusScopeStack;
    ImVector_ImGuiItemFlags ItemFlagsStack;
    ImVector_ImGuiGroupData GroupStack;
    ImVector_ImGuiPopupData OpenPopupStack;
    ImVector_ImGuiPopupData BeginPopupStack;
    ImVector_ImGuiTreeNodeStackData TreeNodeStack;
    ImVector_ImGuiViewportPPtr Viewports;
    ImGuiViewportP* CurrentViewport;
    ImGuiViewportP* MouseViewport;
    ImGuiViewportP* MouseLastHoveredViewport;
    ImGuiID PlatformLastFocusedViewportId;
    ImGuiPlatformMonitor FallbackMonitor;
    ImRect PlatformMonitorsFullWorkRect;
    int ViewportCreatedCount;
    int PlatformWindowsCreatedCount;
    int ViewportFocusedStampCount;
    ImGuiWindow* NavWindow;
    ImGuiID NavId;
    ImGuiID NavFocusScopeId;
    ImGuiNavLayer NavLayer;
    ImGuiID NavActivateId;
    ImGuiID NavActivateDownId;
    ImGuiID NavActivatePressedId;
    ImGuiActivateFlags NavActivateFlags;
    ImVector_ImGuiFocusScopeData NavFocusRoute;
    ImGuiID NavHighlightActivatedId;
    float NavHighlightActivatedTimer;
    ImGuiID NavNextActivateId;
    ImGuiActivateFlags NavNextActivateFlags;
    ImGuiInputSource NavInputSource;
    ImGuiSelectionUserData NavLastValidSelectionUserData;
    _Bool NavIdIsAlive;
    _Bool NavMousePosDirty;
    _Bool NavDisableHighlight;
    _Bool NavDisableMouseHover;
    _Bool NavAnyRequest;
    _Bool NavInitRequest;
    _Bool NavInitRequestFromMove;
    ImGuiNavItemData NavInitResult;
    _Bool NavMoveSubmitted;
    _Bool NavMoveScoringItems;
    _Bool NavMoveForwardToNextFrame;
    ImGuiNavMoveFlags NavMoveFlags;
    ImGuiScrollFlags NavMoveScrollFlags;
    ImGuiKeyChord NavMoveKeyMods;
    ImGuiDir NavMoveDir;
    ImGuiDir NavMoveDirForDebug;
    ImGuiDir NavMoveClipDir;
    ImRect NavScoringRect;
    ImRect NavScoringNoClipRect;
    int NavScoringDebugCount;
    int NavTabbingDir;
    int NavTabbingCounter;
    ImGuiNavItemData NavMoveResultLocal;
    ImGuiNavItemData NavMoveResultLocalVisible;
    ImGuiNavItemData NavMoveResultOther;
    ImGuiNavItemData NavTabbingResultFirst;
    ImGuiID NavJustMovedFromFocusScopeId;
    ImGuiID NavJustMovedToId;
    ImGuiID NavJustMovedToFocusScopeId;
    ImGuiKeyChord NavJustMovedToKeyMods;
    _Bool NavJustMovedToIsTabbing;
    _Bool NavJustMovedToHasSelectionData;
    ImGuiKeyChord ConfigNavWindowingKeyNext;
    ImGuiKeyChord ConfigNavWindowingKeyPrev;
    ImGuiWindow* NavWindowingTarget;
    ImGuiWindow* NavWindowingTargetAnim;
    ImGuiWindow* NavWindowingListWindow;
    float NavWindowingTimer;
    float NavWindowingHighlightAlpha;
    _Bool NavWindowingToggleLayer;
    ImGuiKey NavWindowingToggleKey;
    ImVec2 NavWindowingAccumDeltaPos;
    ImVec2 NavWindowingAccumDeltaSize;
    float DimBgRatio;
    _Bool DragDropActive;
    _Bool DragDropWithinSource;
    _Bool DragDropWithinTarget;
    ImGuiDragDropFlags DragDropSourceFlags;
    int DragDropSourceFrameCount;
    int DragDropMouseButton;
    ImGuiPayload DragDropPayload;
    ImRect DragDropTargetRect;
    ImRect DragDropTargetClipRect;
    ImGuiID DragDropTargetId;
    ImGuiDragDropFlags DragDropAcceptFlags;
    float DragDropAcceptIdCurrRectSurface;
    ImGuiID DragDropAcceptIdCurr;
    ImGuiID DragDropAcceptIdPrev;
    int DragDropAcceptFrameCount;
    ImGuiID DragDropHoldJustPressedId;
    ImVector_unsigned_char DragDropPayloadBufHeap;
    unsigned char DragDropPayloadBufLocal[16];
    int ClipperTempDataStacked;
    ImVector_ImGuiListClipperData ClipperTempData;
    ImGuiTable* CurrentTable;
    ImGuiID DebugBreakInTable;
    int TablesTempDataStacked;
    ImVector_ImGuiTableTempData TablesTempData;
    ImPool_ImGuiTable Tables;
    ImVector_float TablesLastTimeActive;
    ImVector_ImDrawChannel DrawChannelsTempMergeBuffer;
    ImGuiTabBar* CurrentTabBar;
    ImPool_ImGuiTabBar TabBars;
    ImVector_ImGuiPtrOrIndex CurrentTabBarStack;
    ImVector_ImGuiShrinkWidthItem ShrinkWidthBuffer;
    ImGuiBoxSelectState BoxSelectState;
    ImGuiMultiSelectTempData* CurrentMultiSelect;
    int MultiSelectTempDataStacked;
    ImVector_ImGuiMultiSelectTempData MultiSelectTempData;
    ImPool_ImGuiMultiSelectState MultiSelectStorage;
    ImGuiID HoverItemDelayId;
    ImGuiID HoverItemDelayIdPreviousFrame;
    float HoverItemDelayTimer;
    float HoverItemDelayClearTimer;
    ImGuiID HoverItemUnlockedStationaryId;
    ImGuiID HoverWindowUnlockedStationaryId;
    ImGuiMouseCursor MouseCursor;
    float MouseStationaryTimer;
    ImVec2 MouseLastValidPos;
    ImGuiInputTextState InputTextState;
    ImGuiInputTextDeactivatedState InputTextDeactivatedState;
    ImFont InputTextPasswordFont;
    ImGuiID TempInputId;
    ImGuiDataTypeStorage DataTypeZeroValue;
    int BeginMenuDepth;
    int BeginComboDepth;
    ImGuiColorEditFlags ColorEditOptions;
    ImGuiID ColorEditCurrentID;
    ImGuiID ColorEditSavedID;
    float ColorEditSavedHue;
    float ColorEditSavedSat;
    ImU32 ColorEditSavedColor;
    ImVec4 ColorPickerRef;
    ImGuiComboPreviewData ComboPreviewData;
    ImRect WindowResizeBorderExpectedRect;
    _Bool WindowResizeRelativeMode;
    short ScrollbarSeekMode;
    float ScrollbarClickDeltaToGrabCenter;
    float SliderGrabClickOffset;
    float SliderCurrentAccum;
    _Bool SliderCurrentAccumDirty;
    _Bool DragCurrentAccumDirty;
    float DragCurrentAccum;
    float DragSpeedDefaultRatio;
    float DisabledAlphaBackup;
    short DisabledStackSize;
    short LockMarkEdited;
    short TooltipOverrideCount;
    ImVector_char ClipboardHandlerData;
    ImVector_ImGuiID MenusIdSubmittedThisFrame;
    ImGuiTypingSelectState TypingSelectState;
    ImGuiPlatformImeData PlatformImeData;
    ImGuiPlatformImeData PlatformImeDataPrev;
    ImGuiID PlatformImeViewport;
    ImGuiDockContext DockContext;
    void (*DockNodeWindowMenuHandler)(ImGuiContext* ctx, ImGuiDockNode* node, ImGuiTabBar* tab_bar);
    _Bool SettingsLoaded;
    float SettingsDirtyTimer;
    ImGuiTextBuffer SettingsIniData;
    ImVector_ImGuiSettingsHandler SettingsHandlers;
    ImChunkStream_ImGuiWindowSettings SettingsWindows;
    ImChunkStream_ImGuiTableSettings SettingsTables;
    ImVector_ImGuiContextHook Hooks;
    ImGuiID HookIdNext;
    const char* LocalizationTable[ImGuiLocKey_COUNT];
    _Bool LogEnabled;
    ImGuiLogType LogType;
    ImFileHandle LogFile;
    ImGuiTextBuffer LogBuffer;
    const char* LogNextPrefix;
    const char* LogNextSuffix;
    float LogLinePosY;
    _Bool LogLineFirstItem;
    int LogDepthRef;
    int LogDepthToExpand;
    int LogDepthToExpandDefault;
    ImGuiDebugLogFlags DebugLogFlags;
    ImGuiTextBuffer DebugLogBuf;
    ImGuiTextIndex DebugLogIndex;
    ImGuiDebugLogFlags DebugLogAutoDisableFlags;
    ImU8 DebugLogAutoDisableFrames;
    ImU8 DebugLocateFrames;
    _Bool DebugBreakInLocateId;
    ImGuiKeyChord DebugBreakKeyChord;
    ImS8 DebugBeginReturnValueCullDepth;
    _Bool DebugItemPickerActive;
    ImU8 DebugItemPickerMouseButton;
    ImGuiID DebugItemPickerBreakId;
    float DebugFlashStyleColorTime;
    ImVec4 DebugFlashStyleColorBackup;
    ImGuiMetricsConfig DebugMetricsConfig;
    ImGuiIDStackTool DebugIDStackTool;
    ImGuiDebugAllocInfo DebugAllocInfo;
    ImGuiDockNode* DebugHoveredDockNode;
    float FramerateSecPerFrame[60];
    int FramerateSecPerFrameIdx;
    int FramerateSecPerFrameCount;
    float FramerateSecPerFrameAccum;
    int WantCaptureMouseNextFrame;
    int WantCaptureKeyboardNextFrame;
    int WantTextInputNextFrame;
    ImVector_char TempBuffer;
    char TempKeychordName[64];
};
struct ImGuiWindowTempData {
    ImVec2 CursorPos;
    ImVec2 CursorPosPrevLine;
    ImVec2 CursorStartPos;
    ImVec2 CursorMaxPos;
    ImVec2 IdealMaxPos;
    ImVec2 CurrLineSize;
    ImVec2 PrevLineSize;
    float CurrLineTextBaseOffset;
    float PrevLineTextBaseOffset;
    _Bool IsSameLine;
    _Bool IsSetPos;
    ImVec1 Indent;
    ImVec1 ColumnsOffset;
    ImVec1 GroupOffset;
    ImVec2 CursorStartPosLossyness;
    ImGuiNavLayer NavLayerCurrent;
    short NavLayersActiveMask;
    short NavLayersActiveMaskNext;
    _Bool NavIsScrollPushableX;
    _Bool NavHideHighlightOneFrame;
    _Bool NavWindowHasScrollY;
    _Bool MenuBarAppending;
    ImVec2 MenuBarOffset;
    ImGuiMenuColumns MenuColumns;
    int TreeDepth;
    ImU32 TreeHasStackDataDepthMask;
    ImVector_ImGuiWindowPtr ChildWindows;
    ImGuiStorage* StateStorage;
    ImGuiOldColumns* CurrentColumns;
    int CurrentTableIdx;
    ImGuiLayoutType LayoutType;
    ImGuiLayoutType ParentLayoutType;
    ImU32 ModalDimBgColor;
    float ItemWidth;
    float TextWrapPos;
    ImVector_float ItemWidthStack;
    ImVector_float TextWrapPosStack;
};
typedef struct ImVector_ImGuiOldColumns {
    int Size;
    int Capacity;
    ImGuiOldColumns* Data;
} ImVector_ImGuiOldColumns;
struct ImGuiWindow {
    ImGuiContext* Ctx;
    char* Name;
    ImGuiID ID;
    ImGuiWindowFlags Flags, FlagsPreviousFrame;
    ImGuiChildFlags ChildFlags;
    ImGuiWindowClass WindowClass;
    ImGuiViewportP* Viewport;
    ImGuiID ViewportId;
    ImVec2 ViewportPos;
    int ViewportAllowPlatformMonitorExtend;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 SizeFull;
    ImVec2 ContentSize;
    ImVec2 ContentSizeIdeal;
    ImVec2 ContentSizeExplicit;
    ImVec2 WindowPadding;
    float WindowRounding;
    float WindowBorderSize;
    float TitleBarHeight, MenuBarHeight;
    float DecoOuterSizeX1, DecoOuterSizeY1;
    float DecoOuterSizeX2, DecoOuterSizeY2;
    float DecoInnerSizeX1, DecoInnerSizeY1;
    int NameBufLen;
    ImGuiID MoveId;
    ImGuiID TabId;
    ImGuiID ChildId;
    ImGuiID PopupId;
    ImVec2 Scroll;
    ImVec2 ScrollMax;
    ImVec2 ScrollTarget;
    ImVec2 ScrollTargetCenterRatio;
    ImVec2 ScrollTargetEdgeSnapDist;
    ImVec2 ScrollbarSizes;
    _Bool ScrollbarX, ScrollbarY;
    _Bool ViewportOwned;
    _Bool Active;
    _Bool WasActive;
    _Bool WriteAccessed;
    _Bool Collapsed;
    _Bool WantCollapseToggle;
    _Bool SkipItems;
    _Bool SkipRefresh;
    _Bool Appearing;
    _Bool Hidden;
    _Bool IsFallbackWindow;
    _Bool IsExplicitChild;
    _Bool HasCloseButton;
    signed char ResizeBorderHovered;
    signed char ResizeBorderHeld;
    short BeginCount;
    short BeginCountPreviousFrame;
    short BeginOrderWithinParent;
    short BeginOrderWithinContext;
    short FocusOrder;
    ImS8 AutoFitFramesX, AutoFitFramesY;
    _Bool AutoFitOnlyGrows;
    ImGuiDir AutoPosLastDirection;
    ImS8 HiddenFramesCanSkipItems;
    ImS8 HiddenFramesCannotSkipItems;
    ImS8 HiddenFramesForRenderOnly;
    ImS8 DisableInputsFrames;
    ImGuiCond SetWindowPosAllowFlags : 8;
    ImGuiCond SetWindowSizeAllowFlags : 8;
    ImGuiCond SetWindowCollapsedAllowFlags : 8;
    ImGuiCond SetWindowDockAllowFlags : 8;
    ImVec2 SetWindowPosVal;
    ImVec2 SetWindowPosPivot;
    ImVector_ImGuiID IDStack;
    ImGuiWindowTempData DC;
    ImRect OuterRectClipped;
    ImRect InnerRect;
    ImRect InnerClipRect;
    ImRect WorkRect;
    ImRect ParentWorkRect;
    ImRect ClipRect;
    ImRect ContentRegionRect;
    ImVec2ih HitTestHoleSize;
    ImVec2ih HitTestHoleOffset;
    int LastFrameActive;
    int LastFrameJustFocused;
    float LastTimeActive;
    float ItemWidthDefault;
    ImGuiStorage StateStorage;
    ImVector_ImGuiOldColumns ColumnsStorage;
    float FontWindowScale;
    float FontDpiScale;
    int SettingsOffset;
    ImDrawList* DrawList;
    ImDrawList DrawListInst;
    ImGuiWindow* ParentWindow;
    ImGuiWindow* ParentWindowInBeginStack;
    ImGuiWindow* RootWindow;
    ImGuiWindow* RootWindowPopupTree;
    ImGuiWindow* RootWindowDockTree;
    ImGuiWindow* RootWindowForTitleBarHighlight;
    ImGuiWindow* RootWindowForNav;
    ImGuiWindow* ParentWindowForFocusRoute;
    ImGuiWindow* NavLastChildNavWindow;
    ImGuiID NavLastIds[ImGuiNavLayer_COUNT];
    ImRect NavRectRel[ImGuiNavLayer_COUNT];
    ImVec2 NavPreferredScoringPosRel[ImGuiNavLayer_COUNT];
    ImGuiID NavRootFocusScopeId;
    int MemoryDrawListIdxCapacity;
    int MemoryDrawListVtxCapacity;
    _Bool MemoryCompacted;
    _Bool DockIsActive : 1;
    _Bool DockNodeIsVisible : 1;
    _Bool DockTabIsVisible : 1;
    _Bool DockTabWantClose : 1;
    short DockOrder;
    ImGuiWindowDockStyle DockStyle;
    ImGuiDockNode* DockNode;
    ImGuiDockNode* DockNodeAsHost;
    ImGuiID DockId;
    ImGuiItemStatusFlags DockTabItemStatusFlags;
    ImRect DockTabItemRect;
};
typedef enum {
    ImGuiTabBarFlags_DockNode = 1 << 20,
    ImGuiTabBarFlags_IsFocused = 1 << 21,
    ImGuiTabBarFlags_SaveSettings = 1 << 22,
} ImGuiTabBarFlagsPrivate_;
typedef enum {
    ImGuiTabItemFlags_SectionMask_ = ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_Trailing,
    ImGuiTabItemFlags_NoCloseButton = 1 << 20,
    ImGuiTabItemFlags_Button = 1 << 21,
    ImGuiTabItemFlags_Unsorted = 1 << 22,
} ImGuiTabItemFlagsPrivate_;
struct ImGuiTabItem {
    ImGuiID ID;
    ImGuiTabItemFlags Flags;
    ImGuiWindow* Window;
    int LastFrameVisible;
    int LastFrameSelected;
    float Offset;
    float Width;
    float ContentWidth;
    float RequestedWidth;
    ImS32 NameOffset;
    ImS16 BeginOrder;
    ImS16 IndexDuringLayout;
    _Bool WantClose;
};
typedef struct ImVector_ImGuiTabItem {
    int Size;
    int Capacity;
    ImGuiTabItem* Data;
} ImVector_ImGuiTabItem;
struct ImGuiTabBar {
    ImVector_ImGuiTabItem Tabs;
    ImGuiTabBarFlags Flags;
    ImGuiID ID;
    ImGuiID SelectedTabId;
    ImGuiID NextSelectedTabId;
    ImGuiID VisibleTabId;
    int CurrFrameVisible;
    int PrevFrameVisible;
    ImRect BarRect;
    float CurrTabsContentsHeight;
    float PrevTabsContentsHeight;
    float WidthAllTabs;
    float WidthAllTabsIdeal;
    float ScrollingAnim;
    float ScrollingTarget;
    float ScrollingTargetDistToVisibility;
    float ScrollingSpeed;
    float ScrollingRectMinX;
    float ScrollingRectMaxX;
    float SeparatorMinX;
    float SeparatorMaxX;
    ImGuiID ReorderRequestTabId;
    ImS16 ReorderRequestOffset;
    ImS8 BeginCount;
    _Bool WantLayout;
    _Bool VisibleTabWasSubmitted;
    _Bool TabsAddedNew;
    ImS16 TabsActiveCount;
    ImS16 LastTabItemIdx;
    float ItemSpacingY;
    ImVec2 FramePadding;
    ImVec2 BackupCursorPos;
    ImGuiTextBuffer TabsNames;
};
typedef ImS16 ImGuiTableColumnIdx;
typedef ImU16 ImGuiTableDrawChannelIdx;
struct ImGuiTableColumn {
    ImGuiTableColumnFlags Flags;
    float WidthGiven;
    float MinX;
    float MaxX;
    float WidthRequest;
    float WidthAuto;
    float StretchWeight;
    float InitStretchWeightOrWidth;
    ImRect ClipRect;
    ImGuiID UserID;
    float WorkMinX;
    float WorkMaxX;
    float ItemWidth;
    float ContentMaxXFrozen;
    float ContentMaxXUnfrozen;
    float ContentMaxXHeadersUsed;
    float ContentMaxXHeadersIdeal;
    ImS16 NameOffset;
    ImGuiTableColumnIdx DisplayOrder;
    ImGuiTableColumnIdx IndexWithinEnabledSet;
    ImGuiTableColumnIdx PrevEnabledColumn;
    ImGuiTableColumnIdx NextEnabledColumn;
    ImGuiTableColumnIdx SortOrder;
    ImGuiTableDrawChannelIdx DrawChannelCurrent;
    ImGuiTableDrawChannelIdx DrawChannelFrozen;
    ImGuiTableDrawChannelIdx DrawChannelUnfrozen;
    _Bool IsEnabled;
    _Bool IsUserEnabled;
    _Bool IsUserEnabledNextFrame;
    _Bool IsVisibleX;
    _Bool IsVisibleY;
    _Bool IsRequestOutput;
    _Bool IsSkipItems;
    _Bool IsPreserveWidthAuto;
    ImS8 NavLayerCurrent;
    ImU8 AutoFitQueue;
    ImU8 CannotSkipItemsQueue;
    ImU8 SortDirection : 2;
    ImU8 SortDirectionsAvailCount : 2;
    ImU8 SortDirectionsAvailMask : 4;
    ImU8 SortDirectionsAvailList;
};
typedef struct ImGuiTableCellData ImGuiTableCellData;
struct ImGuiTableCellData {
    ImU32 BgColor;
    ImGuiTableColumnIdx Column;
};
struct ImGuiTableHeaderData {
    ImGuiTableColumnIdx Index;
    ImU32 TextColor;
    ImU32 BgColor0;
    ImU32 BgColor1;
};
struct ImGuiTableInstanceData {
    ImGuiID TableInstanceID;
    float LastOuterHeight;
    float LastTopHeadersRowHeight;
    float LastFrozenHeight;
    int HoveredRowLast;
    int HoveredRowNext;
};
typedef struct ImSpan_ImGuiTableColumn {
    ImGuiTableColumn* Data;
    ImGuiTableColumn* DataEnd;
} ImSpan_ImGuiTableColumn;
typedef struct ImSpan_ImGuiTableColumnIdx {
    ImGuiTableColumnIdx* Data;
    ImGuiTableColumnIdx* DataEnd;
} ImSpan_ImGuiTableColumnIdx;
typedef struct ImSpan_ImGuiTableCellData {
    ImGuiTableCellData* Data;
    ImGuiTableCellData* DataEnd;
} ImSpan_ImGuiTableCellData;
typedef struct ImVector_ImGuiTableInstanceData {
    int Size;
    int Capacity;
    ImGuiTableInstanceData* Data;
} ImVector_ImGuiTableInstanceData;
typedef struct ImVector_ImGuiTableColumnSortSpecs {
    int Size;
    int Capacity;
    ImGuiTableColumnSortSpecs* Data;
} ImVector_ImGuiTableColumnSortSpecs;
struct ImGuiTable {
    ImGuiID ID;
    ImGuiTableFlags Flags;
    void* RawData;
    ImGuiTableTempData* TempData;
    ImSpan_ImGuiTableColumn Columns;
    ImSpan_ImGuiTableColumnIdx DisplayOrderToIndex;
    ImSpan_ImGuiTableCellData RowCellData;
    ImBitArrayPtr EnabledMaskByDisplayOrder;
    ImBitArrayPtr EnabledMaskByIndex;
    ImBitArrayPtr VisibleMaskByIndex;
    ImGuiTableFlags SettingsLoadedFlags;
    int SettingsOffset;
    int LastFrameActive;
    int ColumnsCount;
    int CurrentRow;
    int CurrentColumn;
    ImS16 InstanceCurrent;
    ImS16 InstanceInteracted;
    float RowPosY1;
    float RowPosY2;
    float RowMinHeight;
    float RowCellPaddingY;
    float RowTextBaseline;
    float RowIndentOffsetX;
    ImGuiTableRowFlags RowFlags : 16;
    ImGuiTableRowFlags LastRowFlags : 16;
    int RowBgColorCounter;
    ImU32 RowBgColor[2];
    ImU32 BorderColorStrong;
    ImU32 BorderColorLight;
    float BorderX1;
    float BorderX2;
    float HostIndentX;
    float MinColumnWidth;
    float OuterPaddingX;
    float CellPaddingX;
    float CellSpacingX1;
    float CellSpacingX2;
    float InnerWidth;
    float ColumnsGivenWidth;
    float ColumnsAutoFitWidth;
    float ColumnsStretchSumWeights;
    float ResizedColumnNextWidth;
    float ResizeLockMinContentsX2;
    float RefScale;
    float AngledHeadersHeight;
    float AngledHeadersSlope;
    ImRect OuterRect;
    ImRect InnerRect;
    ImRect WorkRect;
    ImRect InnerClipRect;
    ImRect BgClipRect;
    ImRect Bg0ClipRectForDrawCmd;
    ImRect Bg2ClipRectForDrawCmd;
    ImRect HostClipRect;
    ImRect HostBackupInnerClipRect;
    ImGuiWindow* OuterWindow;
    ImGuiWindow* InnerWindow;
    ImGuiTextBuffer ColumnsNames;
    ImDrawListSplitter* DrawSplitter;
    ImGuiTableInstanceData InstanceDataFirst;
    ImVector_ImGuiTableInstanceData InstanceDataExtra;
    ImGuiTableColumnSortSpecs SortSpecsSingle;
    ImVector_ImGuiTableColumnSortSpecs SortSpecsMulti;
    ImGuiTableSortSpecs SortSpecs;
    ImGuiTableColumnIdx SortSpecsCount;
    ImGuiTableColumnIdx ColumnsEnabledCount;
    ImGuiTableColumnIdx ColumnsEnabledFixedCount;
    ImGuiTableColumnIdx DeclColumnsCount;
    ImGuiTableColumnIdx AngledHeadersCount;
    ImGuiTableColumnIdx HoveredColumnBody;
    ImGuiTableColumnIdx HoveredColumnBorder;
    ImGuiTableColumnIdx HighlightColumnHeader;
    ImGuiTableColumnIdx AutoFitSingleColumn;
    ImGuiTableColumnIdx ResizedColumn;
    ImGuiTableColumnIdx LastResizedColumn;
    ImGuiTableColumnIdx HeldHeaderColumn;
    ImGuiTableColumnIdx ReorderColumn;
    ImGuiTableColumnIdx ReorderColumnDir;
    ImGuiTableColumnIdx LeftMostEnabledColumn;
    ImGuiTableColumnIdx RightMostEnabledColumn;
    ImGuiTableColumnIdx LeftMostStretchedColumn;
    ImGuiTableColumnIdx RightMostStretchedColumn;
    ImGuiTableColumnIdx ContextPopupColumn;
    ImGuiTableColumnIdx FreezeRowsRequest;
    ImGuiTableColumnIdx FreezeRowsCount;
    ImGuiTableColumnIdx FreezeColumnsRequest;
    ImGuiTableColumnIdx FreezeColumnsCount;
    ImGuiTableColumnIdx RowCellDataCurrent;
    ImGuiTableDrawChannelIdx DummyDrawChannel;
    ImGuiTableDrawChannelIdx Bg2DrawChannelCurrent;
    ImGuiTableDrawChannelIdx Bg2DrawChannelUnfrozen;
    _Bool IsLayoutLocked;
    _Bool IsInsideRow;
    _Bool IsInitializing;
    _Bool IsSortSpecsDirty;
    _Bool IsUsingHeaders;
    _Bool IsContextPopupOpen;
    _Bool DisableDefaultContextMenu;
    _Bool IsSettingsRequestLoad;
    _Bool IsSettingsDirty;
    _Bool IsDefaultDisplayOrder;
    _Bool IsResetAllRequest;
    _Bool IsResetDisplayOrderRequest;
    _Bool IsUnfrozenRows;
    _Bool IsDefaultSizingPolicy;
    _Bool IsActiveIdAliveBeforeTable;
    _Bool IsActiveIdInTable;
    _Bool HasScrollbarYCurr;
    _Bool HasScrollbarYPrev;
    _Bool MemoryCompacted;
    _Bool HostSkipItems;
};
typedef struct ImVector_ImGuiTableHeaderData {
    int Size;
    int Capacity;
    ImGuiTableHeaderData* Data;
} ImVector_ImGuiTableHeaderData;
struct ImGuiTableTempData {
    int TableIndex;
    float LastTimeActive;
    float AngledHeadersExtraWidth;
    ImVector_ImGuiTableHeaderData AngledHeadersRequests;
    ImVec2 UserOuterSize;
    ImDrawListSplitter DrawSplitter;
    ImRect HostBackupWorkRect;
    ImRect HostBackupParentWorkRect;
    ImVec2 HostBackupPrevLineSize;
    ImVec2 HostBackupCurrLineSize;
    ImVec2 HostBackupCursorMaxPos;
    ImVec1 HostBackupColumnsOffset;
    float HostBackupItemWidth;
    int HostBackupItemWidthStackSize;
};
typedef struct ImGuiTableColumnSettings ImGuiTableColumnSettings;
struct ImGuiTableColumnSettings {
    float WidthOrWeight;
    ImGuiID UserID;
    ImGuiTableColumnIdx Index;
    ImGuiTableColumnIdx DisplayOrder;
    ImGuiTableColumnIdx SortOrder;
    ImU8 SortDirection : 2;
    ImU8 IsEnabled : 1;
    ImU8 IsStretch : 1;
};
struct ImGuiTableSettings {
    ImGuiID ID;
    ImGuiTableFlags SaveFlags;
    float RefScale;
    ImGuiTableColumnIdx ColumnsCount;
    ImGuiTableColumnIdx ColumnsCountMax;
    _Bool WantApply;
};
struct ImFontBuilderIO {
    _Bool (*FontBuilder_Build)(ImFontAtlas* atlas);
};
extern  ImVec2* ImVec2_ImVec2_Nil(void);
extern  void ImVec2_destroy(ImVec2* self);
extern  ImVec2* ImVec2_ImVec2_Float(float _x, float _y);
extern  ImVec4* ImVec4_ImVec4_Nil(void);
extern  void ImVec4_destroy(ImVec4* self);
extern  ImVec4* ImVec4_ImVec4_Float(float _x, float _y, float _z, float _w);
extern  ImGuiContext* imguiCreateContext(ImFontAtlas* shared_font_atlas);
extern  void imguiDestroyContext(ImGuiContext* ctx);
extern  ImGuiContext* imguiGetCurrentContext(void);
extern  void imguiSetCurrentContext(ImGuiContext* ctx);
extern  ImGuiIO* imguiGetIO(void);
extern  ImGuiStyle* imguiGetStyle(void);
extern  void imguiNewFrame(void);
extern  void imguiEndFrame(void);
extern  void imguiRender(void);
extern  ImDrawData* imguiGetDrawData(void);
extern  void imguiShowDemoWindow(_Bool* p_open);
extern  void imguiShowMetricsWindow(_Bool* p_open);
extern  void imguiShowDebugLogWindow(_Bool* p_open);
extern  void imguiShowIDStackToolWindow(_Bool* p_open);
extern  void imguiShowAboutWindow(_Bool* p_open);
extern  void imguiShowStyleEditor(ImGuiStyle* ref);
extern  _Bool imguiShowStyleSelector(const char* label);
extern  void imguiShowFontSelector(const char* label);
extern  void imguiShowUserGuide(void);
extern  const char* imguiGetVersion(void);
extern  void imguiStyleColorsDark(ImGuiStyle* dst);
extern  void imguiStyleColorsLight(ImGuiStyle* dst);
extern  void imguiStyleColorsClassic(ImGuiStyle* dst);
extern  _Bool imguiBegin(const char* name, _Bool* p_open, ImGuiWindowFlags flags);
extern  void imguiEnd(void);
extern  _Bool imguiBeginChild_Str(const char* str_id, const ImVec2 size, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
extern  _Bool imguiBeginChild_ID(ImGuiID id, const ImVec2 size, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
extern  void imguiEndChild(void);
extern  _Bool imguiIsWindowAppearing(void);
extern  _Bool imguiIsWindowCollapsed(void);
extern  _Bool imguiIsWindowFocused(ImGuiFocusedFlags flags);
extern  _Bool imguiIsWindowHovered(ImGuiHoveredFlags flags);
extern  ImDrawList* imguiGetWindowDrawList(void);
extern  float imguiGetWindowDpiScale(void);
extern  void imguiGetWindowPos(ImVec2* pOut);
extern  void imguiGetWindowSize(ImVec2* pOut);
extern  float imguiGetWindowWidth(void);
extern  float imguiGetWindowHeight(void);
extern  ImGuiViewport* imguiGetWindowViewport(void);
extern  void imguiSetNextWindowPos(const ImVec2 pos, ImGuiCond cond, const ImVec2 pivot);
extern  void imguiSetNextWindowSize(const ImVec2 size, ImGuiCond cond);
extern  void imguiSetNextWindowSizeConstraints(const ImVec2 size_min, const ImVec2 size_max, ImGuiSizeCallback custom_callback, void* custom_callback_data);
extern  void imguiSetNextWindowContentSize(const ImVec2 size);
extern  void imguiSetNextWindowCollapsed(_Bool collapsed, ImGuiCond cond);
extern  void imguiSetNextWindowFocus(void);
extern  void imguiSetNextWindowScroll(const ImVec2 scroll);
extern  void imguiSetNextWindowBgAlpha(float alpha);
extern  void imguiSetNextWindowViewport(ImGuiID viewport_id);
extern  void imguiSetWindowPos_Vec2(const ImVec2 pos, ImGuiCond cond);
extern  void imguiSetWindowSize_Vec2(const ImVec2 size, ImGuiCond cond);
extern  void imguiSetWindowCollapsed_Bool(_Bool collapsed, ImGuiCond cond);
extern  void imguiSetWindowFocus_Nil(void);
extern  void imguiSetWindowFontScale(float scale);
extern  void imguiSetWindowPos_Str(const char* name, const ImVec2 pos, ImGuiCond cond);
extern  void imguiSetWindowSize_Str(const char* name, const ImVec2 size, ImGuiCond cond);
extern  void imguiSetWindowCollapsed_Str(const char* name, _Bool collapsed, ImGuiCond cond);
extern  void imguiSetWindowFocus_Str(const char* name);
extern  float imguiGetScrollX(void);
extern  float imguiGetScrollY(void);
extern  void imguiSetScrollX_Float(float scroll_x);
extern  void imguiSetScrollY_Float(float scroll_y);
extern  float imguiGetScrollMaxX(void);
extern  float imguiGetScrollMaxY(void);
extern  void imguiSetScrollHereX(float center_x_ratio);
extern  void imguiSetScrollHereY(float center_y_ratio);
extern  void imguiSetScrollFromPosX_Float(float local_x, float center_x_ratio);
extern  void imguiSetScrollFromPosY_Float(float local_y, float center_y_ratio);
extern  void imguiPushFont(ImFont* font);
extern  void imguiPopFont(void);
extern  void imguiPushStyleColor_U32(ImGuiCol idx, ImU32 col);
extern  void imguiPushStyleColor_Vec4(ImGuiCol idx, const ImVec4 col);
extern  void imguiPopStyleColor(int count);
extern  void imguiPushStyleVar_Float(ImGuiStyleVar idx, float val);
extern  void imguiPushStyleVar_Vec2(ImGuiStyleVar idx, const ImVec2 val);
extern  void imguiPopStyleVar(int count);
extern  void imguiPushItemFlag(ImGuiItemFlags option, _Bool enabled);
extern  void imguiPopItemFlag(void);
extern  void imguiPushItemWidth(float item_width);
extern  void imguiPopItemWidth(void);
extern  void imguiSetNextItemWidth(float item_width);
extern  float imguiCalcItemWidth(void);
extern  void imguiPushTextWrapPos(float wrap_local_pos_x);
extern  void imguiPopTextWrapPos(void);
extern  ImFont* imguiGetFont(void);
extern  float imguiGetFontSize(void);
extern  void imguiGetFontTexUvWhitePixel(ImVec2* pOut);
extern  ImU32 imguiGetColorU32_Col(ImGuiCol idx, float alpha_mul);
extern  ImU32 imguiGetColorU32_Vec4(const ImVec4 col);
extern  ImU32 imguiGetColorU32_U32(ImU32 col, float alpha_mul);
extern  const ImVec4* imguiGetStyleColorVec4(ImGuiCol idx);
extern  void imguiGetCursorScreenPos(ImVec2* pOut);
extern  void imguiSetCursorScreenPos(const ImVec2 pos);
extern  void imguiGetContentRegionAvail(ImVec2* pOut);
extern  void imguiGetCursorPos(ImVec2* pOut);
extern  float imguiGetCursorPosX(void);
extern  float imguiGetCursorPosY(void);
extern  void imguiSetCursorPos(const ImVec2 local_pos);
extern  void imguiSetCursorPosX(float local_x);
extern  void imguiSetCursorPosY(float local_y);
extern  void imguiGetCursorStartPos(ImVec2* pOut);
extern  void imguiSeparator(void);
extern  void imguiSameLine(float offset_from_start_x, float spacing);
extern  void imguiNewLine(void);
extern  void imguiSpacing(void);
extern  void imguiDummy(const ImVec2 size);
extern  void imguiIndent(float indent_w);
extern  void imguiUnindent(float indent_w);
extern  void imguiBeginGroup(void);
extern  void imguiEndGroup(void);
extern  void imguiAlignTextToFramePadding(void);
extern  float imguiGetTextLineHeight(void);
extern  float imguiGetTextLineHeightWithSpacing(void);
extern  float imguiGetFrameHeight(void);
extern  float imguiGetFrameHeightWithSpacing(void);
extern  void imguiPushID_Str(const char* str_id);
extern  void imguiPushID_StrStr(const char* str_id_begin, const char* str_id_end);
extern  void imguiPushID_Ptr(const void* ptr_id);
extern  void imguiPushID_Int(int int_id);
extern  void imguiPopID(void);
extern  ImGuiID imguiGetID_Str(const char* str_id);
extern  ImGuiID imguiGetID_StrStr(const char* str_id_begin, const char* str_id_end);
extern  ImGuiID imguiGetID_Ptr(const void* ptr_id);
extern  ImGuiID imguiGetID_Int(int int_id);
extern  void imguiTextUnformatted(const char* text, const char* text_end);
extern  void imguiText(const char* fmt, ...);
extern  void imguiTextV(const char* fmt, va_list args);
extern  void imguiTextColored(const ImVec4 col, const char* fmt, ...);
extern  void imguiTextColoredV(const ImVec4 col, const char* fmt, va_list args);
extern  void imguiTextDisabled(const char* fmt, ...);
extern  void imguiTextDisabledV(const char* fmt, va_list args);
extern  void imguiTextWrapped(const char* fmt, ...);
extern  void imguiTextWrappedV(const char* fmt, va_list args);
extern  void imguiLabelText(const char* label, const char* fmt, ...);
extern  void imguiLabelTextV(const char* label, const char* fmt, va_list args);
extern  void imguiBulletText(const char* fmt, ...);
extern  void imguiBulletTextV(const char* fmt, va_list args);
extern  void imguiSeparatorText(const char* label);
extern  _Bool imguiButton(const char* label, const ImVec2 size);
extern  _Bool imguiSmallButton(const char* label);
extern  _Bool imguiInvisibleButton(const char* str_id, const ImVec2 size, ImGuiButtonFlags flags);
extern  _Bool imguiArrowButton(const char* str_id, ImGuiDir dir);
extern  _Bool imguiCheckbox(const char* label, _Bool* v);
extern  _Bool imguiCheckboxFlags_IntPtr(const char* label, int* flags, int flags_value);
extern  _Bool imguiCheckboxFlags_UintPtr(const char* label, unsigned int* flags, unsigned int flags_value);
extern  _Bool imguiRadioButton_Bool(const char* label, _Bool active);
extern  _Bool imguiRadioButton_IntPtr(const char* label, int* v, int v_button);
extern  void imguiProgressBar(float fraction, const ImVec2 size_arg, const char* overlay);
extern  void imguiBullet(void);
extern  _Bool imguiTextLink(const char* label);
extern  void imguiTextLinkOpenURL(const char* label, const char* url);
extern  void imguiImage(ImTextureID user_texture_id, const ImVec2 image_size, const ImVec2 uv0, const ImVec2 uv1, const ImVec4 tint_col, const ImVec4 border_col);
extern  _Bool imguiImageButton(const char* str_id, ImTextureID user_texture_id, const ImVec2 image_size, const ImVec2 uv0, const ImVec2 uv1, const ImVec4 bg_col, const ImVec4 tint_col);
extern  _Bool imguiBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags);
extern  void imguiEndCombo(void);
extern  _Bool imguiCombo_Str_arr(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items);
extern  _Bool imguiCombo_Str(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items);
extern  _Bool imguiCombo_FnStrPtr(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items);
extern  _Bool imguiDragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format, const char* format_max,
                                     ImGuiSliderFlags flags);
extern  _Bool imguiDragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragInt2(const char* label, int v[2], float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragInt3(const char* label, int v[3], float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragInt4(const char* label, int v[4], float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* format, const char* format_max, ImGuiSliderFlags flags);
extern  _Bool imguiDragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiDragScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, float v_speed, const void* p_min, const void* p_max, const char* format,
                                 ImGuiSliderFlags flags);
extern  _Bool imguiSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiVSliderFloat(const char* label, const ImVec2 size, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiVSliderInt(const char* label, const ImVec2 size, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiVSliderScalar(const char* label, const ImVec2 size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiInputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
extern  _Bool imguiInputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2 size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
extern  _Bool imguiInputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
extern  _Bool imguiInputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags);
extern  _Bool imguiInputFloat2(const char* label, float v[2], const char* format, ImGuiInputTextFlags flags);
extern  _Bool imguiInputFloat3(const char* label, float v[3], const char* format, ImGuiInputTextFlags flags);
extern  _Bool imguiInputFloat4(const char* label, float v[4], const char* format, ImGuiInputTextFlags flags);
extern  _Bool imguiInputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags);
extern  _Bool imguiInputInt2(const char* label, int v[2], ImGuiInputTextFlags flags);
extern  _Bool imguiInputInt3(const char* label, int v[3], ImGuiInputTextFlags flags);
extern  _Bool imguiInputInt4(const char* label, int v[4], ImGuiInputTextFlags flags);
extern  _Bool imguiInputDouble(const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags);
extern  _Bool imguiInputScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags);
extern  _Bool imguiInputScalarN(const char* label, ImGuiDataType data_type, void* p_data, int components, const void* p_step, const void* p_step_fast, const char* format, ImGuiInputTextFlags flags);
extern  _Bool imguiColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags);
extern  _Bool imguiColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags);
extern  _Bool imguiColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags);
extern  _Bool imguiColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col);
extern  _Bool imguiColorButton(const char* desc_id, const ImVec4 col, ImGuiColorEditFlags flags, const ImVec2 size);
extern  void imguiSetColorEditOptions(ImGuiColorEditFlags flags);
extern  _Bool imguiTreeNode_Str(const char* label);
extern  _Bool imguiTreeNode_StrStr(const char* str_id, const char* fmt, ...);
extern  _Bool imguiTreeNode_Ptr(const void* ptr_id, const char* fmt, ...);
extern  _Bool imguiTreeNodeV_Str(const char* str_id, const char* fmt, va_list args);
extern  _Bool imguiTreeNodeV_Ptr(const void* ptr_id, const char* fmt, va_list args);
extern  _Bool imguiTreeNodeEx_Str(const char* label, ImGuiTreeNodeFlags flags);
extern  _Bool imguiTreeNodeEx_StrStr(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);
extern  _Bool imguiTreeNodeEx_Ptr(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);
extern  _Bool imguiTreeNodeExV_Str(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args);
extern  _Bool imguiTreeNodeExV_Ptr(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args);
extern  void imguiTreePush_Str(const char* str_id);
extern  void imguiTreePush_Ptr(const void* ptr_id);
extern  void imguiTreePop(void);
extern  float imguiGetTreeNodeToLabelSpacing(void);
extern  _Bool imguiCollapsingHeader_TreeNodeFlags(const char* label, ImGuiTreeNodeFlags flags);
extern  _Bool imguiCollapsingHeader_BoolPtr(const char* label, _Bool* p_visible, ImGuiTreeNodeFlags flags);
extern  void imguiSetNextItemOpen(_Bool is_open, ImGuiCond cond);
extern  void imguiSetNextItemStorageID(ImGuiID storage_id);
extern  _Bool imguiSelectable_Bool(const char* label, _Bool selected, ImGuiSelectableFlags flags, const ImVec2 size);
extern  _Bool imguiSelectable_BoolPtr(const char* label, _Bool* p_selected, ImGuiSelectableFlags flags, const ImVec2 size);
extern  ImGuiMultiSelectIO* imguiBeginMultiSelect(ImGuiMultiSelectFlags flags, int selection_size, int items_count);
extern  ImGuiMultiSelectIO* imguiEndMultiSelect(void);
extern  void imguiSetNextItemSelectionUserData(ImGuiSelectionUserData selection_user_data);
extern  _Bool imguiIsItemToggledSelection(void);
extern  _Bool imguiBeginListBox(const char* label, const ImVec2 size);
extern  void imguiEndListBox(void);
extern  _Bool imguiListBox_Str_arr(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items);
extern  _Bool imguiListBox_FnStrPtr(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int height_in_items);
extern  void imguiPlotLines_FloatPtr(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size,
                                        int stride);
extern  void imguiPlotLines_FnFloatPtr(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min,
                                          float scale_max, ImVec2 graph_size);
extern  void imguiPlotHistogram_FloatPtr(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size,
                                            int stride);
extern  void imguiPlotHistogram_FnFloatPtr(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text,
                                              float scale_min, float scale_max, ImVec2 graph_size);
extern  void imguiValue_Bool(const char* prefix, _Bool b);
extern  void imguiValue_Int(const char* prefix, int v);
extern  void imguiValue_Uint(const char* prefix, unsigned int v);
extern  void imguiValue_Float(const char* prefix, float v, const char* float_format);
extern  _Bool imguiBeginMenuBar(void);
extern  void imguiEndMenuBar(void);
extern  _Bool imguiBeginMainMenuBar(void);
extern  void imguiEndMainMenuBar(void);
extern  _Bool imguiBeginMenu(const char* label, _Bool enabled);
extern  void imguiEndMenu(void);
extern  _Bool imguiMenuItem_Bool(const char* label, const char* shortcut, _Bool selected, _Bool enabled);
extern  _Bool imguiMenuItem_BoolPtr(const char* label, const char* shortcut, _Bool* p_selected, _Bool enabled);
extern  _Bool imguiBeginTooltip(void);
extern  void imguiEndTooltip(void);
extern  void imguiSetTooltip(const char* fmt, ...);
extern  void imguiSetTooltipV(const char* fmt, va_list args);
extern  _Bool imguiBeginItemTooltip(void);
extern  void imguiSetItemTooltip(const char* fmt, ...);
extern  void imguiSetItemTooltipV(const char* fmt, va_list args);
extern  _Bool imguiBeginPopup(const char* str_id, ImGuiWindowFlags flags);
extern  _Bool imguiBeginPopupModal(const char* name, _Bool* p_open, ImGuiWindowFlags flags);
extern  void imguiEndPopup(void);
extern  void imguiOpenPopup_Str(const char* str_id, ImGuiPopupFlags popup_flags);
extern  void imguiOpenPopup_ID(ImGuiID id, ImGuiPopupFlags popup_flags);
extern  void imguiOpenPopupOnItemClick(const char* str_id, ImGuiPopupFlags popup_flags);
extern  void imguiCloseCurrentPopup(void);
extern  _Bool imguiBeginPopupContextItem(const char* str_id, ImGuiPopupFlags popup_flags);
extern  _Bool imguiBeginPopupContextWindow(const char* str_id, ImGuiPopupFlags popup_flags);
extern  _Bool imguiBeginPopupContextVoid(const char* str_id, ImGuiPopupFlags popup_flags);
extern  _Bool imguiIsPopupOpen_Str(const char* str_id, ImGuiPopupFlags flags);
extern  _Bool imguiBeginTable(const char* str_id, int columns, ImGuiTableFlags flags, const ImVec2 outer_size, float inner_width);
extern  void imguiEndTable(void);
extern  void imguiTableNextRow(ImGuiTableRowFlags row_flags, float min_row_height);
extern  _Bool imguiTableNextColumn(void);
extern  _Bool imguiTableSetColumnIndex(int column_n);
extern  void imguiTableSetupColumn(const char* label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImGuiID user_id);
extern  void imguiTableSetupScrollFreeze(int cols, int rows);
extern  void imguiTableHeader(const char* label);
extern  void imguiTableHeadersRow(void);
extern  void imguiTableAngledHeadersRow(void);
extern  ImGuiTableSortSpecs* imguiTableGetSortSpecs(void);
extern  int imguiTableGetColumnCount(void);
extern  int imguiTableGetColumnIndex(void);
extern  int imguiTableGetRowIndex(void);
extern  const char* imguiTableGetColumnName_Int(int column_n);
extern  ImGuiTableColumnFlags imguiTableGetColumnFlags(int column_n);
extern  void imguiTableSetColumnEnabled(int column_n, _Bool v);
extern  int imguiTableGetHoveredColumn(void);
extern  void imguiTableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n);
extern  void imguiColumns(int count, const char* id, _Bool border);
extern  void imguiNextColumn(void);
extern  int imguiGetColumnIndex(void);
extern  float imguiGetColumnWidth(int column_index);
extern  void imguiSetColumnWidth(int column_index, float width);
extern  float imguiGetColumnOffset(int column_index);
extern  void imguiSetColumnOffset(int column_index, float offset_x);
extern  int imguiGetColumnsCount(void);
extern  _Bool imguiBeginTabBar(const char* str_id, ImGuiTabBarFlags flags);
extern  void imguiEndTabBar(void);
extern  _Bool imguiBeginTabItem(const char* label, _Bool* p_open, ImGuiTabItemFlags flags);
extern  void imguiEndTabItem(void);
extern  _Bool imguiTabItemButton(const char* label, ImGuiTabItemFlags flags);
extern  void imguiSetTabItemClosed(const char* tab_or_docked_window_label);
extern  ImGuiID imguiDockSpace(ImGuiID dockspace_id, const ImVec2 size, ImGuiDockNodeFlags flags, const ImGuiWindowClass* window_class);
extern  ImGuiID imguiDockSpaceOverViewport(ImGuiID dockspace_id, const ImGuiViewport* viewport, ImGuiDockNodeFlags flags, const ImGuiWindowClass* window_class);
extern  void imguiSetNextWindowDockID(ImGuiID dock_id, ImGuiCond cond);
extern  void imguiSetNextWindowClass(const ImGuiWindowClass* window_class);
extern  ImGuiID imguiGetWindowDockID(void);
extern  _Bool imguiIsWindowDocked(void);
extern  void imguiLogToTTY(int auto_open_depth);
extern  void imguiLogToFile(int auto_open_depth, const char* filename);
extern  void imguiLogToClipboard(int auto_open_depth);
extern  void imguiLogFinish(void);
extern  void imguiLogButtons(void);
extern  void imguiLogTextV(const char* fmt, va_list args);
extern  _Bool imguiBeginDragDropSource(ImGuiDragDropFlags flags);
extern  _Bool imguiSetDragDropPayload(const char* type, const void* data, size_t sz, ImGuiCond cond);
extern  void imguiEndDragDropSource(void);
extern  _Bool imguiBeginDragDropTarget(void);
extern  const ImGuiPayload* imguiAcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags);
extern  void imguiEndDragDropTarget(void);
extern  const ImGuiPayload* imguiGetDragDropPayload(void);
extern  void imguiBeginDisabled(_Bool disabled);
extern  void imguiEndDisabled(void);
extern  void imguiPushClipRect(const ImVec2 clip_rect_min, const ImVec2 clip_rect_max, _Bool intersect_with_current_clip_rect);
extern  void imguiPopClipRect(void);
extern  void imguiSetItemDefaultFocus(void);
extern  void imguiSetKeyboardFocusHere(int offset);
extern  void imguiSetNextItemAllowOverlap(void);
extern  _Bool imguiIsItemHovered(ImGuiHoveredFlags flags);
extern  _Bool imguiIsItemActive(void);
extern  _Bool imguiIsItemFocused(void);
extern  _Bool imguiIsItemClicked(ImGuiMouseButton mouse_button);
extern  _Bool imguiIsItemVisible(void);
extern  _Bool imguiIsItemEdited(void);
extern  _Bool imguiIsItemActivated(void);
extern  _Bool imguiIsItemDeactivated(void);
extern  _Bool imguiIsItemDeactivatedAfterEdit(void);
extern  _Bool imguiIsItemToggledOpen(void);
extern  _Bool imguiIsAnyItemHovered(void);
extern  _Bool imguiIsAnyItemActive(void);
extern  _Bool imguiIsAnyItemFocused(void);
extern  ImGuiID imguiGetItemID(void);
extern  void imguiGetItemRectMin(ImVec2* pOut);
extern  void imguiGetItemRectMax(ImVec2* pOut);
extern  void imguiGetItemRectSize(ImVec2* pOut);
extern  ImGuiViewport* imguiGetMainViewport(void);
extern  ImDrawList* imguiGetBackgroundDrawList(ImGuiViewport* viewport);
extern  ImDrawList* imguiGetForegroundDrawList_ViewportPtr(ImGuiViewport* viewport);
extern  _Bool imguiIsRectVisible_Nil(const ImVec2 size);
extern  _Bool imguiIsRectVisible_Vec2(const ImVec2 rect_min, const ImVec2 rect_max);
extern  double imguiGetTime(void);
extern  int imguiGetFrameCount(void);
extern  ImDrawListSharedData* imguiGetDrawListSharedData(void);
extern  const char* imguiGetStyleColorName(ImGuiCol idx);
extern  void imguiSetStateStorage(ImGuiStorage* storage);
extern  ImGuiStorage* imguiGetStateStorage(void);
extern  void imguiCalcTextSize(ImVec2* pOut, const char* text, const char* text_end, _Bool hide_text_after_double_hash, float wrap_width);
extern  void imguiColorConvertU32ToFloat4(ImVec4* pOut, ImU32 in);
extern  ImU32 imguiColorConvertFloat4ToU32(const ImVec4 in);
extern  void imguiColorConvertRGBtoHSV(float r, float g, float b, float* out_h, float* out_s, float* out_v);
extern  void imguiColorConvertHSVtoRGB(float h, float s, float v, float* out_r, float* out_g, float* out_b);
extern  _Bool imguiIsKeyDown_Nil(ImGuiKey key);
extern  _Bool imguiIsKeyPressed_Bool(ImGuiKey key, _Bool repeat);
extern  _Bool imguiIsKeyReleased_Nil(ImGuiKey key);
extern  _Bool imguiIsKeyChordPressed_Nil(ImGuiKeyChord key_chord);
extern  int imguiGetKeyPressedAmount(ImGuiKey key, float repeat_delay, float rate);
extern  const char* imguiGetKeyName(ImGuiKey key);
extern  void imguiSetNextFrameWantCaptureKeyboard(_Bool want_capture_keyboard);
extern  _Bool imguiShortcut_Nil(ImGuiKeyChord key_chord, ImGuiInputFlags flags);
extern  void imguiSetNextItemShortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags);
extern  void imguiSetItemKeyOwner_Nil(ImGuiKey key);
extern  _Bool imguiIsMouseDown_Nil(ImGuiMouseButton button);
extern  _Bool imguiIsMouseClicked_Bool(ImGuiMouseButton button, _Bool repeat);
extern  _Bool imguiIsMouseReleased_Nil(ImGuiMouseButton button);
extern  _Bool imguiIsMouseDoubleClicked_Nil(ImGuiMouseButton button);
extern  int imguiGetMouseClickedCount(ImGuiMouseButton button);
extern  _Bool imguiIsMouseHoveringRect(const ImVec2 r_min, const ImVec2 r_max, _Bool clip);
extern  _Bool imguiIsMousePosValid(const ImVec2* mouse_pos);
extern  _Bool imguiIsAnyMouseDown(void);
extern  void imguiGetMousePos(ImVec2* pOut);
extern  void imguiGetMousePosOnOpeningCurrentPopup(ImVec2* pOut);
extern  _Bool imguiIsMouseDragging(ImGuiMouseButton button, float lock_threshold);
extern  void imguiGetMouseDragDelta(ImVec2* pOut, ImGuiMouseButton button, float lock_threshold);
extern  void imguiResetMouseDragDelta(ImGuiMouseButton button);
extern  ImGuiMouseCursor imguiGetMouseCursor(void);
extern  void imguiSetMouseCursor(ImGuiMouseCursor cursor_type);
extern  void imguiSetNextFrameWantCaptureMouse(_Bool want_capture_mouse);
extern  const char* imguiGetClipboardText(void);
extern  void imguiSetClipboardText(const char* text);
extern  void imguiLoadIniSettingsFromDisk(const char* ini_filename);
extern  void imguiLoadIniSettingsFromMemory(const char* ini_data, size_t ini_size);
extern  void imguiSaveIniSettingsToDisk(const char* ini_filename);
extern  const char* imguiSaveIniSettingsToMemory(size_t* out_ini_size);
extern  void imguiDebugTextEncoding(const char* text);
extern  void imguiDebugFlashStyleColor(ImGuiCol idx);
extern  void imguiDebugStartItemPicker(void);
extern  _Bool imguiDebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx);
extern  void imguiDebugLog(const char* fmt, ...);
extern  void imguiDebugLogV(const char* fmt, va_list args);
extern  void imguiSetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void* user_data);
extern  void imguiGetAllocatorFunctions(ImGuiMemAllocFunc* p_alloc_func, ImGuiMemFreeFunc* p_free_func, void** p_user_data);
extern  void* imguiMemAlloc(size_t size);
extern  void imguiMemFree(void* ptr);
extern  ImGuiPlatformIO* imguiGetPlatformIO(void);
extern  void imguiUpdatePlatformWindows(void);
extern  void imguiRenderPlatformWindowsDefault(void* platform_render_arg, void* renderer_render_arg);
extern  void imguiDestroyPlatformWindows(void);
extern  ImGuiViewport* imguiFindViewportByID(ImGuiID id);
extern  ImGuiViewport* imguiFindViewportByPlatformHandle(void* platform_handle);
extern  ImGuiTableSortSpecs* ImGuiTableSortSpecs_ImGuiTableSortSpecs(void);
extern  void ImGuiTableSortSpecs_destroy(ImGuiTableSortSpecs* self);
extern  ImGuiTableColumnSortSpecs* ImGuiTableColumnSortSpecs_ImGuiTableColumnSortSpecs(void);
extern  void ImGuiTableColumnSortSpecs_destroy(ImGuiTableColumnSortSpecs* self);
extern  ImGuiStyle* ImGuiStyle_ImGuiStyle(void);
extern  void ImGuiStyle_destroy(ImGuiStyle* self);
extern  void ImGuiStyle_ScaleAllSizes(ImGuiStyle* self, float scale_factor);
extern  void ImGuiIO_AddKeyEvent(ImGuiIO* self, ImGuiKey key, _Bool down);
extern  void ImGuiIO_AddKeyAnalogEvent(ImGuiIO* self, ImGuiKey key, _Bool down, float v);
extern  void ImGuiIO_AddMousePosEvent(ImGuiIO* self, float x, float y);
extern  void ImGuiIO_AddMouseButtonEvent(ImGuiIO* self, int button, _Bool down);
extern  void ImGuiIO_AddMouseWheelEvent(ImGuiIO* self, float wheel_x, float wheel_y);
extern  void ImGuiIO_AddMouseSourceEvent(ImGuiIO* self, ImGuiMouseSource source);
extern  void ImGuiIO_AddMouseViewportEvent(ImGuiIO* self, ImGuiID id);
extern  void ImGuiIO_AddFocusEvent(ImGuiIO* self, _Bool focused);
extern  void ImGuiIO_AddInputCharacter(ImGuiIO* self, unsigned int c);
extern  void ImGuiIO_AddInputCharacterUTF16(ImGuiIO* self, ImWchar16 c);
extern  void ImGuiIO_AddInputCharactersUTF8(ImGuiIO* self, const char* str);
extern  void ImGuiIO_SetKeyEventNativeData(ImGuiIO* self, ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index);
extern  void ImGuiIO_SetAppAcceptingEvents(ImGuiIO* self, _Bool accepting_events);
extern  void ImGuiIO_ClearEventsQueue(ImGuiIO* self);
extern  void ImGuiIO_ClearInputKeys(ImGuiIO* self);
extern  void ImGuiIO_ClearInputMouse(ImGuiIO* self);
extern  ImGuiIO* ImGuiIO_ImGuiIO(void);
extern  void ImGuiIO_destroy(ImGuiIO* self);
extern  ImGuiInputTextCallbackData* ImGuiInputTextCallbackData_ImGuiInputTextCallbackData(void);
extern  void ImGuiInputTextCallbackData_destroy(ImGuiInputTextCallbackData* self);
extern  void ImGuiInputTextCallbackData_DeleteChars(ImGuiInputTextCallbackData* self, int pos, int bytes_count);
extern  void ImGuiInputTextCallbackData_InsertChars(ImGuiInputTextCallbackData* self, int pos, const char* text, const char* text_end);
extern  void ImGuiInputTextCallbackData_SelectAll(ImGuiInputTextCallbackData* self);
extern  void ImGuiInputTextCallbackData_ClearSelection(ImGuiInputTextCallbackData* self);
extern  _Bool ImGuiInputTextCallbackData_HasSelection(ImGuiInputTextCallbackData* self);
extern  ImGuiWindowClass* ImGuiWindowClass_ImGuiWindowClass(void);
extern  void ImGuiWindowClass_destroy(ImGuiWindowClass* self);
extern  ImGuiPayload* ImGuiPayload_ImGuiPayload(void);
extern  void ImGuiPayload_destroy(ImGuiPayload* self);
extern  void ImGuiPayload_Clear(ImGuiPayload* self);
extern  _Bool ImGuiPayload_IsDataType(ImGuiPayload* self, const char* type);
extern  _Bool ImGuiPayload_IsPreview(ImGuiPayload* self);
extern  _Bool ImGuiPayload_IsDelivery(ImGuiPayload* self);
extern  ImGuiOnceUponAFrame* ImGuiOnceUponAFrame_ImGuiOnceUponAFrame(void);
extern  void ImGuiOnceUponAFrame_destroy(ImGuiOnceUponAFrame* self);
extern  ImGuiTextFilter* ImGuiTextFilter_ImGuiTextFilter(const char* default_filter);
extern  void ImGuiTextFilter_destroy(ImGuiTextFilter* self);
extern  _Bool ImGuiTextFilter_Draw(ImGuiTextFilter* self, const char* label, float width);
extern  _Bool ImGuiTextFilter_PassFilter(ImGuiTextFilter* self, const char* text, const char* text_end);
extern  void ImGuiTextFilter_Build(ImGuiTextFilter* self);
extern  void ImGuiTextFilter_Clear(ImGuiTextFilter* self);
extern  _Bool ImGuiTextFilter_IsActive(ImGuiTextFilter* self);
extern  ImGuiTextRange* ImGuiTextRange_ImGuiTextRange_Nil(void);
extern  void ImGuiTextRange_destroy(ImGuiTextRange* self);
extern  ImGuiTextRange* ImGuiTextRange_ImGuiTextRange_Str(const char* _b, const char* _e);
extern  _Bool ImGuiTextRange_empty(ImGuiTextRange* self);
extern  void ImGuiTextRange_split(ImGuiTextRange* self, char separator, ImVector_ImGuiTextRange* out);
extern  ImGuiTextBuffer* ImGuiTextBuffer_ImGuiTextBuffer(void);
extern  void ImGuiTextBuffer_destroy(ImGuiTextBuffer* self);
extern  const char* ImGuiTextBuffer_begin(ImGuiTextBuffer* self);
extern  const char* ImGuiTextBuffer_end(ImGuiTextBuffer* self);
extern  int ImGuiTextBuffer_size(ImGuiTextBuffer* self);
extern  _Bool ImGuiTextBuffer_empty(ImGuiTextBuffer* self);
extern  void ImGuiTextBuffer_clear(ImGuiTextBuffer* self);
extern  void ImGuiTextBuffer_reserve(ImGuiTextBuffer* self, int capacity);
extern  const char* ImGuiTextBuffer_c_str(ImGuiTextBuffer* self);
extern  void ImGuiTextBuffer_append(ImGuiTextBuffer* self, const char* str, const char* str_end);
extern  void ImGuiTextBuffer_appendfv(ImGuiTextBuffer* self, const char* fmt, va_list args);
extern  ImGuiStoragePair* ImGuiStoragePair_ImGuiStoragePair_Int(ImGuiID _key, int _val);
extern  void ImGuiStoragePair_destroy(ImGuiStoragePair* self);
extern  ImGuiStoragePair* ImGuiStoragePair_ImGuiStoragePair_Float(ImGuiID _key, float _val);
extern  ImGuiStoragePair* ImGuiStoragePair_ImGuiStoragePair_Ptr(ImGuiID _key, void* _val);
extern  void ImGuiStorage_Clear(ImGuiStorage* self);
extern  int ImGuiStorage_GetInt(ImGuiStorage* self, ImGuiID key, int default_val);
extern  void ImGuiStorage_SetInt(ImGuiStorage* self, ImGuiID key, int val);
extern  _Bool ImGuiStorage_GetBool(ImGuiStorage* self, ImGuiID key, _Bool default_val);
extern  void ImGuiStorage_SetBool(ImGuiStorage* self, ImGuiID key, _Bool val);
extern  float ImGuiStorage_GetFloat(ImGuiStorage* self, ImGuiID key, float default_val);
extern  void ImGuiStorage_SetFloat(ImGuiStorage* self, ImGuiID key, float val);
extern  void* ImGuiStorage_GetVoidPtr(ImGuiStorage* self, ImGuiID key);
extern  void ImGuiStorage_SetVoidPtr(ImGuiStorage* self, ImGuiID key, void* val);
extern  int* ImGuiStorage_GetIntRef(ImGuiStorage* self, ImGuiID key, int default_val);
extern  _Bool* ImGuiStorage_GetBoolRef(ImGuiStorage* self, ImGuiID key, _Bool default_val);
extern  float* ImGuiStorage_GetFloatRef(ImGuiStorage* self, ImGuiID key, float default_val);
extern  void** ImGuiStorage_GetVoidPtrRef(ImGuiStorage* self, ImGuiID key, void* default_val);
extern  void ImGuiStorage_BuildSortByKey(ImGuiStorage* self);
extern  void ImGuiStorage_SetAllInt(ImGuiStorage* self, int val);
extern  ImGuiListClipper* ImGuiListClipper_ImGuiListClipper(void);
extern  void ImGuiListClipper_destroy(ImGuiListClipper* self);
extern  void ImGuiListClipper_Begin(ImGuiListClipper* self, int items_count, float items_height);
extern  void ImGuiListClipper_End(ImGuiListClipper* self);
extern  _Bool ImGuiListClipper_Step(ImGuiListClipper* self);
extern  void ImGuiListClipper_IncludeItemByIndex(ImGuiListClipper* self, int item_index);
extern  void ImGuiListClipper_IncludeItemsByIndex(ImGuiListClipper* self, int item_begin, int item_end);
extern  void ImGuiListClipper_SeekCursorForItem(ImGuiListClipper* self, int item_index);
extern  ImColor* ImColor_ImColor_Nil(void);
extern  void ImColor_destroy(ImColor* self);
extern  ImColor* ImColor_ImColor_Float(float r, float g, float b, float a);
extern  ImColor* ImColor_ImColor_Vec4(const ImVec4 col);
extern  ImColor* ImColor_ImColor_Int(int r, int g, int b, int a);
extern  ImColor* ImColor_ImColor_U32(ImU32 rgba);
extern  void ImColor_SetHSV(ImColor* self, float h, float s, float v, float a);
extern  void ImColor_HSV(ImColor* pOut, float h, float s, float v, float a);
extern  ImGuiSelectionBasicStorage* ImGuiSelectionBasicStorage_ImGuiSelectionBasicStorage(void);
extern  void ImGuiSelectionBasicStorage_destroy(ImGuiSelectionBasicStorage* self);
extern  void ImGuiSelectionBasicStorage_ApplyRequests(ImGuiSelectionBasicStorage* self, ImGuiMultiSelectIO* ms_io);
extern  _Bool ImGuiSelectionBasicStorage_Contains(ImGuiSelectionBasicStorage* self, ImGuiID id);
extern  void ImGuiSelectionBasicStorage_Clear(ImGuiSelectionBasicStorage* self);
extern  void ImGuiSelectionBasicStorage_Swap(ImGuiSelectionBasicStorage* self, ImGuiSelectionBasicStorage* r);
extern  void ImGuiSelectionBasicStorage_SetItemSelected(ImGuiSelectionBasicStorage* self, ImGuiID id, _Bool selected);
extern  _Bool ImGuiSelectionBasicStorage_GetNextSelectedItem(ImGuiSelectionBasicStorage* self, void** opaque_it, ImGuiID* out_id);
extern  ImGuiID ImGuiSelectionBasicStorage_GetStorageIdFromIndex(ImGuiSelectionBasicStorage* self, int idx);
extern  ImGuiSelectionExternalStorage* ImGuiSelectionExternalStorage_ImGuiSelectionExternalStorage(void);
extern  void ImGuiSelectionExternalStorage_destroy(ImGuiSelectionExternalStorage* self);
extern  void ImGuiSelectionExternalStorage_ApplyRequests(ImGuiSelectionExternalStorage* self, ImGuiMultiSelectIO* ms_io);
extern  ImDrawCmd* ImDrawCmd_ImDrawCmd(void);
extern  void ImDrawCmd_destroy(ImDrawCmd* self);
extern  ImTextureID ImDrawCmd_GetTexID(ImDrawCmd* self);
extern  ImDrawListSplitter* ImDrawListSplitter_ImDrawListSplitter(void);
extern  void ImDrawListSplitter_destroy(ImDrawListSplitter* self);
extern  void ImDrawListSplitter_Clear(ImDrawListSplitter* self);
extern  void ImDrawListSplitter_ClearFreeMemory(ImDrawListSplitter* self);
extern  void ImDrawListSplitter_Split(ImDrawListSplitter* self, ImDrawList* draw_list, int count);
extern  void ImDrawListSplitter_Merge(ImDrawListSplitter* self, ImDrawList* draw_list);
extern  void ImDrawListSplitter_SetCurrentChannel(ImDrawListSplitter* self, ImDrawList* draw_list, int channel_idx);
extern  ImDrawList* ImDrawList_ImDrawList(ImDrawListSharedData* shared_data);
extern  void ImDrawList_destroy(ImDrawList* self);
extern  void ImDrawList_PushClipRect(ImDrawList* self, const ImVec2 clip_rect_min, const ImVec2 clip_rect_max, _Bool intersect_with_current_clip_rect);
extern  void ImDrawList_PushClipRectFullScreen(ImDrawList* self);
extern  void ImDrawList_PopClipRect(ImDrawList* self);
extern  void ImDrawList_PushTextureID(ImDrawList* self, ImTextureID texture_id);
extern  void ImDrawList_PopTextureID(ImDrawList* self);
extern  void ImDrawList_GetClipRectMin(ImVec2* pOut, ImDrawList* self);
extern  void ImDrawList_GetClipRectMax(ImVec2* pOut, ImDrawList* self);
extern  void ImDrawList_AddLine(ImDrawList* self, const ImVec2 p1, const ImVec2 p2, ImU32 col, float thickness);
extern  void ImDrawList_AddRect(ImDrawList* self, const ImVec2 p_min, const ImVec2 p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness);
extern  void ImDrawList_AddRectFilled(ImDrawList* self, const ImVec2 p_min, const ImVec2 p_max, ImU32 col, float rounding, ImDrawFlags flags);
extern  void ImDrawList_AddRectFilledMultiColor(ImDrawList* self, const ImVec2 p_min, const ImVec2 p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
extern  void ImDrawList_AddQuad(ImDrawList* self, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, ImU32 col, float thickness);
extern  void ImDrawList_AddQuadFilled(ImDrawList* self, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, ImU32 col);
extern  void ImDrawList_AddTriangle(ImDrawList* self, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, ImU32 col, float thickness);
extern  void ImDrawList_AddTriangleFilled(ImDrawList* self, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, ImU32 col);
extern  void ImDrawList_AddCircle(ImDrawList* self, const ImVec2 center, float radius, ImU32 col, int num_segments, float thickness);
extern  void ImDrawList_AddCircleFilled(ImDrawList* self, const ImVec2 center, float radius, ImU32 col, int num_segments);
extern  void ImDrawList_AddNgon(ImDrawList* self, const ImVec2 center, float radius, ImU32 col, int num_segments, float thickness);
extern  void ImDrawList_AddNgonFilled(ImDrawList* self, const ImVec2 center, float radius, ImU32 col, int num_segments);
extern  void ImDrawList_AddEllipse(ImDrawList* self, const ImVec2 center, const ImVec2 radius, ImU32 col, float rot, int num_segments, float thickness);
extern  void ImDrawList_AddEllipseFilled(ImDrawList* self, const ImVec2 center, const ImVec2 radius, ImU32 col, float rot, int num_segments);
extern  void ImDrawList_AddText_Vec2(ImDrawList* self, const ImVec2 pos, ImU32 col, const char* text_begin, const char* text_end);
extern  void ImDrawList_AddText_FontPtr(ImDrawList* self, const ImFont* font, float font_size, const ImVec2 pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width,
                                           const ImVec4* cpu_fine_clip_rect);
extern  void ImDrawList_AddBezierCubic(ImDrawList* self, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, ImU32 col, float thickness, int num_segments);
extern  void ImDrawList_AddBezierQuadratic(ImDrawList* self, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, ImU32 col, float thickness, int num_segments);
extern  void ImDrawList_AddPolyline(ImDrawList* self, const ImVec2* points, int num_points, ImU32 col, ImDrawFlags flags, float thickness);
extern  void ImDrawList_AddConvexPolyFilled(ImDrawList* self, const ImVec2* points, int num_points, ImU32 col);
extern  void ImDrawList_AddConcavePolyFilled(ImDrawList* self, const ImVec2* points, int num_points, ImU32 col);
extern  void ImDrawList_AddImage(ImDrawList* self, ImTextureID user_texture_id, const ImVec2 p_min, const ImVec2 p_max, const ImVec2 uv_min, const ImVec2 uv_max, ImU32 col);
extern  void ImDrawList_AddImageQuad(ImDrawList* self, ImTextureID user_texture_id, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, const ImVec2 uv1, const ImVec2 uv2,
                                        const ImVec2 uv3, const ImVec2 uv4, ImU32 col);
extern  void ImDrawList_AddImageRounded(ImDrawList* self, ImTextureID user_texture_id, const ImVec2 p_min, const ImVec2 p_max, const ImVec2 uv_min, const ImVec2 uv_max, ImU32 col, float rounding,
                                           ImDrawFlags flags);
extern  void ImDrawList_PathClear(ImDrawList* self);
extern  void ImDrawList_PathLineTo(ImDrawList* self, const ImVec2 pos);
extern  void ImDrawList_PathLineToMergeDuplicate(ImDrawList* self, const ImVec2 pos);
extern  void ImDrawList_PathFillConvex(ImDrawList* self, ImU32 col);
extern  void ImDrawList_PathFillConcave(ImDrawList* self, ImU32 col);
extern  void ImDrawList_PathStroke(ImDrawList* self, ImU32 col, ImDrawFlags flags, float thickness);
extern  void ImDrawList_PathArcTo(ImDrawList* self, const ImVec2 center, float radius, float a_min, float a_max, int num_segments);
extern  void ImDrawList_PathArcToFast(ImDrawList* self, const ImVec2 center, float radius, int a_min_of_12, int a_max_of_12);
extern  void ImDrawList_PathEllipticalArcTo(ImDrawList* self, const ImVec2 center, const ImVec2 radius, float rot, float a_min, float a_max, int num_segments);
extern  void ImDrawList_PathBezierCubicCurveTo(ImDrawList* self, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, int num_segments);
extern  void ImDrawList_PathBezierQuadraticCurveTo(ImDrawList* self, const ImVec2 p2, const ImVec2 p3, int num_segments);
extern  void ImDrawList_PathRect(ImDrawList* self, const ImVec2 rect_min, const ImVec2 rect_max, float rounding, ImDrawFlags flags);
extern  void ImDrawList_AddCallback(ImDrawList* self, ImDrawCallback callback, void* callback_data);
extern  void ImDrawList_AddDrawCmd(ImDrawList* self);
extern  ImDrawList* ImDrawList_CloneOutput(ImDrawList* self);
extern  void ImDrawList_ChannelsSplit(ImDrawList* self, int count);
extern  void ImDrawList_ChannelsMerge(ImDrawList* self);
extern  void ImDrawList_ChannelsSetCurrent(ImDrawList* self, int n);
extern  void ImDrawList_PrimReserve(ImDrawList* self, int idx_count, int vtx_count);
extern  void ImDrawList_PrimUnreserve(ImDrawList* self, int idx_count, int vtx_count);
extern  void ImDrawList_PrimRect(ImDrawList* self, const ImVec2 a, const ImVec2 b, ImU32 col);
extern  void ImDrawList_PrimRectUV(ImDrawList* self, const ImVec2 a, const ImVec2 b, const ImVec2 uv_a, const ImVec2 uv_b, ImU32 col);
extern  void ImDrawList_PrimQuadUV(ImDrawList* self, const ImVec2 a, const ImVec2 b, const ImVec2 c, const ImVec2 d, const ImVec2 uv_a, const ImVec2 uv_b, const ImVec2 uv_c, const ImVec2 uv_d,
                                      ImU32 col);
extern  void ImDrawList_PrimWriteVtx(ImDrawList* self, const ImVec2 pos, const ImVec2 uv, ImU32 col);
extern  void ImDrawList_PrimWriteIdx(ImDrawList* self, ImDrawIdx idx);
extern  void ImDrawList_PrimVtx(ImDrawList* self, const ImVec2 pos, const ImVec2 uv, ImU32 col);
extern  void ImDrawList__ResetForNewFrame(ImDrawList* self);
extern  void ImDrawList__ClearFreeMemory(ImDrawList* self);
extern  void ImDrawList__PopUnusedDrawCmd(ImDrawList* self);
extern  void ImDrawList__TryMergeDrawCmds(ImDrawList* self);
extern  void ImDrawList__OnChangedClipRect(ImDrawList* self);
extern  void ImDrawList__OnChangedTextureID(ImDrawList* self);
extern  void ImDrawList__OnChangedVtxOffset(ImDrawList* self);
extern  int ImDrawList__CalcCircleAutoSegmentCount(ImDrawList* self, float radius);
extern  void ImDrawList__PathArcToFastEx(ImDrawList* self, const ImVec2 center, float radius, int a_min_sample, int a_max_sample, int a_step);
extern  void ImDrawList__PathArcToN(ImDrawList* self, const ImVec2 center, float radius, float a_min, float a_max, int num_segments);
extern  ImDrawData* ImDrawData_ImDrawData(void);
extern  void ImDrawData_destroy(ImDrawData* self);
extern  void ImDrawData_Clear(ImDrawData* self);
extern  void ImDrawData_AddDrawList(ImDrawData* self, ImDrawList* draw_list);
extern  void ImDrawData_DeIndexAllBuffers(ImDrawData* self);
extern  void ImDrawData_ScaleClipRects(ImDrawData* self, const ImVec2 fb_scale);
extern  ImFontConfig* ImFontConfig_ImFontConfig(void);
extern  void ImFontConfig_destroy(ImFontConfig* self);
extern  ImFontGlyphRangesBuilder* ImFontGlyphRangesBuilder_ImFontGlyphRangesBuilder(void);
extern  void ImFontGlyphRangesBuilder_destroy(ImFontGlyphRangesBuilder* self);
extern  void ImFontGlyphRangesBuilder_Clear(ImFontGlyphRangesBuilder* self);
extern  _Bool ImFontGlyphRangesBuilder_GetBit(ImFontGlyphRangesBuilder* self, size_t n);
extern  void ImFontGlyphRangesBuilder_SetBit(ImFontGlyphRangesBuilder* self, size_t n);
extern  void ImFontGlyphRangesBuilder_AddChar(ImFontGlyphRangesBuilder* self, ImWchar c);
extern  void ImFontGlyphRangesBuilder_AddText(ImFontGlyphRangesBuilder* self, const char* text, const char* text_end);
extern  void ImFontGlyphRangesBuilder_AddRanges(ImFontGlyphRangesBuilder* self, const ImWchar* ranges);
extern  void ImFontGlyphRangesBuilder_BuildRanges(ImFontGlyphRangesBuilder* self, ImVector_ImWchar* out_ranges);
extern  ImFontAtlasCustomRect* ImFontAtlasCustomRect_ImFontAtlasCustomRect(void);
extern  void ImFontAtlasCustomRect_destroy(ImFontAtlasCustomRect* self);
extern  _Bool ImFontAtlasCustomRect_IsPacked(ImFontAtlasCustomRect* self);
extern  ImFontAtlas* ImFontAtlas_ImFontAtlas(void);
extern  void ImFontAtlas_destroy(ImFontAtlas* self);
extern  ImFont* ImFontAtlas_AddFont(ImFontAtlas* self, const ImFontConfig* font_cfg);
extern  ImFont* ImFontAtlas_AddFontDefault(ImFontAtlas* self, const ImFontConfig* font_cfg);
extern  ImFont* ImFontAtlas_AddFontFromFileTTF(ImFontAtlas* self, const char* filename, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges);
extern  ImFont* ImFontAtlas_AddFontFromMemoryTTF(ImFontAtlas* self, void* font_data, int font_data_size, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges);
extern  ImFont* ImFontAtlas_AddFontFromMemoryCompressedTTF(ImFontAtlas* self, const void* compressed_font_data, int compressed_font_data_size, float size_pixels, const ImFontConfig* font_cfg,
                                                              const ImWchar* glyph_ranges);
extern  ImFont* ImFontAtlas_AddFontFromMemoryCompressedBase85TTF(ImFontAtlas* self, const char* compressed_font_data_base85, float size_pixels, const ImFontConfig* font_cfg,
                                                                    const ImWchar* glyph_ranges);
extern  void ImFontAtlas_ClearInputData(ImFontAtlas* self);
extern  void ImFontAtlas_ClearTexData(ImFontAtlas* self);
extern  void ImFontAtlas_ClearFonts(ImFontAtlas* self);
extern  void ImFontAtlas_Clear(ImFontAtlas* self);
extern  _Bool ImFontAtlas_Build(ImFontAtlas* self);
extern  void ImFontAtlas_GetTexDataAsAlpha8(ImFontAtlas* self, unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel);
extern  void ImFontAtlas_GetTexDataAsRGBA32(ImFontAtlas* self, unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel);
extern  _Bool ImFontAtlas_IsBuilt(ImFontAtlas* self);
extern  void ImFontAtlas_SetTexID(ImFontAtlas* self, ImTextureID id);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesDefault(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesGreek(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesKorean(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesJapanese(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesChineseFull(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesChineseSimplifiedCommon(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesCyrillic(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesThai(ImFontAtlas* self);
extern  const ImWchar* ImFontAtlas_GetGlyphRangesVietnamese(ImFontAtlas* self);
extern  int ImFontAtlas_AddCustomRectRegular(ImFontAtlas* self, int width, int height);
extern  int ImFontAtlas_AddCustomRectFontGlyph(ImFontAtlas* self, ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2 offset);
extern  ImFontAtlasCustomRect* ImFontAtlas_GetCustomRectByIndex(ImFontAtlas* self, int index);
extern  void ImFontAtlas_CalcCustomRectUV(ImFontAtlas* self, const ImFontAtlasCustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max);
extern  _Bool ImFontAtlas_GetMouseCursorTexData(ImFontAtlas* self, ImGuiMouseCursor cursor, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2]);
extern  ImFont* ImFont_ImFont(void);
extern  void ImFont_destroy(ImFont* self);
extern  const ImFontGlyph* ImFont_FindGlyph(ImFont* self, ImWchar c);
extern  const ImFontGlyph* ImFont_FindGlyphNoFallback(ImFont* self, ImWchar c);
extern  float ImFont_GetCharAdvance(ImFont* self, ImWchar c);
extern  _Bool ImFont_IsLoaded(ImFont* self);
extern  const char* ImFont_GetDebugName(ImFont* self);
extern  void ImFont_CalcTextSizeA(ImVec2* pOut, ImFont* self, float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining);
extern  const char* ImFont_CalcWordWrapPositionA(ImFont* self, float scale, const char* text, const char* text_end, float wrap_width);
extern  void ImFont_RenderChar(ImFont* self, ImDrawList* draw_list, float size, const ImVec2 pos, ImU32 col, ImWchar c);
extern  void ImFont_RenderText(ImFont* self, ImDrawList* draw_list, float size, const ImVec2 pos, ImU32 col, const ImVec4 clip_rect, const char* text_begin, const char* text_end, float wrap_width,
                                  _Bool cpu_fine_clip);
extern  void ImFont_BuildLookupTable(ImFont* self);
extern  void ImFont_ClearOutputData(ImFont* self);
extern  void ImFont_GrowIndex(ImFont* self, int new_size);
extern  void ImFont_AddGlyph(ImFont* self, const ImFontConfig* src_cfg, ImWchar c, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x);
extern  void ImFont_AddRemapChar(ImFont* self, ImWchar dst, ImWchar src, _Bool overwrite_dst);
extern  void ImFont_SetGlyphVisible(ImFont* self, ImWchar c, _Bool visible);
extern  _Bool ImFont_IsGlyphRangeUnused(ImFont* self, unsigned int c_begin, unsigned int c_last);
extern  ImGuiViewport* ImGuiViewport_ImGuiViewport(void);
extern  void ImGuiViewport_destroy(ImGuiViewport* self);
extern  void ImGuiViewport_GetCenter(ImVec2* pOut, ImGuiViewport* self);
extern  void ImGuiViewport_GetWorkCenter(ImVec2* pOut, ImGuiViewport* self);
extern  ImGuiPlatformIO* ImGuiPlatformIO_ImGuiPlatformIO(void);
extern  void ImGuiPlatformIO_destroy(ImGuiPlatformIO* self);
extern  ImGuiPlatformMonitor* ImGuiPlatformMonitor_ImGuiPlatformMonitor(void);
extern  void ImGuiPlatformMonitor_destroy(ImGuiPlatformMonitor* self);
extern  ImGuiPlatformImeData* ImGuiPlatformImeData_ImGuiPlatformImeData(void);
extern  void ImGuiPlatformImeData_destroy(ImGuiPlatformImeData* self);
extern  ImGuiID imguiImHashData(const void* data, size_t data_size, ImGuiID seed);
extern  ImGuiID imguiImHashStr(const char* data, size_t data_size, ImGuiID seed);
extern  void imguiImQsort(void* base, size_t count, size_t size_of_element, int (*compare_func)(void const*, void const*));
extern  ImU32 imguiImAlphaBlendColors(ImU32 col_a, ImU32 col_b);
extern  _Bool imguiImIsPowerOfTwo_Int(int v);
extern  _Bool imguiImIsPowerOfTwo_U64(ImU64 v);
extern  int imguiImUpperPowerOfTwo(int v);
extern  int imguiImStricmp(const char* str1, const char* str2);
extern  int imguiImStrnicmp(const char* str1, const char* str2, size_t count);
extern  void imguiImStrncpy(char* dst, const char* src, size_t count);
extern  char* imguiImStrdup(const char* str);
extern  char* imguiImStrdupcpy(char* dst, size_t* p_dst_size, const char* str);
extern  const char* imguiImStrchrRange(const char* str_begin, const char* str_end, char c);
extern  const char* imguiImStreolRange(const char* str, const char* str_end);
extern  const char* imguiImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end);
extern  void imguiImStrTrimBlanks(char* str);
extern  const char* imguiImStrSkipBlank(const char* str);
extern  int imguiImStrlenW(const ImWchar* str);
extern  const ImWchar* imguiImStrbolW(const ImWchar* buf_mid_line, const ImWchar* buf_begin);
extern  char imguiImToUpper(char c);
extern  _Bool imguiImCharIsBlankA(char c);
extern  _Bool imguiImCharIsBlankW(unsigned int c);
extern  _Bool imguiImCharIsXdigitA(char c);
extern  int imguiImFormatString(char* buf, size_t buf_size, const char* fmt, ...);
extern  int imguiImFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args);
extern  void imguiImFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...);
extern  void imguiImFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args);
extern  const char* imguiImParseFormatFindStart(const char* format);
extern  const char* imguiImParseFormatFindEnd(const char* format);
extern  const char* imguiImParseFormatTrimDecorations(const char* format, char* buf, size_t buf_size);
extern  void imguiImParseFormatSanitizeForPrinting(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
extern  const char* imguiImParseFormatSanitizeForScanning(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
extern  int imguiImParseFormatPrecision(const char* format, int default_value);
extern  const char* imguiImTextCharToUtf8(char out_buf[5], unsigned int c);
extern  int imguiImTextStrToUtf8(char* out_buf, int out_buf_size, const ImWchar* in_text, const ImWchar* in_text_end);
extern  int imguiImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);
extern  int imguiImTextStrFromUtf8(ImWchar* out_buf, int out_buf_size, const char* in_text, const char* in_text_end, const char** in_remaining);
extern  int imguiImTextCountCharsFromUtf8(const char* in_text, const char* in_text_end);
extern  int imguiImTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end);
extern  int imguiImTextCountUtf8BytesFromStr(const ImWchar* in_text, const ImWchar* in_text_end);
extern  const char* imguiImTextFindPreviousUtf8Codepoint(const char* in_text_start, const char* in_text_curr);
extern  int imguiImTextCountLines(const char* in_text, const char* in_text_end);
extern  ImFileHandle imguiImFileOpen(const char* filename, const char* mode);
extern  _Bool imguiImFileClose(ImFileHandle file);
extern  ImU64 imguiImFileGetSize(ImFileHandle file);
extern  ImU64 imguiImFileRead(void* data, ImU64 size, ImU64 count, ImFileHandle file);
extern  ImU64 imguiImFileWrite(const void* data, ImU64 size, ImU64 count, ImFileHandle file);
extern  void* imguiImFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size, int padding_bytes);
extern  float imguiImPow_Float(float x, float y);
extern  double imguiImPow_double(double x, double y);
extern  float imguiImLog_Float(float x);
extern  double imguiImLog_double(double x);
extern  int imguiImAbs_Int(int x);
extern  float imguiImAbs_Float(float x);
extern  double imguiImAbs_double(double x);
extern  float imguiImSign_Float(float x);
extern  double imguiImSign_double(double x);
extern  float imguiImRsqrt_Float(float x);
extern  double imguiImRsqrt_double(double x);
extern  void imguiImMin(ImVec2* pOut, const ImVec2 lhs, const ImVec2 rhs);
extern  void imguiImMax(ImVec2* pOut, const ImVec2 lhs, const ImVec2 rhs);
extern  void imguiImClamp(ImVec2* pOut, const ImVec2 v, const ImVec2 mn, const ImVec2 mx);
extern  void imguiImLerp_Vec2Float(ImVec2* pOut, const ImVec2 a, const ImVec2 b, float t);
extern  void imguiImLerp_Vec2Vec2(ImVec2* pOut, const ImVec2 a, const ImVec2 b, const ImVec2 t);
extern  void imguiImLerp_Vec4(ImVec4* pOut, const ImVec4 a, const ImVec4 b, float t);
extern  float imguiImSaturate(float f);
extern  float imguiImLengthSqr_Vec2(const ImVec2 lhs);
extern  float imguiImLengthSqr_Vec4(const ImVec4 lhs);
extern  float imguiImInvLength(const ImVec2 lhs, float fail_value);
extern  float imguiImTrunc_Float(float f);
extern  void imguiImTrunc_Vec2(ImVec2* pOut, const ImVec2 v);
extern  float imguiImFloor_Float(float f);
extern  void imguiImFloor_Vec2(ImVec2* pOut, const ImVec2 v);
extern  int imguiImModPositive(int a, int b);
extern  float imguiImDot(const ImVec2 a, const ImVec2 b);
extern  void imguiImRotate(ImVec2* pOut, const ImVec2 v, float cos_a, float sin_a);
extern  float imguiImLinearSweep(float current, float target, float speed);
extern  float imguiImLinearRemapClamp(float s0, float s1, float d0, float d1, float x);
extern  void imguiImMul(ImVec2* pOut, const ImVec2 lhs, const ImVec2 rhs);
extern  _Bool imguiImIsFloatAboveGuaranteedIntegerPrecision(float f);
extern  float imguiImExponentialMovingAverage(float avg, float sample, int n);
extern  void imguiImBezierCubicCalc(ImVec2* pOut, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, float t);
extern  void imguiImBezierCubicClosestPoint(ImVec2* pOut, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, const ImVec2 p, int num_segments);
extern  void imguiImBezierCubicClosestPointCasteljau(ImVec2* pOut, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, const ImVec2 p4, const ImVec2 p, float tess_tol);
extern  void imguiImBezierQuadraticCalc(ImVec2* pOut, const ImVec2 p1, const ImVec2 p2, const ImVec2 p3, float t);
extern  void imguiImLineClosestPoint(ImVec2* pOut, const ImVec2 a, const ImVec2 b, const ImVec2 p);
extern  _Bool imguiImTriangleContainsPoint(const ImVec2 a, const ImVec2 b, const ImVec2 c, const ImVec2 p);
extern  void imguiImTriangleClosestPoint(ImVec2* pOut, const ImVec2 a, const ImVec2 b, const ImVec2 c, const ImVec2 p);
extern  void imguiImTriangleBarycentricCoords(const ImVec2 a, const ImVec2 b, const ImVec2 c, const ImVec2 p, float* out_u, float* out_v, float* out_w);
extern  float imguiImTriangleArea(const ImVec2 a, const ImVec2 b, const ImVec2 c);
extern  _Bool imguiImTriangleIsClockwise(const ImVec2 a, const ImVec2 b, const ImVec2 c);
extern  ImVec1* ImVec1_ImVec1_Nil(void);
extern  void ImVec1_destroy(ImVec1* self);
extern  ImVec1* ImVec1_ImVec1_Float(float _x);
extern  ImVec2ih* ImVec2ih_ImVec2ih_Nil(void);
extern  void ImVec2ih_destroy(ImVec2ih* self);
extern  ImVec2ih* ImVec2ih_ImVec2ih_short(short _x, short _y);
extern  ImVec2ih* ImVec2ih_ImVec2ih_Vec2(const ImVec2 rhs);
extern  ImRect* ImRect_ImRect_Nil(void);
extern  void ImRect_destroy(ImRect* self);
extern  ImRect* ImRect_ImRect_Vec2(const ImVec2 min, const ImVec2 max);
extern  ImRect* ImRect_ImRect_Vec4(const ImVec4 v);
extern  ImRect* ImRect_ImRect_Float(float x1, float y1, float x2, float y2);
extern  void ImRect_GetCenter(ImVec2* pOut, ImRect* self);
extern  void ImRect_GetSize(ImVec2* pOut, ImRect* self);
extern  float ImRect_GetWidth(ImRect* self);
extern  float ImRect_GetHeight(ImRect* self);
extern  float ImRect_GetArea(ImRect* self);
extern  void ImRect_GetTL(ImVec2* pOut, ImRect* self);
extern  void ImRect_GetTR(ImVec2* pOut, ImRect* self);
extern  void ImRect_GetBL(ImVec2* pOut, ImRect* self);
extern  void ImRect_GetBR(ImVec2* pOut, ImRect* self);
extern  _Bool ImRect_Contains_Vec2(ImRect* self, const ImVec2 p);
extern  _Bool ImRect_Contains_Rect(ImRect* self, const ImRect r);
extern  _Bool ImRect_ContainsWithPad(ImRect* self, const ImVec2 p, const ImVec2 pad);
extern  _Bool ImRect_Overlaps(ImRect* self, const ImRect r);
extern  void ImRect_Add_Vec2(ImRect* self, const ImVec2 p);
extern  void ImRect_Add_Rect(ImRect* self, const ImRect r);
extern  void ImRect_Expand_Float(ImRect* self, const float amount);
extern  void ImRect_Expand_Vec2(ImRect* self, const ImVec2 amount);
extern  void ImRect_Translate(ImRect* self, const ImVec2 d);
extern  void ImRect_TranslateX(ImRect* self, float dx);
extern  void ImRect_TranslateY(ImRect* self, float dy);
extern  void ImRect_ClipWith(ImRect* self, const ImRect r);
extern  void ImRect_ClipWithFull(ImRect* self, const ImRect r);
extern  void ImRect_Floor(ImRect* self);
extern  _Bool ImRect_IsInverted(ImRect* self);
extern  void ImRect_ToVec4(ImVec4* pOut, ImRect* self);
extern  size_t imguiImBitArrayGetStorageSizeInBytes(int bitcount);
extern  void imguiImBitArrayClearAllBits(ImU32* arr, int bitcount);
extern  _Bool imguiImBitArrayTestBit(const ImU32* arr, int n);
extern  void imguiImBitArrayClearBit(ImU32* arr, int n);
extern  void imguiImBitArraySetBit(ImU32* arr, int n);
extern  void imguiImBitArraySetBitRange(ImU32* arr, int n, int n2);
extern  void ImBitVector_Create(ImBitVector* self, int sz);
extern  void ImBitVector_Clear(ImBitVector* self);
extern  _Bool ImBitVector_TestBit(ImBitVector* self, int n);
extern  void ImBitVector_SetBit(ImBitVector* self, int n);
extern  void ImBitVector_ClearBit(ImBitVector* self, int n);
extern  void ImGuiTextIndex_clear(ImGuiTextIndex* self);
extern  int ImGuiTextIndex_size(ImGuiTextIndex* self);
extern  const char* ImGuiTextIndex_get_line_begin(ImGuiTextIndex* self, const char* base, int n);
extern  const char* ImGuiTextIndex_get_line_end(ImGuiTextIndex* self, const char* base, int n);
extern  void ImGuiTextIndex_append(ImGuiTextIndex* self, const char* base, int old_size, int new_size);
extern  ImGuiStoragePair* imguiImLowerBound(ImGuiStoragePair* in_begin, ImGuiStoragePair* in_end, ImGuiID key);
extern  ImDrawListSharedData* ImDrawListSharedData_ImDrawListSharedData(void);
extern  void ImDrawListSharedData_destroy(ImDrawListSharedData* self);
extern  void ImDrawListSharedData_SetCircleTessellationMaxError(ImDrawListSharedData* self, float max_error);
extern  ImDrawDataBuilder* ImDrawDataBuilder_ImDrawDataBuilder(void);
extern  void ImDrawDataBuilder_destroy(ImDrawDataBuilder* self);
extern  void* ImGuiDataVarInfo_GetVarPtr(ImGuiDataVarInfo* self, void* parent);
extern  ImGuiStyleMod* ImGuiStyleMod_ImGuiStyleMod_Int(ImGuiStyleVar idx, int v);
extern  void ImGuiStyleMod_destroy(ImGuiStyleMod* self);
extern  ImGuiStyleMod* ImGuiStyleMod_ImGuiStyleMod_Float(ImGuiStyleVar idx, float v);
extern  ImGuiStyleMod* ImGuiStyleMod_ImGuiStyleMod_Vec2(ImGuiStyleVar idx, ImVec2 v);
extern  ImGuiComboPreviewData* ImGuiComboPreviewData_ImGuiComboPreviewData(void);
extern  void ImGuiComboPreviewData_destroy(ImGuiComboPreviewData* self);
extern  ImGuiMenuColumns* ImGuiMenuColumns_ImGuiMenuColumns(void);
extern  void ImGuiMenuColumns_destroy(ImGuiMenuColumns* self);
extern  void ImGuiMenuColumns_Update(ImGuiMenuColumns* self, float spacing, _Bool window_reappearing);
extern  float ImGuiMenuColumns_DeclColumns(ImGuiMenuColumns* self, float w_icon, float w_label, float w_shortcut, float w_mark);
extern  void ImGuiMenuColumns_CalcNextTotalWidth(ImGuiMenuColumns* self, _Bool update_offsets);
extern  ImGuiInputTextDeactivatedState* ImGuiInputTextDeactivatedState_ImGuiInputTextDeactivatedState(void);
extern  void ImGuiInputTextDeactivatedState_destroy(ImGuiInputTextDeactivatedState* self);
extern  void ImGuiInputTextDeactivatedState_ClearFreeMemory(ImGuiInputTextDeactivatedState* self);
extern  ImGuiInputTextState* ImGuiInputTextState_ImGuiInputTextState(void);
extern  void ImGuiInputTextState_destroy(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_ClearText(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_ClearFreeMemory(ImGuiInputTextState* self);
extern  int ImGuiInputTextState_GetUndoAvailCount(ImGuiInputTextState* self);
extern  int ImGuiInputTextState_GetRedoAvailCount(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_OnKeyPressed(ImGuiInputTextState* self, int key);
extern  void ImGuiInputTextState_CursorAnimReset(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_CursorClamp(ImGuiInputTextState* self);
extern  _Bool ImGuiInputTextState_HasSelection(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_ClearSelection(ImGuiInputTextState* self);
extern  int ImGuiInputTextState_GetCursorPos(ImGuiInputTextState* self);
extern  int ImGuiInputTextState_GetSelectionStart(ImGuiInputTextState* self);
extern  int ImGuiInputTextState_GetSelectionEnd(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_SelectAll(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_ReloadUserBufAndSelectAll(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_ReloadUserBufAndKeepSelection(ImGuiInputTextState* self);
extern  void ImGuiInputTextState_ReloadUserBufAndMoveToEnd(ImGuiInputTextState* self);
extern  ImGuiNextWindowData* ImGuiNextWindowData_ImGuiNextWindowData(void);
extern  void ImGuiNextWindowData_destroy(ImGuiNextWindowData* self);
extern  void ImGuiNextWindowData_ClearFlags(ImGuiNextWindowData* self);
extern  ImGuiNextItemData* ImGuiNextItemData_ImGuiNextItemData(void);
extern  void ImGuiNextItemData_destroy(ImGuiNextItemData* self);
extern  void ImGuiNextItemData_ClearFlags(ImGuiNextItemData* self);
extern  ImGuiLastItemData* ImGuiLastItemData_ImGuiLastItemData(void);
extern  void ImGuiLastItemData_destroy(ImGuiLastItemData* self);
extern  ImGuiStackSizes* ImGuiStackSizes_ImGuiStackSizes(void);
extern  void ImGuiStackSizes_destroy(ImGuiStackSizes* self);
extern  void ImGuiStackSizes_SetToContextState(ImGuiStackSizes* self, ImGuiContext* ctx);
extern  void ImGuiStackSizes_CompareWithContextState(ImGuiStackSizes* self, ImGuiContext* ctx);
extern  ImGuiPtrOrIndex* ImGuiPtrOrIndex_ImGuiPtrOrIndex_Ptr(void* ptr);
extern  void ImGuiPtrOrIndex_destroy(ImGuiPtrOrIndex* self);
extern  ImGuiPtrOrIndex* ImGuiPtrOrIndex_ImGuiPtrOrIndex_Int(int index);
extern  ImGuiPopupData* ImGuiPopupData_ImGuiPopupData(void);
extern  void ImGuiPopupData_destroy(ImGuiPopupData* self);
extern  ImGuiInputEvent* ImGuiInputEvent_ImGuiInputEvent(void);
extern  void ImGuiInputEvent_destroy(ImGuiInputEvent* self);
extern  ImGuiKeyRoutingData* ImGuiKeyRoutingData_ImGuiKeyRoutingData(void);
extern  void ImGuiKeyRoutingData_destroy(ImGuiKeyRoutingData* self);
extern  ImGuiKeyRoutingTable* ImGuiKeyRoutingTable_ImGuiKeyRoutingTable(void);
extern  void ImGuiKeyRoutingTable_destroy(ImGuiKeyRoutingTable* self);
extern  void ImGuiKeyRoutingTable_Clear(ImGuiKeyRoutingTable* self);
extern  ImGuiKeyOwnerData* ImGuiKeyOwnerData_ImGuiKeyOwnerData(void);
extern  void ImGuiKeyOwnerData_destroy(ImGuiKeyOwnerData* self);
extern  ImGuiListClipperRange ImGuiListClipperRange_FromIndices(int min, int max);
extern  ImGuiListClipperRange ImGuiListClipperRange_FromPositions(float y1, float y2, int off_min, int off_max);
extern  ImGuiListClipperData* ImGuiListClipperData_ImGuiListClipperData(void);
extern  void ImGuiListClipperData_destroy(ImGuiListClipperData* self);
extern  void ImGuiListClipperData_Reset(ImGuiListClipperData* self, ImGuiListClipper* clipper);
extern  ImGuiNavItemData* ImGuiNavItemData_ImGuiNavItemData(void);
extern  void ImGuiNavItemData_destroy(ImGuiNavItemData* self);
extern  void ImGuiNavItemData_Clear(ImGuiNavItemData* self);
extern  ImGuiTypingSelectState* ImGuiTypingSelectState_ImGuiTypingSelectState(void);
extern  void ImGuiTypingSelectState_destroy(ImGuiTypingSelectState* self);
extern  void ImGuiTypingSelectState_Clear(ImGuiTypingSelectState* self);
extern  ImGuiOldColumnData* ImGuiOldColumnData_ImGuiOldColumnData(void);
extern  void ImGuiOldColumnData_destroy(ImGuiOldColumnData* self);
extern  ImGuiOldColumns* ImGuiOldColumns_ImGuiOldColumns(void);
extern  void ImGuiOldColumns_destroy(ImGuiOldColumns* self);
extern  ImGuiBoxSelectState* ImGuiBoxSelectState_ImGuiBoxSelectState(void);
extern  void ImGuiBoxSelectState_destroy(ImGuiBoxSelectState* self);
extern  ImGuiMultiSelectTempData* ImGuiMultiSelectTempData_ImGuiMultiSelectTempData(void);
extern  void ImGuiMultiSelectTempData_destroy(ImGuiMultiSelectTempData* self);
extern  void ImGuiMultiSelectTempData_Clear(ImGuiMultiSelectTempData* self);
extern  void ImGuiMultiSelectTempData_ClearIO(ImGuiMultiSelectTempData* self);
extern  ImGuiMultiSelectState* ImGuiMultiSelectState_ImGuiMultiSelectState(void);
extern  void ImGuiMultiSelectState_destroy(ImGuiMultiSelectState* self);
extern  ImGuiDockNode* ImGuiDockNode_ImGuiDockNode(ImGuiID id);
extern  void ImGuiDockNode_destroy(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsRootNode(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsDockSpace(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsFloatingNode(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsCentralNode(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsHiddenTabBar(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsNoTabBar(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsSplitNode(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsLeafNode(ImGuiDockNode* self);
extern  _Bool ImGuiDockNode_IsEmpty(ImGuiDockNode* self);
extern  void ImGuiDockNode_Rect(ImRect* pOut, ImGuiDockNode* self);
extern  void ImGuiDockNode_SetLocalFlags(ImGuiDockNode* self, ImGuiDockNodeFlags flags);
extern  void ImGuiDockNode_UpdateMergedFlags(ImGuiDockNode* self);
extern  ImGuiDockContext* ImGuiDockContext_ImGuiDockContext(void);
extern  void ImGuiDockContext_destroy(ImGuiDockContext* self);
extern  ImGuiViewportP* ImGuiViewportP_ImGuiViewportP(void);
extern  void ImGuiViewportP_destroy(ImGuiViewportP* self);
extern  void ImGuiViewportP_ClearRequestFlags(ImGuiViewportP* self);
extern  void ImGuiViewportP_CalcWorkRectPos(ImVec2* pOut, ImGuiViewportP* self, const ImVec2 off_min);
extern  void ImGuiViewportP_CalcWorkRectSize(ImVec2* pOut, ImGuiViewportP* self, const ImVec2 off_min, const ImVec2 off_max);
extern  void ImGuiViewportP_UpdateWorkRect(ImGuiViewportP* self);
extern  void ImGuiViewportP_GetMainRect(ImRect* pOut, ImGuiViewportP* self);
extern  void ImGuiViewportP_GetWorkRect(ImRect* pOut, ImGuiViewportP* self);
extern  void ImGuiViewportP_GetBuildWorkRect(ImRect* pOut, ImGuiViewportP* self);
extern  ImGuiWindowSettings* ImGuiWindowSettings_ImGuiWindowSettings(void);
extern  void ImGuiWindowSettings_destroy(ImGuiWindowSettings* self);
extern  char* ImGuiWindowSettings_GetName(ImGuiWindowSettings* self);
extern  ImGuiSettingsHandler* ImGuiSettingsHandler_ImGuiSettingsHandler(void);
extern  void ImGuiSettingsHandler_destroy(ImGuiSettingsHandler* self);
extern  ImGuiDebugAllocInfo* ImGuiDebugAllocInfo_ImGuiDebugAllocInfo(void);
extern  void ImGuiDebugAllocInfo_destroy(ImGuiDebugAllocInfo* self);
extern  ImGuiStackLevelInfo* ImGuiStackLevelInfo_ImGuiStackLevelInfo(void);
extern  void ImGuiStackLevelInfo_destroy(ImGuiStackLevelInfo* self);
extern  ImGuiIDStackTool* ImGuiIDStackTool_ImGuiIDStackTool(void);
extern  void ImGuiIDStackTool_destroy(ImGuiIDStackTool* self);
extern  ImGuiContextHook* ImGuiContextHook_ImGuiContextHook(void);
extern  void ImGuiContextHook_destroy(ImGuiContextHook* self);
extern  ImGuiContext* ImGuiContext_ImGuiContext(ImFontAtlas* shared_font_atlas);
extern  void ImGuiContext_destroy(ImGuiContext* self);
extern  ImGuiWindow* ImGuiWindow_ImGuiWindow(ImGuiContext* context, const char* name);
extern  void ImGuiWindow_destroy(ImGuiWindow* self);
extern  ImGuiID ImGuiWindow_GetID_Str(ImGuiWindow* self, const char* str, const char* str_end);
extern  ImGuiID ImGuiWindow_GetID_Ptr(ImGuiWindow* self, const void* ptr);
extern  ImGuiID ImGuiWindow_GetID_Int(ImGuiWindow* self, int n);
extern  ImGuiID ImGuiWindow_GetIDFromRectangle(ImGuiWindow* self, const ImRect r_abs);
extern  void ImGuiWindow_Rect(ImRect* pOut, ImGuiWindow* self);
extern  float ImGuiWindow_CalcFontSize(ImGuiWindow* self);
extern  void ImGuiWindow_TitleBarRect(ImRect* pOut, ImGuiWindow* self);
extern  void ImGuiWindow_MenuBarRect(ImRect* pOut, ImGuiWindow* self);
extern  ImGuiTabItem* ImGuiTabItem_ImGuiTabItem(void);
extern  void ImGuiTabItem_destroy(ImGuiTabItem* self);
extern  ImGuiTabBar* ImGuiTabBar_ImGuiTabBar(void);
extern  void ImGuiTabBar_destroy(ImGuiTabBar* self);
extern  ImGuiTableColumn* ImGuiTableColumn_ImGuiTableColumn(void);
extern  void ImGuiTableColumn_destroy(ImGuiTableColumn* self);
extern  ImGuiTableInstanceData* ImGuiTableInstanceData_ImGuiTableInstanceData(void);
extern  void ImGuiTableInstanceData_destroy(ImGuiTableInstanceData* self);
extern  ImGuiTable* ImGuiTable_ImGuiTable(void);
extern  void ImGuiTable_destroy(ImGuiTable* self);
extern  ImGuiTableTempData* ImGuiTableTempData_ImGuiTableTempData(void);
extern  void ImGuiTableTempData_destroy(ImGuiTableTempData* self);
extern  ImGuiTableColumnSettings* ImGuiTableColumnSettings_ImGuiTableColumnSettings(void);
extern  void ImGuiTableColumnSettings_destroy(ImGuiTableColumnSettings* self);
extern  ImGuiTableSettings* ImGuiTableSettings_ImGuiTableSettings(void);
extern  void ImGuiTableSettings_destroy(ImGuiTableSettings* self);
extern  ImGuiTableColumnSettings* ImGuiTableSettings_GetColumnSettings(ImGuiTableSettings* self);
extern  ImGuiWindow* imguiGetCurrentWindowRead(void);
extern  ImGuiWindow* imguiGetCurrentWindow(void);
extern  ImGuiWindow* imguiFindWindowByID(ImGuiID id);
extern  ImGuiWindow* imguiFindWindowByName(const char* name);
extern  void imguiUpdateWindowParentAndRootLinks(ImGuiWindow* window, ImGuiWindowFlags flags, ImGuiWindow* parent_window);
extern  void imguiUpdateWindowSkipRefresh(ImGuiWindow* window);
extern  void imguiCalcWindowNextAutoFitSize(ImVec2* pOut, ImGuiWindow* window);
extern  _Bool imguiIsWindowChildOf(ImGuiWindow* window, ImGuiWindow* potential_parent, _Bool popup_hierarchy, _Bool dock_hierarchy);
extern  _Bool imguiIsWindowWithinBeginStackOf(ImGuiWindow* window, ImGuiWindow* potential_parent);
extern  _Bool imguiIsWindowAbove(ImGuiWindow* potential_above, ImGuiWindow* potential_below);
extern  _Bool imguiIsWindowNavFocusable(ImGuiWindow* window);
extern  void imguiSetWindowPos_WindowPtr(ImGuiWindow* window, const ImVec2 pos, ImGuiCond cond);
extern  void imguiSetWindowSize_WindowPtr(ImGuiWindow* window, const ImVec2 size, ImGuiCond cond);
extern  void imguiSetWindowCollapsed_WindowPtr(ImGuiWindow* window, _Bool collapsed, ImGuiCond cond);
extern  void imguiSetWindowHitTestHole(ImGuiWindow* window, const ImVec2 pos, const ImVec2 size);
extern  void imguiSetWindowHiddenAndSkipItemsForCurrentFrame(ImGuiWindow* window);
extern  void imguiSetWindowParentWindowForFocusRoute(ImGuiWindow* window, ImGuiWindow* parent_window);
extern  void imguiWindowRectAbsToRel(ImRect* pOut, ImGuiWindow* window, const ImRect r);
extern  void imguiWindowRectRelToAbs(ImRect* pOut, ImGuiWindow* window, const ImRect r);
extern  void imguiWindowPosRelToAbs(ImVec2* pOut, ImGuiWindow* window, const ImVec2 p);
extern  void imguiWindowPosAbsToRel(ImVec2* pOut, ImGuiWindow* window, const ImVec2 p);
extern  void imguiFocusWindow(ImGuiWindow* window, ImGuiFocusRequestFlags flags);
extern  void imguiFocusTopMostWindowUnderOne(ImGuiWindow* under_this_window, ImGuiWindow* imguinore_window, ImGuiViewport* filter_viewport, ImGuiFocusRequestFlags flags);
extern  void imguiBringWindowToFocusFront(ImGuiWindow* window);
extern  void imguiBringWindowToDisplayFront(ImGuiWindow* window);
extern  void imguiBringWindowToDisplayBack(ImGuiWindow* window);
extern  void imguiBringWindowToDisplayBehind(ImGuiWindow* window, ImGuiWindow* above_window);
extern  int imguiFindWindowDisplayIndex(ImGuiWindow* window);
extern  ImGuiWindow* imguiFindBottomMostVisibleWindowWithinBeginStack(ImGuiWindow* window);
extern  void imguiSetNextWindowRefreshPolicy(ImGuiWindowRefreshFlags flags);
extern  void imguiSetCurrentFont(ImFont* font);
extern  ImFont* imguiGetDefaultFont(void);
extern  ImDrawList* imguiGetForegroundDrawList_WindowPtr(ImGuiWindow* window);
extern  void imguiAddDrawListToDrawDataEx(ImDrawData* draw_data, ImVector_ImDrawListPtr* out_list, ImDrawList* draw_list);
extern  void imguiInitialize(void);
extern  void imguiShutdown(void);
extern  void imguiUpdateInputEvents(_Bool trickle_fast_inputs);
extern  void imguiUpdateHoveredWindowAndCaptureFlags(void);
extern  void imguiFindHoveredWindowEx(const ImVec2 pos, _Bool find_first_and_in_any_viewport, ImGuiWindow** out_hovered_window, ImGuiWindow** out_hovered_window_under_moving_window);
extern  void imguiStartMouseMovingWindow(ImGuiWindow* window);
extern  void imguiStartMouseMovingWindowOrNode(ImGuiWindow* window, ImGuiDockNode* node, _Bool undock);
extern  void imguiUpdateMouseMovingWindowNewFrame(void);
extern  void imguiUpdateMouseMovingWindowEndFrame(void);
extern  ImGuiID imguiAddContextHook(ImGuiContext* context, const ImGuiContextHook* hook);
extern  void imguiRemoveContextHook(ImGuiContext* context, ImGuiID hook_to_remove);
extern  void imguiCallContextHooks(ImGuiContext* context, ImGuiContextHookType type);
extern  void imguiTranslateWindowsInViewport(ImGuiViewportP* viewport, const ImVec2 old_pos, const ImVec2 new_pos);
extern  void imguiScaleWindowsInViewport(ImGuiViewportP* viewport, float scale);
extern  void imguiDestroyPlatformWindow(ImGuiViewportP* viewport);
extern  void imguiSetWindowViewport(ImGuiWindow* window, ImGuiViewportP* viewport);
extern  void imguiSetCurrentViewport(ImGuiWindow* window, ImGuiViewportP* viewport);
extern  const ImGuiPlatformMonitor* imguiGetViewportPlatformMonitor(ImGuiViewport* viewport);
extern  ImGuiViewportP* imguiFindHoveredViewportFromPlatformWindowStack(const ImVec2 mouse_platform_pos);
extern  void imguiMarkIniSettingsDirty_Nil(void);
extern  void imguiMarkIniSettingsDirty_WindowPtr(ImGuiWindow* window);
extern  void imguiClearIniSettings(void);
extern  void imguiAddSettingsHandler(const ImGuiSettingsHandler* handler);
extern  void imguiRemoveSettingsHandler(const char* type_name);
extern  ImGuiSettingsHandler* imguiFindSettingsHandler(const char* type_name);
extern  ImGuiWindowSettings* imguiCreateNewWindowSettings(const char* name);
extern  ImGuiWindowSettings* imguiFindWindowSettingsByID(ImGuiID id);
extern  ImGuiWindowSettings* imguiFindWindowSettingsByWindow(ImGuiWindow* window);
extern  void imguiClearWindowSettings(const char* name);
extern  void imguiLocalizeRegisterEntries(const ImGuiLocEntry* entries, int count);
extern  const char* imguiLocalizeGetMsg(ImGuiLocKey key);
extern  void imguiSetScrollX_WindowPtr(ImGuiWindow* window, float scroll_x);
extern  void imguiSetScrollY_WindowPtr(ImGuiWindow* window, float scroll_y);
extern  void imguiSetScrollFromPosX_WindowPtr(ImGuiWindow* window, float local_x, float center_x_ratio);
extern  void imguiSetScrollFromPosY_WindowPtr(ImGuiWindow* window, float local_y, float center_y_ratio);
extern  void imguiScrollToItem(ImGuiScrollFlags flags);
extern  void imguiScrollToRect(ImGuiWindow* window, const ImRect rect, ImGuiScrollFlags flags);
extern  void imguiScrollToRectEx(ImVec2* pOut, ImGuiWindow* window, const ImRect rect, ImGuiScrollFlags flags);
extern  void imguiScrollToBringRectIntoView(ImGuiWindow* window, const ImRect rect);
extern  ImGuiItemStatusFlags imguiGetItemStatusFlags(void);
extern  ImGuiItemFlags imguiGetItemFlags(void);
extern  ImGuiID imguiGetActiveID(void);
extern  ImGuiID imguiGetFocusID(void);
extern  void imguiSetActiveID(ImGuiID id, ImGuiWindow* window);
extern  void imguiSetFocusID(ImGuiID id, ImGuiWindow* window);
extern  void imguiClearActiveID(void);
extern  ImGuiID imguiGetHoveredID(void);
extern  void imguiSetHoveredID(ImGuiID id);
extern  void imguiKeepAliveID(ImGuiID id);
extern  void imguiMarkItemEdited(ImGuiID id);
extern  void imguiPushOverrideID(ImGuiID id);
extern  ImGuiID imguiGetIDWithSeed_Str(const char* str_id_begin, const char* str_id_end, ImGuiID seed);
extern  ImGuiID imguiGetIDWithSeed_Int(int n, ImGuiID seed);
extern  void imguiItemSize_Vec2(const ImVec2 size, float text_baseline_y);
extern  void imguiItemSize_Rect(const ImRect bb, float text_baseline_y);
extern  _Bool imguiItemAdd(const ImRect bb, ImGuiID id, const ImRect* nav_bb, ImGuiItemFlags extra_flags);
extern  _Bool imguiItemHoverable(const ImRect bb, ImGuiID id, ImGuiItemFlags item_flags);
extern  _Bool imguiIsWindowContentHoverable(ImGuiWindow* window, ImGuiHoveredFlags flags);
extern  _Bool imguiIsClippedEx(const ImRect bb, ImGuiID id);
extern  void imguiSetLastItemData(ImGuiID item_id, ImGuiItemFlags in_flags, ImGuiItemStatusFlags status_flags, const ImRect item_rect);
extern  void imguiCalcItemSize(ImVec2* pOut, ImVec2 size, float default_w, float default_h);
extern  float imguiCalcWrapWidthForPos(const ImVec2 pos, float wrap_pos_x);
extern  void imguiPushMultiItemsWidths(int components, float width_full);
extern  void imguiShrinkWidths(ImGuiShrinkWidthItem* items, int count, float width_excess);
extern  const ImGuiDataVarInfo* imguiGetStyleVarInfo(ImGuiStyleVar idx);
extern  void imguiBeginDisabledOverrideReenable(void);
extern  void imguiEndDisabledOverrideReenable(void);
extern  void imguiLogBegin(ImGuiLogType type, int auto_open_depth);
extern  void imguiLogToBuffer(int auto_open_depth);
extern  void imguiLogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end);
extern  void imguiLogSetNextTextDecoration(const char* prefix, const char* suffix);
extern  _Bool imguiBeginChildEx(const char* name, ImGuiID id, const ImVec2 size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
extern  _Bool imguiBeginPopupEx(ImGuiID id, ImGuiWindowFlags extra_window_flags);
extern  void imguiOpenPopupEx(ImGuiID id, ImGuiPopupFlags popup_flags);
extern  void imguiClosePopupToLevel(int remaining, _Bool restore_focus_to_window_under_popup);
extern  void imguiClosePopupsOverWindow(ImGuiWindow* ref_window, _Bool restore_focus_to_window_under_popup);
extern  void imguiClosePopupsExceptModals(void);
extern  _Bool imguiIsPopupOpen_ID(ImGuiID id, ImGuiPopupFlags popup_flags);
extern  void imguiGetPopupAllowedExtentRect(ImRect* pOut, ImGuiWindow* window);
extern  ImGuiWindow* imguiGetTopMostPopupModal(void);
extern  ImGuiWindow* imguiGetTopMostAndVisiblePopupModal(void);
extern  ImGuiWindow* imguiFindBlockingModal(ImGuiWindow* window);
extern  void imguiFindBestWindowPosForPopup(ImVec2* pOut, ImGuiWindow* window);
extern  void imguiFindBestWindowPosForPopupEx(ImVec2* pOut, const ImVec2 ref_pos, const ImVec2 size, ImGuiDir* last_dir, const ImRect r_outer, const ImRect r_avoid,
                                                 ImGuiPopupPositionPolicy policy);
extern  _Bool imguiBeginTooltipEx(ImGuiTooltipFlags tooltip_flags, ImGuiWindowFlags extra_window_flags);
extern  _Bool imguiBeginTooltipHidden(void);
extern  _Bool imguiBeginViewportSideBar(const char* name, ImGuiViewport* viewport, ImGuiDir dir, float size, ImGuiWindowFlags window_flags);
extern  _Bool imguiBeginMenuEx(const char* label, const char* icon, _Bool enabled);
extern  _Bool imguiMenuItemEx(const char* label, const char* icon, const char* shortcut, _Bool selected, _Bool enabled);
extern  _Bool imguiBeginComboPopup(ImGuiID popup_id, const ImRect bb, ImGuiComboFlags flags);
extern  _Bool imguiBeginComboPreview(void);
extern  void imguiEndComboPreview(void);
extern  void imguiNavInitWindow(ImGuiWindow* window, _Bool force_reinit);
extern  void imguiNavInitRequestApplyResult(void);
extern  _Bool imguiNavMoveRequestButNoResultYet(void);
extern  void imguiNavMoveRequestSubmit(ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags);
extern  void imguiNavMoveRequestForward(ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags);
extern  void imguiNavMoveRequestResolveWithLastItem(ImGuiNavItemData* result);
extern  void imguiNavMoveRequestResolveWithPastTreeNode(ImGuiNavItemData* result, ImGuiTreeNodeStackData* tree_node_data);
extern  void imguiNavMoveRequestCancel(void);
extern  void imguiNavMoveRequestApplyResult(void);
extern  void imguiNavMoveRequestTryWrapping(ImGuiWindow* window, ImGuiNavMoveFlags move_flags);
extern  void imguiNavHighlightActivated(ImGuiID id);
extern  void imguiNavClearPreferredPosForAxis(ImGuiAxis axis);
extern  void imguiNavRestoreHighlightAfterMove(void);
extern  void imguiNavUpdateCurrentWindowIsScrollPushableX(void);
extern  void imguiSetNavWindow(ImGuiWindow* window);
extern  void imguiSetNavID(ImGuiID id, ImGuiNavLayer nav_layer, ImGuiID focus_scope_id, const ImRect rect_rel);
extern  void imguiSetNavFocusScope(ImGuiID focus_scope_id);
extern  void imguiFocusItem(void);
extern  void imguiActivateItemByID(ImGuiID id);
extern  _Bool imguiIsNamedKey(ImGuiKey key);
extern  _Bool imguiIsNamedKeyOrMod(ImGuiKey key);
extern  _Bool imguiIsLegacyKey(ImGuiKey key);
extern  _Bool imguiIsKeyboardKey(ImGuiKey key);
extern  _Bool imguiIsGamepadKey(ImGuiKey key);
extern  _Bool imguiIsMouseKey(ImGuiKey key);
extern  _Bool imguiIsAliasKey(ImGuiKey key);
extern  _Bool imguiIsModKey(ImGuiKey key);
extern  ImGuiKeyChord imguiFixupKeyChord(ImGuiKeyChord key_chord);
extern  ImGuiKey imguiConvertSingleModFlagToKey(ImGuiKey key);
extern  ImGuiKeyData* imguiGetKeyData_ContextPtr(ImGuiContext* ctx, ImGuiKey key);
extern  ImGuiKeyData* imguiGetKeyData_Key(ImGuiKey key);
extern  const char* imguiGetKeyChordName(ImGuiKeyChord key_chord);
extern  ImGuiKey imguiMouseButtonToKey(ImGuiMouseButton button);
extern  _Bool imguiIsMouseDragPastThreshold(ImGuiMouseButton button, float lock_threshold);
extern  void imguiGetKeyMagnitude2d(ImVec2* pOut, ImGuiKey key_left, ImGuiKey key_right, ImGuiKey key_up, ImGuiKey key_down);
extern  float imguiGetNavTweakPressedAmount(ImGuiAxis axis);
extern  int imguiCalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate);
extern  void imguiGetTypematicRepeatRate(ImGuiInputFlags flags, float* repeat_delay, float* repeat_rate);
extern  void imguiTeleportMousePos(const ImVec2 pos);
extern  void imguiSetActiveIdUsingAllKeyboardKeys(void);
extern  _Bool imguiIsActiveIdUsingNavDir(ImGuiDir dir);
extern  ImGuiID imguiGetKeyOwner(ImGuiKey key);
extern  void imguiSetKeyOwner(ImGuiKey key, ImGuiID owner_id, ImGuiInputFlags flags);
extern  void imguiSetKeyOwnersForKeyChord(ImGuiKeyChord key, ImGuiID owner_id, ImGuiInputFlags flags);
extern  void imguiSetItemKeyOwner_InputFlags(ImGuiKey key, ImGuiInputFlags flags);
extern  _Bool imguiTestKeyOwner(ImGuiKey key, ImGuiID owner_id);
extern  ImGuiKeyOwnerData* imguiGetKeyOwnerData(ImGuiContext* ctx, ImGuiKey key);
extern  _Bool imguiIsKeyDown_ID(ImGuiKey key, ImGuiID owner_id);
extern  _Bool imguiIsKeyPressed_InputFlags(ImGuiKey key, ImGuiInputFlags flags, ImGuiID owner_id);
extern  _Bool imguiIsKeyReleased_ID(ImGuiKey key, ImGuiID owner_id);
extern  _Bool imguiIsKeyChordPressed_InputFlags(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
extern  _Bool imguiIsMouseDown_ID(ImGuiMouseButton button, ImGuiID owner_id);
extern  _Bool imguiIsMouseClicked_InputFlags(ImGuiMouseButton button, ImGuiInputFlags flags, ImGuiID owner_id);
extern  _Bool imguiIsMouseReleased_ID(ImGuiMouseButton button, ImGuiID owner_id);
extern  _Bool imguiIsMouseDoubleClicked_ID(ImGuiMouseButton button, ImGuiID owner_id);
extern  _Bool imguiShortcut_ID(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
extern  _Bool imguiSetShortcutRouting(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
extern  _Bool imguiTestShortcutRouting(ImGuiKeyChord key_chord, ImGuiID owner_id);
extern  ImGuiKeyRoutingData* imguiGetShortcutRoutingData(ImGuiKeyChord key_chord);
extern  void imguiDockContextInitialize(ImGuiContext* ctx);
extern  void imguiDockContextShutdown(ImGuiContext* ctx);
extern  void imguiDockContextClearNodes(ImGuiContext* ctx, ImGuiID root_id, _Bool clear_settings_refs);
extern  void imguiDockContextRebuildNodes(ImGuiContext* ctx);
extern  void imguiDockContextNewFrameUpdateUndocking(ImGuiContext* ctx);
extern  void imguiDockContextNewFrameUpdateDocking(ImGuiContext* ctx);
extern  void imguiDockContextEndFrame(ImGuiContext* ctx);
extern  ImGuiID imguiDockContextGenNodeID(ImGuiContext* ctx);
extern  void imguiDockContextQueueDock(ImGuiContext* ctx, ImGuiWindow* target, ImGuiDockNode* target_node, ImGuiWindow* payload, ImGuiDir split_dir, float split_ratio, _Bool split_outer);
extern  void imguiDockContextQueueUndockWindow(ImGuiContext* ctx, ImGuiWindow* window);
extern  void imguiDockContextQueueUndockNode(ImGuiContext* ctx, ImGuiDockNode* node);
extern  void imguiDockContextProcessUndockWindow(ImGuiContext* ctx, ImGuiWindow* window, _Bool clear_persistent_docking_ref);
extern  void imguiDockContextProcessUndockNode(ImGuiContext* ctx, ImGuiDockNode* node);
extern  _Bool imguiDockContextCalcDropPosForDocking(ImGuiWindow* target, ImGuiDockNode* target_node, ImGuiWindow* payload_window, ImGuiDockNode* payload_node, ImGuiDir split_dir, _Bool split_outer,
                                                      ImVec2* out_pos);
extern  ImGuiDockNode* imguiDockContextFindNodeByID(ImGuiContext* ctx, ImGuiID id);
extern  void imguiDockNodeWindowMenuHandler_Default(ImGuiContext* ctx, ImGuiDockNode* node, ImGuiTabBar* tab_bar);
extern  _Bool imguiDockNodeBeginAmendTabBar(ImGuiDockNode* node);
extern  void imguiDockNodeEndAmendTabBar(void);
extern  ImGuiDockNode* imguiDockNodeGetRootNode(ImGuiDockNode* node);
extern  _Bool imguiDockNodeIsInHierarchyOf(ImGuiDockNode* node, ImGuiDockNode* parent);
extern  int imguiDockNodeGetDepth(const ImGuiDockNode* node);
extern  ImGuiID imguiDockNodeGetWindowMenuButtonId(const ImGuiDockNode* node);
extern  ImGuiDockNode* imguiGetWindowDockNode(void);
extern  _Bool imguiGetWindowAlwaysWantOwnTabBar(ImGuiWindow* window);
extern  void imguiBeginDocked(ImGuiWindow* window, _Bool* p_open);
extern  void imguiBeginDockableDragDropSource(ImGuiWindow* window);
extern  void imguiBeginDockableDragDropTarget(ImGuiWindow* window);
extern  void imguiSetWindowDock(ImGuiWindow* window, ImGuiID dock_id, ImGuiCond cond);
extern  void imguiDockBuilderDockWindow(const char* window_name, ImGuiID node_id);
extern  ImGuiDockNode* imguiDockBuilderGetNode(ImGuiID node_id);
extern  ImGuiDockNode* imguiDockBuilderGetCentralNode(ImGuiID node_id);
extern  ImGuiID imguiDockBuilderAddNode(ImGuiID node_id, ImGuiDockNodeFlags flags);
extern  void imguiDockBuilderRemoveNode(ImGuiID node_id);
extern  void imguiDockBuilderRemoveNodeDockedWindows(ImGuiID node_id, _Bool clear_settings_refs);
extern  void imguiDockBuilderRemoveNodeChildNodes(ImGuiID node_id);
extern  void imguiDockBuilderSetNodePos(ImGuiID node_id, ImVec2 pos);
extern  void imguiDockBuilderSetNodeSize(ImGuiID node_id, ImVec2 size);
extern  ImGuiID imguiDockBuilderSplitNode(ImGuiID node_id, ImGuiDir split_dir, float size_ratio_for_node_at_dir, ImGuiID* out_id_at_dir, ImGuiID* out_id_at_opposite_dir);
extern  void imguiDockBuilderCopyDockSpace(ImGuiID src_dockspace_id, ImGuiID dst_dockspace_id, ImVector_const_charPtr* in_window_remap_pairs);
extern  void imguiDockBuilderCopyNode(ImGuiID src_node_id, ImGuiID dst_node_id, ImVector_ImGuiID* out_node_remap_pairs);
extern  void imguiDockBuilderCopyWindowSettings(const char* src_name, const char* dst_name);
extern  void imguiDockBuilderFinish(ImGuiID node_id);
extern  void imguiPushFocusScope(ImGuiID id);
extern  void imguiPopFocusScope(void);
extern  ImGuiID imguiGetCurrentFocusScope(void);
extern  _Bool imguiIsDragDropActive(void);
extern  _Bool imguiBeginDragDropTargetCustom(const ImRect bb, ImGuiID id);
extern  void imguiClearDragDrop(void);
extern  _Bool imguiIsDragDropPayloadBeingAccepted(void);
extern  void imguiRenderDragDropTargetRect(const ImRect bb, const ImRect item_clip_rect);
extern  ImGuiTypingSelectRequest* imguiGetTypingSelectRequest(ImGuiTypingSelectFlags flags);
extern  int imguiTypingSelectFindMatch(ImGuiTypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
extern  int imguiTypingSelectFindNextSingleCharMatch(ImGuiTypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
extern  int imguiTypingSelectFindBestLeadingMatch(ImGuiTypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data);
extern  _Bool imguiBeginBoxSelect(const ImRect scope_rect, ImGuiWindow* window, ImGuiID box_select_id, ImGuiMultiSelectFlags ms_flags);
extern  void imguiEndBoxSelect(const ImRect scope_rect, ImGuiMultiSelectFlags ms_flags);
extern  void imguiMultiSelectItemHeader(ImGuiID id, _Bool* p_selected, ImGuiButtonFlags* p_button_flags);
extern  void imguiMultiSelectItemFooter(ImGuiID id, _Bool* p_selected, _Bool* p_pressed);
extern  void imguiMultiSelectAddSetAll(ImGuiMultiSelectTempData* ms, _Bool selected);
extern  void imguiMultiSelectAddSetRange(ImGuiMultiSelectTempData* ms, _Bool selected, int range_dir, ImGuiSelectionUserData first_item, ImGuiSelectionUserData last_item);
extern  ImGuiBoxSelectState* imguiGetBoxSelectState(ImGuiID id);
extern  ImGuiMultiSelectState* imguiGetMultiSelectState(ImGuiID id);
extern  void imguiSetWindowClipRectBeforeSetChannel(ImGuiWindow* window, const ImRect clip_rect);
extern  void imguiBeginColumns(const char* str_id, int count, ImGuiOldColumnFlags flags);
extern  void imguiEndColumns(void);
extern  void imguiPushColumnClipRect(int column_index);
extern  void imguiPushColumnsBackground(void);
extern  void imguiPopColumnsBackground(void);
extern  ImGuiID imguiGetColumnsID(const char* str_id, int count);
extern  ImGuiOldColumns* imguiFindOrCreateColumns(ImGuiWindow* window, ImGuiID id);
extern  float imguiGetColumnOffsetFromNorm(const ImGuiOldColumns* columns, float offset_norm);
extern  float imguiGetColumnNormFromOffset(const ImGuiOldColumns* columns, float offset);
extern  void imguiTableOpenContextMenu(int column_n);
extern  void imguiTableSetColumnWidth(int column_n, float width);
extern  void imguiTableSetColumnSortDirection(int column_n, ImGuiSortDirection sort_direction, _Bool append_to_sort_specs);
extern  int imguiTableGetHoveredRow(void);
extern  float imguiTableGetHeaderRowHeight(void);
extern  float imguiTableGetHeaderAngledMaxLabelWidth(void);
extern  void imguiTablePushBackgroundChannel(void);
extern  void imguiTablePopBackgroundChannel(void);
extern  void imguiTableAngledHeadersRowEx(ImGuiID row_id, float angle, float max_label_width, const ImGuiTableHeaderData* data, int data_count);
extern  ImGuiTable* imguiGetCurrentTable(void);
extern  ImGuiTable* imguiTableFindByID(ImGuiID id);
extern  _Bool imguiBeginTableEx(const char* name, ImGuiID id, int columns_count, ImGuiTableFlags flags, const ImVec2 outer_size, float inner_width);
extern  void imguiTableBeginInitMemory(ImGuiTable* table, int columns_count);
extern  void imguiTableBeginApplyRequests(ImGuiTable* table);
extern  void imguiTableSetupDrawChannels(ImGuiTable* table);
extern  void imguiTableUpdateLayout(ImGuiTable* table);
extern  void imguiTableUpdateBorders(ImGuiTable* table);
extern  void imguiTableUpdateColumnsWeightFromWidth(ImGuiTable* table);
extern  void imguiTableDrawBorders(ImGuiTable* table);
extern  void imguiTableDrawDefaultContextMenu(ImGuiTable* table, ImGuiTableFlags flags_for_section_to_display);
extern  _Bool imguiTableBeginContextMenuPopup(ImGuiTable* table);
extern  void imguiTableMergeDrawChannels(ImGuiTable* table);
extern  ImGuiTableInstanceData* imguiTableGetInstanceData(ImGuiTable* table, int instance_no);
extern  ImGuiID imguiTableGetInstanceID(ImGuiTable* table, int instance_no);
extern  void imguiTableSortSpecsSanitize(ImGuiTable* table);
extern  void imguiTableSortSpecsBuild(ImGuiTable* table);
extern  ImGuiSortDirection imguiTableGetColumnNextSortDirection(ImGuiTableColumn* column);
extern  void imguiTableFixColumnSortDirection(ImGuiTable* table, ImGuiTableColumn* column);
extern  float imguiTableGetColumnWidthAuto(ImGuiTable* table, ImGuiTableColumn* column);
extern  void imguiTableBeginRow(ImGuiTable* table);
extern  void imguiTableEndRow(ImGuiTable* table);
extern  void imguiTableBeginCell(ImGuiTable* table, int column_n);
extern  void imguiTableEndCell(ImGuiTable* table);
extern  void imguiTableGetCellBgRect(ImRect* pOut, const ImGuiTable* table, int column_n);
extern  const char* imguiTableGetColumnName_TablePtr(const ImGuiTable* table, int column_n);
extern  ImGuiID imguiTableGetColumnResizeID(ImGuiTable* table, int column_n, int instance_no);
extern  float imguiTableGetMaxColumnWidth(const ImGuiTable* table, int column_n);
extern  void imguiTableSetColumnWidthAutoSingle(ImGuiTable* table, int column_n);
extern  void imguiTableSetColumnWidthAutoAll(ImGuiTable* table);
extern  void imguiTableRemove(ImGuiTable* table);
extern  void imguiTableGcCompactTransientBuffers_TablePtr(ImGuiTable* table);
extern  void imguiTableGcCompactTransientBuffers_TableTempDataPtr(ImGuiTableTempData* table);
extern  void imguiTableGcCompactSettings(void);
extern  void imguiTableLoadSettings(ImGuiTable* table);
extern  void imguiTableSaveSettings(ImGuiTable* table);
extern  void imguiTableResetSettings(ImGuiTable* table);
extern  ImGuiTableSettings* imguiTableGetBoundSettings(ImGuiTable* table);
extern  void imguiTableSettingsAddSettingsHandler(void);
extern  ImGuiTableSettings* imguiTableSettingsCreate(ImGuiID id, int columns_count);
extern  ImGuiTableSettings* imguiTableSettingsFindByID(ImGuiID id);
extern  ImGuiTabBar* imguiGetCurrentTabBar(void);
extern  _Bool imguiBeginTabBarEx(ImGuiTabBar* tab_bar, const ImRect bb, ImGuiTabBarFlags flags);
extern  ImGuiTabItem* imguiTabBarFindTabByID(ImGuiTabBar* tab_bar, ImGuiID tab_id);
extern  ImGuiTabItem* imguiTabBarFindTabByOrder(ImGuiTabBar* tab_bar, int order);
extern  ImGuiTabItem* imguiTabBarFindMostRecentlySelectedTabForActiveWindow(ImGuiTabBar* tab_bar);
extern  ImGuiTabItem* imguiTabBarGetCurrentTab(ImGuiTabBar* tab_bar);
extern  int imguiTabBarGetTabOrder(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
extern  const char* imguiTabBarGetTabName(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
extern  void imguiTabBarAddTab(ImGuiTabBar* tab_bar, ImGuiTabItemFlags tab_flags, ImGuiWindow* window);
extern  void imguiTabBarRemoveTab(ImGuiTabBar* tab_bar, ImGuiID tab_id);
extern  void imguiTabBarCloseTab(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
extern  void imguiTabBarQueueFocus(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
extern  void imguiTabBarQueueReorder(ImGuiTabBar* tab_bar, ImGuiTabItem* tab, int offset);
extern  void imguiTabBarQueueReorderFromMousePos(ImGuiTabBar* tab_bar, ImGuiTabItem* tab, ImVec2 mouse_pos);
extern  _Bool imguiTabBarProcessReorder(ImGuiTabBar* tab_bar);
extern  _Bool imguiTabItemEx(ImGuiTabBar* tab_bar, const char* label, _Bool* p_open, ImGuiTabItemFlags flags, ImGuiWindow* docked_window);
extern  void imguiTabItemCalcSize_Str(ImVec2* pOut, const char* label, _Bool has_close_button_or_unsaved_marker);
extern  void imguiTabItemCalcSize_WindowPtr(ImVec2* pOut, ImGuiWindow* window);
extern  void imguiTabItemBackground(ImDrawList* draw_list, const ImRect bb, ImGuiTabItemFlags flags, ImU32 col);
extern  void imguiTabItemLabelAndCloseButton(ImDrawList* draw_list, const ImRect bb, ImGuiTabItemFlags flags, ImVec2 frame_padding, const char* label, ImGuiID tab_id, ImGuiID close_button_id,
                                                _Bool is_contents_visible, _Bool* out_just_closed, _Bool* out_text_clipped);
extern  void imguiRenderText(ImVec2 pos, const char* text, const char* text_end, _Bool hide_text_after_hash);
extern  void imguiRenderTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width);
extern  void imguiRenderTextClipped(const ImVec2 pos_min, const ImVec2 pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2 align,
                                       const ImRect* clip_rect);
extern  void imguiRenderTextClippedEx(ImDrawList* draw_list, const ImVec2 pos_min, const ImVec2 pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known,
                                         const ImVec2 align, const ImRect* clip_rect);
extern  void imguiRenderTextEllipsis(ImDrawList* draw_list, const ImVec2 pos_min, const ImVec2 pos_max, float clip_max_x, float ellipsis_max_x, const char* text, const char* text_end,
                                        const ImVec2* text_size_if_known);
extern  void imguiRenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, _Bool border, float rounding);
extern  void imguiRenderFrameBorder(ImVec2 p_min, ImVec2 p_max, float rounding);
extern  void imguiRenderColorRectWithAlphaCheckerboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, float grid_step, ImVec2 grid_off, float rounding, ImDrawFlags flags);
extern  void imguiRenderNavHighlight(const ImRect bb, ImGuiID id, ImGuiNavHighlightFlags flags);
extern  const char* imguiFindRenderedTextEnd(const char* text, const char* text_end);
extern  void imguiRenderMouseCursor(ImVec2 pos, float scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow);
extern  void imguiRenderArrow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, ImGuiDir dir, float scale);
extern  void imguiRenderBullet(ImDrawList* draw_list, ImVec2 pos, ImU32 col);
extern  void imguiRenderCheckMark(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz);
extern  void imguiRenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col);
extern  void imguiRenderArrowDockMenu(ImDrawList* draw_list, ImVec2 p_min, float sz, ImU32 col);
extern  void imguiRenderRectFilledRangeH(ImDrawList* draw_list, const ImRect rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding);
extern  void imguiRenderRectFilledWithHole(ImDrawList* draw_list, const ImRect outer, const ImRect inner, ImU32 col, float rounding);
extern  ImDrawFlags imguiCalcRoundingFlagsForRectInRect(const ImRect r_in, const ImRect r_outer, float threshold);
extern  void imguiTextEx(const char* text, const char* text_end, ImGuiTextFlags flags);
extern  _Bool imguiButtonEx(const char* label, const ImVec2 size_arg, ImGuiButtonFlags flags);
extern  _Bool imguiArrowButtonEx(const char* str_id, ImGuiDir dir, ImVec2 size_arg, ImGuiButtonFlags flags);
extern  _Bool imguiImageButtonEx(ImGuiID id, ImTextureID texture_id, const ImVec2 image_size, const ImVec2 uv0, const ImVec2 uv1, const ImVec4 bg_col, const ImVec4 tint_col, ImGuiButtonFlags flags);
extern  void imguiSeparatorEx(ImGuiSeparatorFlags flags, float thickness);
extern  void imguiSeparatorTextEx(ImGuiID id, const char* label, const char* label_end, float extra_width);
extern  _Bool imguiCheckboxFlags_S64Ptr(const char* label, ImS64* flags, ImS64 flags_value);
extern  _Bool imguiCheckboxFlags_U64Ptr(const char* label, ImU64* flags, ImU64 flags_value);
extern  _Bool imguiCloseButton(ImGuiID id, const ImVec2 pos);
extern  _Bool imguiCollapseButton(ImGuiID id, const ImVec2 pos, ImGuiDockNode* dock_node);
extern  void imguiScrollbar(ImGuiAxis axis);
extern  _Bool imguiScrollbarEx(const ImRect bb, ImGuiID id, ImGuiAxis axis, ImS64* p_scroll_v, ImS64 avail_v, ImS64 contents_v, ImDrawFlags flags);
extern  void imguiGetWindowScrollbarRect(ImRect* pOut, ImGuiWindow* window, ImGuiAxis axis);
extern  ImGuiID imguiGetWindowScrollbarID(ImGuiWindow* window, ImGuiAxis axis);
extern  ImGuiID imguiGetWindowResizeCornerID(ImGuiWindow* window, int n);
extern  ImGuiID imguiGetWindowResizeBorderID(ImGuiWindow* window, ImGuiDir dir);
extern  _Bool imguiButtonBehavior(const ImRect bb, ImGuiID id, _Bool* out_hovered, _Bool* out_held, ImGuiButtonFlags flags);
extern  _Bool imguiDragBehavior(ImGuiID id, ImGuiDataType data_type, void* p_v, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
extern  _Bool imguiSliderBehavior(const ImRect bb, ImGuiID id, ImGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags,
                                    ImRect* out_grab_bb);
extern  _Bool imguiSplitterBehavior(const ImRect bb, ImGuiID id, ImGuiAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend, float hover_visibility_delay,
                                      ImU32 bg_col);
extern  _Bool imguiTreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end);
extern  void imguiTreePushOverrideID(ImGuiID id);
extern  _Bool imguiTreeNodeGetOpen(ImGuiID storage_id);
extern  void imguiTreeNodeSetOpen(ImGuiID storage_id, _Bool open);
extern  _Bool imguiTreeNodeUpdateNextOpen(ImGuiID storage_id, ImGuiTreeNodeFlags flags);
extern  const ImGuiDataTypeInfo* imguiDataTypeGetInfo(ImGuiDataType data_type);
extern  int imguiDataTypeFormatString(char* buf, int buf_size, ImGuiDataType data_type, const void* p_data, const char* format);
extern  void imguiDataTypeApplyOp(ImGuiDataType data_type, int op, void* output, const void* arg_1, const void* arg_2);
extern  _Bool imguiDataTypeApplyFromText(const char* buf, ImGuiDataType data_type, void* p_data, const char* format, void* p_data_when_empty);
extern  int imguiDataTypeCompare(ImGuiDataType data_type, const void* arg_1, const void* arg_2);
extern  _Bool imguiDataTypeClamp(ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max);
extern  _Bool imguiInputTextEx(const char* label, const char* hint, char* buf, int buf_size, const ImVec2 size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
extern  void imguiInputTextDeactivateHook(ImGuiID id);
extern  _Bool imguiTempInputText(const ImRect bb, ImGuiID id, const char* label, char* buf, int buf_size, ImGuiInputTextFlags flags);
extern  _Bool imguiTempInputScalar(const ImRect bb, ImGuiID id, const char* label, ImGuiDataType data_type, void* p_data, const char* format, const void* p_clamp_min, const void* p_clamp_max);
extern  _Bool imguiTempInputIsActive(ImGuiID id);
extern  ImGuiInputTextState* imguiGetInputTextState(ImGuiID id);
extern  void imguiSetNextItemRefVal(ImGuiDataType data_type, void* p_data);
extern  void imguiColorTooltip(const char* text, const float* col, ImGuiColorEditFlags flags);
extern  void imguiColorEditOptionsPopup(const float* col, ImGuiColorEditFlags flags);
extern  void imguiColorPickerOptionsPopup(const float* ref_col, ImGuiColorEditFlags flags);
extern  int imguiPlotEx(ImGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text,
                           float scale_min, float scale_max, const ImVec2 size_arg);
extern  void imguiShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1);
extern  void imguiShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2 a, const ImVec2 b, const ImVec2 uv_a, const ImVec2 uv_b, _Bool clamp);
extern  void imguiShadeVertsTransformPos(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2 pivot_in, float cos_a, float sin_a, const ImVec2 pivot_out);
extern  void imguiGcCompactTransientMiscBuffers(void);
extern  void imguiGcCompactTransientWindowBuffers(ImGuiWindow* window);
extern  void imguiGcAwakeTransientWindowBuffers(ImGuiWindow* window);
extern  void imguiDebugAllocHook(ImGuiDebugAllocInfo* info, int frame_count, void* ptr, size_t size);
extern  void imguiErrorCheckEndFrameRecover(ImGuiErrorLogCallback log_callback, void* user_data);
extern  void imguiErrorCheckEndWindowRecover(ImGuiErrorLogCallback log_callback, void* user_data);
extern  void imguiErrorCheckUsingSetCursorPosToExtendParentBoundaries(void);
extern  void imguiDebugDrawCursorPos(ImU32 col);
extern  void imguiDebugDrawLineExtents(ImU32 col);
extern  void imguiDebugDrawItemRect(ImU32 col);
extern  void imguiDebugTextUnformattedWithLocateItem(const char* line_begin, const char* line_end);
extern  void imguiDebugLocateItem(ImGuiID target_id);
extern  void imguiDebugLocateItemOnHover(ImGuiID target_id);
extern  void imguiDebugLocateItemResolveWithLastItem(void);
extern  void imguiDebugBreakClearData(void);
extern  _Bool imguiDebugBreakButton(const char* label, const char* description_of_location);
extern  void imguiDebugBreakButtonTooltip(_Bool keyboard_only, const char* description_of_location);
extern  void imguiShowFontAtlas(ImFontAtlas* atlas);
extern  void imguiDebugHookIdInfo(ImGuiID id, ImGuiDataType data_type, const void* data_id, const void* data_id_end);
extern  void imguiDebugNodeColumns(ImGuiOldColumns* columns);
extern  void imguiDebugNodeDockNode(ImGuiDockNode* node, const char* label);
extern  void imguiDebugNodeDrawList(ImGuiWindow* window, ImGuiViewportP* viewport, const ImDrawList* draw_list, const char* label);
extern  void imguiDebugNodeDrawCmdShowMeshAndBoundingBox(ImDrawList* out_draw_list, const ImDrawList* draw_list, const ImDrawCmd* draw_cmd, _Bool show_mesh, _Bool show_aabb);
extern  void imguiDebugNodeFont(ImFont* font);
extern  void imguiDebugNodeFontGlyph(ImFont* font, const ImFontGlyph* glyph);
extern  void imguiDebugNodeStorage(ImGuiStorage* storage, const char* label);
extern  void imguiDebugNodeTabBar(ImGuiTabBar* tab_bar, const char* label);
extern  void imguiDebugNodeTable(ImGuiTable* table);
extern  void imguiDebugNodeTableSettings(ImGuiTableSettings* settings);
extern  void imguiDebugNodeInputTextState(ImGuiInputTextState* state);
extern  void imguiDebugNodeTypingSelectState(ImGuiTypingSelectState* state);
extern  void imguiDebugNodeMultiSelectState(ImGuiMultiSelectState* state);
extern  void imguiDebugNodeWindow(ImGuiWindow* window, const char* label);
extern  void imguiDebugNodeWindowSettings(ImGuiWindowSettings* settings);
extern  void imguiDebugNodeWindowsList(ImVector_ImGuiWindowPtr* windows, const char* label);
extern  void imguiDebugNodeWindowsListByBeginStackParent(ImGuiWindow** windows, int windows_size, ImGuiWindow* parent_in_begin_stack);
extern  void imguiDebugNodeViewport(ImGuiViewportP* viewport);
extern  void imguiDebugNodePlatformMonitor(ImGuiPlatformMonitor* monitor, const char* label, int idx);
extern  void imguiDebugRenderKeyboardPreview(ImDrawList* draw_list);
extern  void imguiDebugRenderViewportThumbnail(ImDrawList* draw_list, ImGuiViewportP* viewport, const ImRect bb);
extern  const ImFontBuilderIO* imguiImFontAtlasGetBuilderForStbTruetype(void);
extern  void imguiImFontAtlasUpdateConfigDataPointers(ImFontAtlas* atlas);
extern  void imguiImFontAtlasBuildInit(ImFontAtlas* atlas);
extern  void imguiImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent);
extern  void imguiImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* stbrp_context_opaque);
extern  void imguiImFontAtlasBuildFinish(ImFontAtlas* atlas);
extern  void imguiImFontAtlasBuildRender8bppRectFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned char in_marker_pixel_value);
extern  void imguiImFontAtlasBuildRender32bppRectFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned int in_marker_pixel_value);
extern  void imguiImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_multiply_factor);
extern  void imguiImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride);
extern  void imguiLogText(const char* fmt, ...);
extern  void ImGuiTextBuffer_appendf(struct ImGuiTextBuffer* buffer, const char* fmt, ...);
extern  float imguiGET_FLT_MAX(void);
extern  float imguiGET_FLT_MIN(void);
extern  ImVector_ImWchar* ImVector_ImWchar_create(void);
extern  void ImVector_ImWchar_destroy(ImVector_ImWchar* self);
extern  void ImVector_ImWchar_Init(ImVector_ImWchar* p);
extern  void ImVector_ImWchar_UnInit(ImVector_ImWchar* p);
extern  void ImGuiPlatformIO_Set_Platform_GetWindowPos(ImGuiPlatformIO* platform_io, void (*user_callback)(ImGuiViewport* vp, ImVec2* out_pos));
extern  void ImGuiPlatformIO_Set_Platform_GetWindowSize(ImGuiPlatformIO* platform_io, void (*user_callback)(ImGuiViewport* vp, ImVec2* out_size));

]]

local M = {
    love = {},
    _common = {}
}

-- local library_path = assert(package.searchpath("cimgui", package.cpath))
-- M.C = ffi.load(library_path)

M.C = ffi.C

M.ImDrawFlags_None = 0
M.ImDrawFlags_Closed = 1
M.ImDrawFlags_RoundCornersTopLeft = 16
M.ImDrawFlags_RoundCornersTopRight = 32
M.ImDrawFlags_RoundCornersBottomLeft = 64
M.ImDrawFlags_RoundCornersBottomRight = 128
M.ImDrawFlags_RoundCornersNone = 256
M.ImDrawFlags_RoundCornersTop = 48
M.ImDrawFlags_RoundCornersBottom = 192
M.ImDrawFlags_RoundCornersLeft = 80
M.ImDrawFlags_RoundCornersRight = 160
M.ImDrawFlags_RoundCornersAll = 240
M.ImDrawFlags_RoundCornersDefault_ = 240
M.ImDrawFlags_RoundCornersMask_ = 496
M.ImDrawListFlags_None = 0
M.ImDrawListFlags_AntiAliasedLines = 1
M.ImDrawListFlags_AntiAliasedLinesUseTex = 2
M.ImDrawListFlags_AntiAliasedFill = 4
M.ImDrawListFlags_AllowVtxOffset = 8
M.ImFontAtlasFlags_None = 0
M.ImFontAtlasFlags_NoPowerOfTwoHeight = 1
M.ImFontAtlasFlags_NoMouseCursors = 2
M.ImFontAtlasFlags_NoBakedLines = 4
M.ImGuiActivateFlags_None = 0
M.ImGuiActivateFlags_PreferInput = 1
M.ImGuiActivateFlags_PreferTweak = 2
M.ImGuiActivateFlags_TryToPreserveState = 4
M.ImGuiActivateFlags_FromTabbing = 8
M.ImGuiActivateFlags_FromShortcut = 16
M.ImGuiAxis_None = -1
M.ImGuiAxis_X = 0
M.ImGuiAxis_Y = 1
M.ImGuiBackendFlags_None = 0
M.ImGuiBackendFlags_HasGamepad = 1
M.ImGuiBackendFlags_HasMouseCursors = 2
M.ImGuiBackendFlags_HasSetMousePos = 4
M.ImGuiBackendFlags_RendererHasVtxOffset = 8
M.ImGuiBackendFlags_PlatformHasViewports = 1024
M.ImGuiBackendFlags_HasMouseHoveredViewport = 2048
M.ImGuiBackendFlags_RendererHasViewports = 4096
M.ImGuiButtonFlags_PressedOnClick = 16
M.ImGuiButtonFlags_PressedOnClickRelease = 32
M.ImGuiButtonFlags_PressedOnClickReleaseAnywhere = 64
M.ImGuiButtonFlags_PressedOnRelease = 128
M.ImGuiButtonFlags_PressedOnDoubleClick = 256
M.ImGuiButtonFlags_PressedOnDragDropHold = 512
M.ImGuiButtonFlags_Repeat = 1024
M.ImGuiButtonFlags_FlattenChildren = 2048
M.ImGuiButtonFlags_AllowOverlap = 4096
M.ImGuiButtonFlags_DontClosePopups = 8192
M.ImGuiButtonFlags_AlignTextBaseLine = 32768
M.ImGuiButtonFlags_NoKeyModifiers = 65536
M.ImGuiButtonFlags_NoHoldingActiveId = 131072
M.ImGuiButtonFlags_NoNavFocus = 262144
M.ImGuiButtonFlags_NoHoveredOnFocus = 524288
M.ImGuiButtonFlags_NoSetKeyOwner = 1048576
M.ImGuiButtonFlags_NoTestKeyOwner = 2097152
M.ImGuiButtonFlags_PressedOnMask_ = 1008
M.ImGuiButtonFlags_PressedOnDefault_ = 32
M.ImGuiButtonFlags_None = 0
M.ImGuiButtonFlags_MouseButtonLeft = 1
M.ImGuiButtonFlags_MouseButtonRight = 2
M.ImGuiButtonFlags_MouseButtonMiddle = 4
M.ImGuiButtonFlags_MouseButtonMask_ = 7
M.ImGuiChildFlags_None = 0
M.ImGuiChildFlags_Border = 1
M.ImGuiChildFlags_AlwaysUseWindowPadding = 2
M.ImGuiChildFlags_ResizeX = 4
M.ImGuiChildFlags_ResizeY = 8
M.ImGuiChildFlags_AutoResizeX = 16
M.ImGuiChildFlags_AutoResizeY = 32
M.ImGuiChildFlags_AlwaysAutoResize = 64
M.ImGuiChildFlags_FrameStyle = 128
M.ImGuiChildFlags_NavFlattened = 256
M.ImGuiCol_Text = 0
M.ImGuiCol_TextDisabled = 1
M.ImGuiCol_WindowBg = 2
M.ImGuiCol_ChildBg = 3
M.ImGuiCol_PopupBg = 4
M.ImGuiCol_Border = 5
M.ImGuiCol_BorderShadow = 6
M.ImGuiCol_FrameBg = 7
M.ImGuiCol_FrameBgHovered = 8
M.ImGuiCol_FrameBgActive = 9
M.ImGuiCol_TitleBg = 10
M.ImGuiCol_TitleBgActive = 11
M.ImGuiCol_TitleBgCollapsed = 12
M.ImGuiCol_MenuBarBg = 13
M.ImGuiCol_ScrollbarBg = 14
M.ImGuiCol_ScrollbarGrab = 15
M.ImGuiCol_ScrollbarGrabHovered = 16
M.ImGuiCol_ScrollbarGrabActive = 17
M.ImGuiCol_CheckMark = 18
M.ImGuiCol_SliderGrab = 19
M.ImGuiCol_SliderGrabActive = 20
M.ImGuiCol_Button = 21
M.ImGuiCol_ButtonHovered = 22
M.ImGuiCol_ButtonActive = 23
M.ImGuiCol_Header = 24
M.ImGuiCol_HeaderHovered = 25
M.ImGuiCol_HeaderActive = 26
M.ImGuiCol_Separator = 27
M.ImGuiCol_SeparatorHovered = 28
M.ImGuiCol_SeparatorActive = 29
M.ImGuiCol_ResizeGrip = 30
M.ImGuiCol_ResizeGripHovered = 31
M.ImGuiCol_ResizeGripActive = 32
M.ImGuiCol_TabHovered = 33
M.ImGuiCol_Tab = 34
M.ImGuiCol_TabSelected = 35
M.ImGuiCol_TabSelectedOverline = 36
M.ImGuiCol_TabDimmed = 37
M.ImGuiCol_TabDimmedSelected = 38
M.ImGuiCol_TabDimmedSelectedOverline = 39
M.ImGuiCol_DockingPreview = 40
M.ImGuiCol_DockingEmptyBg = 41
M.ImGuiCol_PlotLines = 42
M.ImGuiCol_PlotLinesHovered = 43
M.ImGuiCol_PlotHistogram = 44
M.ImGuiCol_PlotHistogramHovered = 45
M.ImGuiCol_TableHeaderBg = 46
M.ImGuiCol_TableBorderStrong = 47
M.ImGuiCol_TableBorderLight = 48
M.ImGuiCol_TableRowBg = 49
M.ImGuiCol_TableRowBgAlt = 50
M.ImGuiCol_TextLink = 51
M.ImGuiCol_TextSelectedBg = 52
M.ImGuiCol_DragDropTarget = 53
M.ImGuiCol_NavHighlight = 54
M.ImGuiCol_NavWindowingHighlight = 55
M.ImGuiCol_NavWindowingDimBg = 56
M.ImGuiCol_ModalWindowDimBg = 57
M.ImGuiCol_COUNT = 58
M.ImGuiColorEditFlags_None = 0
M.ImGuiColorEditFlags_NoAlpha = 2
M.ImGuiColorEditFlags_NoPicker = 4
M.ImGuiColorEditFlags_NoOptions = 8
M.ImGuiColorEditFlags_NoSmallPreview = 16
M.ImGuiColorEditFlags_NoInputs = 32
M.ImGuiColorEditFlags_NoTooltip = 64
M.ImGuiColorEditFlags_NoLabel = 128
M.ImGuiColorEditFlags_NoSidePreview = 256
M.ImGuiColorEditFlags_NoDragDrop = 512
M.ImGuiColorEditFlags_NoBorder = 1024
M.ImGuiColorEditFlags_AlphaBar = 65536
M.ImGuiColorEditFlags_AlphaPreview = 131072
M.ImGuiColorEditFlags_AlphaPreviewHalf = 262144
M.ImGuiColorEditFlags_HDR = 524288
M.ImGuiColorEditFlags_DisplayRGB = 1048576
M.ImGuiColorEditFlags_DisplayHSV = 2097152
M.ImGuiColorEditFlags_DisplayHex = 4194304
M.ImGuiColorEditFlags_Uint8 = 8388608
M.ImGuiColorEditFlags_Float = 16777216
M.ImGuiColorEditFlags_PickerHueBar = 33554432
M.ImGuiColorEditFlags_PickerHueWheel = 67108864
M.ImGuiColorEditFlags_InputRGB = 134217728
M.ImGuiColorEditFlags_InputHSV = 268435456
M.ImGuiColorEditFlags_DefaultOptions_ = 177209344
M.ImGuiColorEditFlags_DisplayMask_ = 7340032
M.ImGuiColorEditFlags_DataTypeMask_ = 25165824
M.ImGuiColorEditFlags_PickerMask_ = 100663296
M.ImGuiColorEditFlags_InputMask_ = 402653184
M.ImGuiComboFlags_CustomPreview = 1048576
M.ImGuiComboFlags_None = 0
M.ImGuiComboFlags_PopupAlignLeft = 1
M.ImGuiComboFlags_HeightSmall = 2
M.ImGuiComboFlags_HeightRegular = 4
M.ImGuiComboFlags_HeightLarge = 8
M.ImGuiComboFlags_HeightLargest = 16
M.ImGuiComboFlags_NoArrowButton = 32
M.ImGuiComboFlags_NoPreview = 64
M.ImGuiComboFlags_WidthFitPreview = 128
M.ImGuiComboFlags_HeightMask_ = 30
M.ImGuiCond_None = 0
M.ImGuiCond_Always = 1
M.ImGuiCond_Once = 2
M.ImGuiCond_FirstUseEver = 4
M.ImGuiCond_Appearing = 8
M.ImGuiConfigFlags_None = 0
M.ImGuiConfigFlags_NavEnableKeyboard = 1
M.ImGuiConfigFlags_NavEnableGamepad = 2
M.ImGuiConfigFlags_NavEnableSetMousePos = 4
M.ImGuiConfigFlags_NavNoCaptureKeyboard = 8
M.ImGuiConfigFlags_NoMouse = 16
M.ImGuiConfigFlags_NoMouseCursorChange = 32
M.ImGuiConfigFlags_NoKeyboard = 64
M.ImGuiConfigFlags_DockingEnable = 128
M.ImGuiConfigFlags_ViewportsEnable = 1024
M.ImGuiConfigFlags_DpiEnableScaleViewports = 16384
M.ImGuiConfigFlags_DpiEnableScaleFonts = 32768
M.ImGuiConfigFlags_IsSRGB = 1048576
M.ImGuiConfigFlags_IsTouchScreen = 2097152
M.ImGuiContextHookType_NewFramePre = 0
M.ImGuiContextHookType_NewFramePost = 1
M.ImGuiContextHookType_EndFramePre = 2
M.ImGuiContextHookType_EndFramePost = 3
M.ImGuiContextHookType_RenderPre = 4
M.ImGuiContextHookType_RenderPost = 5
M.ImGuiContextHookType_Shutdown = 6
M.ImGuiContextHookType_PendingRemoval_ = 7
M.ImGuiDataAuthority_Auto = 0
M.ImGuiDataAuthority_DockNode = 1
M.ImGuiDataAuthority_Window = 2
M.ImGuiDataType_String = 12
M.ImGuiDataType_Pointer = 13
M.ImGuiDataType_ID = 14
M.ImGuiDataType_S8 = 0
M.ImGuiDataType_U8 = 1
M.ImGuiDataType_S16 = 2
M.ImGuiDataType_U16 = 3
M.ImGuiDataType_S32 = 4
M.ImGuiDataType_U32 = 5
M.ImGuiDataType_S64 = 6
M.ImGuiDataType_U64 = 7
M.ImGuiDataType_Float = 8
M.ImGuiDataType_Double = 9
M.ImGuiDataType_Bool = 10
M.ImGuiDataType_COUNT = 11
M.ImGuiDebugLogFlags_None = 0
M.ImGuiDebugLogFlags_EventActiveId = 1
M.ImGuiDebugLogFlags_EventFocus = 2
M.ImGuiDebugLogFlags_EventPopup = 4
M.ImGuiDebugLogFlags_EventNav = 8
M.ImGuiDebugLogFlags_EventClipper = 16
M.ImGuiDebugLogFlags_EventSelection = 32
M.ImGuiDebugLogFlags_EventIO = 64
M.ImGuiDebugLogFlags_EventInputRouting = 128
M.ImGuiDebugLogFlags_EventDocking = 256
M.ImGuiDebugLogFlags_EventViewport = 512
M.ImGuiDebugLogFlags_EventMask_ = 1023
M.ImGuiDebugLogFlags_OutputToTTY = 1048576
M.ImGuiDebugLogFlags_OutputToTestEngine = 2097152
M.ImGuiDir_None = -1
M.ImGuiDir_Left = 0
M.ImGuiDir_Right = 1
M.ImGuiDir_Up = 2
M.ImGuiDir_Down = 3
M.ImGuiDir_COUNT = 4
M.ImGuiDockNodeFlags_DockSpace = 1024
M.ImGuiDockNodeFlags_CentralNode = 2048
M.ImGuiDockNodeFlags_NoTabBar = 4096
M.ImGuiDockNodeFlags_HiddenTabBar = 8192
M.ImGuiDockNodeFlags_NoWindowMenuButton = 16384
M.ImGuiDockNodeFlags_NoCloseButton = 32768
M.ImGuiDockNodeFlags_NoResizeX = 65536
M.ImGuiDockNodeFlags_NoResizeY = 131072
M.ImGuiDockNodeFlags_DockedWindowsInFocusRoute = 262144
M.ImGuiDockNodeFlags_NoDockingSplitOther = 524288
M.ImGuiDockNodeFlags_NoDockingOverMe = 1048576
M.ImGuiDockNodeFlags_NoDockingOverOther = 2097152
M.ImGuiDockNodeFlags_NoDockingOverEmpty = 4194304
M.ImGuiDockNodeFlags_NoDocking = 7864336
M.ImGuiDockNodeFlags_SharedFlagsInheritMask_ = -1
M.ImGuiDockNodeFlags_NoResizeFlagsMask_ = 196640
M.ImGuiDockNodeFlags_LocalFlagsTransferMask_ = 260208
M.ImGuiDockNodeFlags_SavedFlagsMask_ = 261152
M.ImGuiDockNodeFlags_None = 0
M.ImGuiDockNodeFlags_KeepAliveOnly = 1
M.ImGuiDockNodeFlags_NoDockingOverCentralNode = 4
M.ImGuiDockNodeFlags_PassthruCentralNode = 8
M.ImGuiDockNodeFlags_NoDockingSplit = 16
M.ImGuiDockNodeFlags_NoResize = 32
M.ImGuiDockNodeFlags_AutoHideTabBar = 64
M.ImGuiDockNodeFlags_NoUndocking = 128
M.ImGuiDockNodeState_Unknown = 0
M.ImGuiDockNodeState_HostWindowHiddenBecauseSingleWindow = 1
M.ImGuiDockNodeState_HostWindowHiddenBecauseWindowsAreResizing = 2
M.ImGuiDockNodeState_HostWindowVisible = 3
M.ImGuiDragDropFlags_None = 0
M.ImGuiDragDropFlags_SourceNoPreviewTooltip = 1
M.ImGuiDragDropFlags_SourceNoDisableHover = 2
M.ImGuiDragDropFlags_SourceNoHoldToOpenOthers = 4
M.ImGuiDragDropFlags_SourceAllowNullID = 8
M.ImGuiDragDropFlags_SourceExtern = 16
M.ImGuiDragDropFlags_PayloadAutoExpire = 32
M.ImGuiDragDropFlags_PayloadNoCrossContext = 64
M.ImGuiDragDropFlags_PayloadNoCrossProcess = 128
M.ImGuiDragDropFlags_AcceptBeforeDelivery = 1024
M.ImGuiDragDropFlags_AcceptNoDrawDefaultRect = 2048
M.ImGuiDragDropFlags_AcceptNoPreviewTooltip = 4096
M.ImGuiDragDropFlags_AcceptPeekOnly = 3072
M.ImGuiFocusRequestFlags_None = 0
M.ImGuiFocusRequestFlags_RestoreFocusedChild = 1
M.ImGuiFocusRequestFlags_UnlessBelowModal = 2
M.ImGuiFocusedFlags_None = 0
M.ImGuiFocusedFlags_ChildWindows = 1
M.ImGuiFocusedFlags_RootWindow = 2
M.ImGuiFocusedFlags_AnyWindow = 4
M.ImGuiFocusedFlags_NoPopupHierarchy = 8
M.ImGuiFocusedFlags_DockHierarchy = 16
M.ImGuiFocusedFlags_RootAndChildWindows = 3
M.ImGuiHoveredFlags_DelayMask_ = 245760
M.ImGuiHoveredFlags_AllowedMaskForIsWindowHovered = 12479
M.ImGuiHoveredFlags_AllowedMaskForIsItemHovered = 262048
M.ImGuiHoveredFlags_None = 0
M.ImGuiHoveredFlags_ChildWindows = 1
M.ImGuiHoveredFlags_RootWindow = 2
M.ImGuiHoveredFlags_AnyWindow = 4
M.ImGuiHoveredFlags_NoPopupHierarchy = 8
M.ImGuiHoveredFlags_DockHierarchy = 16
M.ImGuiHoveredFlags_AllowWhenBlockedByPopup = 32
M.ImGuiHoveredFlags_AllowWhenBlockedByActiveItem = 128
M.ImGuiHoveredFlags_AllowWhenOverlappedByItem = 256
M.ImGuiHoveredFlags_AllowWhenOverlappedByWindow = 512
M.ImGuiHoveredFlags_AllowWhenDisabled = 1024
M.ImGuiHoveredFlags_NoNavOverride = 2048
M.ImGuiHoveredFlags_AllowWhenOverlapped = 768
M.ImGuiHoveredFlags_RectOnly = 928
M.ImGuiHoveredFlags_RootAndChildWindows = 3
M.ImGuiHoveredFlags_ForTooltip = 4096
M.ImGuiHoveredFlags_Stationary = 8192
M.ImGuiHoveredFlags_DelayNone = 16384
M.ImGuiHoveredFlags_DelayShort = 32768
M.ImGuiHoveredFlags_DelayNormal = 65536
M.ImGuiHoveredFlags_NoSharedDelay = 131072
M.ImGuiInputEventType_None = 0
M.ImGuiInputEventType_MousePos = 1
M.ImGuiInputEventType_MouseWheel = 2
M.ImGuiInputEventType_MouseButton = 3
M.ImGuiInputEventType_MouseViewport = 4
M.ImGuiInputEventType_Key = 5
M.ImGuiInputEventType_Text = 6
M.ImGuiInputEventType_Focus = 7
M.ImGuiInputEventType_COUNT = 8
M.ImGuiInputFlags_RepeatRateDefault = 2
M.ImGuiInputFlags_RepeatRateNavMove = 4
M.ImGuiInputFlags_RepeatRateNavTweak = 8
M.ImGuiInputFlags_RepeatUntilRelease = 16
M.ImGuiInputFlags_RepeatUntilKeyModsChange = 32
M.ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone = 64
M.ImGuiInputFlags_RepeatUntilOtherKeyPress = 128
M.ImGuiInputFlags_LockThisFrame = 1048576
M.ImGuiInputFlags_LockUntilRelease = 2097152
M.ImGuiInputFlags_CondHovered = 4194304
M.ImGuiInputFlags_CondActive = 8388608
M.ImGuiInputFlags_CondDefault_ = 12582912
M.ImGuiInputFlags_RepeatRateMask_ = 14
M.ImGuiInputFlags_RepeatUntilMask_ = 240
M.ImGuiInputFlags_RepeatMask_ = 255
M.ImGuiInputFlags_CondMask_ = 12582912
M.ImGuiInputFlags_RouteTypeMask_ = 15360
M.ImGuiInputFlags_RouteOptionsMask_ = 245760
M.ImGuiInputFlags_SupportedByIsKeyPressed = 255
M.ImGuiInputFlags_SupportedByIsMouseClicked = 1
M.ImGuiInputFlags_SupportedByShortcut = 261375
M.ImGuiInputFlags_SupportedBySetNextItemShortcut = 523519
M.ImGuiInputFlags_SupportedBySetKeyOwner = 3145728
M.ImGuiInputFlags_SupportedBySetItemKeyOwner = 15728640
M.ImGuiInputFlags_None = 0
M.ImGuiInputFlags_Repeat = 1
M.ImGuiInputFlags_RouteActive = 1024
M.ImGuiInputFlags_RouteFocused = 2048
M.ImGuiInputFlags_RouteGlobal = 4096
M.ImGuiInputFlags_RouteAlways = 8192
M.ImGuiInputFlags_RouteOverFocused = 16384
M.ImGuiInputFlags_RouteOverActive = 32768
M.ImGuiInputFlags_RouteUnlessBgFocused = 65536
M.ImGuiInputFlags_RouteFromRootWindow = 131072
M.ImGuiInputFlags_Tooltip = 262144
M.ImGuiInputSource_None = 0
M.ImGuiInputSource_Mouse = 1
M.ImGuiInputSource_Keyboard = 2
M.ImGuiInputSource_Gamepad = 3
M.ImGuiInputSource_COUNT = 4
M.ImGuiInputTextFlags_Multiline = 67108864
M.ImGuiInputTextFlags_NoMarkEdited = 134217728
M.ImGuiInputTextFlags_MergedItem = 268435456
M.ImGuiInputTextFlags_LocalizeDecimalPoint = 536870912
M.ImGuiInputTextFlags_None = 0
M.ImGuiInputTextFlags_CharsDecimal = 1
M.ImGuiInputTextFlags_CharsHexadecimal = 2
M.ImGuiInputTextFlags_CharsScientific = 4
M.ImGuiInputTextFlags_CharsUppercase = 8
M.ImGuiInputTextFlags_CharsNoBlank = 16
M.ImGuiInputTextFlags_AllowTabInput = 32
M.ImGuiInputTextFlags_EnterReturnsTrue = 64
M.ImGuiInputTextFlags_EscapeClearsAll = 128
M.ImGuiInputTextFlags_CtrlEnterForNewLine = 256
M.ImGuiInputTextFlags_ReadOnly = 512
M.ImGuiInputTextFlags_Password = 1024
M.ImGuiInputTextFlags_AlwaysOverwrite = 2048
M.ImGuiInputTextFlags_AutoSelectAll = 4096
M.ImGuiInputTextFlags_ParseEmptyRefVal = 8192
M.ImGuiInputTextFlags_DisplayEmptyRefVal = 16384
M.ImGuiInputTextFlags_NoHorizontalScroll = 32768
M.ImGuiInputTextFlags_NoUndoRedo = 65536
M.ImGuiInputTextFlags_CallbackCompletion = 131072
M.ImGuiInputTextFlags_CallbackHistory = 262144
M.ImGuiInputTextFlags_CallbackAlways = 524288
M.ImGuiInputTextFlags_CallbackCharFilter = 1048576
M.ImGuiInputTextFlags_CallbackResize = 2097152
M.ImGuiInputTextFlags_CallbackEdit = 4194304
M.ImGuiItemFlags_Disabled = 1024
M.ImGuiItemFlags_ReadOnly = 2048
M.ImGuiItemFlags_MixedValue = 4096
M.ImGuiItemFlags_NoWindowHoverableCheck = 8192
M.ImGuiItemFlags_AllowOverlap = 16384
M.ImGuiItemFlags_Inputable = 1048576
M.ImGuiItemFlags_HasSelectionUserData = 2097152
M.ImGuiItemFlags_IsMultiSelect = 4194304
M.ImGuiItemFlags_Default_ = 16
M.ImGuiItemFlags_None = 0
M.ImGuiItemFlags_NoTabStop = 1
M.ImGuiItemFlags_NoNav = 2
M.ImGuiItemFlags_NoNavDefaultFocus = 4
M.ImGuiItemFlags_ButtonRepeat = 8
M.ImGuiItemFlags_AutoClosePopups = 16
M.ImGuiItemStatusFlags_None = 0
M.ImGuiItemStatusFlags_HoveredRect = 1
M.ImGuiItemStatusFlags_HasDisplayRect = 2
M.ImGuiItemStatusFlags_Edited = 4
M.ImGuiItemStatusFlags_ToggledSelection = 8
M.ImGuiItemStatusFlags_ToggledOpen = 16
M.ImGuiItemStatusFlags_HasDeactivated = 32
M.ImGuiItemStatusFlags_Deactivated = 64
M.ImGuiItemStatusFlags_HoveredWindow = 128
M.ImGuiItemStatusFlags_Visible = 256
M.ImGuiItemStatusFlags_HasClipRect = 512
M.ImGuiItemStatusFlags_HasShortcut = 1024
M.ImGuiKey_None = 0
M.ImGuiKey_Tab = 512
M.ImGuiKey_LeftArrow = 513
M.ImGuiKey_RightArrow = 514
M.ImGuiKey_UpArrow = 515
M.ImGuiKey_DownArrow = 516
M.ImGuiKey_PageUp = 517
M.ImGuiKey_PageDown = 518
M.ImGuiKey_Home = 519
M.ImGuiKey_End = 520
M.ImGuiKey_Insert = 521
M.ImGuiKey_Delete = 522
M.ImGuiKey_Backspace = 523
M.ImGuiKey_Space = 524
M.ImGuiKey_Enter = 525
M.ImGuiKey_Escape = 526
M.ImGuiKey_LeftCtrl = 527
M.ImGuiKey_LeftShift = 528
M.ImGuiKey_LeftAlt = 529
M.ImGuiKey_LeftSuper = 530
M.ImGuiKey_RightCtrl = 531
M.ImGuiKey_RightShift = 532
M.ImGuiKey_RightAlt = 533
M.ImGuiKey_RightSuper = 534
M.ImGuiKey_Menu = 535
M.ImGuiKey_0 = 536
M.ImGuiKey_1 = 537
M.ImGuiKey_2 = 538
M.ImGuiKey_3 = 539
M.ImGuiKey_4 = 540
M.ImGuiKey_5 = 541
M.ImGuiKey_6 = 542
M.ImGuiKey_7 = 543
M.ImGuiKey_8 = 544
M.ImGuiKey_9 = 545
M.ImGuiKey_A = 546
M.ImGuiKey_B = 547
M.ImGuiKey_C = 548
M.ImGuiKey_D = 549
M.ImGuiKey_E = 550
M.ImGuiKey_F = 551
M.ImGuiKey_G = 552
M.ImGuiKey_H = 553
M.ImGuiKey_I = 554
M.ImGuiKey_J = 555
M.ImGuiKey_K = 556
M.ImGuiKey_L = 557
M.ImGuiKey_M = 558
M.ImGuiKey_N = 559
M.ImGuiKey_O = 560
M.ImGuiKey_P = 561
M.ImGuiKey_Q = 562
M.ImGuiKey_R = 563
M.ImGuiKey_S = 564
M.ImGuiKey_T = 565
M.ImGuiKey_U = 566
M.ImGuiKey_V = 567
M.ImGuiKey_W = 568
M.ImGuiKey_X = 569
M.ImGuiKey_Y = 570
M.ImGuiKey_Z = 571
M.ImGuiKey_F1 = 572
M.ImGuiKey_F2 = 573
M.ImGuiKey_F3 = 574
M.ImGuiKey_F4 = 575
M.ImGuiKey_F5 = 576
M.ImGuiKey_F6 = 577
M.ImGuiKey_F7 = 578
M.ImGuiKey_F8 = 579
M.ImGuiKey_F9 = 580
M.ImGuiKey_F10 = 581
M.ImGuiKey_F11 = 582
M.ImGuiKey_F12 = 583
M.ImGuiKey_F13 = 584
M.ImGuiKey_F14 = 585
M.ImGuiKey_F15 = 586
M.ImGuiKey_F16 = 587
M.ImGuiKey_F17 = 588
M.ImGuiKey_F18 = 589
M.ImGuiKey_F19 = 590
M.ImGuiKey_F20 = 591
M.ImGuiKey_F21 = 592
M.ImGuiKey_F22 = 593
M.ImGuiKey_F23 = 594
M.ImGuiKey_F24 = 595
M.ImGuiKey_Apostrophe = 596
M.ImGuiKey_Comma = 597
M.ImGuiKey_Minus = 598
M.ImGuiKey_Period = 599
M.ImGuiKey_Slash = 600
M.ImGuiKey_Semicolon = 601
M.ImGuiKey_Equal = 602
M.ImGuiKey_LeftBracket = 603
M.ImGuiKey_Backslash = 604
M.ImGuiKey_RightBracket = 605
M.ImGuiKey_GraveAccent = 606
M.ImGuiKey_CapsLock = 607
M.ImGuiKey_ScrollLock = 608
M.ImGuiKey_NumLock = 609
M.ImGuiKey_PrintScreen = 610
M.ImGuiKey_Pause = 611
M.ImGuiKey_Keypad0 = 612
M.ImGuiKey_Keypad1 = 613
M.ImGuiKey_Keypad2 = 614
M.ImGuiKey_Keypad3 = 615
M.ImGuiKey_Keypad4 = 616
M.ImGuiKey_Keypad5 = 617
M.ImGuiKey_Keypad6 = 618
M.ImGuiKey_Keypad7 = 619
M.ImGuiKey_Keypad8 = 620
M.ImGuiKey_Keypad9 = 621
M.ImGuiKey_KeypadDecimal = 622
M.ImGuiKey_KeypadDivide = 623
M.ImGuiKey_KeypadMultiply = 624
M.ImGuiKey_KeypadSubtract = 625
M.ImGuiKey_KeypadAdd = 626
M.ImGuiKey_KeypadEnter = 627
M.ImGuiKey_KeypadEqual = 628
M.ImGuiKey_AppBack = 629
M.ImGuiKey_AppForward = 630
M.ImGuiKey_GamepadStart = 631
M.ImGuiKey_GamepadBack = 632
M.ImGuiKey_GamepadFaceLeft = 633
M.ImGuiKey_GamepadFaceRight = 634
M.ImGuiKey_GamepadFaceUp = 635
M.ImGuiKey_GamepadFaceDown = 636
M.ImGuiKey_GamepadDpadLeft = 637
M.ImGuiKey_GamepadDpadRight = 638
M.ImGuiKey_GamepadDpadUp = 639
M.ImGuiKey_GamepadDpadDown = 640
M.ImGuiKey_GamepadL1 = 641
M.ImGuiKey_GamepadR1 = 642
M.ImGuiKey_GamepadL2 = 643
M.ImGuiKey_GamepadR2 = 644
M.ImGuiKey_GamepadL3 = 645
M.ImGuiKey_GamepadR3 = 646
M.ImGuiKey_GamepadLStickLeft = 647
M.ImGuiKey_GamepadLStickRight = 648
M.ImGuiKey_GamepadLStickUp = 649
M.ImGuiKey_GamepadLStickDown = 650
M.ImGuiKey_GamepadRStickLeft = 651
M.ImGuiKey_GamepadRStickRight = 652
M.ImGuiKey_GamepadRStickUp = 653
M.ImGuiKey_GamepadRStickDown = 654
M.ImGuiKey_MouseLeft = 655
M.ImGuiKey_MouseRight = 656
M.ImGuiKey_MouseMiddle = 657
M.ImGuiKey_MouseX1 = 658
M.ImGuiKey_MouseX2 = 659
M.ImGuiKey_MouseWheelX = 660
M.ImGuiKey_MouseWheelY = 661
M.ImGuiKey_ReservedForModCtrl = 662
M.ImGuiKey_ReservedForModShift = 663
M.ImGuiKey_ReservedForModAlt = 664
M.ImGuiKey_ReservedForModSuper = 665
M.ImGuiKey_COUNT = 666
M.ImGuiMod_None = 0
M.ImGuiMod_Ctrl = 4096
M.ImGuiMod_Shift = 8192
M.ImGuiMod_Alt = 16384
M.ImGuiMod_Super = 32768
M.ImGuiMod_Mask_ = 61440
M.ImGuiKey_NamedKey_BEGIN = 512
M.ImGuiKey_NamedKey_END = 666
M.ImGuiKey_NamedKey_COUNT = 154
M.ImGuiKey_KeysData_SIZE = 154
M.ImGuiKey_KeysData_OFFSET = 512
M.ImGuiLayoutType_Horizontal = 0
M.ImGuiLayoutType_Vertical = 1
M.ImGuiLocKey_VersionStr = 0
M.ImGuiLocKey_TableSizeOne = 1
M.ImGuiLocKey_TableSizeAllFit = 2
M.ImGuiLocKey_TableSizeAllDefault = 3
M.ImGuiLocKey_TableResetOrder = 4
M.ImGuiLocKey_WindowingMainMenuBar = 5
M.ImGuiLocKey_WindowingPopup = 6
M.ImGuiLocKey_WindowingUntitled = 7
M.ImGuiLocKey_CopyLink = 8
M.ImGuiLocKey_DockingHideTabBar = 9
M.ImGuiLocKey_DockingHoldShiftToDock = 10
M.ImGuiLocKey_DockingDragToUndockOrMoveNode = 11
M.ImGuiLocKey_COUNT = 12
M.ImGuiLogType_None = 0
M.ImGuiLogType_TTY = 1
M.ImGuiLogType_File = 2
M.ImGuiLogType_Buffer = 3
M.ImGuiLogType_Clipboard = 4
M.ImGuiMouseButton_Left = 0
M.ImGuiMouseButton_Right = 1
M.ImGuiMouseButton_Middle = 2
M.ImGuiMouseButton_COUNT = 5
M.ImGuiMouseCursor_None = -1
M.ImGuiMouseCursor_Arrow = 0
M.ImGuiMouseCursor_TextInput = 1
M.ImGuiMouseCursor_ResizeAll = 2
M.ImGuiMouseCursor_ResizeNS = 3
M.ImGuiMouseCursor_ResizeEW = 4
M.ImGuiMouseCursor_ResizeNESW = 5
M.ImGuiMouseCursor_ResizeNWSE = 6
M.ImGuiMouseCursor_Hand = 7
M.ImGuiMouseCursor_NotAllowed = 8
M.ImGuiMouseCursor_COUNT = 9
M.ImGuiMouseSource_Mouse = 0
M.ImGuiMouseSource_TouchScreen = 1
M.ImGuiMouseSource_Pen = 2
M.ImGuiMouseSource_COUNT = 3
M.ImGuiMultiSelectFlags_None = 0
M.ImGuiMultiSelectFlags_SingleSelect = 1
M.ImGuiMultiSelectFlags_NoSelectAll = 2
M.ImGuiMultiSelectFlags_NoRangeSelect = 4
M.ImGuiMultiSelectFlags_NoAutoSelect = 8
M.ImGuiMultiSelectFlags_NoAutoClear = 16
M.ImGuiMultiSelectFlags_NoAutoClearOnReselect = 32
M.ImGuiMultiSelectFlags_BoxSelect1d = 64
M.ImGuiMultiSelectFlags_BoxSelect2d = 128
M.ImGuiMultiSelectFlags_BoxSelectNoScroll = 256
M.ImGuiMultiSelectFlags_ClearOnEscape = 512
M.ImGuiMultiSelectFlags_ClearOnClickVoid = 1024
M.ImGuiMultiSelectFlags_ScopeWindow = 2048
M.ImGuiMultiSelectFlags_ScopeRect = 4096
M.ImGuiMultiSelectFlags_SelectOnClick = 8192
M.ImGuiMultiSelectFlags_SelectOnClickRelease = 16384
M.ImGuiMultiSelectFlags_NavWrapX = 65536
M.ImGuiNavHighlightFlags_None = 0
M.ImGuiNavHighlightFlags_Compact = 2
M.ImGuiNavHighlightFlags_AlwaysDraw = 4
M.ImGuiNavHighlightFlags_NoRounding = 8
M.ImGuiNavLayer_Main = 0
M.ImGuiNavLayer_Menu = 1
M.ImGuiNavLayer_COUNT = 2
M.ImGuiNavMoveFlags_None = 0
M.ImGuiNavMoveFlags_LoopX = 1
M.ImGuiNavMoveFlags_LoopY = 2
M.ImGuiNavMoveFlags_WrapX = 4
M.ImGuiNavMoveFlags_WrapY = 8
M.ImGuiNavMoveFlags_WrapMask_ = 15
M.ImGuiNavMoveFlags_AllowCurrentNavId = 16
M.ImGuiNavMoveFlags_AlsoScoreVisibleSet = 32
M.ImGuiNavMoveFlags_ScrollToEdgeY = 64
M.ImGuiNavMoveFlags_Forwarded = 128
M.ImGuiNavMoveFlags_DebugNoResult = 256
M.ImGuiNavMoveFlags_FocusApi = 512
M.ImGuiNavMoveFlags_IsTabbing = 1024
M.ImGuiNavMoveFlags_IsPageMove = 2048
M.ImGuiNavMoveFlags_Activate = 4096
M.ImGuiNavMoveFlags_NoSelect = 8192
M.ImGuiNavMoveFlags_NoSetNavHighlight = 16384
M.ImGuiNavMoveFlags_NoClearActiveId = 32768
M.ImGuiNextItemDataFlags_None = 0
M.ImGuiNextItemDataFlags_HasWidth = 1
M.ImGuiNextItemDataFlags_HasOpen = 2
M.ImGuiNextItemDataFlags_HasShortcut = 4
M.ImGuiNextItemDataFlags_HasRefVal = 8
M.ImGuiNextItemDataFlags_HasStorageID = 16
M.ImGuiNextWindowDataFlags_None = 0
M.ImGuiNextWindowDataFlags_HasPos = 1
M.ImGuiNextWindowDataFlags_HasSize = 2
M.ImGuiNextWindowDataFlags_HasContentSize = 4
M.ImGuiNextWindowDataFlags_HasCollapsed = 8
M.ImGuiNextWindowDataFlags_HasSizeConstraint = 16
M.ImGuiNextWindowDataFlags_HasFocus = 32
M.ImGuiNextWindowDataFlags_HasBgAlpha = 64
M.ImGuiNextWindowDataFlags_HasScroll = 128
M.ImGuiNextWindowDataFlags_HasChildFlags = 256
M.ImGuiNextWindowDataFlags_HasRefreshPolicy = 512
M.ImGuiNextWindowDataFlags_HasViewport = 1024
M.ImGuiNextWindowDataFlags_HasDock = 2048
M.ImGuiNextWindowDataFlags_HasWindowClass = 4096
M.ImGuiOldColumnFlags_None = 0
M.ImGuiOldColumnFlags_NoBorder = 1
M.ImGuiOldColumnFlags_NoResize = 2
M.ImGuiOldColumnFlags_NoPreserveWidths = 4
M.ImGuiOldColumnFlags_NoForceWithinWindow = 8
M.ImGuiOldColumnFlags_GrowParentContentsSize = 16
M.ImGuiPlotType_Lines = 0
M.ImGuiPlotType_Histogram = 1
M.ImGuiPopupFlags_None = 0
M.ImGuiPopupFlags_MouseButtonLeft = 0
M.ImGuiPopupFlags_MouseButtonRight = 1
M.ImGuiPopupFlags_MouseButtonMiddle = 2
M.ImGuiPopupFlags_MouseButtonMask_ = 31
M.ImGuiPopupFlags_MouseButtonDefault_ = 1
M.ImGuiPopupFlags_NoReopen = 32
M.ImGuiPopupFlags_NoOpenOverExistingPopup = 128
M.ImGuiPopupFlags_NoOpenOverItems = 256
M.ImGuiPopupFlags_AnyPopupId = 1024
M.ImGuiPopupFlags_AnyPopupLevel = 2048
M.ImGuiPopupFlags_AnyPopup = 3072
M.ImGuiPopupPositionPolicy_Default = 0
M.ImGuiPopupPositionPolicy_ComboBox = 1
M.ImGuiPopupPositionPolicy_Tooltip = 2
M.ImGuiScrollFlags_None = 0
M.ImGuiScrollFlags_KeepVisibleEdgeX = 1
M.ImGuiScrollFlags_KeepVisibleEdgeY = 2
M.ImGuiScrollFlags_KeepVisibleCenterX = 4
M.ImGuiScrollFlags_KeepVisibleCenterY = 8
M.ImGuiScrollFlags_AlwaysCenterX = 16
M.ImGuiScrollFlags_AlwaysCenterY = 32
M.ImGuiScrollFlags_NoScrollParent = 64
M.ImGuiScrollFlags_MaskX_ = 21
M.ImGuiScrollFlags_MaskY_ = 42
M.ImGuiSelectableFlags_NoHoldingActiveID = 1048576
M.ImGuiSelectableFlags_SelectOnNav = 2097152
M.ImGuiSelectableFlags_SelectOnClick = 4194304
M.ImGuiSelectableFlags_SelectOnRelease = 8388608
M.ImGuiSelectableFlags_SpanAvailWidth = 16777216
M.ImGuiSelectableFlags_SetNavIdOnHover = 33554432
M.ImGuiSelectableFlags_NoPadWithHalfSpacing = 67108864
M.ImGuiSelectableFlags_NoSetKeyOwner = 134217728
M.ImGuiSelectableFlags_None = 0
M.ImGuiSelectableFlags_NoAutoClosePopups = 1
M.ImGuiSelectableFlags_SpanAllColumns = 2
M.ImGuiSelectableFlags_AllowDoubleClick = 4
M.ImGuiSelectableFlags_Disabled = 8
M.ImGuiSelectableFlags_AllowOverlap = 16
M.ImGuiSelectableFlags_Highlight = 32
M.ImGuiSelectionRequestType_None = 0
M.ImGuiSelectionRequestType_SetAll = 1
M.ImGuiSelectionRequestType_SetRange = 2
M.ImGuiSeparatorFlags_None = 0
M.ImGuiSeparatorFlags_Horizontal = 1
M.ImGuiSeparatorFlags_Vertical = 2
M.ImGuiSeparatorFlags_SpanAllColumns = 4
M.ImGuiSliderFlags_Vertical = 1048576
M.ImGuiSliderFlags_ReadOnly = 2097152
M.ImGuiSliderFlags_None = 0
M.ImGuiSliderFlags_AlwaysClamp = 16
M.ImGuiSliderFlags_Logarithmic = 32
M.ImGuiSliderFlags_NoRoundToFormat = 64
M.ImGuiSliderFlags_NoInput = 128
M.ImGuiSliderFlags_WrapAround = 256
M.ImGuiSliderFlags_InvalidMask_ = 1879048207
M.ImGuiSortDirection_None = 0
M.ImGuiSortDirection_Ascending = 1
M.ImGuiSortDirection_Descending = 2
M.ImGuiStyleVar_Alpha = 0
M.ImGuiStyleVar_DisabledAlpha = 1
M.ImGuiStyleVar_WindowPadding = 2
M.ImGuiStyleVar_WindowRounding = 3
M.ImGuiStyleVar_WindowBorderSize = 4
M.ImGuiStyleVar_WindowMinSize = 5
M.ImGuiStyleVar_WindowTitleAlign = 6
M.ImGuiStyleVar_ChildRounding = 7
M.ImGuiStyleVar_ChildBorderSize = 8
M.ImGuiStyleVar_PopupRounding = 9
M.ImGuiStyleVar_PopupBorderSize = 10
M.ImGuiStyleVar_FramePadding = 11
M.ImGuiStyleVar_FrameRounding = 12
M.ImGuiStyleVar_FrameBorderSize = 13
M.ImGuiStyleVar_ItemSpacing = 14
M.ImGuiStyleVar_ItemInnerSpacing = 15
M.ImGuiStyleVar_IndentSpacing = 16
M.ImGuiStyleVar_CellPadding = 17
M.ImGuiStyleVar_ScrollbarSize = 18
M.ImGuiStyleVar_ScrollbarRounding = 19
M.ImGuiStyleVar_GrabMinSize = 20
M.ImGuiStyleVar_GrabRounding = 21
M.ImGuiStyleVar_TabRounding = 22
M.ImGuiStyleVar_TabBorderSize = 23
M.ImGuiStyleVar_TabBarBorderSize = 24
M.ImGuiStyleVar_TabBarOverlineSize = 25
M.ImGuiStyleVar_TableAngledHeadersAngle = 26
M.ImGuiStyleVar_TableAngledHeadersTextAlign = 27
M.ImGuiStyleVar_ButtonTextAlign = 28
M.ImGuiStyleVar_SelectableTextAlign = 29
M.ImGuiStyleVar_SeparatorTextBorderSize = 30
M.ImGuiStyleVar_SeparatorTextAlign = 31
M.ImGuiStyleVar_SeparatorTextPadding = 32
M.ImGuiStyleVar_DockingSeparatorSize = 33
M.ImGuiStyleVar_COUNT = 34
M.ImGuiTabBarFlags_DockNode = 1048576
M.ImGuiTabBarFlags_IsFocused = 2097152
M.ImGuiTabBarFlags_SaveSettings = 4194304
M.ImGuiTabBarFlags_None = 0
M.ImGuiTabBarFlags_Reorderable = 1
M.ImGuiTabBarFlags_AutoSelectNewTabs = 2
M.ImGuiTabBarFlags_TabListPopupButton = 4
M.ImGuiTabBarFlags_NoCloseWithMiddleMouseButton = 8
M.ImGuiTabBarFlags_NoTabListScrollingButtons = 16
M.ImGuiTabBarFlags_NoTooltip = 32
M.ImGuiTabBarFlags_DrawSelectedOverline = 64
M.ImGuiTabBarFlags_FittingPolicyResizeDown = 128
M.ImGuiTabBarFlags_FittingPolicyScroll = 256
M.ImGuiTabBarFlags_FittingPolicyMask_ = 384
M.ImGuiTabBarFlags_FittingPolicyDefault_ = 128
M.ImGuiTabItemFlags_SectionMask_ = 192
M.ImGuiTabItemFlags_NoCloseButton = 1048576
M.ImGuiTabItemFlags_Button = 2097152
M.ImGuiTabItemFlags_Unsorted = 4194304
M.ImGuiTabItemFlags_None = 0
M.ImGuiTabItemFlags_UnsavedDocument = 1
M.ImGuiTabItemFlags_SetSelected = 2
M.ImGuiTabItemFlags_NoCloseWithMiddleMouseButton = 4
M.ImGuiTabItemFlags_NoPushId = 8
M.ImGuiTabItemFlags_NoTooltip = 16
M.ImGuiTabItemFlags_NoReorder = 32
M.ImGuiTabItemFlags_Leading = 64
M.ImGuiTabItemFlags_Trailing = 128
M.ImGuiTabItemFlags_NoAssumedClosure = 256
M.ImGuiTableBgTarget_None = 0
M.ImGuiTableBgTarget_RowBg0 = 1
M.ImGuiTableBgTarget_RowBg1 = 2
M.ImGuiTableBgTarget_CellBg = 3
M.ImGuiTableColumnFlags_None = 0
M.ImGuiTableColumnFlags_Disabled = 1
M.ImGuiTableColumnFlags_DefaultHide = 2
M.ImGuiTableColumnFlags_DefaultSort = 4
M.ImGuiTableColumnFlags_WidthStretch = 8
M.ImGuiTableColumnFlags_WidthFixed = 16
M.ImGuiTableColumnFlags_NoResize = 32
M.ImGuiTableColumnFlags_NoReorder = 64
M.ImGuiTableColumnFlags_NoHide = 128
M.ImGuiTableColumnFlags_NoClip = 256
M.ImGuiTableColumnFlags_NoSort = 512
M.ImGuiTableColumnFlags_NoSortAscending = 1024
M.ImGuiTableColumnFlags_NoSortDescending = 2048
M.ImGuiTableColumnFlags_NoHeaderLabel = 4096
M.ImGuiTableColumnFlags_NoHeaderWidth = 8192
M.ImGuiTableColumnFlags_PreferSortAscending = 16384
M.ImGuiTableColumnFlags_PreferSortDescending = 32768
M.ImGuiTableColumnFlags_IndentEnable = 65536
M.ImGuiTableColumnFlags_IndentDisable = 131072
M.ImGuiTableColumnFlags_AngledHeader = 262144
M.ImGuiTableColumnFlags_IsEnabled = 16777216
M.ImGuiTableColumnFlags_IsVisible = 33554432
M.ImGuiTableColumnFlags_IsSorted = 67108864
M.ImGuiTableColumnFlags_IsHovered = 134217728
M.ImGuiTableColumnFlags_WidthMask_ = 24
M.ImGuiTableColumnFlags_IndentMask_ = 196608
M.ImGuiTableColumnFlags_StatusMask_ = 251658240
M.ImGuiTableColumnFlags_NoDirectResize_ = 1073741824
M.ImGuiTableFlags_None = 0
M.ImGuiTableFlags_Resizable = 1
M.ImGuiTableFlags_Reorderable = 2
M.ImGuiTableFlags_Hideable = 4
M.ImGuiTableFlags_Sortable = 8
M.ImGuiTableFlags_NoSavedSettings = 16
M.ImGuiTableFlags_ContextMenuInBody = 32
M.ImGuiTableFlags_RowBg = 64
M.ImGuiTableFlags_BordersInnerH = 128
M.ImGuiTableFlags_BordersOuterH = 256
M.ImGuiTableFlags_BordersInnerV = 512
M.ImGuiTableFlags_BordersOuterV = 1024
M.ImGuiTableFlags_BordersH = 384
M.ImGuiTableFlags_BordersV = 1536
M.ImGuiTableFlags_BordersInner = 640
M.ImGuiTableFlags_BordersOuter = 1280
M.ImGuiTableFlags_Borders = 1920
M.ImGuiTableFlags_NoBordersInBody = 2048
M.ImGuiTableFlags_NoBordersInBodyUntilResize = 4096
M.ImGuiTableFlags_SizingFixedFit = 8192
M.ImGuiTableFlags_SizingFixedSame = 16384
M.ImGuiTableFlags_SizingStretchProp = 24576
M.ImGuiTableFlags_SizingStretchSame = 32768
M.ImGuiTableFlags_NoHostExtendX = 65536
M.ImGuiTableFlags_NoHostExtendY = 131072
M.ImGuiTableFlags_NoKeepColumnsVisible = 262144
M.ImGuiTableFlags_PreciseWidths = 524288
M.ImGuiTableFlags_NoClip = 1048576
M.ImGuiTableFlags_PadOuterX = 2097152
M.ImGuiTableFlags_NoPadOuterX = 4194304
M.ImGuiTableFlags_NoPadInnerX = 8388608
M.ImGuiTableFlags_ScrollX = 16777216
M.ImGuiTableFlags_ScrollY = 33554432
M.ImGuiTableFlags_SortMulti = 67108864
M.ImGuiTableFlags_SortTristate = 134217728
M.ImGuiTableFlags_HighlightHoveredColumn = 268435456
M.ImGuiTableFlags_SizingMask_ = 57344
M.ImGuiTableRowFlags_None = 0
M.ImGuiTableRowFlags_Headers = 1
M.ImGuiTextFlags_None = 0
M.ImGuiTextFlags_NoWidthForLargeClippedText = 1
M.ImGuiTooltipFlags_None = 0
M.ImGuiTooltipFlags_OverridePrevious = 2
M.ImGuiTreeNodeFlags_ClipLabelForTrailingButton = 268435456
M.ImGuiTreeNodeFlags_UpsideDownArrow = 536870912
M.ImGuiTreeNodeFlags_None = 0
M.ImGuiTreeNodeFlags_Selected = 1
M.ImGuiTreeNodeFlags_Framed = 2
M.ImGuiTreeNodeFlags_AllowOverlap = 4
M.ImGuiTreeNodeFlags_NoTreePushOnOpen = 8
M.ImGuiTreeNodeFlags_NoAutoOpenOnLog = 16
M.ImGuiTreeNodeFlags_DefaultOpen = 32
M.ImGuiTreeNodeFlags_OpenOnDoubleClick = 64
M.ImGuiTreeNodeFlags_OpenOnArrow = 128
M.ImGuiTreeNodeFlags_Leaf = 256
M.ImGuiTreeNodeFlags_Bullet = 512
M.ImGuiTreeNodeFlags_FramePadding = 1024
M.ImGuiTreeNodeFlags_SpanAvailWidth = 2048
M.ImGuiTreeNodeFlags_SpanFullWidth = 4096
M.ImGuiTreeNodeFlags_SpanTextWidth = 8192
M.ImGuiTreeNodeFlags_SpanAllColumns = 16384
M.ImGuiTreeNodeFlags_NavLeftJumpsBackHere = 32768
M.ImGuiTreeNodeFlags_CollapsingHeader = 26
M.ImGuiTypingSelectFlags_None = 0
M.ImGuiTypingSelectFlags_AllowBackspace = 1
M.ImGuiTypingSelectFlags_AllowSingleCharMode = 2
M.ImGuiViewportFlags_None = 0
M.ImGuiViewportFlags_IsPlatformWindow = 1
M.ImGuiViewportFlags_IsPlatformMonitor = 2
M.ImGuiViewportFlags_OwnedByApp = 4
M.ImGuiViewportFlags_NoDecoration = 8
M.ImGuiViewportFlags_NoTaskBarIcon = 16
M.ImGuiViewportFlags_NoFocusOnAppearing = 32
M.ImGuiViewportFlags_NoFocusOnClick = 64
M.ImGuiViewportFlags_NoInputs = 128
M.ImGuiViewportFlags_NoRendererClear = 256
M.ImGuiViewportFlags_NoAutoMerge = 512
M.ImGuiViewportFlags_TopMost = 1024
M.ImGuiViewportFlags_CanHostOtherWindows = 2048
M.ImGuiViewportFlags_IsMinimized = 4096
M.ImGuiViewportFlags_IsFocused = 8192
M.ImGuiWindowDockStyleCol_Text = 0
M.ImGuiWindowDockStyleCol_TabHovered = 1
M.ImGuiWindowDockStyleCol_TabFocused = 2
M.ImGuiWindowDockStyleCol_TabSelected = 3
M.ImGuiWindowDockStyleCol_TabSelectedOverline = 4
M.ImGuiWindowDockStyleCol_TabDimmed = 5
M.ImGuiWindowDockStyleCol_TabDimmedSelected = 6
M.ImGuiWindowDockStyleCol_TabDimmedSelectedOverline = 7
M.ImGuiWindowDockStyleCol_COUNT = 8
M.ImGuiWindowFlags_None = 0
M.ImGuiWindowFlags_NoTitleBar = 1
M.ImGuiWindowFlags_NoResize = 2
M.ImGuiWindowFlags_NoMove = 4
M.ImGuiWindowFlags_NoScrollbar = 8
M.ImGuiWindowFlags_NoScrollWithMouse = 16
M.ImGuiWindowFlags_NoCollapse = 32
M.ImGuiWindowFlags_AlwaysAutoResize = 64
M.ImGuiWindowFlags_NoBackground = 128
M.ImGuiWindowFlags_NoSavedSettings = 256
M.ImGuiWindowFlags_NoMouseInputs = 512
M.ImGuiWindowFlags_MenuBar = 1024
M.ImGuiWindowFlags_HorizontalScrollbar = 2048
M.ImGuiWindowFlags_NoFocusOnAppearing = 4096
M.ImGuiWindowFlags_NoBringToFrontOnFocus = 8192
M.ImGuiWindowFlags_AlwaysVerticalScrollbar = 16384
M.ImGuiWindowFlags_AlwaysHorizontalScrollbar = 32768
M.ImGuiWindowFlags_NoNavInputs = 65536
M.ImGuiWindowFlags_NoNavFocus = 131072
M.ImGuiWindowFlags_UnsavedDocument = 262144
M.ImGuiWindowFlags_NoDocking = 524288
M.ImGuiWindowFlags_NoNav = 196608
M.ImGuiWindowFlags_NoDecoration = 43
M.ImGuiWindowFlags_NoInputs = 197120
M.ImGuiWindowFlags_ChildWindow = 16777216
M.ImGuiWindowFlags_Tooltip = 33554432
M.ImGuiWindowFlags_Popup = 67108864
M.ImGuiWindowFlags_Modal = 134217728
M.ImGuiWindowFlags_ChildMenu = 268435456
M.ImGuiWindowFlags_DockNodeHost = 536870912
M.ImGuiWindowRefreshFlags_None = 0
M.ImGuiWindowRefreshFlags_TryToAvoidRefresh = 1
M.ImGuiWindowRefreshFlags_RefreshOnHover = 2
M.ImGuiWindowRefreshFlags_RefreshOnFocus = 4

M.gen = function()
    local function sorted_entries(t)
        local tmp = {}
        for k in pairs(t) do
            tmp[#tmp + 1] = k
        end
        table.sort(tmp)
        return tmp
    end

    function script_path()
        local str = debug.getinfo(2, "S").source:sub(2)
        return str:match("(.*/)")
    end

    -- imgui_cdef.lua
    local cparser = default_require("libs/cparser")

    local tmpfile = io.tmpfile()
    cparser.cpp("source/vendor/cimgui.h", tmpfile, {"-U__GNUC__", "-DCIMGUI_DEFINE_ENUMS_AND_STRUCTS"})

    tmpfile:seek("set")
    local data = tmpfile:read("*all")
    tmpfile:close()

    -- print(data)

    local cdef = {'require("ffi").cdef[[', 'typedef struct FILE FILE;', data:gsub("#.-\n", ""), ']]'}

    local f = assert(io.open("source/game/script/libs/cimgui/imgui_cdef.lua", "w"))
    f:write(table.concat(cdef, "\n"))
    f:close()

end

-- local env = {
--     assert = assert,
--     type = type,
--     tonumber = tonumber,
--     tostring = tostring,
--     require = require,
--     error = error,
--     getmetatable = getmetatable,
--     setmetatable = setmetatable,
--     string = string,
--     table = table,
--     love = love,
--     jit = jit,
-- }
-- setfenv(1, env)

-- local ffi = default_require("ffi")

local C = M.C
local _common = M._common

-- add metamethods to ImVec2 and ImVec4

local ct = ffi.typeof("ImVec2")
local ImVec2 = {}
function ImVec2.__add(u, v)
    assert(type(u) == type(v) and ffi.istype(u, v), "One of the summands in not an ImVec2.")
    return ct(u.x + v.x, u.y + v.y)
end
function ImVec2.__sub(u, v)
    assert(type(u) == type(v) and ffi.istype(u, v), "One of the summands in not an ImVec2.")
    return ct(u.x - v.x, u.y - v.y)
end
function ImVec2.__unm(u)
    return ct(-u.x, -u.y)
end
function ImVec2.__mul(u, v)
    local nu, nv = tonumber(u), tonumber(v)
    if nu then
        return ct(nu * v.x, nu * v.y)
    elseif nv then
        return ct(nv * u.x, nv * u.y)
    else
        error("ImVec2 can only be multipliead by a numerical type.")
    end
end
function ImVec2.__div(u, a)
    a = assert(tonumber(a), "ImVec2 can only be divided by a numerical type.")
    return ct(u.x / a, u.y / a)
end

local ct = ffi.typeof("ImVec4")
local ImVec4 = {}
function ImVec4.__add(u, v)
    assert(type(u) == type(v) and ffi.istype(u, v), "One of the summands in not an ImVec4.")
    return ct(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w)
end
function ImVec4.__sub(u, v)
    assert(type(u) == type(v) and ffi.istype(u, v), "One of the summands in not an ImVec4.")
    return ct(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w)
end
function ImVec4.__unm(u)
    return ct(-u.x, -u.y, -u.z, -u.w)
end
function ImVec4.__mul(u, v)
    local nu, nv = tonumber(u), tonumber(v)
    if nu then
        return v:__new(nu * v.x, nu * v.y, nu * v.z, nu * v.w)
    elseif nv then
        return ct(nv * u.x, nv * u.y, nv * u.z, nv * u.w)
    else
        error("ImVec4 can only be multipliead by a numerical type.")
    end
end
function ImVec4.__div(u, a)
    a = assert(tonumber(a), "ImVec4 can only be divided by a numerical type.")
    return ct(u.x / a, u.y / a, u.z / a, u.w / a)
end

-- wrap FLT_MIN, FLT_MAX

local FLT_MIN, FLT_MAX = C.imguiGET_FLT_MIN(), C.imguiGET_FLT_MAX()
M.FLT_MIN, M.FLT_MAX = FLT_MIN, FLT_MAX

-- handwritten functions

M.ImVector_ImWchar = function()
    jit.off(true)
    local p = C.ImVector_ImWchar_create()
    return ffi.gc(p[0], C.ImVector_ImWchar_destroy)
end

-----------------------
-- BEGIN GENERATED CODE
-----------------------

local ImBitVector = ImBitVector or {}
ImBitVector.__index = ImBitVector
ImBitVector["Clear"] = ImBitVector["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImBitVector_Clear(i1)
    return out
end
ImBitVector["ClearBit"] = ImBitVector["ClearBit"] or function(i1, i2)
    jit.off(true)
    local out = C.ImBitVector_ClearBit(i1, i2)
    return out
end
ImBitVector["Create"] = ImBitVector["Create"] or function(i1, i2)
    jit.off(true)
    local out = C.ImBitVector_Create(i1, i2)
    return out
end
ImBitVector["SetBit"] = ImBitVector["SetBit"] or function(i1, i2)
    jit.off(true)
    local out = C.ImBitVector_SetBit(i1, i2)
    return out
end
ImBitVector["TestBit"] = ImBitVector["TestBit"] or function(i1, i2)
    jit.off(true)
    local out = C.ImBitVector_TestBit(i1, i2)
    return out
end
M.ImBitVector = ImBitVector
ffi.metatype("ImBitVector", ImBitVector)

local ImColor = ImColor or {}
ImColor.__index = ImColor
ImColor["HSV"] = ImColor["HSV"] or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = 1.0
    end
    local o1 = M.ImColor_Nil()
    local out = C.ImColor_HSV(o1, i1, i2, i3, i4)
    return o1, out
end
ImColor["SetHSV"] = ImColor["SetHSV"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = 1.0
    end
    local out = C.ImColor_SetHSV(i1, i2, i3, i4, i5)
    return out
end
M.ImColor_Nil = M.ImColor_Nil or function()
    jit.off(true)
    local p = C.ImColor_ImColor_Nil()
    return ffi.gc(p[0], C.ImColor_destroy)
end
M.ImColor_Float = M.ImColor_Float or function(i1, i2, i3, i4)
    jit.off(true)
    local p = C.ImColor_ImColor_Float(i1, i2, i3, i4)
    return ffi.gc(p[0], C.ImColor_destroy)
end
M.ImColor_Vec4 = M.ImColor_Vec4 or function(i1)
    jit.off(true)
    local p = C.ImColor_ImColor_Vec4(i1)
    return ffi.gc(p[0], C.ImColor_destroy)
end
M.ImColor_Int = M.ImColor_Int or function(i1, i2, i3, i4)
    jit.off(true)
    local p = C.ImColor_ImColor_Int(i1, i2, i3, i4)
    return ffi.gc(p[0], C.ImColor_destroy)
end
M.ImColor_U32 = M.ImColor_U32 or function(i1)
    jit.off(true)
    local p = C.ImColor_ImColor_U32(i1)
    return ffi.gc(p[0], C.ImColor_destroy)
end
M.ImColor = ImColor
ffi.metatype("ImColor", ImColor)

local ImDrawCmd = ImDrawCmd or {}
ImDrawCmd.__index = ImDrawCmd
ImDrawCmd["GetTexID"] = ImDrawCmd["GetTexID"] or function(i1)
    jit.off(true)
    local out = C.ImDrawCmd_GetTexID(i1)
    return out
end
local mt = getmetatable(ImDrawCmd) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImDrawCmd_ImDrawCmd()
    return ffi.gc(p[0], C.ImDrawCmd_destroy)
end
setmetatable(ImDrawCmd, mt)
M.ImDrawCmd = ImDrawCmd
ffi.metatype("ImDrawCmd", ImDrawCmd)

local ImDrawData = ImDrawData or {}
ImDrawData.__index = ImDrawData
ImDrawData["AddDrawList"] = ImDrawData["AddDrawList"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawData_AddDrawList(i1, i2)
    return out
end
ImDrawData["Clear"] = ImDrawData["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImDrawData_Clear(i1)
    return out
end
ImDrawData["DeIndexAllBuffers"] = ImDrawData["DeIndexAllBuffers"] or function(i1)
    jit.off(true)
    local out = C.ImDrawData_DeIndexAllBuffers(i1)
    return out
end
ImDrawData["ScaleClipRects"] = ImDrawData["ScaleClipRects"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawData_ScaleClipRects(i1, i2)
    return out
end
local mt = getmetatable(ImDrawData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImDrawData_ImDrawData()
    return ffi.gc(p[0], C.ImDrawData_destroy)
end
setmetatable(ImDrawData, mt)
M.ImDrawData = ImDrawData
ffi.metatype("ImDrawData", ImDrawData)

local ImDrawDataBuilder = ImDrawDataBuilder or {}
ImDrawDataBuilder.__index = ImDrawDataBuilder
local mt = getmetatable(ImDrawDataBuilder) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImDrawDataBuilder_ImDrawDataBuilder()
    return ffi.gc(p[0], C.ImDrawDataBuilder_destroy)
end
setmetatable(ImDrawDataBuilder, mt)
M.ImDrawDataBuilder = ImDrawDataBuilder
ffi.metatype("ImDrawDataBuilder", ImDrawDataBuilder)

local ImDrawList = ImDrawList or {}
ImDrawList.__index = ImDrawList
ImDrawList["AddBezierCubic"] = ImDrawList["AddBezierCubic"] or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    if i8 == nil then
        i8 = 0
    end
    local out = C.ImDrawList_AddBezierCubic(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
ImDrawList["AddBezierQuadratic"] = ImDrawList["AddBezierQuadratic"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i7 == nil then
        i7 = 0
    end
    local out = C.ImDrawList_AddBezierQuadratic(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImDrawList["AddCallback"] = ImDrawList["AddCallback"] or function(i1, i2, i3)
    jit.off(true)
    if not ffi.istype("ImDrawCallback", i2) then
        local str = tostring(i2)
        _common.callbacks[str] = i2
        i2 = ffi.cast("ImDrawCallback", str)
    end
    local out = C.ImDrawList_AddCallback(i1, i2, i3)
    return out
end
ImDrawList["AddCircle"] = ImDrawList["AddCircle"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = 0
    end
    if i6 == nil then
        i6 = 1.0
    end
    local out = C.ImDrawList_AddCircle(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["AddCircleFilled"] = ImDrawList["AddCircleFilled"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = 0
    end
    local out = C.ImDrawList_AddCircleFilled(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["AddConcavePolyFilled"] = ImDrawList["AddConcavePolyFilled"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImDrawList_AddConcavePolyFilled(i1, i2, i3, i4)
    return out
end
ImDrawList["AddConvexPolyFilled"] = ImDrawList["AddConvexPolyFilled"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImDrawList_AddConvexPolyFilled(i1, i2, i3, i4)
    return out
end
ImDrawList["AddDrawCmd"] = ImDrawList["AddDrawCmd"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList_AddDrawCmd(i1)
    return out
end
ImDrawList["AddEllipse"] = ImDrawList["AddEllipse"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = 0
    end
    if i7 == nil then
        i7 = 1.0
    end
    local out = C.ImDrawList_AddEllipse(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImDrawList["AddEllipseFilled"] = ImDrawList["AddEllipseFilled"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.ImDrawList_AddEllipseFilled(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["AddImage"] = ImDrawList["AddImage"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i5 == nil then
        i5 = M.ImVec2_Float(0, 0)
    end
    if i6 == nil then
        i6 = M.ImVec2_Float(1, 1)
    end
    if i7 == nil then
        i7 = 4294967295
    end
    local ptr = ffi.cast("void *", i2)
    _common.textures[tostring(ptr)] = i2
    i2 = ptr
    local out = C.ImDrawList_AddImage(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImDrawList["AddImageQuad"] = ImDrawList["AddImageQuad"] or function(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11)
    jit.off(true)
    if i7 == nil then
        i7 = M.ImVec2_Float(0, 0)
    end
    if i8 == nil then
        i8 = M.ImVec2_Float(1, 0)
    end
    if i9 == nil then
        i9 = M.ImVec2_Float(1, 1)
    end
    if i10 == nil then
        i10 = M.ImVec2_Float(0, 1)
    end
    if i11 == nil then
        i11 = 4294967295
    end
    local ptr = ffi.cast("void *", i2)
    _common.textures[tostring(ptr)] = i2
    i2 = ptr
    local out = C.ImDrawList_AddImageQuad(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11)
    return out
end
ImDrawList["AddImageRounded"] = ImDrawList["AddImageRounded"] or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i9 == nil then
        i9 = 0
    end
    local ptr = ffi.cast("void *", i2)
    _common.textures[tostring(ptr)] = i2
    i2 = ptr
    local out = C.ImDrawList_AddImageRounded(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
ImDrawList["AddLine"] = ImDrawList["AddLine"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = 1.0
    end
    local out = C.ImDrawList_AddLine(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["AddNgon"] = ImDrawList["AddNgon"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i6 == nil then
        i6 = 1.0
    end
    local out = C.ImDrawList_AddNgon(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["AddNgonFilled"] = ImDrawList["AddNgonFilled"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.ImDrawList_AddNgonFilled(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["AddPolyline"] = ImDrawList["AddPolyline"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.ImDrawList_AddPolyline(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["AddQuad"] = ImDrawList["AddQuad"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i7 == nil then
        i7 = 1.0
    end
    local out = C.ImDrawList_AddQuad(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImDrawList["AddQuadFilled"] = ImDrawList["AddQuadFilled"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.ImDrawList_AddQuadFilled(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["AddRect"] = ImDrawList["AddRect"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = 0
    end
    if i7 == nil then
        i7 = 1.0
    end
    local out = C.ImDrawList_AddRect(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImDrawList["AddRectFilled"] = ImDrawList["AddRectFilled"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.ImDrawList_AddRectFilled(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["AddRectFilledMultiColor"] = ImDrawList["AddRectFilledMultiColor"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    local out = C.ImDrawList_AddRectFilledMultiColor(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImDrawList["AddText_Vec2"] = ImDrawList["AddText_Vec2"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.ImDrawList_AddText_Vec2(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["AddText_FontPtr"] = ImDrawList["AddText_FontPtr"] or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i8 == nil then
        i8 = 0.0
    end
    local out = C.ImDrawList_AddText_FontPtr(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
ImDrawList["AddTriangle"] = ImDrawList["AddTriangle"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i6 == nil then
        i6 = 1.0
    end
    local out = C.ImDrawList_AddTriangle(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["AddTriangleFilled"] = ImDrawList["AddTriangleFilled"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.ImDrawList_AddTriangleFilled(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["ChannelsMerge"] = ImDrawList["ChannelsMerge"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList_ChannelsMerge(i1)
    return out
end
ImDrawList["ChannelsSetCurrent"] = ImDrawList["ChannelsSetCurrent"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList_ChannelsSetCurrent(i1, i2)
    return out
end
ImDrawList["ChannelsSplit"] = ImDrawList["ChannelsSplit"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList_ChannelsSplit(i1, i2)
    return out
end
ImDrawList["CloneOutput"] = ImDrawList["CloneOutput"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList_CloneOutput(i1)
    return out
end
ImDrawList["GetClipRectMax"] = ImDrawList["GetClipRectMax"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImDrawList_GetClipRectMax(o1, i1)
    return o1, out
end
ImDrawList["GetClipRectMin"] = ImDrawList["GetClipRectMin"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImDrawList_GetClipRectMin(o1, i1)
    return o1, out
end
ImDrawList["PathArcTo"] = ImDrawList["PathArcTo"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i6 == nil then
        i6 = 0
    end
    local out = C.ImDrawList_PathArcTo(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["PathArcToFast"] = ImDrawList["PathArcToFast"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.ImDrawList_PathArcToFast(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["PathBezierCubicCurveTo"] = ImDrawList["PathBezierCubicCurveTo"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = 0
    end
    local out = C.ImDrawList_PathBezierCubicCurveTo(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["PathBezierQuadraticCurveTo"] = ImDrawList["PathBezierQuadraticCurveTo"] or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    local out = C.ImDrawList_PathBezierQuadraticCurveTo(i1, i2, i3, i4)
    return out
end
ImDrawList["PathClear"] = ImDrawList["PathClear"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList_PathClear(i1)
    return out
end
ImDrawList["PathEllipticalArcTo"] = ImDrawList["PathEllipticalArcTo"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i7 == nil then
        i7 = 0
    end
    local out = C.ImDrawList_PathEllipticalArcTo(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImDrawList["PathFillConcave"] = ImDrawList["PathFillConcave"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList_PathFillConcave(i1, i2)
    return out
end
ImDrawList["PathFillConvex"] = ImDrawList["PathFillConvex"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList_PathFillConvex(i1, i2)
    return out
end
ImDrawList["PathLineTo"] = ImDrawList["PathLineTo"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList_PathLineTo(i1, i2)
    return out
end
ImDrawList["PathLineToMergeDuplicate"] = ImDrawList["PathLineToMergeDuplicate"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList_PathLineToMergeDuplicate(i1, i2)
    return out
end
ImDrawList["PathRect"] = ImDrawList["PathRect"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i4 == nil then
        i4 = 0.0
    end
    if i5 == nil then
        i5 = 0
    end
    local out = C.ImDrawList_PathRect(i1, i2, i3, i4, i5)
    return out
end
ImDrawList["PathStroke"] = ImDrawList["PathStroke"] or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    if i4 == nil then
        i4 = 1.0
    end
    local out = C.ImDrawList_PathStroke(i1, i2, i3, i4)
    return out
end
ImDrawList["PopClipRect"] = ImDrawList["PopClipRect"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList_PopClipRect(i1)
    return out
end
ImDrawList["PopTextureID"] = ImDrawList["PopTextureID"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList_PopTextureID(i1)
    return out
end
ImDrawList["PrimQuadUV"] = ImDrawList["PrimQuadUV"] or function(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    jit.off(true)
    local out = C.ImDrawList_PrimQuadUV(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    return out
end
ImDrawList["PrimRect"] = ImDrawList["PrimRect"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImDrawList_PrimRect(i1, i2, i3, i4)
    return out
end
ImDrawList["PrimRectUV"] = ImDrawList["PrimRectUV"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.ImDrawList_PrimRectUV(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["PrimReserve"] = ImDrawList["PrimReserve"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImDrawList_PrimReserve(i1, i2, i3)
    return out
end
ImDrawList["PrimUnreserve"] = ImDrawList["PrimUnreserve"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImDrawList_PrimUnreserve(i1, i2, i3)
    return out
end
ImDrawList["PrimVtx"] = ImDrawList["PrimVtx"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImDrawList_PrimVtx(i1, i2, i3, i4)
    return out
end
ImDrawList["PrimWriteIdx"] = ImDrawList["PrimWriteIdx"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList_PrimWriteIdx(i1, i2)
    return out
end
ImDrawList["PrimWriteVtx"] = ImDrawList["PrimWriteVtx"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImDrawList_PrimWriteVtx(i1, i2, i3, i4)
    return out
end
ImDrawList["PushClipRect"] = ImDrawList["PushClipRect"] or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = false
    end
    local out = C.ImDrawList_PushClipRect(i1, i2, i3, i4)
    return out
end
ImDrawList["PushClipRectFullScreen"] = ImDrawList["PushClipRectFullScreen"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList_PushClipRectFullScreen(i1)
    return out
end
ImDrawList["PushTextureID"] = ImDrawList["PushTextureID"] or function(i1, i2)
    jit.off(true)
    local ptr = ffi.cast("void *", i2)
    _common.textures[tostring(ptr)] = i2
    i2 = ptr
    local out = C.ImDrawList_PushTextureID(i1, i2)
    return out
end
ImDrawList["_CalcCircleAutoSegmentCount"] = ImDrawList["_CalcCircleAutoSegmentCount"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawList__CalcCircleAutoSegmentCount(i1, i2)
    return out
end
ImDrawList["_ClearFreeMemory"] = ImDrawList["_ClearFreeMemory"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList__ClearFreeMemory(i1)
    return out
end
ImDrawList["_OnChangedClipRect"] = ImDrawList["_OnChangedClipRect"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList__OnChangedClipRect(i1)
    return out
end
ImDrawList["_OnChangedTextureID"] = ImDrawList["_OnChangedTextureID"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList__OnChangedTextureID(i1)
    return out
end
ImDrawList["_OnChangedVtxOffset"] = ImDrawList["_OnChangedVtxOffset"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList__OnChangedVtxOffset(i1)
    return out
end
ImDrawList["_PathArcToFastEx"] = ImDrawList["_PathArcToFastEx"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.ImDrawList__PathArcToFastEx(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["_PathArcToN"] = ImDrawList["_PathArcToN"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.ImDrawList__PathArcToN(i1, i2, i3, i4, i5, i6)
    return out
end
ImDrawList["_PopUnusedDrawCmd"] = ImDrawList["_PopUnusedDrawCmd"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList__PopUnusedDrawCmd(i1)
    return out
end
ImDrawList["_ResetForNewFrame"] = ImDrawList["_ResetForNewFrame"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList__ResetForNewFrame(i1)
    return out
end
ImDrawList["_TryMergeDrawCmds"] = ImDrawList["_TryMergeDrawCmds"] or function(i1)
    jit.off(true)
    local out = C.ImDrawList__TryMergeDrawCmds(i1)
    return out
end
local mt = getmetatable(ImDrawList) or {}
mt.__call = mt.__call or function(self, i1)
    jit.off(true)
    local p = C.ImDrawList_ImDrawList(i1)
    return ffi.gc(p[0], C.ImDrawList_destroy)
end
setmetatable(ImDrawList, mt)
M.ImDrawList = ImDrawList
ffi.metatype("ImDrawList", ImDrawList)

local ImDrawListSharedData = ImDrawListSharedData or {}
ImDrawListSharedData.__index = ImDrawListSharedData
ImDrawListSharedData["SetCircleTessellationMaxError"] = ImDrawListSharedData["SetCircleTessellationMaxError"] or
                                                            function(i1, i2)
        jit.off(true)
        local out = C.ImDrawListSharedData_SetCircleTessellationMaxError(i1, i2)
        return out
    end
local mt = getmetatable(ImDrawListSharedData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImDrawListSharedData_ImDrawListSharedData()
    return ffi.gc(p[0], C.ImDrawListSharedData_destroy)
end
setmetatable(ImDrawListSharedData, mt)
M.ImDrawListSharedData = ImDrawListSharedData
ffi.metatype("ImDrawListSharedData", ImDrawListSharedData)

local ImDrawListSplitter = ImDrawListSplitter or {}
ImDrawListSplitter.__index = ImDrawListSplitter
ImDrawListSplitter["Clear"] = ImDrawListSplitter["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImDrawListSplitter_Clear(i1)
    return out
end
ImDrawListSplitter["ClearFreeMemory"] = ImDrawListSplitter["ClearFreeMemory"] or function(i1)
    jit.off(true)
    local out = C.ImDrawListSplitter_ClearFreeMemory(i1)
    return out
end
ImDrawListSplitter["Merge"] = ImDrawListSplitter["Merge"] or function(i1, i2)
    jit.off(true)
    local out = C.ImDrawListSplitter_Merge(i1, i2)
    return out
end
ImDrawListSplitter["SetCurrentChannel"] = ImDrawListSplitter["SetCurrentChannel"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImDrawListSplitter_SetCurrentChannel(i1, i2, i3)
    return out
end
ImDrawListSplitter["Split"] = ImDrawListSplitter["Split"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImDrawListSplitter_Split(i1, i2, i3)
    return out
end
local mt = getmetatable(ImDrawListSplitter) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImDrawListSplitter_ImDrawListSplitter()
    return ffi.gc(p[0], C.ImDrawListSplitter_destroy)
end
setmetatable(ImDrawListSplitter, mt)
M.ImDrawListSplitter = ImDrawListSplitter
ffi.metatype("ImDrawListSplitter", ImDrawListSplitter)

local ImFont = ImFont or {}
ImFont.__index = ImFont
ImFont["AddGlyph"] = ImFont["AddGlyph"] or function(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12)
    jit.off(true)
    local out = C.ImFont_AddGlyph(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12)
    return out
end
ImFont["AddRemapChar"] = ImFont["AddRemapChar"] or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = true
    end
    local out = C.ImFont_AddRemapChar(i1, i2, i3, i4)
    return out
end
ImFont["BuildLookupTable"] = ImFont["BuildLookupTable"] or function(i1)
    jit.off(true)
    local out = C.ImFont_BuildLookupTable(i1)
    return out
end
ImFont["CalcTextSizeA"] = ImFont["CalcTextSizeA"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImFont_CalcTextSizeA(o1, i1, i2, i3, i4, i5, i6, i7)
    return o1, out
end
ImFont["CalcWordWrapPositionA"] = ImFont["CalcWordWrapPositionA"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.ImFont_CalcWordWrapPositionA(i1, i2, i3, i4, i5)
    return out
end
ImFont["ClearOutputData"] = ImFont["ClearOutputData"] or function(i1)
    jit.off(true)
    local out = C.ImFont_ClearOutputData(i1)
    return out
end
ImFont["FindGlyph"] = ImFont["FindGlyph"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFont_FindGlyph(i1, i2)
    return out
end
ImFont["FindGlyphNoFallback"] = ImFont["FindGlyphNoFallback"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFont_FindGlyphNoFallback(i1, i2)
    return out
end
ImFont["GetCharAdvance"] = ImFont["GetCharAdvance"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFont_GetCharAdvance(i1, i2)
    return out
end
ImFont["GetDebugName"] = ImFont["GetDebugName"] or function(i1)
    jit.off(true)
    local out = C.ImFont_GetDebugName(i1)
    return out
end
ImFont["GrowIndex"] = ImFont["GrowIndex"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFont_GrowIndex(i1, i2)
    return out
end
ImFont["IsGlyphRangeUnused"] = ImFont["IsGlyphRangeUnused"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImFont_IsGlyphRangeUnused(i1, i2, i3)
    return out
end
ImFont["IsLoaded"] = ImFont["IsLoaded"] or function(i1)
    jit.off(true)
    local out = C.ImFont_IsLoaded(i1)
    return out
end
ImFont["RenderChar"] = ImFont["RenderChar"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.ImFont_RenderChar(i1, i2, i3, i4, i5, i6)
    return out
end
ImFont["RenderText"] = ImFont["RenderText"] or function(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    jit.off(true)
    if i9 == nil then
        i9 = 0.0
    end
    if i10 == nil then
        i10 = false
    end
    local out = C.ImFont_RenderText(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    return out
end
ImFont["SetGlyphVisible"] = ImFont["SetGlyphVisible"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImFont_SetGlyphVisible(i1, i2, i3)
    return out
end
local mt = getmetatable(ImFont) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImFont_ImFont()
    return ffi.gc(p[0], C.ImFont_destroy)
end
setmetatable(ImFont, mt)
M.ImFont = ImFont
ffi.metatype("ImFont", ImFont)

local ImFontAtlas = ImFontAtlas or {}
ImFontAtlas.__index = ImFontAtlas
ImFontAtlas["AddCustomRectFontGlyph"] = ImFontAtlas["AddCustomRectFontGlyph"] or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i7 == nil then
        i7 = M.ImVec2_Float(0, 0)
    end
    local out = C.ImFontAtlas_AddCustomRectFontGlyph(i1, i2, i3, i4, i5, i6, i7)
    return out
end
ImFontAtlas["AddCustomRectRegular"] = ImFontAtlas["AddCustomRectRegular"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImFontAtlas_AddCustomRectRegular(i1, i2, i3)
    return out
end
ImFontAtlas["AddFont"] = ImFontAtlas["AddFont"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFontAtlas_AddFont(i1, i2)
    return out
end
ImFontAtlas["AddFontDefault"] = ImFontAtlas["AddFontDefault"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFontAtlas_AddFontDefault(i1, i2)
    return out
end
ImFontAtlas["AddFontFromFileTTF"] = ImFontAtlas["AddFontFromFileTTF"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.ImFontAtlas_AddFontFromFileTTF(i1, i2, i3, i4, i5)
    return out
end
ImFontAtlas["AddFontFromMemoryCompressedBase85TTF"] = ImFontAtlas["AddFontFromMemoryCompressedBase85TTF"] or
                                                          function(i1, i2, i3, i4, i5)
        jit.off(true)
        local out = C.ImFontAtlas_AddFontFromMemoryCompressedBase85TTF(i1, i2, i3, i4, i5)
        return out
    end
ImFontAtlas["AddFontFromMemoryCompressedTTF"] = ImFontAtlas["AddFontFromMemoryCompressedTTF"] or
                                                    function(i1, i2, i3, i4, i5, i6)
        jit.off(true)
        local out = C.ImFontAtlas_AddFontFromMemoryCompressedTTF(i1, i2, i3, i4, i5, i6)
        return out
    end
ImFontAtlas["AddFontFromMemoryTTF"] = ImFontAtlas["AddFontFromMemoryTTF"] or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.ImFontAtlas_AddFontFromMemoryTTF(i1, i2, i3, i4, i5, i6)
    return out
end
ImFontAtlas["Build"] = ImFontAtlas["Build"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_Build(i1)
    return out
end
ImFontAtlas["CalcCustomRectUV"] = ImFontAtlas["CalcCustomRectUV"] or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local o2 = M.ImVec2_Nil()
    local out = C.ImFontAtlas_CalcCustomRectUV(i1, i2, o1, o2)
    return o1, o2, out
end
ImFontAtlas["Clear"] = ImFontAtlas["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_Clear(i1)
    return out
end
ImFontAtlas["ClearFonts"] = ImFontAtlas["ClearFonts"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_ClearFonts(i1)
    return out
end
ImFontAtlas["ClearInputData"] = ImFontAtlas["ClearInputData"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_ClearInputData(i1)
    return out
end
ImFontAtlas["ClearTexData"] = ImFontAtlas["ClearTexData"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_ClearTexData(i1)
    return out
end
ImFontAtlas["GetCustomRectByIndex"] = ImFontAtlas["GetCustomRectByIndex"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFontAtlas_GetCustomRectByIndex(i1, i2)
    return out
end
ImFontAtlas["GetGlyphRangesChineseFull"] = ImFontAtlas["GetGlyphRangesChineseFull"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesChineseFull(i1)
    return out
end
ImFontAtlas["GetGlyphRangesChineseSimplifiedCommon"] = ImFontAtlas["GetGlyphRangesChineseSimplifiedCommon"] or
                                                           function(i1)
        jit.off(true)
        local out = C.ImFontAtlas_GetGlyphRangesChineseSimplifiedCommon(i1)
        return out
    end
ImFontAtlas["GetGlyphRangesCyrillic"] = ImFontAtlas["GetGlyphRangesCyrillic"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesCyrillic(i1)
    return out
end
ImFontAtlas["GetGlyphRangesDefault"] = ImFontAtlas["GetGlyphRangesDefault"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesDefault(i1)
    return out
end
ImFontAtlas["GetGlyphRangesGreek"] = ImFontAtlas["GetGlyphRangesGreek"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesGreek(i1)
    return out
end
ImFontAtlas["GetGlyphRangesJapanese"] = ImFontAtlas["GetGlyphRangesJapanese"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesJapanese(i1)
    return out
end
ImFontAtlas["GetGlyphRangesKorean"] = ImFontAtlas["GetGlyphRangesKorean"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesKorean(i1)
    return out
end
ImFontAtlas["GetGlyphRangesThai"] = ImFontAtlas["GetGlyphRangesThai"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesThai(i1)
    return out
end
ImFontAtlas["GetGlyphRangesVietnamese"] = ImFontAtlas["GetGlyphRangesVietnamese"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_GetGlyphRangesVietnamese(i1)
    return out
end
ImFontAtlas["GetMouseCursorTexData"] = ImFontAtlas["GetMouseCursorTexData"] or function(i1, i2, i3, i4)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local o2 = M.ImVec2_Nil()
    local out = C.ImFontAtlas_GetMouseCursorTexData(i1, i2, o1, o2, i3, i4)
    return o1, o2, out
end
ImFontAtlas["GetTexDataAsAlpha8"] = ImFontAtlas["GetTexDataAsAlpha8"] or function(i1)
    jit.off(true)
    local o1 = ffi.new("unsigned char*[1]")
    local o2 = ffi.new("int[1]")
    local o3 = ffi.new("int[1]")
    local o4 = ffi.new("int[1]")
    local out = C.ImFontAtlas_GetTexDataAsAlpha8(i1, o1, o2, o3, o4)
    return o1[0], o2[0], o3[0], o4[0], out
end
ImFontAtlas["GetTexDataAsRGBA32"] = ImFontAtlas["GetTexDataAsRGBA32"] or function(i1)
    jit.off(true)
    local o1 = ffi.new("unsigned char*[1]")
    local o2 = ffi.new("int[1]")
    local o3 = ffi.new("int[1]")
    local o4 = ffi.new("int[1]")
    local out = C.ImFontAtlas_GetTexDataAsRGBA32(i1, o1, o2, o3, o4)
    return o1[0], o2[0], o3[0], o4[0], out
end
ImFontAtlas["IsBuilt"] = ImFontAtlas["IsBuilt"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlas_IsBuilt(i1)
    return out
end
ImFontAtlas["SetTexID"] = ImFontAtlas["SetTexID"] or function(i1, i2)
    jit.off(true)
    local ptr = ffi.cast("void *", i2)
    _common.textures[tostring(ptr)] = i2
    i2 = ptr
    local out = C.ImFontAtlas_SetTexID(i1, i2)
    return out
end
local mt = getmetatable(ImFontAtlas) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImFontAtlas_ImFontAtlas()
    return ffi.gc(p[0], C.ImFontAtlas_destroy)
end
setmetatable(ImFontAtlas, mt)
M.ImFontAtlas = ImFontAtlas
ffi.metatype("ImFontAtlas", ImFontAtlas)

local ImFontAtlasCustomRect = ImFontAtlasCustomRect or {}
ImFontAtlasCustomRect.__index = ImFontAtlasCustomRect
ImFontAtlasCustomRect["IsPacked"] = ImFontAtlasCustomRect["IsPacked"] or function(i1)
    jit.off(true)
    local out = C.ImFontAtlasCustomRect_IsPacked(i1)
    return out
end
local mt = getmetatable(ImFontAtlasCustomRect) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImFontAtlasCustomRect_ImFontAtlasCustomRect()
    return ffi.gc(p[0], C.ImFontAtlasCustomRect_destroy)
end
setmetatable(ImFontAtlasCustomRect, mt)
M.ImFontAtlasCustomRect = ImFontAtlasCustomRect
ffi.metatype("ImFontAtlasCustomRect", ImFontAtlasCustomRect)

local ImFontConfig = ImFontConfig or {}
ImFontConfig.__index = ImFontConfig
local mt = getmetatable(ImFontConfig) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImFontConfig_ImFontConfig()
    return ffi.gc(p[0], C.ImFontConfig_destroy)
end
setmetatable(ImFontConfig, mt)
M.ImFontConfig = ImFontConfig
ffi.metatype("ImFontConfig", ImFontConfig)

local ImFontGlyphRangesBuilder = ImFontGlyphRangesBuilder or {}
ImFontGlyphRangesBuilder.__index = ImFontGlyphRangesBuilder
ImFontGlyphRangesBuilder["AddChar"] = ImFontGlyphRangesBuilder["AddChar"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFontGlyphRangesBuilder_AddChar(i1, i2)
    return out
end
ImFontGlyphRangesBuilder["AddRanges"] = ImFontGlyphRangesBuilder["AddRanges"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFontGlyphRangesBuilder_AddRanges(i1, i2)
    return out
end
ImFontGlyphRangesBuilder["AddText"] = ImFontGlyphRangesBuilder["AddText"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImFontGlyphRangesBuilder_AddText(i1, i2, i3)
    return out
end
ImFontGlyphRangesBuilder["BuildRanges"] = ImFontGlyphRangesBuilder["BuildRanges"] or function(i1)
    jit.off(true)
    local o1 = M.ImVector_ImWchar()
    local out = C.ImFontGlyphRangesBuilder_BuildRanges(i1, o1)
    return o1, out
end
ImFontGlyphRangesBuilder["Clear"] = ImFontGlyphRangesBuilder["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImFontGlyphRangesBuilder_Clear(i1)
    return out
end
ImFontGlyphRangesBuilder["GetBit"] = ImFontGlyphRangesBuilder["GetBit"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFontGlyphRangesBuilder_GetBit(i1, i2)
    return out
end
ImFontGlyphRangesBuilder["SetBit"] = ImFontGlyphRangesBuilder["SetBit"] or function(i1, i2)
    jit.off(true)
    local out = C.ImFontGlyphRangesBuilder_SetBit(i1, i2)
    return out
end
local mt = getmetatable(ImFontGlyphRangesBuilder) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImFontGlyphRangesBuilder_ImFontGlyphRangesBuilder()
    return ffi.gc(p[0], C.ImFontGlyphRangesBuilder_destroy)
end
setmetatable(ImFontGlyphRangesBuilder, mt)
M.ImFontGlyphRangesBuilder = ImFontGlyphRangesBuilder
ffi.metatype("ImFontGlyphRangesBuilder", ImFontGlyphRangesBuilder)

local ImGuiBoxSelectState = ImGuiBoxSelectState or {}
ImGuiBoxSelectState.__index = ImGuiBoxSelectState
local mt = getmetatable(ImGuiBoxSelectState) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiBoxSelectState_ImGuiBoxSelectState()
    return ffi.gc(p[0], C.ImGuiBoxSelectState_destroy)
end
setmetatable(ImGuiBoxSelectState, mt)
M.ImGuiBoxSelectState = ImGuiBoxSelectState
ffi.metatype("ImGuiBoxSelectState", ImGuiBoxSelectState)

local ImGuiComboPreviewData = ImGuiComboPreviewData or {}
ImGuiComboPreviewData.__index = ImGuiComboPreviewData
local mt = getmetatable(ImGuiComboPreviewData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiComboPreviewData_ImGuiComboPreviewData()
    return ffi.gc(p[0], C.ImGuiComboPreviewData_destroy)
end
setmetatable(ImGuiComboPreviewData, mt)
M.ImGuiComboPreviewData = ImGuiComboPreviewData
ffi.metatype("ImGuiComboPreviewData", ImGuiComboPreviewData)

local ImGuiContext = ImGuiContext or {}
ImGuiContext.__index = ImGuiContext
local mt = getmetatable(ImGuiContext) or {}
mt.__call = mt.__call or function(self, i1)
    jit.off(true)
    local p = C.ImGuiContext_ImGuiContext(i1)
    return ffi.gc(p[0], C.ImGuiContext_destroy)
end
setmetatable(ImGuiContext, mt)
M.ImGuiContext = ImGuiContext
ffi.metatype("ImGuiContext", ImGuiContext)

local ImGuiContextHook = ImGuiContextHook or {}
ImGuiContextHook.__index = ImGuiContextHook
local mt = getmetatable(ImGuiContextHook) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiContextHook_ImGuiContextHook()
    return ffi.gc(p[0], C.ImGuiContextHook_destroy)
end
setmetatable(ImGuiContextHook, mt)
M.ImGuiContextHook = ImGuiContextHook
ffi.metatype("ImGuiContextHook", ImGuiContextHook)

local ImGuiDataVarInfo = ImGuiDataVarInfo or {}
ImGuiDataVarInfo.__index = ImGuiDataVarInfo
ImGuiDataVarInfo["GetVarPtr"] = ImGuiDataVarInfo["GetVarPtr"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiDataVarInfo_GetVarPtr(i1, i2)
    return out
end
M.ImGuiDataVarInfo = ImGuiDataVarInfo
ffi.metatype("ImGuiDataVarInfo", ImGuiDataVarInfo)

local ImGuiDebugAllocInfo = ImGuiDebugAllocInfo or {}
ImGuiDebugAllocInfo.__index = ImGuiDebugAllocInfo
local mt = getmetatable(ImGuiDebugAllocInfo) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiDebugAllocInfo_ImGuiDebugAllocInfo()
    return ffi.gc(p[0], C.ImGuiDebugAllocInfo_destroy)
end
setmetatable(ImGuiDebugAllocInfo, mt)
M.ImGuiDebugAllocInfo = ImGuiDebugAllocInfo
ffi.metatype("ImGuiDebugAllocInfo", ImGuiDebugAllocInfo)

local ImGuiDockContext = ImGuiDockContext or {}
ImGuiDockContext.__index = ImGuiDockContext
local mt = getmetatable(ImGuiDockContext) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiDockContext_ImGuiDockContext()
    return ffi.gc(p[0], C.ImGuiDockContext_destroy)
end
setmetatable(ImGuiDockContext, mt)
M.ImGuiDockContext = ImGuiDockContext
ffi.metatype("ImGuiDockContext", ImGuiDockContext)

local ImGuiDockNode = ImGuiDockNode or {}
ImGuiDockNode.__index = ImGuiDockNode
ImGuiDockNode["IsCentralNode"] = ImGuiDockNode["IsCentralNode"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsCentralNode(i1)
    return out
end
ImGuiDockNode["IsDockSpace"] = ImGuiDockNode["IsDockSpace"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsDockSpace(i1)
    return out
end
ImGuiDockNode["IsEmpty"] = ImGuiDockNode["IsEmpty"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsEmpty(i1)
    return out
end
ImGuiDockNode["IsFloatingNode"] = ImGuiDockNode["IsFloatingNode"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsFloatingNode(i1)
    return out
end
ImGuiDockNode["IsHiddenTabBar"] = ImGuiDockNode["IsHiddenTabBar"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsHiddenTabBar(i1)
    return out
end
ImGuiDockNode["IsLeafNode"] = ImGuiDockNode["IsLeafNode"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsLeafNode(i1)
    return out
end
ImGuiDockNode["IsNoTabBar"] = ImGuiDockNode["IsNoTabBar"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsNoTabBar(i1)
    return out
end
ImGuiDockNode["IsRootNode"] = ImGuiDockNode["IsRootNode"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsRootNode(i1)
    return out
end
ImGuiDockNode["IsSplitNode"] = ImGuiDockNode["IsSplitNode"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_IsSplitNode(i1)
    return out
end
ImGuiDockNode["Rect"] = ImGuiDockNode["Rect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiDockNode_Rect(i1, i2)
    return out
end
ImGuiDockNode["SetLocalFlags"] = ImGuiDockNode["SetLocalFlags"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiDockNode_SetLocalFlags(i1, i2)
    return out
end
ImGuiDockNode["UpdateMergedFlags"] = ImGuiDockNode["UpdateMergedFlags"] or function(i1)
    jit.off(true)
    local out = C.ImGuiDockNode_UpdateMergedFlags(i1)
    return out
end
local mt = getmetatable(ImGuiDockNode) or {}
mt.__call = mt.__call or function(self, i1)
    jit.off(true)
    local p = C.ImGuiDockNode_ImGuiDockNode(i1)
    return ffi.gc(p[0], C.ImGuiDockNode_destroy)
end
setmetatable(ImGuiDockNode, mt)
M.ImGuiDockNode = ImGuiDockNode
ffi.metatype("ImGuiDockNode", ImGuiDockNode)

local ImGuiIDStackTool = ImGuiIDStackTool or {}
ImGuiIDStackTool.__index = ImGuiIDStackTool
local mt = getmetatable(ImGuiIDStackTool) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiIDStackTool_ImGuiIDStackTool()
    return ffi.gc(p[0], C.ImGuiIDStackTool_destroy)
end
setmetatable(ImGuiIDStackTool, mt)
M.ImGuiIDStackTool = ImGuiIDStackTool
ffi.metatype("ImGuiIDStackTool", ImGuiIDStackTool)

local ImGuiIO = ImGuiIO or {}
ImGuiIO.__index = ImGuiIO
ImGuiIO["AddFocusEvent"] = ImGuiIO["AddFocusEvent"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiIO_AddFocusEvent(i1, i2)
    return out
end
ImGuiIO["AddInputCharacter"] = ImGuiIO["AddInputCharacter"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiIO_AddInputCharacter(i1, i2)
    return out
end
ImGuiIO["AddInputCharacterUTF16"] = ImGuiIO["AddInputCharacterUTF16"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiIO_AddInputCharacterUTF16(i1, i2)
    return out
end
ImGuiIO["AddInputCharactersUTF8"] = ImGuiIO["AddInputCharactersUTF8"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiIO_AddInputCharactersUTF8(i1, i2)
    return out
end
ImGuiIO["AddKeyAnalogEvent"] = ImGuiIO["AddKeyAnalogEvent"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImGuiIO_AddKeyAnalogEvent(i1, i2, i3, i4)
    return out
end
ImGuiIO["AddKeyEvent"] = ImGuiIO["AddKeyEvent"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiIO_AddKeyEvent(i1, i2, i3)
    return out
end
ImGuiIO["AddMouseButtonEvent"] = ImGuiIO["AddMouseButtonEvent"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiIO_AddMouseButtonEvent(i1, i2, i3)
    return out
end
ImGuiIO["AddMousePosEvent"] = ImGuiIO["AddMousePosEvent"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiIO_AddMousePosEvent(i1, i2, i3)
    return out
end
ImGuiIO["AddMouseSourceEvent"] = ImGuiIO["AddMouseSourceEvent"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiIO_AddMouseSourceEvent(i1, i2)
    return out
end
ImGuiIO["AddMouseViewportEvent"] = ImGuiIO["AddMouseViewportEvent"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiIO_AddMouseViewportEvent(i1, i2)
    return out
end
ImGuiIO["AddMouseWheelEvent"] = ImGuiIO["AddMouseWheelEvent"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiIO_AddMouseWheelEvent(i1, i2, i3)
    return out
end
ImGuiIO["ClearEventsQueue"] = ImGuiIO["ClearEventsQueue"] or function(i1)
    jit.off(true)
    local out = C.ImGuiIO_ClearEventsQueue(i1)
    return out
end
ImGuiIO["ClearInputKeys"] = ImGuiIO["ClearInputKeys"] or function(i1)
    jit.off(true)
    local out = C.ImGuiIO_ClearInputKeys(i1)
    return out
end
ImGuiIO["ClearInputMouse"] = ImGuiIO["ClearInputMouse"] or function(i1)
    jit.off(true)
    local out = C.ImGuiIO_ClearInputMouse(i1)
    return out
end
ImGuiIO["SetAppAcceptingEvents"] = ImGuiIO["SetAppAcceptingEvents"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiIO_SetAppAcceptingEvents(i1, i2)
    return out
end
ImGuiIO["SetKeyEventNativeData"] = ImGuiIO["SetKeyEventNativeData"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = -1
    end
    local out = C.ImGuiIO_SetKeyEventNativeData(i1, i2, i3, i4, i5)
    return out
end
local mt = getmetatable(ImGuiIO) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiIO_ImGuiIO()
    return ffi.gc(p[0], C.ImGuiIO_destroy)
end
setmetatable(ImGuiIO, mt)
M.ImGuiIO = ImGuiIO
ffi.metatype("ImGuiIO", ImGuiIO)

local ImGuiInputEvent = ImGuiInputEvent or {}
ImGuiInputEvent.__index = ImGuiInputEvent
local mt = getmetatable(ImGuiInputEvent) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiInputEvent_ImGuiInputEvent()
    return ffi.gc(p[0], C.ImGuiInputEvent_destroy)
end
setmetatable(ImGuiInputEvent, mt)
M.ImGuiInputEvent = ImGuiInputEvent
ffi.metatype("ImGuiInputEvent", ImGuiInputEvent)

local ImGuiInputTextCallbackData = ImGuiInputTextCallbackData or {}
ImGuiInputTextCallbackData.__index = ImGuiInputTextCallbackData
ImGuiInputTextCallbackData["ClearSelection"] = ImGuiInputTextCallbackData["ClearSelection"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextCallbackData_ClearSelection(i1)
    return out
end
ImGuiInputTextCallbackData["DeleteChars"] = ImGuiInputTextCallbackData["DeleteChars"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiInputTextCallbackData_DeleteChars(i1, i2, i3)
    return out
end
ImGuiInputTextCallbackData["HasSelection"] = ImGuiInputTextCallbackData["HasSelection"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextCallbackData_HasSelection(i1)
    return out
end
ImGuiInputTextCallbackData["InsertChars"] = ImGuiInputTextCallbackData["InsertChars"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImGuiInputTextCallbackData_InsertChars(i1, i2, i3, i4)
    return out
end
ImGuiInputTextCallbackData["SelectAll"] = ImGuiInputTextCallbackData["SelectAll"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextCallbackData_SelectAll(i1)
    return out
end
local mt = getmetatable(ImGuiInputTextCallbackData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiInputTextCallbackData_ImGuiInputTextCallbackData()
    return ffi.gc(p[0], C.ImGuiInputTextCallbackData_destroy)
end
setmetatable(ImGuiInputTextCallbackData, mt)
M.ImGuiInputTextCallbackData = ImGuiInputTextCallbackData
ffi.metatype("ImGuiInputTextCallbackData", ImGuiInputTextCallbackData)

local ImGuiInputTextDeactivatedState = ImGuiInputTextDeactivatedState or {}
ImGuiInputTextDeactivatedState.__index = ImGuiInputTextDeactivatedState
ImGuiInputTextDeactivatedState["ClearFreeMemory"] = ImGuiInputTextDeactivatedState["ClearFreeMemory"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextDeactivatedState_ClearFreeMemory(i1)
    return out
end
local mt = getmetatable(ImGuiInputTextDeactivatedState) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiInputTextDeactivatedState_ImGuiInputTextDeactivatedState()
    return ffi.gc(p[0], C.ImGuiInputTextDeactivatedState_destroy)
end
setmetatable(ImGuiInputTextDeactivatedState, mt)
M.ImGuiInputTextDeactivatedState = ImGuiInputTextDeactivatedState
ffi.metatype("ImGuiInputTextDeactivatedState", ImGuiInputTextDeactivatedState)

local ImGuiInputTextState = ImGuiInputTextState or {}
ImGuiInputTextState.__index = ImGuiInputTextState
ImGuiInputTextState["ClearFreeMemory"] = ImGuiInputTextState["ClearFreeMemory"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_ClearFreeMemory(i1)
    return out
end
ImGuiInputTextState["ClearSelection"] = ImGuiInputTextState["ClearSelection"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_ClearSelection(i1)
    return out
end
ImGuiInputTextState["ClearText"] = ImGuiInputTextState["ClearText"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_ClearText(i1)
    return out
end
ImGuiInputTextState["CursorAnimReset"] = ImGuiInputTextState["CursorAnimReset"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_CursorAnimReset(i1)
    return out
end
ImGuiInputTextState["CursorClamp"] = ImGuiInputTextState["CursorClamp"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_CursorClamp(i1)
    return out
end
ImGuiInputTextState["GetCursorPos"] = ImGuiInputTextState["GetCursorPos"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_GetCursorPos(i1)
    return out
end
ImGuiInputTextState["GetRedoAvailCount"] = ImGuiInputTextState["GetRedoAvailCount"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_GetRedoAvailCount(i1)
    return out
end
ImGuiInputTextState["GetSelectionEnd"] = ImGuiInputTextState["GetSelectionEnd"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_GetSelectionEnd(i1)
    return out
end
ImGuiInputTextState["GetSelectionStart"] = ImGuiInputTextState["GetSelectionStart"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_GetSelectionStart(i1)
    return out
end
ImGuiInputTextState["GetUndoAvailCount"] = ImGuiInputTextState["GetUndoAvailCount"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_GetUndoAvailCount(i1)
    return out
end
ImGuiInputTextState["HasSelection"] = ImGuiInputTextState["HasSelection"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_HasSelection(i1)
    return out
end
ImGuiInputTextState["OnKeyPressed"] = ImGuiInputTextState["OnKeyPressed"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiInputTextState_OnKeyPressed(i1, i2)
    return out
end
ImGuiInputTextState["ReloadUserBufAndKeepSelection"] = ImGuiInputTextState["ReloadUserBufAndKeepSelection"] or
                                                           function(i1)
        jit.off(true)
        local out = C.ImGuiInputTextState_ReloadUserBufAndKeepSelection(i1)
        return out
    end
ImGuiInputTextState["ReloadUserBufAndMoveToEnd"] = ImGuiInputTextState["ReloadUserBufAndMoveToEnd"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_ReloadUserBufAndMoveToEnd(i1)
    return out
end
ImGuiInputTextState["ReloadUserBufAndSelectAll"] = ImGuiInputTextState["ReloadUserBufAndSelectAll"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_ReloadUserBufAndSelectAll(i1)
    return out
end
ImGuiInputTextState["SelectAll"] = ImGuiInputTextState["SelectAll"] or function(i1)
    jit.off(true)
    local out = C.ImGuiInputTextState_SelectAll(i1)
    return out
end
local mt = getmetatable(ImGuiInputTextState) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiInputTextState_ImGuiInputTextState()
    return ffi.gc(p[0], C.ImGuiInputTextState_destroy)
end
setmetatable(ImGuiInputTextState, mt)
M.ImGuiInputTextState = ImGuiInputTextState
ffi.metatype("ImGuiInputTextState", ImGuiInputTextState)

local ImGuiKeyOwnerData = ImGuiKeyOwnerData or {}
ImGuiKeyOwnerData.__index = ImGuiKeyOwnerData
local mt = getmetatable(ImGuiKeyOwnerData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiKeyOwnerData_ImGuiKeyOwnerData()
    return ffi.gc(p[0], C.ImGuiKeyOwnerData_destroy)
end
setmetatable(ImGuiKeyOwnerData, mt)
M.ImGuiKeyOwnerData = ImGuiKeyOwnerData
ffi.metatype("ImGuiKeyOwnerData", ImGuiKeyOwnerData)

local ImGuiKeyRoutingData = ImGuiKeyRoutingData or {}
ImGuiKeyRoutingData.__index = ImGuiKeyRoutingData
local mt = getmetatable(ImGuiKeyRoutingData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiKeyRoutingData_ImGuiKeyRoutingData()
    return ffi.gc(p[0], C.ImGuiKeyRoutingData_destroy)
end
setmetatable(ImGuiKeyRoutingData, mt)
M.ImGuiKeyRoutingData = ImGuiKeyRoutingData
ffi.metatype("ImGuiKeyRoutingData", ImGuiKeyRoutingData)

local ImGuiKeyRoutingTable = ImGuiKeyRoutingTable or {}
ImGuiKeyRoutingTable.__index = ImGuiKeyRoutingTable
ImGuiKeyRoutingTable["Clear"] = ImGuiKeyRoutingTable["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiKeyRoutingTable_Clear(i1)
    return out
end
local mt = getmetatable(ImGuiKeyRoutingTable) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiKeyRoutingTable_ImGuiKeyRoutingTable()
    return ffi.gc(p[0], C.ImGuiKeyRoutingTable_destroy)
end
setmetatable(ImGuiKeyRoutingTable, mt)
M.ImGuiKeyRoutingTable = ImGuiKeyRoutingTable
ffi.metatype("ImGuiKeyRoutingTable", ImGuiKeyRoutingTable)

local ImGuiLastItemData = ImGuiLastItemData or {}
ImGuiLastItemData.__index = ImGuiLastItemData
local mt = getmetatable(ImGuiLastItemData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiLastItemData_ImGuiLastItemData()
    return ffi.gc(p[0], C.ImGuiLastItemData_destroy)
end
setmetatable(ImGuiLastItemData, mt)
M.ImGuiLastItemData = ImGuiLastItemData
ffi.metatype("ImGuiLastItemData", ImGuiLastItemData)

local ImGuiListClipper = ImGuiListClipper or {}
ImGuiListClipper.__index = ImGuiListClipper
ImGuiListClipper["Begin"] = ImGuiListClipper["Begin"] or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = -1.0
    end
    local out = C.ImGuiListClipper_Begin(i1, i2, i3)
    return out
end
ImGuiListClipper["End"] = ImGuiListClipper["End"] or function(i1)
    jit.off(true)
    local out = C.ImGuiListClipper_End(i1)
    return out
end
ImGuiListClipper["IncludeItemByIndex"] = ImGuiListClipper["IncludeItemByIndex"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiListClipper_IncludeItemByIndex(i1, i2)
    return out
end
ImGuiListClipper["IncludeItemsByIndex"] = ImGuiListClipper["IncludeItemsByIndex"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiListClipper_IncludeItemsByIndex(i1, i2, i3)
    return out
end
ImGuiListClipper["SeekCursorForItem"] = ImGuiListClipper["SeekCursorForItem"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiListClipper_SeekCursorForItem(i1, i2)
    return out
end
ImGuiListClipper["Step"] = ImGuiListClipper["Step"] or function(i1)
    jit.off(true)
    local out = C.ImGuiListClipper_Step(i1)
    return out
end
local mt = getmetatable(ImGuiListClipper) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiListClipper_ImGuiListClipper()
    return ffi.gc(p[0], C.ImGuiListClipper_destroy)
end
setmetatable(ImGuiListClipper, mt)
M.ImGuiListClipper = ImGuiListClipper
ffi.metatype("ImGuiListClipper", ImGuiListClipper)

local ImGuiListClipperData = ImGuiListClipperData or {}
ImGuiListClipperData.__index = ImGuiListClipperData
ImGuiListClipperData["Reset"] = ImGuiListClipperData["Reset"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiListClipperData_Reset(i1, i2)
    return out
end
local mt = getmetatable(ImGuiListClipperData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiListClipperData_ImGuiListClipperData()
    return ffi.gc(p[0], C.ImGuiListClipperData_destroy)
end
setmetatable(ImGuiListClipperData, mt)
M.ImGuiListClipperData = ImGuiListClipperData
ffi.metatype("ImGuiListClipperData", ImGuiListClipperData)

local ImGuiListClipperRange = ImGuiListClipperRange or {}
ImGuiListClipperRange.__index = ImGuiListClipperRange
ImGuiListClipperRange["FromIndices"] = ImGuiListClipperRange["FromIndices"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiListClipperRange_FromIndices(i1, i2)
    return out
end
ImGuiListClipperRange["FromPositions"] = ImGuiListClipperRange["FromPositions"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImGuiListClipperRange_FromPositions(i1, i2, i3, i4)
    return out
end
M.ImGuiListClipperRange = ImGuiListClipperRange
ffi.metatype("ImGuiListClipperRange", ImGuiListClipperRange)

local ImGuiMenuColumns = ImGuiMenuColumns or {}
ImGuiMenuColumns.__index = ImGuiMenuColumns
ImGuiMenuColumns["CalcNextTotalWidth"] = ImGuiMenuColumns["CalcNextTotalWidth"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiMenuColumns_CalcNextTotalWidth(i1, i2)
    return out
end
ImGuiMenuColumns["DeclColumns"] = ImGuiMenuColumns["DeclColumns"] or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.ImGuiMenuColumns_DeclColumns(i1, i2, i3, i4, i5)
    return out
end
ImGuiMenuColumns["Update"] = ImGuiMenuColumns["Update"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiMenuColumns_Update(i1, i2, i3)
    return out
end
local mt = getmetatable(ImGuiMenuColumns) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiMenuColumns_ImGuiMenuColumns()
    return ffi.gc(p[0], C.ImGuiMenuColumns_destroy)
end
setmetatable(ImGuiMenuColumns, mt)
M.ImGuiMenuColumns = ImGuiMenuColumns
ffi.metatype("ImGuiMenuColumns", ImGuiMenuColumns)

local ImGuiMultiSelectState = ImGuiMultiSelectState or {}
ImGuiMultiSelectState.__index = ImGuiMultiSelectState
local mt = getmetatable(ImGuiMultiSelectState) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiMultiSelectState_ImGuiMultiSelectState()
    return ffi.gc(p[0], C.ImGuiMultiSelectState_destroy)
end
setmetatable(ImGuiMultiSelectState, mt)
M.ImGuiMultiSelectState = ImGuiMultiSelectState
ffi.metatype("ImGuiMultiSelectState", ImGuiMultiSelectState)

local ImGuiMultiSelectTempData = ImGuiMultiSelectTempData or {}
ImGuiMultiSelectTempData.__index = ImGuiMultiSelectTempData
ImGuiMultiSelectTempData["Clear"] = ImGuiMultiSelectTempData["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiMultiSelectTempData_Clear(i1)
    return out
end
ImGuiMultiSelectTempData["ClearIO"] = ImGuiMultiSelectTempData["ClearIO"] or function(i1)
    jit.off(true)
    local out = C.ImGuiMultiSelectTempData_ClearIO(i1)
    return out
end
local mt = getmetatable(ImGuiMultiSelectTempData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiMultiSelectTempData_ImGuiMultiSelectTempData()
    return ffi.gc(p[0], C.ImGuiMultiSelectTempData_destroy)
end
setmetatable(ImGuiMultiSelectTempData, mt)
M.ImGuiMultiSelectTempData = ImGuiMultiSelectTempData
ffi.metatype("ImGuiMultiSelectTempData", ImGuiMultiSelectTempData)

local ImGuiNavItemData = ImGuiNavItemData or {}
ImGuiNavItemData.__index = ImGuiNavItemData
ImGuiNavItemData["Clear"] = ImGuiNavItemData["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiNavItemData_Clear(i1)
    return out
end
local mt = getmetatable(ImGuiNavItemData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiNavItemData_ImGuiNavItemData()
    return ffi.gc(p[0], C.ImGuiNavItemData_destroy)
end
setmetatable(ImGuiNavItemData, mt)
M.ImGuiNavItemData = ImGuiNavItemData
ffi.metatype("ImGuiNavItemData", ImGuiNavItemData)

local ImGuiNextItemData = ImGuiNextItemData or {}
ImGuiNextItemData.__index = ImGuiNextItemData
ImGuiNextItemData["ClearFlags"] = ImGuiNextItemData["ClearFlags"] or function(i1)
    jit.off(true)
    local out = C.ImGuiNextItemData_ClearFlags(i1)
    return out
end
local mt = getmetatable(ImGuiNextItemData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiNextItemData_ImGuiNextItemData()
    return ffi.gc(p[0], C.ImGuiNextItemData_destroy)
end
setmetatable(ImGuiNextItemData, mt)
M.ImGuiNextItemData = ImGuiNextItemData
ffi.metatype("ImGuiNextItemData", ImGuiNextItemData)

local ImGuiNextWindowData = ImGuiNextWindowData or {}
ImGuiNextWindowData.__index = ImGuiNextWindowData
ImGuiNextWindowData["ClearFlags"] = ImGuiNextWindowData["ClearFlags"] or function(i1)
    jit.off(true)
    local out = C.ImGuiNextWindowData_ClearFlags(i1)
    return out
end
local mt = getmetatable(ImGuiNextWindowData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiNextWindowData_ImGuiNextWindowData()
    return ffi.gc(p[0], C.ImGuiNextWindowData_destroy)
end
setmetatable(ImGuiNextWindowData, mt)
M.ImGuiNextWindowData = ImGuiNextWindowData
ffi.metatype("ImGuiNextWindowData", ImGuiNextWindowData)

local ImGuiOldColumnData = ImGuiOldColumnData or {}
ImGuiOldColumnData.__index = ImGuiOldColumnData
local mt = getmetatable(ImGuiOldColumnData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiOldColumnData_ImGuiOldColumnData()
    return ffi.gc(p[0], C.ImGuiOldColumnData_destroy)
end
setmetatable(ImGuiOldColumnData, mt)
M.ImGuiOldColumnData = ImGuiOldColumnData
ffi.metatype("ImGuiOldColumnData", ImGuiOldColumnData)

local ImGuiOldColumns = ImGuiOldColumns or {}
ImGuiOldColumns.__index = ImGuiOldColumns
local mt = getmetatable(ImGuiOldColumns) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiOldColumns_ImGuiOldColumns()
    return ffi.gc(p[0], C.ImGuiOldColumns_destroy)
end
setmetatable(ImGuiOldColumns, mt)
M.ImGuiOldColumns = ImGuiOldColumns
ffi.metatype("ImGuiOldColumns", ImGuiOldColumns)

local ImGuiOnceUponAFrame = ImGuiOnceUponAFrame or {}
ImGuiOnceUponAFrame.__index = ImGuiOnceUponAFrame
local mt = getmetatable(ImGuiOnceUponAFrame) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiOnceUponAFrame_ImGuiOnceUponAFrame()
    return ffi.gc(p[0], C.ImGuiOnceUponAFrame_destroy)
end
setmetatable(ImGuiOnceUponAFrame, mt)
M.ImGuiOnceUponAFrame = ImGuiOnceUponAFrame
ffi.metatype("ImGuiOnceUponAFrame", ImGuiOnceUponAFrame)

local ImGuiPayload = ImGuiPayload or {}
ImGuiPayload.__index = ImGuiPayload
ImGuiPayload["Clear"] = ImGuiPayload["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiPayload_Clear(i1)
    return out
end
ImGuiPayload["IsDataType"] = ImGuiPayload["IsDataType"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiPayload_IsDataType(i1, i2)
    return out
end
ImGuiPayload["IsDelivery"] = ImGuiPayload["IsDelivery"] or function(i1)
    jit.off(true)
    local out = C.ImGuiPayload_IsDelivery(i1)
    return out
end
ImGuiPayload["IsPreview"] = ImGuiPayload["IsPreview"] or function(i1)
    jit.off(true)
    local out = C.ImGuiPayload_IsPreview(i1)
    return out
end
local mt = getmetatable(ImGuiPayload) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiPayload_ImGuiPayload()
    return ffi.gc(p[0], C.ImGuiPayload_destroy)
end
setmetatable(ImGuiPayload, mt)
M.ImGuiPayload = ImGuiPayload
ffi.metatype("ImGuiPayload", ImGuiPayload)

local ImGuiPlatformIO = ImGuiPlatformIO or {}
ImGuiPlatformIO.__index = ImGuiPlatformIO
local mt = getmetatable(ImGuiPlatformIO) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiPlatformIO_ImGuiPlatformIO()
    return ffi.gc(p[0], C.ImGuiPlatformIO_destroy)
end
setmetatable(ImGuiPlatformIO, mt)
M.ImGuiPlatformIO = ImGuiPlatformIO
ffi.metatype("ImGuiPlatformIO", ImGuiPlatformIO)

local ImGuiPlatformImeData = ImGuiPlatformImeData or {}
ImGuiPlatformImeData.__index = ImGuiPlatformImeData
local mt = getmetatable(ImGuiPlatformImeData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiPlatformImeData_ImGuiPlatformImeData()
    return ffi.gc(p[0], C.ImGuiPlatformImeData_destroy)
end
setmetatable(ImGuiPlatformImeData, mt)
M.ImGuiPlatformImeData = ImGuiPlatformImeData
ffi.metatype("ImGuiPlatformImeData", ImGuiPlatformImeData)

local ImGuiPlatformMonitor = ImGuiPlatformMonitor or {}
ImGuiPlatformMonitor.__index = ImGuiPlatformMonitor
local mt = getmetatable(ImGuiPlatformMonitor) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiPlatformMonitor_ImGuiPlatformMonitor()
    return ffi.gc(p[0], C.ImGuiPlatformMonitor_destroy)
end
setmetatable(ImGuiPlatformMonitor, mt)
M.ImGuiPlatformMonitor = ImGuiPlatformMonitor
ffi.metatype("ImGuiPlatformMonitor", ImGuiPlatformMonitor)

local ImGuiPopupData = ImGuiPopupData or {}
ImGuiPopupData.__index = ImGuiPopupData
local mt = getmetatable(ImGuiPopupData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiPopupData_ImGuiPopupData()
    return ffi.gc(p[0], C.ImGuiPopupData_destroy)
end
setmetatable(ImGuiPopupData, mt)
M.ImGuiPopupData = ImGuiPopupData
ffi.metatype("ImGuiPopupData", ImGuiPopupData)

local ImGuiPtrOrIndex = ImGuiPtrOrIndex or {}
ImGuiPtrOrIndex.__index = ImGuiPtrOrIndex
M.ImGuiPtrOrIndex_Ptr = M.ImGuiPtrOrIndex_Ptr or function(i1)
    jit.off(true)
    local p = C.ImGuiPtrOrIndex_ImGuiPtrOrIndex_Ptr(i1)
    return ffi.gc(p[0], C.ImGuiPtrOrIndex_destroy)
end
M.ImGuiPtrOrIndex_Int = M.ImGuiPtrOrIndex_Int or function(i1)
    jit.off(true)
    local p = C.ImGuiPtrOrIndex_ImGuiPtrOrIndex_Int(i1)
    return ffi.gc(p[0], C.ImGuiPtrOrIndex_destroy)
end
M.ImGuiPtrOrIndex = ImGuiPtrOrIndex
ffi.metatype("ImGuiPtrOrIndex", ImGuiPtrOrIndex)

local ImGuiSelectionBasicStorage = ImGuiSelectionBasicStorage or {}
ImGuiSelectionBasicStorage.__index = ImGuiSelectionBasicStorage
ImGuiSelectionBasicStorage["ApplyRequests"] = ImGuiSelectionBasicStorage["ApplyRequests"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiSelectionBasicStorage_ApplyRequests(i1, i2)
    return out
end
ImGuiSelectionBasicStorage["Clear"] = ImGuiSelectionBasicStorage["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiSelectionBasicStorage_Clear(i1)
    return out
end
ImGuiSelectionBasicStorage["Contains"] = ImGuiSelectionBasicStorage["Contains"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiSelectionBasicStorage_Contains(i1, i2)
    return out
end
ImGuiSelectionBasicStorage["GetNextSelectedItem"] = ImGuiSelectionBasicStorage["GetNextSelectedItem"] or
                                                        function(i1, i2, i3)
        jit.off(true)
        local out = C.ImGuiSelectionBasicStorage_GetNextSelectedItem(i1, i2, i3)
        return out
    end
ImGuiSelectionBasicStorage["GetStorageIdFromIndex"] = ImGuiSelectionBasicStorage["GetStorageIdFromIndex"] or
                                                          function(i1, i2)
        jit.off(true)
        local out = C.ImGuiSelectionBasicStorage_GetStorageIdFromIndex(i1, i2)
        return out
    end
ImGuiSelectionBasicStorage["SetItemSelected"] = ImGuiSelectionBasicStorage["SetItemSelected"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiSelectionBasicStorage_SetItemSelected(i1, i2, i3)
    return out
end
ImGuiSelectionBasicStorage["Swap"] = ImGuiSelectionBasicStorage["Swap"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiSelectionBasicStorage_Swap(i1, i2)
    return out
end
local mt = getmetatable(ImGuiSelectionBasicStorage) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiSelectionBasicStorage_ImGuiSelectionBasicStorage()
    return ffi.gc(p[0], C.ImGuiSelectionBasicStorage_destroy)
end
setmetatable(ImGuiSelectionBasicStorage, mt)
M.ImGuiSelectionBasicStorage = ImGuiSelectionBasicStorage
ffi.metatype("ImGuiSelectionBasicStorage", ImGuiSelectionBasicStorage)

local ImGuiSelectionExternalStorage = ImGuiSelectionExternalStorage or {}
ImGuiSelectionExternalStorage.__index = ImGuiSelectionExternalStorage
ImGuiSelectionExternalStorage["ApplyRequests"] = ImGuiSelectionExternalStorage["ApplyRequests"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiSelectionExternalStorage_ApplyRequests(i1, i2)
    return out
end
local mt = getmetatable(ImGuiSelectionExternalStorage) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiSelectionExternalStorage_ImGuiSelectionExternalStorage()
    return ffi.gc(p[0], C.ImGuiSelectionExternalStorage_destroy)
end
setmetatable(ImGuiSelectionExternalStorage, mt)
M.ImGuiSelectionExternalStorage = ImGuiSelectionExternalStorage
ffi.metatype("ImGuiSelectionExternalStorage", ImGuiSelectionExternalStorage)

local ImGuiSettingsHandler = ImGuiSettingsHandler or {}
ImGuiSettingsHandler.__index = ImGuiSettingsHandler
local mt = getmetatable(ImGuiSettingsHandler) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiSettingsHandler_ImGuiSettingsHandler()
    return ffi.gc(p[0], C.ImGuiSettingsHandler_destroy)
end
setmetatable(ImGuiSettingsHandler, mt)
M.ImGuiSettingsHandler = ImGuiSettingsHandler
ffi.metatype("ImGuiSettingsHandler", ImGuiSettingsHandler)

local ImGuiStackLevelInfo = ImGuiStackLevelInfo or {}
ImGuiStackLevelInfo.__index = ImGuiStackLevelInfo
local mt = getmetatable(ImGuiStackLevelInfo) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiStackLevelInfo_ImGuiStackLevelInfo()
    return ffi.gc(p[0], C.ImGuiStackLevelInfo_destroy)
end
setmetatable(ImGuiStackLevelInfo, mt)
M.ImGuiStackLevelInfo = ImGuiStackLevelInfo
ffi.metatype("ImGuiStackLevelInfo", ImGuiStackLevelInfo)

local ImGuiStackSizes = ImGuiStackSizes or {}
ImGuiStackSizes.__index = ImGuiStackSizes
ImGuiStackSizes["CompareWithContextState"] = ImGuiStackSizes["CompareWithContextState"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiStackSizes_CompareWithContextState(i1, i2)
    return out
end
ImGuiStackSizes["SetToContextState"] = ImGuiStackSizes["SetToContextState"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiStackSizes_SetToContextState(i1, i2)
    return out
end
local mt = getmetatable(ImGuiStackSizes) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiStackSizes_ImGuiStackSizes()
    return ffi.gc(p[0], C.ImGuiStackSizes_destroy)
end
setmetatable(ImGuiStackSizes, mt)
M.ImGuiStackSizes = ImGuiStackSizes
ffi.metatype("ImGuiStackSizes", ImGuiStackSizes)

local ImGuiStorage = ImGuiStorage or {}
ImGuiStorage.__index = ImGuiStorage
ImGuiStorage["BuildSortByKey"] = ImGuiStorage["BuildSortByKey"] or function(i1)
    jit.off(true)
    local out = C.ImGuiStorage_BuildSortByKey(i1)
    return out
end
ImGuiStorage["Clear"] = ImGuiStorage["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiStorage_Clear(i1)
    return out
end
ImGuiStorage["GetBool"] = ImGuiStorage["GetBool"] or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = false
    end
    local out = C.ImGuiStorage_GetBool(i1, i2, i3)
    return out
end
ImGuiStorage["GetBoolRef"] = ImGuiStorage["GetBoolRef"] or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = false
    end
    local out = C.ImGuiStorage_GetBoolRef(i1, i2, i3)
    return out
end
ImGuiStorage["GetFloat"] = ImGuiStorage["GetFloat"] or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0.0
    end
    local out = C.ImGuiStorage_GetFloat(i1, i2, i3)
    return out
end
ImGuiStorage["GetFloatRef"] = ImGuiStorage["GetFloatRef"] or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0.0
    end
    local out = C.ImGuiStorage_GetFloatRef(i1, i2, i3)
    return out
end
ImGuiStorage["GetInt"] = ImGuiStorage["GetInt"] or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.ImGuiStorage_GetInt(i1, i2, i3)
    return out
end
ImGuiStorage["GetIntRef"] = ImGuiStorage["GetIntRef"] or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.ImGuiStorage_GetIntRef(i1, i2, i3)
    return out
end
ImGuiStorage["GetVoidPtr"] = ImGuiStorage["GetVoidPtr"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiStorage_GetVoidPtr(i1, i2)
    return out
end
ImGuiStorage["GetVoidPtrRef"] = ImGuiStorage["GetVoidPtrRef"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiStorage_GetVoidPtrRef(i1, i2, i3)
    return out
end
ImGuiStorage["SetAllInt"] = ImGuiStorage["SetAllInt"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiStorage_SetAllInt(i1, i2)
    return out
end
ImGuiStorage["SetBool"] = ImGuiStorage["SetBool"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiStorage_SetBool(i1, i2, i3)
    return out
end
ImGuiStorage["SetFloat"] = ImGuiStorage["SetFloat"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiStorage_SetFloat(i1, i2, i3)
    return out
end
ImGuiStorage["SetInt"] = ImGuiStorage["SetInt"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiStorage_SetInt(i1, i2, i3)
    return out
end
ImGuiStorage["SetVoidPtr"] = ImGuiStorage["SetVoidPtr"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiStorage_SetVoidPtr(i1, i2, i3)
    return out
end
M.ImGuiStorage = ImGuiStorage
ffi.metatype("ImGuiStorage", ImGuiStorage)

local ImGuiStoragePair = ImGuiStoragePair or {}
ImGuiStoragePair.__index = ImGuiStoragePair
M.ImGuiStoragePair_Int = M.ImGuiStoragePair_Int or function(i1, i2)
    jit.off(true)
    local p = C.ImGuiStoragePair_ImGuiStoragePair_Int(i1, i2)
    return ffi.gc(p[0], C.ImGuiStoragePair_destroy)
end
M.ImGuiStoragePair_Float = M.ImGuiStoragePair_Float or function(i1, i2)
    jit.off(true)
    local p = C.ImGuiStoragePair_ImGuiStoragePair_Float(i1, i2)
    return ffi.gc(p[0], C.ImGuiStoragePair_destroy)
end
M.ImGuiStoragePair_Ptr = M.ImGuiStoragePair_Ptr or function(i1, i2)
    jit.off(true)
    local p = C.ImGuiStoragePair_ImGuiStoragePair_Ptr(i1, i2)
    return ffi.gc(p[0], C.ImGuiStoragePair_destroy)
end
M.ImGuiStoragePair = ImGuiStoragePair
ffi.metatype("ImGuiStoragePair", ImGuiStoragePair)

local ImGuiStyle = ImGuiStyle or {}
ImGuiStyle.__index = ImGuiStyle
ImGuiStyle["ScaleAllSizes"] = ImGuiStyle["ScaleAllSizes"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiStyle_ScaleAllSizes(i1, i2)
    return out
end
local mt = getmetatable(ImGuiStyle) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiStyle_ImGuiStyle()
    return ffi.gc(p[0], C.ImGuiStyle_destroy)
end
setmetatable(ImGuiStyle, mt)
M.ImGuiStyle = ImGuiStyle
ffi.metatype("ImGuiStyle", ImGuiStyle)

local ImGuiStyleMod = ImGuiStyleMod or {}
ImGuiStyleMod.__index = ImGuiStyleMod
M.ImGuiStyleMod_Int = M.ImGuiStyleMod_Int or function(i1, i2)
    jit.off(true)
    local p = C.ImGuiStyleMod_ImGuiStyleMod_Int(i1, i2)
    return ffi.gc(p[0], C.ImGuiStyleMod_destroy)
end
M.ImGuiStyleMod_Float = M.ImGuiStyleMod_Float or function(i1, i2)
    jit.off(true)
    local p = C.ImGuiStyleMod_ImGuiStyleMod_Float(i1, i2)
    return ffi.gc(p[0], C.ImGuiStyleMod_destroy)
end
M.ImGuiStyleMod_Vec2 = M.ImGuiStyleMod_Vec2 or function(i1, i2)
    jit.off(true)
    local p = C.ImGuiStyleMod_ImGuiStyleMod_Vec2(i1, i2)
    return ffi.gc(p[0], C.ImGuiStyleMod_destroy)
end
M.ImGuiStyleMod = ImGuiStyleMod
ffi.metatype("ImGuiStyleMod", ImGuiStyleMod)

local ImGuiTabBar = ImGuiTabBar or {}
ImGuiTabBar.__index = ImGuiTabBar
local mt = getmetatable(ImGuiTabBar) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTabBar_ImGuiTabBar()
    return ffi.gc(p[0], C.ImGuiTabBar_destroy)
end
setmetatable(ImGuiTabBar, mt)
M.ImGuiTabBar = ImGuiTabBar
ffi.metatype("ImGuiTabBar", ImGuiTabBar)

local ImGuiTabItem = ImGuiTabItem or {}
ImGuiTabItem.__index = ImGuiTabItem
local mt = getmetatable(ImGuiTabItem) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTabItem_ImGuiTabItem()
    return ffi.gc(p[0], C.ImGuiTabItem_destroy)
end
setmetatable(ImGuiTabItem, mt)
M.ImGuiTabItem = ImGuiTabItem
ffi.metatype("ImGuiTabItem", ImGuiTabItem)

local ImGuiTable = ImGuiTable or {}
ImGuiTable.__index = ImGuiTable
local mt = getmetatable(ImGuiTable) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTable_ImGuiTable()
    return ffi.gc(p[0], C.ImGuiTable_destroy)
end
setmetatable(ImGuiTable, mt)
M.ImGuiTable = ImGuiTable
ffi.metatype("ImGuiTable", ImGuiTable)

local ImGuiTableColumn = ImGuiTableColumn or {}
ImGuiTableColumn.__index = ImGuiTableColumn
local mt = getmetatable(ImGuiTableColumn) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTableColumn_ImGuiTableColumn()
    return ffi.gc(p[0], C.ImGuiTableColumn_destroy)
end
setmetatable(ImGuiTableColumn, mt)
M.ImGuiTableColumn = ImGuiTableColumn
ffi.metatype("ImGuiTableColumn", ImGuiTableColumn)

local ImGuiTableColumnSettings = ImGuiTableColumnSettings or {}
ImGuiTableColumnSettings.__index = ImGuiTableColumnSettings
local mt = getmetatable(ImGuiTableColumnSettings) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTableColumnSettings_ImGuiTableColumnSettings()
    return ffi.gc(p[0], C.ImGuiTableColumnSettings_destroy)
end
setmetatable(ImGuiTableColumnSettings, mt)
M.ImGuiTableColumnSettings = ImGuiTableColumnSettings
ffi.metatype("ImGuiTableColumnSettings", ImGuiTableColumnSettings)

local ImGuiTableColumnSortSpecs = ImGuiTableColumnSortSpecs or {}
ImGuiTableColumnSortSpecs.__index = ImGuiTableColumnSortSpecs
local mt = getmetatable(ImGuiTableColumnSortSpecs) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTableColumnSortSpecs_ImGuiTableColumnSortSpecs()
    return ffi.gc(p[0], C.ImGuiTableColumnSortSpecs_destroy)
end
setmetatable(ImGuiTableColumnSortSpecs, mt)
M.ImGuiTableColumnSortSpecs = ImGuiTableColumnSortSpecs
ffi.metatype("ImGuiTableColumnSortSpecs", ImGuiTableColumnSortSpecs)

local ImGuiTableInstanceData = ImGuiTableInstanceData or {}
ImGuiTableInstanceData.__index = ImGuiTableInstanceData
local mt = getmetatable(ImGuiTableInstanceData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTableInstanceData_ImGuiTableInstanceData()
    return ffi.gc(p[0], C.ImGuiTableInstanceData_destroy)
end
setmetatable(ImGuiTableInstanceData, mt)
M.ImGuiTableInstanceData = ImGuiTableInstanceData
ffi.metatype("ImGuiTableInstanceData", ImGuiTableInstanceData)

local ImGuiTableSettings = ImGuiTableSettings or {}
ImGuiTableSettings.__index = ImGuiTableSettings
ImGuiTableSettings["GetColumnSettings"] = ImGuiTableSettings["GetColumnSettings"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTableSettings_GetColumnSettings(i1)
    return out
end
local mt = getmetatable(ImGuiTableSettings) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTableSettings_ImGuiTableSettings()
    return ffi.gc(p[0], C.ImGuiTableSettings_destroy)
end
setmetatable(ImGuiTableSettings, mt)
M.ImGuiTableSettings = ImGuiTableSettings
ffi.metatype("ImGuiTableSettings", ImGuiTableSettings)

local ImGuiTableSortSpecs = ImGuiTableSortSpecs or {}
ImGuiTableSortSpecs.__index = ImGuiTableSortSpecs
local mt = getmetatable(ImGuiTableSortSpecs) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTableSortSpecs_ImGuiTableSortSpecs()
    return ffi.gc(p[0], C.ImGuiTableSortSpecs_destroy)
end
setmetatable(ImGuiTableSortSpecs, mt)
M.ImGuiTableSortSpecs = ImGuiTableSortSpecs
ffi.metatype("ImGuiTableSortSpecs", ImGuiTableSortSpecs)

local ImGuiTableTempData = ImGuiTableTempData or {}
ImGuiTableTempData.__index = ImGuiTableTempData
local mt = getmetatable(ImGuiTableTempData) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTableTempData_ImGuiTableTempData()
    return ffi.gc(p[0], C.ImGuiTableTempData_destroy)
end
setmetatable(ImGuiTableTempData, mt)
M.ImGuiTableTempData = ImGuiTableTempData
ffi.metatype("ImGuiTableTempData", ImGuiTableTempData)

local ImGuiTextBuffer = ImGuiTextBuffer or {}
ImGuiTextBuffer.__index = ImGuiTextBuffer
ImGuiTextBuffer["append"] = ImGuiTextBuffer["append"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiTextBuffer_append(i1, i2, i3)
    return out
end
ImGuiTextBuffer["appendf"] = ImGuiTextBuffer["appendf"] or function(i1, i2, ...)
    jit.off(true)
    local out = C.ImGuiTextBuffer_appendf(i1, i2, ...)
    return out
end
ImGuiTextBuffer["begin"] = ImGuiTextBuffer["begin"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextBuffer_begin(i1)
    return out
end
ImGuiTextBuffer["c_str"] = ImGuiTextBuffer["c_str"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextBuffer_c_str(i1)
    return out
end
ImGuiTextBuffer["clear"] = ImGuiTextBuffer["clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextBuffer_clear(i1)
    return out
end
ImGuiTextBuffer["empty"] = ImGuiTextBuffer["empty"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextBuffer_empty(i1)
    return out
end
ImGuiTextBuffer["end"] = ImGuiTextBuffer["end"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextBuffer_end(i1)
    return out
end
ImGuiTextBuffer.c_end = ImGuiTextBuffer["end"]
ImGuiTextBuffer["reserve"] = ImGuiTextBuffer["reserve"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiTextBuffer_reserve(i1, i2)
    return out
end
ImGuiTextBuffer["size"] = ImGuiTextBuffer["size"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextBuffer_size(i1)
    return out
end
local mt = getmetatable(ImGuiTextBuffer) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTextBuffer_ImGuiTextBuffer()
    return ffi.gc(p[0], C.ImGuiTextBuffer_destroy)
end
setmetatable(ImGuiTextBuffer, mt)
M.ImGuiTextBuffer = ImGuiTextBuffer
ffi.metatype("ImGuiTextBuffer", ImGuiTextBuffer)

local ImGuiTextFilter = ImGuiTextFilter or {}
ImGuiTextFilter.__index = ImGuiTextFilter
ImGuiTextFilter["Build"] = ImGuiTextFilter["Build"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextFilter_Build(i1)
    return out
end
ImGuiTextFilter["Clear"] = ImGuiTextFilter["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextFilter_Clear(i1)
    return out
end
ImGuiTextFilter["Draw"] = ImGuiTextFilter["Draw"] or function(i1, i2, i3)
    jit.off(true)
    if i2 == nil then
        i2 = "Filter(inc,-exc)"
    end
    if i3 == nil then
        i3 = 0.0
    end
    local out = C.ImGuiTextFilter_Draw(i1, i2, i3)
    return out
end
ImGuiTextFilter["IsActive"] = ImGuiTextFilter["IsActive"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextFilter_IsActive(i1)
    return out
end
ImGuiTextFilter["PassFilter"] = ImGuiTextFilter["PassFilter"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiTextFilter_PassFilter(i1, i2, i3)
    return out
end
local mt = getmetatable(ImGuiTextFilter) or {}
mt.__call = mt.__call or function(self, i1)
    jit.off(true)
    local p = C.ImGuiTextFilter_ImGuiTextFilter(i1)
    return ffi.gc(p[0], C.ImGuiTextFilter_destroy)
end
setmetatable(ImGuiTextFilter, mt)
M.ImGuiTextFilter = ImGuiTextFilter
ffi.metatype("ImGuiTextFilter", ImGuiTextFilter)

local ImGuiTextIndex = ImGuiTextIndex or {}
ImGuiTextIndex.__index = ImGuiTextIndex
ImGuiTextIndex["append"] = ImGuiTextIndex["append"] or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.ImGuiTextIndex_append(i1, i2, i3, i4)
    return out
end
ImGuiTextIndex["clear"] = ImGuiTextIndex["clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextIndex_clear(i1)
    return out
end
ImGuiTextIndex["get_line_begin"] = ImGuiTextIndex["get_line_begin"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiTextIndex_get_line_begin(i1, i2, i3)
    return out
end
ImGuiTextIndex["get_line_end"] = ImGuiTextIndex["get_line_end"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiTextIndex_get_line_end(i1, i2, i3)
    return out
end
ImGuiTextIndex["size"] = ImGuiTextIndex["size"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextIndex_size(i1)
    return out
end
M.ImGuiTextIndex = ImGuiTextIndex
ffi.metatype("ImGuiTextIndex", ImGuiTextIndex)

local ImGuiTextRange = ImGuiTextRange or {}
ImGuiTextRange.__index = ImGuiTextRange
ImGuiTextRange["empty"] = ImGuiTextRange["empty"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTextRange_empty(i1)
    return out
end
ImGuiTextRange["split"] = ImGuiTextRange["split"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiTextRange_split(i1, i2, i3)
    return out
end
M.ImGuiTextRange_Nil = M.ImGuiTextRange_Nil or function()
    jit.off(true)
    local p = C.ImGuiTextRange_ImGuiTextRange_Nil()
    return ffi.gc(p[0], C.ImGuiTextRange_destroy)
end
M.ImGuiTextRange_Str = M.ImGuiTextRange_Str or function(i1, i2)
    jit.off(true)
    local p = C.ImGuiTextRange_ImGuiTextRange_Str(i1, i2)
    return ffi.gc(p[0], C.ImGuiTextRange_destroy)
end
M.ImGuiTextRange = ImGuiTextRange
ffi.metatype("ImGuiTextRange", ImGuiTextRange)

local ImGuiTypingSelectState = ImGuiTypingSelectState or {}
ImGuiTypingSelectState.__index = ImGuiTypingSelectState
ImGuiTypingSelectState["Clear"] = ImGuiTypingSelectState["Clear"] or function(i1)
    jit.off(true)
    local out = C.ImGuiTypingSelectState_Clear(i1)
    return out
end
local mt = getmetatable(ImGuiTypingSelectState) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiTypingSelectState_ImGuiTypingSelectState()
    return ffi.gc(p[0], C.ImGuiTypingSelectState_destroy)
end
setmetatable(ImGuiTypingSelectState, mt)
M.ImGuiTypingSelectState = ImGuiTypingSelectState
ffi.metatype("ImGuiTypingSelectState", ImGuiTypingSelectState)

local ImGuiViewport = ImGuiViewport or {}
ImGuiViewport.__index = ImGuiViewport
ImGuiViewport["GetCenter"] = ImGuiViewport["GetCenter"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImGuiViewport_GetCenter(o1, i1)
    return o1, out
end
ImGuiViewport["GetWorkCenter"] = ImGuiViewport["GetWorkCenter"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImGuiViewport_GetWorkCenter(o1, i1)
    return o1, out
end
local mt = getmetatable(ImGuiViewport) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiViewport_ImGuiViewport()
    return ffi.gc(p[0], C.ImGuiViewport_destroy)
end
setmetatable(ImGuiViewport, mt)
M.ImGuiViewport = ImGuiViewport
ffi.metatype("ImGuiViewport", ImGuiViewport)

local ImGuiViewportP = ImGuiViewportP or {}
ImGuiViewportP.__index = ImGuiViewportP
ImGuiViewportP["CalcWorkRectPos"] = ImGuiViewportP["CalcWorkRectPos"] or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImGuiViewportP_CalcWorkRectPos(o1, i1, i2)
    return o1, out
end
ImGuiViewportP["CalcWorkRectSize"] = ImGuiViewportP["CalcWorkRectSize"] or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImGuiViewportP_CalcWorkRectSize(o1, i1, i2, i3)
    return o1, out
end
ImGuiViewportP["ClearRequestFlags"] = ImGuiViewportP["ClearRequestFlags"] or function(i1)
    jit.off(true)
    local out = C.ImGuiViewportP_ClearRequestFlags(i1)
    return out
end
ImGuiViewportP["GetBuildWorkRect"] = ImGuiViewportP["GetBuildWorkRect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiViewportP_GetBuildWorkRect(i1, i2)
    return out
end
ImGuiViewportP["GetMainRect"] = ImGuiViewportP["GetMainRect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiViewportP_GetMainRect(i1, i2)
    return out
end
ImGuiViewportP["GetWorkRect"] = ImGuiViewportP["GetWorkRect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiViewportP_GetWorkRect(i1, i2)
    return out
end
ImGuiViewportP["UpdateWorkRect"] = ImGuiViewportP["UpdateWorkRect"] or function(i1)
    jit.off(true)
    local out = C.ImGuiViewportP_UpdateWorkRect(i1)
    return out
end
local mt = getmetatable(ImGuiViewportP) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiViewportP_ImGuiViewportP()
    return ffi.gc(p[0], C.ImGuiViewportP_destroy)
end
setmetatable(ImGuiViewportP, mt)
M.ImGuiViewportP = ImGuiViewportP
ffi.metatype("ImGuiViewportP", ImGuiViewportP)

local ImGuiWindow = ImGuiWindow or {}
ImGuiWindow.__index = ImGuiWindow
ImGuiWindow["CalcFontSize"] = ImGuiWindow["CalcFontSize"] or function(i1)
    jit.off(true)
    local out = C.ImGuiWindow_CalcFontSize(i1)
    return out
end
ImGuiWindow["GetID_Str"] = ImGuiWindow["GetID_Str"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImGuiWindow_GetID_Str(i1, i2, i3)
    return out
end
ImGuiWindow["GetID_Ptr"] = ImGuiWindow["GetID_Ptr"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiWindow_GetID_Ptr(i1, i2)
    return out
end
ImGuiWindow["GetID_Int"] = ImGuiWindow["GetID_Int"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiWindow_GetID_Int(i1, i2)
    return out
end
ImGuiWindow["GetIDFromRectangle"] = ImGuiWindow["GetIDFromRectangle"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiWindow_GetIDFromRectangle(i1, i2)
    return out
end
ImGuiWindow["MenuBarRect"] = ImGuiWindow["MenuBarRect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiWindow_MenuBarRect(i1, i2)
    return out
end
ImGuiWindow["Rect"] = ImGuiWindow["Rect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiWindow_Rect(i1, i2)
    return out
end
ImGuiWindow["TitleBarRect"] = ImGuiWindow["TitleBarRect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImGuiWindow_TitleBarRect(i1, i2)
    return out
end
local mt = getmetatable(ImGuiWindow) or {}
mt.__call = mt.__call or function(self, i1, i2)
    jit.off(true)
    local p = C.ImGuiWindow_ImGuiWindow(i1, i2)
    return ffi.gc(p[0], C.ImGuiWindow_destroy)
end
setmetatable(ImGuiWindow, mt)
M.ImGuiWindow = ImGuiWindow
ffi.metatype("ImGuiWindow", ImGuiWindow)

local ImGuiWindowClass = ImGuiWindowClass or {}
ImGuiWindowClass.__index = ImGuiWindowClass
local mt = getmetatable(ImGuiWindowClass) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiWindowClass_ImGuiWindowClass()
    return ffi.gc(p[0], C.ImGuiWindowClass_destroy)
end
setmetatable(ImGuiWindowClass, mt)
M.ImGuiWindowClass = ImGuiWindowClass
ffi.metatype("ImGuiWindowClass", ImGuiWindowClass)

local ImGuiWindowSettings = ImGuiWindowSettings or {}
ImGuiWindowSettings.__index = ImGuiWindowSettings
ImGuiWindowSettings["GetName"] = ImGuiWindowSettings["GetName"] or function(i1)
    jit.off(true)
    local out = C.ImGuiWindowSettings_GetName(i1)
    return out
end
local mt = getmetatable(ImGuiWindowSettings) or {}
mt.__call = mt.__call or function(self)
    jit.off(true)
    local p = C.ImGuiWindowSettings_ImGuiWindowSettings()
    return ffi.gc(p[0], C.ImGuiWindowSettings_destroy)
end
setmetatable(ImGuiWindowSettings, mt)
M.ImGuiWindowSettings = ImGuiWindowSettings
ffi.metatype("ImGuiWindowSettings", ImGuiWindowSettings)

local ImRect = ImRect or {}
ImRect.__index = ImRect
ImRect["Add_Vec2"] = ImRect["Add_Vec2"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Add_Vec2(i1, i2)
    return out
end
ImRect["Add_Rect"] = ImRect["Add_Rect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Add_Rect(i1, i2)
    return out
end
ImRect["ClipWith"] = ImRect["ClipWith"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_ClipWith(i1, i2)
    return out
end
ImRect["ClipWithFull"] = ImRect["ClipWithFull"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_ClipWithFull(i1, i2)
    return out
end
ImRect["Contains_Vec2"] = ImRect["Contains_Vec2"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Contains_Vec2(i1, i2)
    return out
end
ImRect["Contains_Rect"] = ImRect["Contains_Rect"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Contains_Rect(i1, i2)
    return out
end
ImRect["ContainsWithPad"] = ImRect["ContainsWithPad"] or function(i1, i2, i3)
    jit.off(true)
    local out = C.ImRect_ContainsWithPad(i1, i2, i3)
    return out
end
ImRect["Expand_Float"] = ImRect["Expand_Float"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Expand_Float(i1, i2)
    return out
end
ImRect["Expand_Vec2"] = ImRect["Expand_Vec2"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Expand_Vec2(i1, i2)
    return out
end
ImRect["Floor"] = ImRect["Floor"] or function(i1)
    jit.off(true)
    local out = C.ImRect_Floor(i1)
    return out
end
ImRect["GetArea"] = ImRect["GetArea"] or function(i1)
    jit.off(true)
    local out = C.ImRect_GetArea(i1)
    return out
end
ImRect["GetBL"] = ImRect["GetBL"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImRect_GetBL(o1, i1)
    return o1, out
end
ImRect["GetBR"] = ImRect["GetBR"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImRect_GetBR(o1, i1)
    return o1, out
end
ImRect["GetCenter"] = ImRect["GetCenter"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImRect_GetCenter(o1, i1)
    return o1, out
end
ImRect["GetHeight"] = ImRect["GetHeight"] or function(i1)
    jit.off(true)
    local out = C.ImRect_GetHeight(i1)
    return out
end
ImRect["GetSize"] = ImRect["GetSize"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImRect_GetSize(o1, i1)
    return o1, out
end
ImRect["GetTL"] = ImRect["GetTL"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImRect_GetTL(o1, i1)
    return o1, out
end
ImRect["GetTR"] = ImRect["GetTR"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.ImRect_GetTR(o1, i1)
    return o1, out
end
ImRect["GetWidth"] = ImRect["GetWidth"] or function(i1)
    jit.off(true)
    local out = C.ImRect_GetWidth(i1)
    return out
end
ImRect["IsInverted"] = ImRect["IsInverted"] or function(i1)
    jit.off(true)
    local out = C.ImRect_IsInverted(i1)
    return out
end
ImRect["Overlaps"] = ImRect["Overlaps"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Overlaps(i1, i2)
    return out
end
ImRect["ToVec4"] = ImRect["ToVec4"] or function(i1)
    jit.off(true)
    local o1 = M.ImVec4_Nil()
    local out = C.ImRect_ToVec4(o1, i1)
    return o1, out
end
ImRect["Translate"] = ImRect["Translate"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_Translate(i1, i2)
    return out
end
ImRect["TranslateX"] = ImRect["TranslateX"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_TranslateX(i1, i2)
    return out
end
ImRect["TranslateY"] = ImRect["TranslateY"] or function(i1, i2)
    jit.off(true)
    local out = C.ImRect_TranslateY(i1, i2)
    return out
end
M.ImRect_Nil = M.ImRect_Nil or function()
    jit.off(true)
    local p = C.ImRect_ImRect_Nil()
    return ffi.gc(p[0], C.ImRect_destroy)
end
M.ImRect_Vec2 = M.ImRect_Vec2 or function(i1, i2)
    jit.off(true)
    local p = C.ImRect_ImRect_Vec2(i1, i2)
    return ffi.gc(p[0], C.ImRect_destroy)
end
M.ImRect_Vec4 = M.ImRect_Vec4 or function(i1)
    jit.off(true)
    local p = C.ImRect_ImRect_Vec4(i1)
    return ffi.gc(p[0], C.ImRect_destroy)
end
M.ImRect_Float = M.ImRect_Float or function(i1, i2, i3, i4)
    jit.off(true)
    local p = C.ImRect_ImRect_Float(i1, i2, i3, i4)
    return ffi.gc(p[0], C.ImRect_destroy)
end
M.ImRect = ImRect
ffi.metatype("ImRect", ImRect)

local ImVec1 = ImVec1 or {}
ImVec1.__index = ImVec1
M.ImVec1_Nil = M.ImVec1_Nil or function()
    jit.off(true)
    local p = C.ImVec1_ImVec1_Nil()
    return ffi.gc(p[0], C.ImVec1_destroy)
end
M.ImVec1_Float = M.ImVec1_Float or function(i1)
    jit.off(true)
    local p = C.ImVec1_ImVec1_Float(i1)
    return ffi.gc(p[0], C.ImVec1_destroy)
end
M.ImVec1 = ImVec1
ffi.metatype("ImVec1", ImVec1)

local ImVec2 = ImVec2 or {}
ImVec2.__index = ImVec2
M.ImVec2_Nil = M.ImVec2_Nil or function()
    jit.off(true)
    local p = C.ImVec2_ImVec2_Nil()
    return ffi.gc(p[0], C.ImVec2_destroy)
end
M.ImVec2_Float = M.ImVec2_Float or function(i1, i2)
    jit.off(true)
    local p = C.ImVec2_ImVec2_Float(i1, i2)
    return ffi.gc(p[0], C.ImVec2_destroy)
end
M.ImVec2 = ImVec2
ffi.metatype("ImVec2", ImVec2)

local ImVec2ih = ImVec2ih or {}
ImVec2ih.__index = ImVec2ih
M.ImVec2ih_Nil = M.ImVec2ih_Nil or function()
    jit.off(true)
    local p = C.ImVec2ih_ImVec2ih_Nil()
    return ffi.gc(p[0], C.ImVec2ih_destroy)
end
M.ImVec2ih_short = M.ImVec2ih_short or function(i1, i2)
    jit.off(true)
    local p = C.ImVec2ih_ImVec2ih_short(i1, i2)
    return ffi.gc(p[0], C.ImVec2ih_destroy)
end
M.ImVec2ih_Vec2 = M.ImVec2ih_Vec2 or function(i1)
    jit.off(true)
    local p = C.ImVec2ih_ImVec2ih_Vec2(i1)
    return ffi.gc(p[0], C.ImVec2ih_destroy)
end
M.ImVec2ih = ImVec2ih
ffi.metatype("ImVec2ih", ImVec2ih)

local ImVec4 = ImVec4 or {}
ImVec4.__index = ImVec4
M.ImVec4_Nil = M.ImVec4_Nil or function()
    jit.off(true)
    local p = C.ImVec4_ImVec4_Nil()
    return ffi.gc(p[0], C.ImVec4_destroy)
end
M.ImVec4_Float = M.ImVec4_Float or function(i1, i2, i3, i4)
    jit.off(true)
    local p = C.ImVec4_ImVec4_Float(i1, i2, i3, i4)
    return ffi.gc(p[0], C.ImVec4_destroy)
end
M.ImVec4 = ImVec4
ffi.metatype("ImVec4", ImVec4)

M.AcceptDragDropPayload = M.AcceptDragDropPayload or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiAcceptDragDropPayload(i1, i2)
    return out
end
M.ActivateItemByID = M.ActivateItemByID or function(i1)
    jit.off(true)
    local out = C.imguiActivateItemByID(i1)
    return out
end
M.AddContextHook = M.AddContextHook or function(i1, i2)
    jit.off(true)
    local out = C.imguiAddContextHook(i1, i2)
    return out
end
M.AddDrawListToDrawDataEx = M.AddDrawListToDrawDataEx or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiAddDrawListToDrawDataEx(i1, i2, i3)
    return out
end
M.AddSettingsHandler = M.AddSettingsHandler or function(i1)
    jit.off(true)
    local out = C.imguiAddSettingsHandler(i1)
    return out
end
M.AlignTextToFramePadding = M.AlignTextToFramePadding or function()
    jit.off(true)
    local out = C.imguiAlignTextToFramePadding()
    return out
end
M.ArrowButton = M.ArrowButton or function(i1, i2)
    jit.off(true)
    local out = C.imguiArrowButton(i1, i2)
    return out
end
M.ArrowButtonEx = M.ArrowButtonEx or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiArrowButtonEx(i1, i2, i3, i4)
    return out
end
M.Begin = M.Begin or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiBegin(i1, i2, i3)
    return out
end
M.BeginBoxSelect = M.BeginBoxSelect or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiBeginBoxSelect(i1, i2, i3, i4)
    return out
end
M.BeginChild_Str = M.BeginChild_Str or function(i1, i2, i3, i4)
    jit.off(true)
    if i2 == nil then
        i2 = M.ImVec2_Float(0, 0)
    end
    if i3 == nil then
        i3 = 0
    end
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiBeginChild_Str(i1, i2, i3, i4)
    return out
end
M.BeginChild_ID = M.BeginChild_ID or function(i1, i2, i3, i4)
    jit.off(true)
    if i2 == nil then
        i2 = M.ImVec2_Float(0, 0)
    end
    if i3 == nil then
        i3 = 0
    end
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiBeginChild_ID(i1, i2, i3, i4)
    return out
end
M.BeginChildEx = M.BeginChildEx or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiBeginChildEx(i1, i2, i3, i4, i5)
    return out
end
M.BeginColumns = M.BeginColumns or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiBeginColumns(i1, i2, i3)
    return out
end
M.BeginCombo = M.BeginCombo or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiBeginCombo(i1, i2, i3)
    return out
end
M.BeginComboPopup = M.BeginComboPopup or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiBeginComboPopup(i1, i2, i3)
    return out
end
M.BeginComboPreview = M.BeginComboPreview or function()
    jit.off(true)
    local out = C.imguiBeginComboPreview()
    return out
end
M.BeginDisabled = M.BeginDisabled or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = true
    end
    local out = C.imguiBeginDisabled(i1)
    return out
end
M.BeginDisabledOverrideReenable = M.BeginDisabledOverrideReenable or function()
    jit.off(true)
    local out = C.imguiBeginDisabledOverrideReenable()
    return out
end
M.BeginDockableDragDropSource = M.BeginDockableDragDropSource or function(i1)
    jit.off(true)
    local out = C.imguiBeginDockableDragDropSource(i1)
    return out
end
M.BeginDockableDragDropTarget = M.BeginDockableDragDropTarget or function(i1)
    jit.off(true)
    local out = C.imguiBeginDockableDragDropTarget(i1)
    return out
end
M.BeginDocked = M.BeginDocked or function(i1, i2)
    jit.off(true)
    local out = C.imguiBeginDocked(i1, i2)
    return out
end
M.BeginDragDropSource = M.BeginDragDropSource or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiBeginDragDropSource(i1)
    return out
end
M.BeginDragDropTarget = M.BeginDragDropTarget or function()
    jit.off(true)
    local out = C.imguiBeginDragDropTarget()
    return out
end
M.BeginDragDropTargetCustom = M.BeginDragDropTargetCustom or function(i1, i2)
    jit.off(true)
    local out = C.imguiBeginDragDropTargetCustom(i1, i2)
    return out
end
M.BeginGroup = M.BeginGroup or function()
    jit.off(true)
    local out = C.imguiBeginGroup()
    return out
end
M.BeginItemTooltip = M.BeginItemTooltip or function()
    jit.off(true)
    local out = C.imguiBeginItemTooltip()
    return out
end
M.BeginListBox = M.BeginListBox or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiBeginListBox(i1, i2)
    return out
end
M.BeginMainMenuBar = M.BeginMainMenuBar or function()
    jit.off(true)
    local out = C.imguiBeginMainMenuBar()
    return out
end
M.BeginMenu = M.BeginMenu or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = true
    end
    local out = C.imguiBeginMenu(i1, i2)
    return out
end
M.BeginMenuBar = M.BeginMenuBar or function()
    jit.off(true)
    local out = C.imguiBeginMenuBar()
    return out
end
M.BeginMenuEx = M.BeginMenuEx or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = true
    end
    local out = C.imguiBeginMenuEx(i1, i2, i3)
    return out
end
M.BeginMultiSelect = M.BeginMultiSelect or function(i1, i2, i3)
    jit.off(true)
    if i2 == nil then
        i2 = -1
    end
    if i3 == nil then
        i3 = -1
    end
    local out = C.imguiBeginMultiSelect(i1, i2, i3)
    return out
end
M.BeginPopup = M.BeginPopup or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiBeginPopup(i1, i2)
    return out
end
M.BeginPopupContextItem = M.BeginPopupContextItem or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 1
    end
    local out = C.imguiBeginPopupContextItem(i1, i2)
    return out
end
M.BeginPopupContextVoid = M.BeginPopupContextVoid or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 1
    end
    local out = C.imguiBeginPopupContextVoid(i1, i2)
    return out
end
M.BeginPopupContextWindow = M.BeginPopupContextWindow or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 1
    end
    local out = C.imguiBeginPopupContextWindow(i1, i2)
    return out
end
M.BeginPopupEx = M.BeginPopupEx or function(i1, i2)
    jit.off(true)
    local out = C.imguiBeginPopupEx(i1, i2)
    return out
end
M.BeginPopupModal = M.BeginPopupModal or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiBeginPopupModal(i1, i2, i3)
    return out
end
M.BeginTabBar = M.BeginTabBar or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiBeginTabBar(i1, i2)
    return out
end
M.BeginTabBarEx = M.BeginTabBarEx or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiBeginTabBarEx(i1, i2, i3)
    return out
end
M.BeginTabItem = M.BeginTabItem or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiBeginTabItem(i1, i2, i3)
    return out
end
M.BeginTable = M.BeginTable or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    if i4 == nil then
        i4 = M.ImVec2_Float(0.0, 0.0)
    end
    if i5 == nil then
        i5 = 0.0
    end
    local out = C.imguiBeginTable(i1, i2, i3, i4, i5)
    return out
end
M.BeginTableEx = M.BeginTableEx or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    if i5 == nil then
        i5 = M.ImVec2_Float(0, 0)
    end
    if i6 == nil then
        i6 = 0.0
    end
    local out = C.imguiBeginTableEx(i1, i2, i3, i4, i5, i6)
    return out
end
M.BeginTooltip = M.BeginTooltip or function()
    jit.off(true)
    local out = C.imguiBeginTooltip()
    return out
end
M.BeginTooltipEx = M.BeginTooltipEx or function(i1, i2)
    jit.off(true)
    local out = C.imguiBeginTooltipEx(i1, i2)
    return out
end
M.BeginTooltipHidden = M.BeginTooltipHidden or function()
    jit.off(true)
    local out = C.imguiBeginTooltipHidden()
    return out
end
M.BeginViewportSideBar = M.BeginViewportSideBar or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiBeginViewportSideBar(i1, i2, i3, i4, i5)
    return out
end
M.BringWindowToDisplayBack = M.BringWindowToDisplayBack or function(i1)
    jit.off(true)
    local out = C.imguiBringWindowToDisplayBack(i1)
    return out
end
M.BringWindowToDisplayBehind = M.BringWindowToDisplayBehind or function(i1, i2)
    jit.off(true)
    local out = C.imguiBringWindowToDisplayBehind(i1, i2)
    return out
end
M.BringWindowToDisplayFront = M.BringWindowToDisplayFront or function(i1)
    jit.off(true)
    local out = C.imguiBringWindowToDisplayFront(i1)
    return out
end
M.BringWindowToFocusFront = M.BringWindowToFocusFront or function(i1)
    jit.off(true)
    local out = C.imguiBringWindowToFocusFront(i1)
    return out
end
M.Bullet = M.Bullet or function()
    jit.off(true)
    local out = C.imguiBullet()
    return out
end
M.BulletText = M.BulletText or function(i1, ...)
    jit.off(true)
    local out = C.imguiBulletText(i1, ...)
    return out
end
M.Button = M.Button or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiButton(i1, i2)
    return out
end
M.ButtonBehavior = M.ButtonBehavior or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = 0
    end
    local out = C.imguiButtonBehavior(i1, i2, i3, i4, i5)
    return out
end
M.ButtonEx = M.ButtonEx or function(i1, i2, i3)
    jit.off(true)
    if i2 == nil then
        i2 = M.ImVec2_Float(0, 0)
    end
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiButtonEx(i1, i2, i3)
    return out
end
M.CalcItemSize = M.CalcItemSize or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiCalcItemSize(o1, i1, i2, i3)
    return o1, out
end
M.CalcItemWidth = M.CalcItemWidth or function()
    jit.off(true)
    local out = C.imguiCalcItemWidth()
    return out
end
M.CalcRoundingFlagsForRectInRect = M.CalcRoundingFlagsForRectInRect or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiCalcRoundingFlagsForRectInRect(i1, i2, i3)
    return out
end
M.CalcTextSize = M.CalcTextSize or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = false
    end
    if i4 == nil then
        i4 = -1.0
    end
    local o1 = M.ImVec2_Nil()
    local out = C.imguiCalcTextSize(o1, i1, i2, i3, i4)
    return o1, out
end
M.CalcTypematicRepeatAmount = M.CalcTypematicRepeatAmount or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiCalcTypematicRepeatAmount(i1, i2, i3, i4)
    return out
end
M.CalcWindowNextAutoFitSize = M.CalcWindowNextAutoFitSize or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiCalcWindowNextAutoFitSize(o1, i1)
    return o1, out
end
M.CalcWrapWidthForPos = M.CalcWrapWidthForPos or function(i1, i2)
    jit.off(true)
    local out = C.imguiCalcWrapWidthForPos(i1, i2)
    return out
end
M.CallContextHooks = M.CallContextHooks or function(i1, i2)
    jit.off(true)
    local out = C.imguiCallContextHooks(i1, i2)
    return out
end
M.Checkbox = M.Checkbox or function(i1, i2)
    jit.off(true)
    local out = C.imguiCheckbox(i1, i2)
    return out
end
M.CheckboxFlags_IntPtr = M.CheckboxFlags_IntPtr or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiCheckboxFlags_IntPtr(i1, i2, i3)
    return out
end
M.CheckboxFlags_UintPtr = M.CheckboxFlags_UintPtr or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiCheckboxFlags_UintPtr(i1, i2, i3)
    return out
end
M.CheckboxFlags_S64Ptr = M.CheckboxFlags_S64Ptr or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiCheckboxFlags_S64Ptr(i1, i2, i3)
    return out
end
M.CheckboxFlags_U64Ptr = M.CheckboxFlags_U64Ptr or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiCheckboxFlags_U64Ptr(i1, i2, i3)
    return out
end
M.ClearActiveID = M.ClearActiveID or function()
    jit.off(true)
    local out = C.imguiClearActiveID()
    return out
end
M.ClearDragDrop = M.ClearDragDrop or function()
    jit.off(true)
    local out = C.imguiClearDragDrop()
    return out
end
M.ClearIniSettings = M.ClearIniSettings or function()
    jit.off(true)
    local out = C.imguiClearIniSettings()
    return out
end
M.ClearWindowSettings = M.ClearWindowSettings or function(i1)
    jit.off(true)
    local out = C.imguiClearWindowSettings(i1)
    return out
end
M.CloseButton = M.CloseButton or function(i1, i2)
    jit.off(true)
    local out = C.imguiCloseButton(i1, i2)
    return out
end
M.CloseCurrentPopup = M.CloseCurrentPopup or function()
    jit.off(true)
    local out = C.imguiCloseCurrentPopup()
    return out
end
M.ClosePopupToLevel = M.ClosePopupToLevel or function(i1, i2)
    jit.off(true)
    local out = C.imguiClosePopupToLevel(i1, i2)
    return out
end
M.ClosePopupsExceptModals = M.ClosePopupsExceptModals or function()
    jit.off(true)
    local out = C.imguiClosePopupsExceptModals()
    return out
end
M.ClosePopupsOverWindow = M.ClosePopupsOverWindow or function(i1, i2)
    jit.off(true)
    local out = C.imguiClosePopupsOverWindow(i1, i2)
    return out
end
M.CollapseButton = M.CollapseButton or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiCollapseButton(i1, i2, i3)
    return out
end
M.CollapsingHeader_TreeNodeFlags = M.CollapsingHeader_TreeNodeFlags or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiCollapsingHeader_TreeNodeFlags(i1, i2)
    return out
end
M.CollapsingHeader_BoolPtr = M.CollapsingHeader_BoolPtr or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiCollapsingHeader_BoolPtr(i1, i2, i3)
    return out
end
M.ColorButton = M.ColorButton or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    if i4 == nil then
        i4 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiColorButton(i1, i2, i3, i4)
    return out
end
M.ColorConvertFloat4ToU32 = M.ColorConvertFloat4ToU32 or function(i1)
    jit.off(true)
    local out = C.imguiColorConvertFloat4ToU32(i1)
    return out
end
M.ColorConvertHSVtoRGB = M.ColorConvertHSVtoRGB or function(i1, i2, i3)
    jit.off(true)
    local o1 = ffi.new("float[1]")
    local o2 = ffi.new("float[1]")
    local o3 = ffi.new("float[1]")
    local out = C.imguiColorConvertHSVtoRGB(i1, i2, i3, o1, o2, o3)
    return o1[0], o2[0], o3[0], out
end
M.ColorConvertRGBtoHSV = M.ColorConvertRGBtoHSV or function(i1, i2, i3)
    jit.off(true)
    local o1 = ffi.new("float[1]")
    local o2 = ffi.new("float[1]")
    local o3 = ffi.new("float[1]")
    local out = C.imguiColorConvertRGBtoHSV(i1, i2, i3, o1, o2, o3)
    return o1[0], o2[0], o3[0], out
end
M.ColorConvertU32ToFloat4 = M.ColorConvertU32ToFloat4 or function(i1)
    jit.off(true)
    local o1 = M.ImVec4_Nil()
    local out = C.imguiColorConvertU32ToFloat4(o1, i1)
    return o1, out
end
M.ColorEdit3 = M.ColorEdit3 or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiColorEdit3(i1, i2, i3)
    return out
end
M.ColorEdit4 = M.ColorEdit4 or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiColorEdit4(i1, i2, i3)
    return out
end
M.ColorEditOptionsPopup = M.ColorEditOptionsPopup or function(i1, i2)
    jit.off(true)
    local out = C.imguiColorEditOptionsPopup(i1, i2)
    return out
end
M.ColorPicker3 = M.ColorPicker3 or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiColorPicker3(i1, i2, i3)
    return out
end
M.ColorPicker4 = M.ColorPicker4 or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiColorPicker4(i1, i2, i3, i4)
    return out
end
M.ColorPickerOptionsPopup = M.ColorPickerOptionsPopup or function(i1, i2)
    jit.off(true)
    local out = C.imguiColorPickerOptionsPopup(i1, i2)
    return out
end
M.ColorTooltip = M.ColorTooltip or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiColorTooltip(i1, i2, i3)
    return out
end
M.Columns = M.Columns or function(i1, i2, i3)
    jit.off(true)
    if i1 == nil then
        i1 = 1
    end
    if i3 == nil then
        i3 = true
    end
    local out = C.imguiColumns(i1, i2, i3)
    return out
end
M.Combo_Str_arr = M.Combo_Str_arr or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = -1
    end
    local out = C.imguiCombo_Str_arr(i1, i2, i3, i4, i5)
    return out
end
M.Combo_Str = M.Combo_Str or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = -1
    end
    local out = C.imguiCombo_Str(i1, i2, i3, i4)
    return out
end
M.Combo_FnStrPtr = M.Combo_FnStrPtr or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i6 == nil then
        i6 = -1
    end
    local out = C.imguiCombo_FnStrPtr(i1, i2, i3, i4, i5, i6)
    return out
end
M.ConvertSingleModFlagToKey = M.ConvertSingleModFlagToKey or function(i1)
    jit.off(true)
    local out = C.imguiConvertSingleModFlagToKey(i1)
    return out
end
M.CreateContext = M.CreateContext or function(i1)
    jit.off(true)
    local out = C.imguiCreateContext(i1)
    return out
end
M.CreateNewWindowSettings = M.CreateNewWindowSettings or function(i1)
    jit.off(true)
    local out = C.imguiCreateNewWindowSettings(i1)
    return out
end
M.DataTypeApplyFromText = M.DataTypeApplyFromText or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiDataTypeApplyFromText(i1, i2, i3, i4, i5)
    return out
end
M.DataTypeApplyOp = M.DataTypeApplyOp or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiDataTypeApplyOp(i1, i2, i3, i4, i5)
    return out
end
M.DataTypeClamp = M.DataTypeClamp or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiDataTypeClamp(i1, i2, i3, i4)
    return out
end
M.DataTypeCompare = M.DataTypeCompare or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDataTypeCompare(i1, i2, i3)
    return out
end
M.DataTypeFormatString = M.DataTypeFormatString or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiDataTypeFormatString(i1, i2, i3, i4, i5)
    return out
end
M.DataTypeGetInfo = M.DataTypeGetInfo or function(i1)
    jit.off(true)
    local out = C.imguiDataTypeGetInfo(i1)
    return out
end
M.DebugAllocHook = M.DebugAllocHook or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiDebugAllocHook(i1, i2, i3, i4)
    return out
end
M.DebugBreakButton = M.DebugBreakButton or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugBreakButton(i1, i2)
    return out
end
M.DebugBreakButtonTooltip = M.DebugBreakButtonTooltip or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugBreakButtonTooltip(i1, i2)
    return out
end
M.DebugBreakClearData = M.DebugBreakClearData or function()
    jit.off(true)
    local out = C.imguiDebugBreakClearData()
    return out
end
M.DebugCheckVersionAndDataLayout = M.DebugCheckVersionAndDataLayout or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    local out = C.imguiDebugCheckVersionAndDataLayout(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DebugDrawCursorPos = M.DebugDrawCursorPos or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 4278190335
    end
    local out = C.imguiDebugDrawCursorPos(i1)
    return out
end
M.DebugDrawItemRect = M.DebugDrawItemRect or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 4278190335
    end
    local out = C.imguiDebugDrawItemRect(i1)
    return out
end
M.DebugDrawLineExtents = M.DebugDrawLineExtents or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 4278190335
    end
    local out = C.imguiDebugDrawLineExtents(i1)
    return out
end
M.DebugFlashStyleColor = M.DebugFlashStyleColor or function(i1)
    jit.off(true)
    local out = C.imguiDebugFlashStyleColor(i1)
    return out
end
M.DebugHookIdInfo = M.DebugHookIdInfo or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiDebugHookIdInfo(i1, i2, i3, i4)
    return out
end
M.DebugLocateItem = M.DebugLocateItem or function(i1)
    jit.off(true)
    local out = C.imguiDebugLocateItem(i1)
    return out
end
M.DebugLocateItemOnHover = M.DebugLocateItemOnHover or function(i1)
    jit.off(true)
    local out = C.imguiDebugLocateItemOnHover(i1)
    return out
end
M.DebugLocateItemResolveWithLastItem = M.DebugLocateItemResolveWithLastItem or function()
    jit.off(true)
    local out = C.imguiDebugLocateItemResolveWithLastItem()
    return out
end
M.DebugLog = M.DebugLog or function(i1, ...)
    jit.off(true)
    local out = C.imguiDebugLog(i1, ...)
    return out
end
M.DebugNodeColumns = M.DebugNodeColumns or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeColumns(i1)
    return out
end
M.DebugNodeDockNode = M.DebugNodeDockNode or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugNodeDockNode(i1, i2)
    return out
end
M.DebugNodeDrawCmdShowMeshAndBoundingBox = M.DebugNodeDrawCmdShowMeshAndBoundingBox or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiDebugNodeDrawCmdShowMeshAndBoundingBox(i1, i2, i3, i4, i5)
    return out
end
M.DebugNodeDrawList = M.DebugNodeDrawList or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiDebugNodeDrawList(i1, i2, i3, i4)
    return out
end
M.DebugNodeFont = M.DebugNodeFont or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeFont(i1)
    return out
end
M.DebugNodeFontGlyph = M.DebugNodeFontGlyph or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugNodeFontGlyph(i1, i2)
    return out
end
M.DebugNodeInputTextState = M.DebugNodeInputTextState or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeInputTextState(i1)
    return out
end
M.DebugNodeMultiSelectState = M.DebugNodeMultiSelectState or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeMultiSelectState(i1)
    return out
end
M.DebugNodePlatformMonitor = M.DebugNodePlatformMonitor or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDebugNodePlatformMonitor(i1, i2, i3)
    return out
end
M.DebugNodeStorage = M.DebugNodeStorage or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugNodeStorage(i1, i2)
    return out
end
M.DebugNodeTabBar = M.DebugNodeTabBar or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugNodeTabBar(i1, i2)
    return out
end
M.DebugNodeTable = M.DebugNodeTable or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeTable(i1)
    return out
end
M.DebugNodeTableSettings = M.DebugNodeTableSettings or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeTableSettings(i1)
    return out
end
M.DebugNodeTypingSelectState = M.DebugNodeTypingSelectState or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeTypingSelectState(i1)
    return out
end
M.DebugNodeViewport = M.DebugNodeViewport or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeViewport(i1)
    return out
end
M.DebugNodeWindow = M.DebugNodeWindow or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugNodeWindow(i1, i2)
    return out
end
M.DebugNodeWindowSettings = M.DebugNodeWindowSettings or function(i1)
    jit.off(true)
    local out = C.imguiDebugNodeWindowSettings(i1)
    return out
end
M.DebugNodeWindowsList = M.DebugNodeWindowsList or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugNodeWindowsList(i1, i2)
    return out
end
M.DebugNodeWindowsListByBeginStackParent = M.DebugNodeWindowsListByBeginStackParent or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDebugNodeWindowsListByBeginStackParent(i1, i2, i3)
    return out
end
M.DebugRenderKeyboardPreview = M.DebugRenderKeyboardPreview or function(i1)
    jit.off(true)
    local out = C.imguiDebugRenderKeyboardPreview(i1)
    return out
end
M.DebugRenderViewportThumbnail = M.DebugRenderViewportThumbnail or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDebugRenderViewportThumbnail(i1, i2, i3)
    return out
end
M.DebugStartItemPicker = M.DebugStartItemPicker or function()
    jit.off(true)
    local out = C.imguiDebugStartItemPicker()
    return out
end
M.DebugTextEncoding = M.DebugTextEncoding or function(i1)
    jit.off(true)
    local out = C.imguiDebugTextEncoding(i1)
    return out
end
M.DebugTextUnformattedWithLocateItem = M.DebugTextUnformattedWithLocateItem or function(i1, i2)
    jit.off(true)
    local out = C.imguiDebugTextUnformattedWithLocateItem(i1, i2)
    return out
end
M.DestroyContext = M.DestroyContext or function(i1)
    jit.off(true)
    local out = C.imguiDestroyContext(i1)
    return out
end
M.DestroyPlatformWindow = M.DestroyPlatformWindow or function(i1)
    jit.off(true)
    local out = C.imguiDestroyPlatformWindow(i1)
    return out
end
M.DestroyPlatformWindows = M.DestroyPlatformWindows or function()
    jit.off(true)
    local out = C.imguiDestroyPlatformWindows()
    return out
end
M.DockBuilderAddNode = M.DockBuilderAddNode or function(i1, i2)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiDockBuilderAddNode(i1, i2)
    return out
end
M.DockBuilderCopyDockSpace = M.DockBuilderCopyDockSpace or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDockBuilderCopyDockSpace(i1, i2, i3)
    return out
end
M.DockBuilderCopyNode = M.DockBuilderCopyNode or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDockBuilderCopyNode(i1, i2, i3)
    return out
end
M.DockBuilderCopyWindowSettings = M.DockBuilderCopyWindowSettings or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockBuilderCopyWindowSettings(i1, i2)
    return out
end
M.DockBuilderDockWindow = M.DockBuilderDockWindow or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockBuilderDockWindow(i1, i2)
    return out
end
M.DockBuilderFinish = M.DockBuilderFinish or function(i1)
    jit.off(true)
    local out = C.imguiDockBuilderFinish(i1)
    return out
end
M.DockBuilderGetCentralNode = M.DockBuilderGetCentralNode or function(i1)
    jit.off(true)
    local out = C.imguiDockBuilderGetCentralNode(i1)
    return out
end
M.DockBuilderGetNode = M.DockBuilderGetNode or function(i1)
    jit.off(true)
    local out = C.imguiDockBuilderGetNode(i1)
    return out
end
M.DockBuilderRemoveNode = M.DockBuilderRemoveNode or function(i1)
    jit.off(true)
    local out = C.imguiDockBuilderRemoveNode(i1)
    return out
end
M.DockBuilderRemoveNodeChildNodes = M.DockBuilderRemoveNodeChildNodes or function(i1)
    jit.off(true)
    local out = C.imguiDockBuilderRemoveNodeChildNodes(i1)
    return out
end
M.DockBuilderRemoveNodeDockedWindows = M.DockBuilderRemoveNodeDockedWindows or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = true
    end
    local out = C.imguiDockBuilderRemoveNodeDockedWindows(i1, i2)
    return out
end
M.DockBuilderSetNodePos = M.DockBuilderSetNodePos or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockBuilderSetNodePos(i1, i2)
    return out
end
M.DockBuilderSetNodeSize = M.DockBuilderSetNodeSize or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockBuilderSetNodeSize(i1, i2)
    return out
end
M.DockBuilderSplitNode = M.DockBuilderSplitNode or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiDockBuilderSplitNode(i1, i2, i3, i4, i5)
    return out
end
M.DockContextCalcDropPosForDocking = M.DockContextCalcDropPosForDocking or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiDockContextCalcDropPosForDocking(i1, i2, i3, i4, i5, i6, o1)
    return o1, out
end
M.DockContextClearNodes = M.DockContextClearNodes or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDockContextClearNodes(i1, i2, i3)
    return out
end
M.DockContextEndFrame = M.DockContextEndFrame or function(i1)
    jit.off(true)
    local out = C.imguiDockContextEndFrame(i1)
    return out
end
M.DockContextFindNodeByID = M.DockContextFindNodeByID or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockContextFindNodeByID(i1, i2)
    return out
end
M.DockContextGenNodeID = M.DockContextGenNodeID or function(i1)
    jit.off(true)
    local out = C.imguiDockContextGenNodeID(i1)
    return out
end
M.DockContextInitialize = M.DockContextInitialize or function(i1)
    jit.off(true)
    local out = C.imguiDockContextInitialize(i1)
    return out
end
M.DockContextNewFrameUpdateDocking = M.DockContextNewFrameUpdateDocking or function(i1)
    jit.off(true)
    local out = C.imguiDockContextNewFrameUpdateDocking(i1)
    return out
end
M.DockContextNewFrameUpdateUndocking = M.DockContextNewFrameUpdateUndocking or function(i1)
    jit.off(true)
    local out = C.imguiDockContextNewFrameUpdateUndocking(i1)
    return out
end
M.DockContextProcessUndockNode = M.DockContextProcessUndockNode or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockContextProcessUndockNode(i1, i2)
    return out
end
M.DockContextProcessUndockWindow = M.DockContextProcessUndockWindow or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = true
    end
    local out = C.imguiDockContextProcessUndockWindow(i1, i2, i3)
    return out
end
M.DockContextQueueDock = M.DockContextQueueDock or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    local out = C.imguiDockContextQueueDock(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DockContextQueueUndockNode = M.DockContextQueueUndockNode or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockContextQueueUndockNode(i1, i2)
    return out
end
M.DockContextQueueUndockWindow = M.DockContextQueueUndockWindow or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockContextQueueUndockWindow(i1, i2)
    return out
end
M.DockContextRebuildNodes = M.DockContextRebuildNodes or function(i1)
    jit.off(true)
    local out = C.imguiDockContextRebuildNodes(i1)
    return out
end
M.DockContextShutdown = M.DockContextShutdown or function(i1)
    jit.off(true)
    local out = C.imguiDockContextShutdown(i1)
    return out
end
M.DockNodeBeginAmendTabBar = M.DockNodeBeginAmendTabBar or function(i1)
    jit.off(true)
    local out = C.imguiDockNodeBeginAmendTabBar(i1)
    return out
end
M.DockNodeEndAmendTabBar = M.DockNodeEndAmendTabBar or function()
    jit.off(true)
    local out = C.imguiDockNodeEndAmendTabBar()
    return out
end
M.DockNodeGetDepth = M.DockNodeGetDepth or function(i1)
    jit.off(true)
    local out = C.imguiDockNodeGetDepth(i1)
    return out
end
M.DockNodeGetRootNode = M.DockNodeGetRootNode or function(i1)
    jit.off(true)
    local out = C.imguiDockNodeGetRootNode(i1)
    return out
end
M.DockNodeGetWindowMenuButtonId = M.DockNodeGetWindowMenuButtonId or function(i1)
    jit.off(true)
    local out = C.imguiDockNodeGetWindowMenuButtonId(i1)
    return out
end
M.DockNodeIsInHierarchyOf = M.DockNodeIsInHierarchyOf or function(i1, i2)
    jit.off(true)
    local out = C.imguiDockNodeIsInHierarchyOf(i1, i2)
    return out
end
M.DockNodeWindowMenuHandler_Default = M.DockNodeWindowMenuHandler_Default or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiDockNodeWindowMenuHandler_Default(i1, i2, i3)
    return out
end
M.DockSpace = M.DockSpace or function(i1, i2, i3, i4)
    jit.off(true)
    if i2 == nil then
        i2 = M.ImVec2_Float(0, 0)
    end
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiDockSpace(i1, i2, i3, i4)
    return out
end
M.DockSpaceOverViewport = M.DockSpaceOverViewport or function(i1, i2, i3, i4)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiDockSpaceOverViewport(i1, i2, i3, i4)
    return out
end
M.DragBehavior = M.DragBehavior or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    local out = C.imguiDragBehavior(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.DragFloat = M.DragFloat or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0.0
    end
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = "%.3f"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragFloat(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragFloat2 = M.DragFloat2 or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0.0
    end
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = "%.3f"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragFloat2(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragFloat3 = M.DragFloat3 or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0.0
    end
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = "%.3f"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragFloat3(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragFloat4 = M.DragFloat4 or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0.0
    end
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = "%.3f"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragFloat4(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragFloatRange2 = M.DragFloatRange2 or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i4 == nil then
        i4 = 1.0
    end
    if i5 == nil then
        i5 = 0.0
    end
    if i6 == nil then
        i6 = 0.0
    end
    if i7 == nil then
        i7 = "%.3f"
    end
    if i9 == nil then
        i9 = 0
    end
    local out = C.imguiDragFloatRange2(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.DragInt = M.DragInt or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0
    end
    if i5 == nil then
        i5 = 0
    end
    if i6 == nil then
        i6 = "%d"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragInt(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragInt2 = M.DragInt2 or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0
    end
    if i5 == nil then
        i5 = 0
    end
    if i6 == nil then
        i6 = "%d"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragInt2(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragInt3 = M.DragInt3 or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0
    end
    if i5 == nil then
        i5 = 0
    end
    if i6 == nil then
        i6 = "%d"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragInt3(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragInt4 = M.DragInt4 or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i3 == nil then
        i3 = 1.0
    end
    if i4 == nil then
        i4 = 0
    end
    if i5 == nil then
        i5 = 0
    end
    if i6 == nil then
        i6 = "%d"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiDragInt4(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.DragIntRange2 = M.DragIntRange2 or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i4 == nil then
        i4 = 1.0
    end
    if i5 == nil then
        i5 = 0
    end
    if i6 == nil then
        i6 = 0
    end
    if i7 == nil then
        i7 = "%d"
    end
    if i9 == nil then
        i9 = 0
    end
    local out = C.imguiDragIntRange2(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.DragScalar = M.DragScalar or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    if i4 == nil then
        i4 = 1.0
    end
    if i8 == nil then
        i8 = 0
    end
    local out = C.imguiDragScalar(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.DragScalarN = M.DragScalarN or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i5 == nil then
        i5 = 1.0
    end
    if i9 == nil then
        i9 = 0
    end
    local out = C.imguiDragScalarN(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.Dummy = M.Dummy or function(i1)
    jit.off(true)
    local out = C.imguiDummy(i1)
    return out
end
M.End = M.End or function()
    jit.off(true)
    local out = C.imguiEnd()
    return out
end
M.EndBoxSelect = M.EndBoxSelect or function(i1, i2)
    jit.off(true)
    local out = C.imguiEndBoxSelect(i1, i2)
    return out
end
M.EndChild = M.EndChild or function()
    jit.off(true)
    local out = C.imguiEndChild()
    return out
end
M.EndColumns = M.EndColumns or function()
    jit.off(true)
    local out = C.imguiEndColumns()
    return out
end
M.EndCombo = M.EndCombo or function()
    jit.off(true)
    local out = C.imguiEndCombo()
    return out
end
M.EndComboPreview = M.EndComboPreview or function()
    jit.off(true)
    local out = C.imguiEndComboPreview()
    return out
end
M.EndDisabled = M.EndDisabled or function()
    jit.off(true)
    local out = C.imguiEndDisabled()
    return out
end
M.EndDisabledOverrideReenable = M.EndDisabledOverrideReenable or function()
    jit.off(true)
    local out = C.imguiEndDisabledOverrideReenable()
    return out
end
M.EndDragDropSource = M.EndDragDropSource or function()
    jit.off(true)
    local out = C.imguiEndDragDropSource()
    return out
end
M.EndDragDropTarget = M.EndDragDropTarget or function()
    jit.off(true)
    local out = C.imguiEndDragDropTarget()
    return out
end
M.EndFrame = M.EndFrame or function()
    jit.off(true)
    local out = C.imguiEndFrame()
    return out
end
M.EndGroup = M.EndGroup or function()
    jit.off(true)
    local out = C.imguiEndGroup()
    return out
end
M.EndListBox = M.EndListBox or function()
    jit.off(true)
    local out = C.imguiEndListBox()
    return out
end
M.EndMainMenuBar = M.EndMainMenuBar or function()
    jit.off(true)
    local out = C.imguiEndMainMenuBar()
    return out
end
M.EndMenu = M.EndMenu or function()
    jit.off(true)
    local out = C.imguiEndMenu()
    return out
end
M.EndMenuBar = M.EndMenuBar or function()
    jit.off(true)
    local out = C.imguiEndMenuBar()
    return out
end
M.EndMultiSelect = M.EndMultiSelect or function()
    jit.off(true)
    local out = C.imguiEndMultiSelect()
    return out
end
M.EndPopup = M.EndPopup or function()
    jit.off(true)
    local out = C.imguiEndPopup()
    return out
end
M.EndTabBar = M.EndTabBar or function()
    jit.off(true)
    local out = C.imguiEndTabBar()
    return out
end
M.EndTabItem = M.EndTabItem or function()
    jit.off(true)
    local out = C.imguiEndTabItem()
    return out
end
M.EndTable = M.EndTable or function()
    jit.off(true)
    local out = C.imguiEndTable()
    return out
end
M.EndTooltip = M.EndTooltip or function()
    jit.off(true)
    local out = C.imguiEndTooltip()
    return out
end
M.ErrorCheckEndFrameRecover = M.ErrorCheckEndFrameRecover or function(i1, i2)
    jit.off(true)
    local out = C.imguiErrorCheckEndFrameRecover(i1, i2)
    return out
end
M.ErrorCheckEndWindowRecover = M.ErrorCheckEndWindowRecover or function(i1, i2)
    jit.off(true)
    local out = C.imguiErrorCheckEndWindowRecover(i1, i2)
    return out
end
M.ErrorCheckUsingSetCursorPosToExtendParentBoundaries = M.ErrorCheckUsingSetCursorPosToExtendParentBoundaries or
                                                            function()
        jit.off(true)
        local out = C.imguiErrorCheckUsingSetCursorPosToExtendParentBoundaries()
        return out
    end
M.FindBestWindowPosForPopup = M.FindBestWindowPosForPopup or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiFindBestWindowPosForPopup(o1, i1)
    return o1, out
end
M.FindBestWindowPosForPopupEx = M.FindBestWindowPosForPopupEx or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiFindBestWindowPosForPopupEx(o1, i1, i2, i3, i4, i5, i6)
    return o1, out
end
M.FindBlockingModal = M.FindBlockingModal or function(i1)
    jit.off(true)
    local out = C.imguiFindBlockingModal(i1)
    return out
end
M.FindBottomMostVisibleWindowWithinBeginStack = M.FindBottomMostVisibleWindowWithinBeginStack or function(i1)
    jit.off(true)
    local out = C.imguiFindBottomMostVisibleWindowWithinBeginStack(i1)
    return out
end
M.FindHoveredViewportFromPlatformWindowStack = M.FindHoveredViewportFromPlatformWindowStack or function(i1)
    jit.off(true)
    local out = C.imguiFindHoveredViewportFromPlatformWindowStack(i1)
    return out
end
M.FindHoveredWindowEx = M.FindHoveredWindowEx or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiFindHoveredWindowEx(i1, i2, i3, i4)
    return out
end
M.FindOrCreateColumns = M.FindOrCreateColumns or function(i1, i2)
    jit.off(true)
    local out = C.imguiFindOrCreateColumns(i1, i2)
    return out
end
M.FindRenderedTextEnd = M.FindRenderedTextEnd or function(i1, i2)
    jit.off(true)
    local out = C.imguiFindRenderedTextEnd(i1, i2)
    return out
end
M.FindSettingsHandler = M.FindSettingsHandler or function(i1)
    jit.off(true)
    local out = C.imguiFindSettingsHandler(i1)
    return out
end
M.FindViewportByID = M.FindViewportByID or function(i1)
    jit.off(true)
    local out = C.imguiFindViewportByID(i1)
    return out
end
M.FindViewportByPlatformHandle = M.FindViewportByPlatformHandle or function(i1)
    jit.off(true)
    local out = C.imguiFindViewportByPlatformHandle(i1)
    return out
end
M.FindWindowByID = M.FindWindowByID or function(i1)
    jit.off(true)
    local out = C.imguiFindWindowByID(i1)
    return out
end
M.FindWindowByName = M.FindWindowByName or function(i1)
    jit.off(true)
    local out = C.imguiFindWindowByName(i1)
    return out
end
M.FindWindowDisplayIndex = M.FindWindowDisplayIndex or function(i1)
    jit.off(true)
    local out = C.imguiFindWindowDisplayIndex(i1)
    return out
end
M.FindWindowSettingsByID = M.FindWindowSettingsByID or function(i1)
    jit.off(true)
    local out = C.imguiFindWindowSettingsByID(i1)
    return out
end
M.FindWindowSettingsByWindow = M.FindWindowSettingsByWindow or function(i1)
    jit.off(true)
    local out = C.imguiFindWindowSettingsByWindow(i1)
    return out
end
M.FixupKeyChord = M.FixupKeyChord or function(i1)
    jit.off(true)
    local out = C.imguiFixupKeyChord(i1)
    return out
end
M.FocusItem = M.FocusItem or function()
    jit.off(true)
    local out = C.imguiFocusItem()
    return out
end
M.FocusTopMostWindowUnderOne = M.FocusTopMostWindowUnderOne or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiFocusTopMostWindowUnderOne(i1, i2, i3, i4)
    return out
end
M.FocusWindow = M.FocusWindow or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiFocusWindow(i1, i2)
    return out
end
M.GcAwakeTransientWindowBuffers = M.GcAwakeTransientWindowBuffers or function(i1)
    jit.off(true)
    local out = C.imguiGcAwakeTransientWindowBuffers(i1)
    return out
end
M.GcCompactTransientMiscBuffers = M.GcCompactTransientMiscBuffers or function()
    jit.off(true)
    local out = C.imguiGcCompactTransientMiscBuffers()
    return out
end
M.GcCompactTransientWindowBuffers = M.GcCompactTransientWindowBuffers or function(i1)
    jit.off(true)
    local out = C.imguiGcCompactTransientWindowBuffers(i1)
    return out
end
M.GetActiveID = M.GetActiveID or function()
    jit.off(true)
    local out = C.imguiGetActiveID()
    return out
end
M.GetAllocatorFunctions = M.GetAllocatorFunctions or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiGetAllocatorFunctions(i1, i2, i3)
    return out
end
M.GetBackgroundDrawList = M.GetBackgroundDrawList or function(i1)
    jit.off(true)
    local out = C.imguiGetBackgroundDrawList(i1)
    return out
end
M.GetBoxSelectState = M.GetBoxSelectState or function(i1)
    jit.off(true)
    local out = C.imguiGetBoxSelectState(i1)
    return out
end
M.GetClipboardText = M.GetClipboardText or function()
    jit.off(true)
    local out = C.imguiGetClipboardText()
    return out
end
M.GetColorU32_Col = M.GetColorU32_Col or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 1.0
    end
    local out = C.imguiGetColorU32_Col(i1, i2)
    return out
end
M.GetColorU32_Vec4 = M.GetColorU32_Vec4 or function(i1)
    jit.off(true)
    local out = C.imguiGetColorU32_Vec4(i1)
    return out
end
M.GetColorU32_U32 = M.GetColorU32_U32 or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 1.0
    end
    local out = C.imguiGetColorU32_U32(i1, i2)
    return out
end
M.GetColumnIndex = M.GetColumnIndex or function()
    jit.off(true)
    local out = C.imguiGetColumnIndex()
    return out
end
M.GetColumnNormFromOffset = M.GetColumnNormFromOffset or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetColumnNormFromOffset(i1, i2)
    return out
end
M.GetColumnOffset = M.GetColumnOffset or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiGetColumnOffset(i1)
    return out
end
M.GetColumnOffsetFromNorm = M.GetColumnOffsetFromNorm or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetColumnOffsetFromNorm(i1, i2)
    return out
end
M.GetColumnWidth = M.GetColumnWidth or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiGetColumnWidth(i1)
    return out
end
M.GetColumnsCount = M.GetColumnsCount or function()
    jit.off(true)
    local out = C.imguiGetColumnsCount()
    return out
end
M.GetColumnsID = M.GetColumnsID or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetColumnsID(i1, i2)
    return out
end
M.GetContentRegionAvail = M.GetContentRegionAvail or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetContentRegionAvail(o1)
    return o1, out
end
M.GetCurrentContext = M.GetCurrentContext or function()
    jit.off(true)
    local out = C.imguiGetCurrentContext()
    return out
end
M.GetCurrentFocusScope = M.GetCurrentFocusScope or function()
    jit.off(true)
    local out = C.imguiGetCurrentFocusScope()
    return out
end
M.GetCurrentTabBar = M.GetCurrentTabBar or function()
    jit.off(true)
    local out = C.imguiGetCurrentTabBar()
    return out
end
M.GetCurrentTable = M.GetCurrentTable or function()
    jit.off(true)
    local out = C.imguiGetCurrentTable()
    return out
end
M.GetCurrentWindow = M.GetCurrentWindow or function()
    jit.off(true)
    local out = C.imguiGetCurrentWindow()
    return out
end
M.GetCurrentWindowRead = M.GetCurrentWindowRead or function()
    jit.off(true)
    local out = C.imguiGetCurrentWindowRead()
    return out
end
M.GetCursorPos = M.GetCursorPos or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetCursorPos(o1)
    return o1, out
end
M.GetCursorPosX = M.GetCursorPosX or function()
    jit.off(true)
    local out = C.imguiGetCursorPosX()
    return out
end
M.GetCursorPosY = M.GetCursorPosY or function()
    jit.off(true)
    local out = C.imguiGetCursorPosY()
    return out
end
M.GetCursorScreenPos = M.GetCursorScreenPos or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetCursorScreenPos(o1)
    return o1, out
end
M.GetCursorStartPos = M.GetCursorStartPos or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetCursorStartPos(o1)
    return o1, out
end
M.GetDefaultFont = M.GetDefaultFont or function()
    jit.off(true)
    local out = C.imguiGetDefaultFont()
    return out
end
M.GetDragDropPayload = M.GetDragDropPayload or function()
    jit.off(true)
    local out = C.imguiGetDragDropPayload()
    return out
end
M.GetDrawData = M.GetDrawData or function()
    jit.off(true)
    local out = C.imguiGetDrawData()
    return out
end
M.GetDrawListSharedData = M.GetDrawListSharedData or function()
    jit.off(true)
    local out = C.imguiGetDrawListSharedData()
    return out
end
M.GetFocusID = M.GetFocusID or function()
    jit.off(true)
    local out = C.imguiGetFocusID()
    return out
end
M.GetFont = M.GetFont or function()
    jit.off(true)
    local out = C.imguiGetFont()
    return out
end
M.GetFontSize = M.GetFontSize or function()
    jit.off(true)
    local out = C.imguiGetFontSize()
    return out
end
M.GetFontTexUvWhitePixel = M.GetFontTexUvWhitePixel or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetFontTexUvWhitePixel(o1)
    return o1, out
end
M.GetForegroundDrawList_ViewportPtr = M.GetForegroundDrawList_ViewportPtr or function(i1)
    jit.off(true)
    local out = C.imguiGetForegroundDrawList_ViewportPtr(i1)
    return out
end
M.GetForegroundDrawList_WindowPtr = M.GetForegroundDrawList_WindowPtr or function(i1)
    jit.off(true)
    local out = C.imguiGetForegroundDrawList_WindowPtr(i1)
    return out
end
M.GetFrameCount = M.GetFrameCount or function()
    jit.off(true)
    local out = C.imguiGetFrameCount()
    return out
end
M.GetFrameHeight = M.GetFrameHeight or function()
    jit.off(true)
    local out = C.imguiGetFrameHeight()
    return out
end
M.GetFrameHeightWithSpacing = M.GetFrameHeightWithSpacing or function()
    jit.off(true)
    local out = C.imguiGetFrameHeightWithSpacing()
    return out
end
M.GetHoveredID = M.GetHoveredID or function()
    jit.off(true)
    local out = C.imguiGetHoveredID()
    return out
end
M.GetID_Str = M.GetID_Str or function(i1)
    jit.off(true)
    local out = C.imguiGetID_Str(i1)
    return out
end
M.GetID_StrStr = M.GetID_StrStr or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetID_StrStr(i1, i2)
    return out
end
M.GetID_Ptr = M.GetID_Ptr or function(i1)
    jit.off(true)
    local out = C.imguiGetID_Ptr(i1)
    return out
end
M.GetID_Int = M.GetID_Int or function(i1)
    jit.off(true)
    local out = C.imguiGetID_Int(i1)
    return out
end
M.GetIDWithSeed_Str = M.GetIDWithSeed_Str or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiGetIDWithSeed_Str(i1, i2, i3)
    return out
end
M.GetIDWithSeed_Int = M.GetIDWithSeed_Int or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetIDWithSeed_Int(i1, i2)
    return out
end
M.GetIO = M.GetIO or function()
    jit.off(true)
    local out = C.imguiGetIO()
    return out
end
M.GetInputTextState = M.GetInputTextState or function(i1)
    jit.off(true)
    local out = C.imguiGetInputTextState(i1)
    return out
end
M.GetItemFlags = M.GetItemFlags or function()
    jit.off(true)
    local out = C.imguiGetItemFlags()
    return out
end
M.GetItemID = M.GetItemID or function()
    jit.off(true)
    local out = C.imguiGetItemID()
    return out
end
M.GetItemRectMax = M.GetItemRectMax or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetItemRectMax(o1)
    return o1, out
end
M.GetItemRectMin = M.GetItemRectMin or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetItemRectMin(o1)
    return o1, out
end
M.GetItemRectSize = M.GetItemRectSize or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetItemRectSize(o1)
    return o1, out
end
M.GetItemStatusFlags = M.GetItemStatusFlags or function()
    jit.off(true)
    local out = C.imguiGetItemStatusFlags()
    return out
end
M.GetKeyChordName = M.GetKeyChordName or function(i1)
    jit.off(true)
    local out = C.imguiGetKeyChordName(i1)
    return out
end
M.GetKeyData_ContextPtr = M.GetKeyData_ContextPtr or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetKeyData_ContextPtr(i1, i2)
    return out
end
M.GetKeyData_Key = M.GetKeyData_Key or function(i1)
    jit.off(true)
    local out = C.imguiGetKeyData_Key(i1)
    return out
end
M.GetKeyMagnitude2d = M.GetKeyMagnitude2d or function(i1, i2, i3, i4)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetKeyMagnitude2d(o1, i1, i2, i3, i4)
    return o1, out
end
M.GetKeyName = M.GetKeyName or function(i1)
    jit.off(true)
    local out = C.imguiGetKeyName(i1)
    return out
end
M.GetKeyOwner = M.GetKeyOwner or function(i1)
    jit.off(true)
    local out = C.imguiGetKeyOwner(i1)
    return out
end
M.GetKeyOwnerData = M.GetKeyOwnerData or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetKeyOwnerData(i1, i2)
    return out
end
M.GetKeyPressedAmount = M.GetKeyPressedAmount or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiGetKeyPressedAmount(i1, i2, i3)
    return out
end
M.GetMainViewport = M.GetMainViewport or function()
    jit.off(true)
    local out = C.imguiGetMainViewport()
    return out
end
M.GetMouseClickedCount = M.GetMouseClickedCount or function(i1)
    jit.off(true)
    local out = C.imguiGetMouseClickedCount(i1)
    return out
end
M.GetMouseCursor = M.GetMouseCursor or function()
    jit.off(true)
    local out = C.imguiGetMouseCursor()
    return out
end
M.GetMouseDragDelta = M.GetMouseDragDelta or function(i1, i2)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    if i2 == nil then
        i2 = -1.0
    end
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetMouseDragDelta(o1, i1, i2)
    return o1, out
end
M.GetMousePos = M.GetMousePos or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetMousePos(o1)
    return o1, out
end
M.GetMousePosOnOpeningCurrentPopup = M.GetMousePosOnOpeningCurrentPopup or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetMousePosOnOpeningCurrentPopup(o1)
    return o1, out
end
M.GetMultiSelectState = M.GetMultiSelectState or function(i1)
    jit.off(true)
    local out = C.imguiGetMultiSelectState(i1)
    return out
end
M.GetNavTweakPressedAmount = M.GetNavTweakPressedAmount or function(i1)
    jit.off(true)
    local out = C.imguiGetNavTweakPressedAmount(i1)
    return out
end
M.GetPlatformIO = M.GetPlatformIO or function()
    jit.off(true)
    local out = C.imguiGetPlatformIO()
    return out
end
M.GetPopupAllowedExtentRect = M.GetPopupAllowedExtentRect or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetPopupAllowedExtentRect(i1, i2)
    return out
end
M.GetScrollMaxX = M.GetScrollMaxX or function()
    jit.off(true)
    local out = C.imguiGetScrollMaxX()
    return out
end
M.GetScrollMaxY = M.GetScrollMaxY or function()
    jit.off(true)
    local out = C.imguiGetScrollMaxY()
    return out
end
M.GetScrollX = M.GetScrollX or function()
    jit.off(true)
    local out = C.imguiGetScrollX()
    return out
end
M.GetScrollY = M.GetScrollY or function()
    jit.off(true)
    local out = C.imguiGetScrollY()
    return out
end
M.GetShortcutRoutingData = M.GetShortcutRoutingData or function(i1)
    jit.off(true)
    local out = C.imguiGetShortcutRoutingData(i1)
    return out
end
M.GetStateStorage = M.GetStateStorage or function()
    jit.off(true)
    local out = C.imguiGetStateStorage()
    return out
end
M.GetStyle = M.GetStyle or function()
    jit.off(true)
    local out = C.imguiGetStyle()
    return out
end
M.GetStyleColorName = M.GetStyleColorName or function(i1)
    jit.off(true)
    local out = C.imguiGetStyleColorName(i1)
    return out
end
M.GetStyleColorVec4 = M.GetStyleColorVec4 or function(i1)
    jit.off(true)
    local out = C.imguiGetStyleColorVec4(i1)
    return out
end
M.GetStyleVarInfo = M.GetStyleVarInfo or function(i1)
    jit.off(true)
    local out = C.imguiGetStyleVarInfo(i1)
    return out
end
M.GetTextLineHeight = M.GetTextLineHeight or function()
    jit.off(true)
    local out = C.imguiGetTextLineHeight()
    return out
end
M.GetTextLineHeightWithSpacing = M.GetTextLineHeightWithSpacing or function()
    jit.off(true)
    local out = C.imguiGetTextLineHeightWithSpacing()
    return out
end
M.GetTime = M.GetTime or function()
    jit.off(true)
    local out = C.imguiGetTime()
    return out
end
M.GetTopMostAndVisiblePopupModal = M.GetTopMostAndVisiblePopupModal or function()
    jit.off(true)
    local out = C.imguiGetTopMostAndVisiblePopupModal()
    return out
end
M.GetTopMostPopupModal = M.GetTopMostPopupModal or function()
    jit.off(true)
    local out = C.imguiGetTopMostPopupModal()
    return out
end
M.GetTreeNodeToLabelSpacing = M.GetTreeNodeToLabelSpacing or function()
    jit.off(true)
    local out = C.imguiGetTreeNodeToLabelSpacing()
    return out
end
M.GetTypematicRepeatRate = M.GetTypematicRepeatRate or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiGetTypematicRepeatRate(i1, i2, i3)
    return out
end
M.GetTypingSelectRequest = M.GetTypingSelectRequest or function(i1)
    jit.off(true)
    local out = C.imguiGetTypingSelectRequest(i1)
    return out
end
M.GetVersion = M.GetVersion or function()
    jit.off(true)
    local out = C.imguiGetVersion()
    return out
end
M.GetViewportPlatformMonitor = M.GetViewportPlatformMonitor or function(i1)
    jit.off(true)
    local out = C.imguiGetViewportPlatformMonitor(i1)
    return out
end
M.GetWindowAlwaysWantOwnTabBar = M.GetWindowAlwaysWantOwnTabBar or function(i1)
    jit.off(true)
    local out = C.imguiGetWindowAlwaysWantOwnTabBar(i1)
    return out
end
M.GetWindowDockID = M.GetWindowDockID or function()
    jit.off(true)
    local out = C.imguiGetWindowDockID()
    return out
end
M.GetWindowDockNode = M.GetWindowDockNode or function()
    jit.off(true)
    local out = C.imguiGetWindowDockNode()
    return out
end
M.GetWindowDpiScale = M.GetWindowDpiScale or function()
    jit.off(true)
    local out = C.imguiGetWindowDpiScale()
    return out
end
M.GetWindowDrawList = M.GetWindowDrawList or function()
    jit.off(true)
    local out = C.imguiGetWindowDrawList()
    return out
end
M.GetWindowHeight = M.GetWindowHeight or function()
    jit.off(true)
    local out = C.imguiGetWindowHeight()
    return out
end
M.GetWindowPos = M.GetWindowPos or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetWindowPos(o1)
    return o1, out
end
M.GetWindowResizeBorderID = M.GetWindowResizeBorderID or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetWindowResizeBorderID(i1, i2)
    return out
end
M.GetWindowResizeCornerID = M.GetWindowResizeCornerID or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetWindowResizeCornerID(i1, i2)
    return out
end
M.GetWindowScrollbarID = M.GetWindowScrollbarID or function(i1, i2)
    jit.off(true)
    local out = C.imguiGetWindowScrollbarID(i1, i2)
    return out
end
M.GetWindowScrollbarRect = M.GetWindowScrollbarRect or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiGetWindowScrollbarRect(i1, i2, i3)
    return out
end
M.GetWindowSize = M.GetWindowSize or function()
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiGetWindowSize(o1)
    return o1, out
end
M.GetWindowViewport = M.GetWindowViewport or function()
    jit.off(true)
    local out = C.imguiGetWindowViewport()
    return out
end
M.GetWindowWidth = M.GetWindowWidth or function()
    jit.off(true)
    local out = C.imguiGetWindowWidth()
    return out
end
M.ImAbs_Int = M.ImAbs_Int or function(i1)
    jit.off(true)
    local out = C.imguiImAbs_Int(i1)
    return out
end
M.ImAbs_Float = M.ImAbs_Float or function(i1)
    jit.off(true)
    local out = C.imguiImAbs_Float(i1)
    return out
end
M.ImAbs_double = M.ImAbs_double or function(i1)
    jit.off(true)
    local out = C.imguiImAbs_double(i1)
    return out
end
M.ImAlphaBlendColors = M.ImAlphaBlendColors or function(i1, i2)
    jit.off(true)
    local out = C.imguiImAlphaBlendColors(i1, i2)
    return out
end
M.ImBezierCubicCalc = M.ImBezierCubicCalc or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImBezierCubicCalc(o1, i1, i2, i3, i4, i5)
    return o1, out
end
M.ImBezierCubicClosestPoint = M.ImBezierCubicClosestPoint or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImBezierCubicClosestPoint(o1, i1, i2, i3, i4, i5, i6)
    return o1, out
end
M.ImBezierCubicClosestPointCasteljau = M.ImBezierCubicClosestPointCasteljau or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImBezierCubicClosestPointCasteljau(o1, i1, i2, i3, i4, i5, i6)
    return o1, out
end
M.ImBezierQuadraticCalc = M.ImBezierQuadraticCalc or function(i1, i2, i3, i4)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImBezierQuadraticCalc(o1, i1, i2, i3, i4)
    return o1, out
end
M.ImBitArrayClearAllBits = M.ImBitArrayClearAllBits or function(i1, i2)
    jit.off(true)
    local out = C.imguiImBitArrayClearAllBits(i1, i2)
    return out
end
M.ImBitArrayClearBit = M.ImBitArrayClearBit or function(i1, i2)
    jit.off(true)
    local out = C.imguiImBitArrayClearBit(i1, i2)
    return out
end
M.ImBitArrayGetStorageSizeInBytes = M.ImBitArrayGetStorageSizeInBytes or function(i1)
    jit.off(true)
    local out = C.imguiImBitArrayGetStorageSizeInBytes(i1)
    return out
end
M.ImBitArraySetBit = M.ImBitArraySetBit or function(i1, i2)
    jit.off(true)
    local out = C.imguiImBitArraySetBit(i1, i2)
    return out
end
M.ImBitArraySetBitRange = M.ImBitArraySetBitRange or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImBitArraySetBitRange(i1, i2, i3)
    return out
end
M.ImBitArrayTestBit = M.ImBitArrayTestBit or function(i1, i2)
    jit.off(true)
    local out = C.imguiImBitArrayTestBit(i1, i2)
    return out
end
M.ImCharIsBlankA = M.ImCharIsBlankA or function(i1)
    jit.off(true)
    local out = C.imguiImCharIsBlankA(i1)
    return out
end
M.ImCharIsBlankW = M.ImCharIsBlankW or function(i1)
    jit.off(true)
    local out = C.imguiImCharIsBlankW(i1)
    return out
end
M.ImCharIsXdigitA = M.ImCharIsXdigitA or function(i1)
    jit.off(true)
    local out = C.imguiImCharIsXdigitA(i1)
    return out
end
M.ImClamp = M.ImClamp or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImClamp(o1, i1, i2, i3)
    return o1, out
end
M.ImDot = M.ImDot or function(i1, i2)
    jit.off(true)
    local out = C.imguiImDot(i1, i2)
    return out
end
M.ImExponentialMovingAverage = M.ImExponentialMovingAverage or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImExponentialMovingAverage(i1, i2, i3)
    return out
end
M.ImFileClose = M.ImFileClose or function(i1)
    jit.off(true)
    local out = C.imguiImFileClose(i1)
    return out
end
M.ImFileGetSize = M.ImFileGetSize or function(i1)
    jit.off(true)
    local out = C.imguiImFileGetSize(i1)
    return out
end
M.ImFileLoadToMemory = M.ImFileLoadToMemory or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local o1 = ffi.new("size_t[1]")
    local out = C.imguiImFileLoadToMemory(i1, i2, o1, i3)
    return o1[0], out
end
M.ImFileOpen = M.ImFileOpen or function(i1, i2)
    jit.off(true)
    local out = C.imguiImFileOpen(i1, i2)
    return out
end
M.ImFileRead = M.ImFileRead or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiImFileRead(i1, i2, i3, i4)
    return out
end
M.ImFileWrite = M.ImFileWrite or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiImFileWrite(i1, i2, i3, i4)
    return out
end
M.ImFloor_Float = M.ImFloor_Float or function(i1)
    jit.off(true)
    local out = C.imguiImFloor_Float(i1)
    return out
end
M.ImFloor_Vec2 = M.ImFloor_Vec2 or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImFloor_Vec2(o1, i1)
    return o1, out
end
M.ImFontAtlasBuildFinish = M.ImFontAtlasBuildFinish or function(i1)
    jit.off(true)
    local out = C.imguiImFontAtlasBuildFinish(i1)
    return out
end
M.ImFontAtlasBuildInit = M.ImFontAtlasBuildInit or function(i1)
    jit.off(true)
    local out = C.imguiImFontAtlasBuildInit(i1)
    return out
end
M.ImFontAtlasBuildMultiplyCalcLookupTable = M.ImFontAtlasBuildMultiplyCalcLookupTable or function(i1, i2)
    jit.off(true)
    local out = C.imguiImFontAtlasBuildMultiplyCalcLookupTable(i1, i2)
    return out
end
M.ImFontAtlasBuildMultiplyRectAlpha8 = M.ImFontAtlasBuildMultiplyRectAlpha8 or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    local out = C.imguiImFontAtlasBuildMultiplyRectAlpha8(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.ImFontAtlasBuildPackCustomRects = M.ImFontAtlasBuildPackCustomRects or function(i1, i2)
    jit.off(true)
    local out = C.imguiImFontAtlasBuildPackCustomRects(i1, i2)
    return out
end
M.ImFontAtlasBuildRender32bppRectFromString = M.ImFontAtlasBuildRender32bppRectFromString or
                                                  function(i1, i2, i3, i4, i5, i6, i7, i8)
        jit.off(true)
        local out = C.imguiImFontAtlasBuildRender32bppRectFromString(i1, i2, i3, i4, i5, i6, i7, i8)
        return out
    end
M.ImFontAtlasBuildRender8bppRectFromString = M.ImFontAtlasBuildRender8bppRectFromString or
                                                 function(i1, i2, i3, i4, i5, i6, i7, i8)
        jit.off(true)
        local out = C.imguiImFontAtlasBuildRender8bppRectFromString(i1, i2, i3, i4, i5, i6, i7, i8)
        return out
    end
M.ImFontAtlasBuildSetupFont = M.ImFontAtlasBuildSetupFont or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiImFontAtlasBuildSetupFont(i1, i2, i3, i4, i5)
    return out
end
M.ImFontAtlasGetBuilderForStbTruetype = M.ImFontAtlasGetBuilderForStbTruetype or function()
    jit.off(true)
    local out = C.imguiImFontAtlasGetBuilderForStbTruetype()
    return out
end
M.ImFontAtlasUpdateConfigDataPointers = M.ImFontAtlasUpdateConfigDataPointers or function(i1)
    jit.off(true)
    local out = C.imguiImFontAtlasUpdateConfigDataPointers(i1)
    return out
end
M.ImFormatString = M.ImFormatString or function(i1, i2, i3, ...)
    jit.off(true)
    local out = C.imguiImFormatString(i1, i2, i3, ...)
    return out
end
M.ImFormatStringToTempBuffer = M.ImFormatStringToTempBuffer or function(i1, i2, i3, ...)
    jit.off(true)
    local out = C.imguiImFormatStringToTempBuffer(i1, i2, i3, ...)
    return out
end
M.ImHashData = M.ImHashData or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiImHashData(i1, i2, i3)
    return out
end
M.ImHashStr = M.ImHashStr or function(i1, i2, i3)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiImHashStr(i1, i2, i3)
    return out
end
M.ImInvLength = M.ImInvLength or function(i1, i2)
    jit.off(true)
    local out = C.imguiImInvLength(i1, i2)
    return out
end
M.ImIsFloatAboveGuaranteedIntegerPrecision = M.ImIsFloatAboveGuaranteedIntegerPrecision or function(i1)
    jit.off(true)
    local out = C.imguiImIsFloatAboveGuaranteedIntegerPrecision(i1)
    return out
end
M.ImIsPowerOfTwo_Int = M.ImIsPowerOfTwo_Int or function(i1)
    jit.off(true)
    local out = C.imguiImIsPowerOfTwo_Int(i1)
    return out
end
M.ImIsPowerOfTwo_U64 = M.ImIsPowerOfTwo_U64 or function(i1)
    jit.off(true)
    local out = C.imguiImIsPowerOfTwo_U64(i1)
    return out
end
M.ImLengthSqr_Vec2 = M.ImLengthSqr_Vec2 or function(i1)
    jit.off(true)
    local out = C.imguiImLengthSqr_Vec2(i1)
    return out
end
M.ImLengthSqr_Vec4 = M.ImLengthSqr_Vec4 or function(i1)
    jit.off(true)
    local out = C.imguiImLengthSqr_Vec4(i1)
    return out
end
M.ImLerp_Vec2Float = M.ImLerp_Vec2Float or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImLerp_Vec2Float(o1, i1, i2, i3)
    return o1, out
end
M.ImLerp_Vec2Vec2 = M.ImLerp_Vec2Vec2 or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImLerp_Vec2Vec2(o1, i1, i2, i3)
    return o1, out
end
M.ImLerp_Vec4 = M.ImLerp_Vec4 or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec4_Nil()
    local out = C.imguiImLerp_Vec4(o1, i1, i2, i3)
    return o1, out
end
M.ImLineClosestPoint = M.ImLineClosestPoint or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImLineClosestPoint(o1, i1, i2, i3)
    return o1, out
end
M.ImLinearRemapClamp = M.ImLinearRemapClamp or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiImLinearRemapClamp(i1, i2, i3, i4, i5)
    return out
end
M.ImLinearSweep = M.ImLinearSweep or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImLinearSweep(i1, i2, i3)
    return out
end
M.ImLog_Float = M.ImLog_Float or function(i1)
    jit.off(true)
    local out = C.imguiImLog_Float(i1)
    return out
end
M.ImLog_double = M.ImLog_double or function(i1)
    jit.off(true)
    local out = C.imguiImLog_double(i1)
    return out
end
M.ImLowerBound = M.ImLowerBound or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImLowerBound(i1, i2, i3)
    return out
end
M.ImMax = M.ImMax or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImMax(o1, i1, i2)
    return o1, out
end
M.ImMin = M.ImMin or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImMin(o1, i1, i2)
    return o1, out
end
M.ImModPositive = M.ImModPositive or function(i1, i2)
    jit.off(true)
    local out = C.imguiImModPositive(i1, i2)
    return out
end
M.ImMul = M.ImMul or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImMul(o1, i1, i2)
    return o1, out
end
M.ImParseFormatFindEnd = M.ImParseFormatFindEnd or function(i1)
    jit.off(true)
    local out = C.imguiImParseFormatFindEnd(i1)
    return out
end
M.ImParseFormatFindStart = M.ImParseFormatFindStart or function(i1)
    jit.off(true)
    local out = C.imguiImParseFormatFindStart(i1)
    return out
end
M.ImParseFormatPrecision = M.ImParseFormatPrecision or function(i1, i2)
    jit.off(true)
    local out = C.imguiImParseFormatPrecision(i1, i2)
    return out
end
M.ImParseFormatSanitizeForPrinting = M.ImParseFormatSanitizeForPrinting or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImParseFormatSanitizeForPrinting(i1, i2, i3)
    return out
end
M.ImParseFormatSanitizeForScanning = M.ImParseFormatSanitizeForScanning or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImParseFormatSanitizeForScanning(i1, i2, i3)
    return out
end
M.ImParseFormatTrimDecorations = M.ImParseFormatTrimDecorations or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImParseFormatTrimDecorations(i1, i2, i3)
    return out
end
M.ImPow_Float = M.ImPow_Float or function(i1, i2)
    jit.off(true)
    local out = C.imguiImPow_Float(i1, i2)
    return out
end
M.ImPow_double = M.ImPow_double or function(i1, i2)
    jit.off(true)
    local out = C.imguiImPow_double(i1, i2)
    return out
end
M.ImQsort = M.ImQsort or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiImQsort(i1, i2, i3, i4)
    return out
end
M.ImRotate = M.ImRotate or function(i1, i2, i3)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImRotate(o1, i1, i2, i3)
    return o1, out
end
M.ImRsqrt_Float = M.ImRsqrt_Float or function(i1)
    jit.off(true)
    local out = C.imguiImRsqrt_Float(i1)
    return out
end
M.ImRsqrt_double = M.ImRsqrt_double or function(i1)
    jit.off(true)
    local out = C.imguiImRsqrt_double(i1)
    return out
end
M.ImSaturate = M.ImSaturate or function(i1)
    jit.off(true)
    local out = C.imguiImSaturate(i1)
    return out
end
M.ImSign_Float = M.ImSign_Float or function(i1)
    jit.off(true)
    local out = C.imguiImSign_Float(i1)
    return out
end
M.ImSign_double = M.ImSign_double or function(i1)
    jit.off(true)
    local out = C.imguiImSign_double(i1)
    return out
end
M.ImStrSkipBlank = M.ImStrSkipBlank or function(i1)
    jit.off(true)
    local out = C.imguiImStrSkipBlank(i1)
    return out
end
M.ImStrTrimBlanks = M.ImStrTrimBlanks or function(i1)
    jit.off(true)
    local out = C.imguiImStrTrimBlanks(i1)
    return out
end
M.ImStrbolW = M.ImStrbolW or function(i1, i2)
    jit.off(true)
    local out = C.imguiImStrbolW(i1, i2)
    return out
end
M.ImStrchrRange = M.ImStrchrRange or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImStrchrRange(i1, i2, i3)
    return out
end
M.ImStrdup = M.ImStrdup or function(i1)
    jit.off(true)
    local out = C.imguiImStrdup(i1)
    return out
end
M.ImStrdupcpy = M.ImStrdupcpy or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImStrdupcpy(i1, i2, i3)
    return out
end
M.ImStreolRange = M.ImStreolRange or function(i1, i2)
    jit.off(true)
    local out = C.imguiImStreolRange(i1, i2)
    return out
end
M.ImStricmp = M.ImStricmp or function(i1, i2)
    jit.off(true)
    local out = C.imguiImStricmp(i1, i2)
    return out
end
M.ImStristr = M.ImStristr or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiImStristr(i1, i2, i3, i4)
    return out
end
M.ImStrlenW = M.ImStrlenW or function(i1)
    jit.off(true)
    local out = C.imguiImStrlenW(i1)
    return out
end
M.ImStrncpy = M.ImStrncpy or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImStrncpy(i1, i2, i3)
    return out
end
M.ImStrnicmp = M.ImStrnicmp or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImStrnicmp(i1, i2, i3)
    return out
end
M.ImTextCharFromUtf8 = M.ImTextCharFromUtf8 or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImTextCharFromUtf8(i1, i2, i3)
    return out
end
M.ImTextCharToUtf8 = M.ImTextCharToUtf8 or function(i1, i2)
    jit.off(true)
    local out = C.imguiImTextCharToUtf8(i1, i2)
    return out
end
M.ImTextCountCharsFromUtf8 = M.ImTextCountCharsFromUtf8 or function(i1, i2)
    jit.off(true)
    local out = C.imguiImTextCountCharsFromUtf8(i1, i2)
    return out
end
M.ImTextCountLines = M.ImTextCountLines or function(i1, i2)
    jit.off(true)
    local out = C.imguiImTextCountLines(i1, i2)
    return out
end
M.ImTextCountUtf8BytesFromChar = M.ImTextCountUtf8BytesFromChar or function(i1, i2)
    jit.off(true)
    local out = C.imguiImTextCountUtf8BytesFromChar(i1, i2)
    return out
end
M.ImTextCountUtf8BytesFromStr = M.ImTextCountUtf8BytesFromStr or function(i1, i2)
    jit.off(true)
    local out = C.imguiImTextCountUtf8BytesFromStr(i1, i2)
    return out
end
M.ImTextFindPreviousUtf8Codepoint = M.ImTextFindPreviousUtf8Codepoint or function(i1, i2)
    jit.off(true)
    local out = C.imguiImTextFindPreviousUtf8Codepoint(i1, i2)
    return out
end
M.ImTextStrFromUtf8 = M.ImTextStrFromUtf8 or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiImTextStrFromUtf8(i1, i2, i3, i4, i5)
    return out
end
M.ImTextStrToUtf8 = M.ImTextStrToUtf8 or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiImTextStrToUtf8(i1, i2, i3, i4)
    return out
end
M.ImToUpper = M.ImToUpper or function(i1)
    jit.off(true)
    local out = C.imguiImToUpper(i1)
    return out
end
M.ImTriangleArea = M.ImTriangleArea or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImTriangleArea(i1, i2, i3)
    return out
end
M.ImTriangleBarycentricCoords = M.ImTriangleBarycentricCoords or function(i1, i2, i3, i4)
    jit.off(true)
    local o1 = ffi.new("float[1]")
    local o2 = ffi.new("float[1]")
    local o3 = ffi.new("float[1]")
    local out = C.imguiImTriangleBarycentricCoords(i1, i2, i3, i4, o1, o2, o3)
    return o1[0], o2[0], o3[0], out
end
M.ImTriangleClosestPoint = M.ImTriangleClosestPoint or function(i1, i2, i3, i4)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImTriangleClosestPoint(o1, i1, i2, i3, i4)
    return o1, out
end
M.ImTriangleContainsPoint = M.ImTriangleContainsPoint or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiImTriangleContainsPoint(i1, i2, i3, i4)
    return out
end
M.ImTriangleIsClockwise = M.ImTriangleIsClockwise or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiImTriangleIsClockwise(i1, i2, i3)
    return out
end
M.ImTrunc_Float = M.ImTrunc_Float or function(i1)
    jit.off(true)
    local out = C.imguiImTrunc_Float(i1)
    return out
end
M.ImTrunc_Vec2 = M.ImTrunc_Vec2 or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiImTrunc_Vec2(o1, i1)
    return o1, out
end
M.ImUpperPowerOfTwo = M.ImUpperPowerOfTwo or function(i1)
    jit.off(true)
    local out = C.imguiImUpperPowerOfTwo(i1)
    return out
end
M.Image = M.Image or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i3 == nil then
        i3 = M.ImVec2_Float(0, 0)
    end
    if i4 == nil then
        i4 = M.ImVec2_Float(1, 1)
    end
    if i5 == nil then
        i5 = M.ImVec4_Float(1, 1, 1, 1)
    end
    if i6 == nil then
        i6 = M.ImVec4_Float(0, 0, 0, 0)
    end
    local ptr = ffi.cast("void *", i1)
    _common.textures[tostring(ptr)] = i1
    i1 = ptr
    local out = C.imguiImage(i1, i2, i3, i4, i5, i6)
    return out
end
M.ImageButton = M.ImageButton or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i4 == nil then
        i4 = M.ImVec2_Float(0, 0)
    end
    if i5 == nil then
        i5 = M.ImVec2_Float(1, 1)
    end
    if i6 == nil then
        i6 = M.ImVec4_Float(0, 0, 0, 0)
    end
    if i7 == nil then
        i7 = M.ImVec4_Float(1, 1, 1, 1)
    end
    local ptr = ffi.cast("void *", i2)
    _common.textures[tostring(ptr)] = i2
    i2 = ptr
    local out = C.imguiImageButton(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.ImageButtonEx = M.ImageButtonEx or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    if i8 == nil then
        i8 = 0
    end
    local ptr = ffi.cast("void *", i2)
    _common.textures[tostring(ptr)] = i2
    i2 = ptr
    local out = C.imguiImageButtonEx(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.Indent = M.Indent or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0.0
    end
    local out = C.imguiIndent(i1)
    return out
end
M.Initialize = M.Initialize or function()
    jit.off(true)
    local out = C.imguiInitialize()
    return out
end
M.InputDouble = M.InputDouble or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i3 == nil then
        i3 = 0.0
    end
    if i4 == nil then
        i4 = 0.0
    end
    if i5 == nil then
        i5 = "%.6f"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiInputDouble(i1, i2, i3, i4, i5, i6)
    return out
end
M.InputFloat = M.InputFloat or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i3 == nil then
        i3 = 0.0
    end
    if i4 == nil then
        i4 = 0.0
    end
    if i5 == nil then
        i5 = "%.3f"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiInputFloat(i1, i2, i3, i4, i5, i6)
    return out
end
M.InputFloat2 = M.InputFloat2 or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = "%.3f"
    end
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiInputFloat2(i1, i2, i3, i4)
    return out
end
M.InputFloat3 = M.InputFloat3 or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = "%.3f"
    end
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiInputFloat3(i1, i2, i3, i4)
    return out
end
M.InputFloat4 = M.InputFloat4 or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = "%.3f"
    end
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiInputFloat4(i1, i2, i3, i4)
    return out
end
M.InputInt = M.InputInt or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i3 == nil then
        i3 = 1
    end
    if i4 == nil then
        i4 = 100
    end
    if i5 == nil then
        i5 = 0
    end
    local out = C.imguiInputInt(i1, i2, i3, i4, i5)
    return out
end
M.InputInt2 = M.InputInt2 or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiInputInt2(i1, i2, i3)
    return out
end
M.InputInt3 = M.InputInt3 or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiInputInt3(i1, i2, i3)
    return out
end
M.InputInt4 = M.InputInt4 or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiInputInt4(i1, i2, i3)
    return out
end
M.InputScalar = M.InputScalar or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiInputScalar(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.InputScalarN = M.InputScalarN or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    if i8 == nil then
        i8 = 0
    end
    local out = C.imguiInputScalarN(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.InputText = M.InputText or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiInputText(i1, i2, i3, i4, i5, i6)
    return out
end
M.InputTextDeactivateHook = M.InputTextDeactivateHook or function(i1)
    jit.off(true)
    local out = C.imguiInputTextDeactivateHook(i1)
    return out
end
M.InputTextEx = M.InputTextEx or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    local out = C.imguiInputTextEx(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.InputTextMultiline = M.InputTextMultiline or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i4 == nil then
        i4 = M.ImVec2_Float(0, 0)
    end
    if i5 == nil then
        i5 = 0
    end
    local out = C.imguiInputTextMultiline(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.InputTextWithHint = M.InputTextWithHint or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i5 == nil then
        i5 = 0
    end
    local out = C.imguiInputTextWithHint(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.InvisibleButton = M.InvisibleButton or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiInvisibleButton(i1, i2, i3)
    return out
end
M.IsActiveIdUsingNavDir = M.IsActiveIdUsingNavDir or function(i1)
    jit.off(true)
    local out = C.imguiIsActiveIdUsingNavDir(i1)
    return out
end
M.IsAliasKey = M.IsAliasKey or function(i1)
    jit.off(true)
    local out = C.imguiIsAliasKey(i1)
    return out
end
M.IsAnyItemActive = M.IsAnyItemActive or function()
    jit.off(true)
    local out = C.imguiIsAnyItemActive()
    return out
end
M.IsAnyItemFocused = M.IsAnyItemFocused or function()
    jit.off(true)
    local out = C.imguiIsAnyItemFocused()
    return out
end
M.IsAnyItemHovered = M.IsAnyItemHovered or function()
    jit.off(true)
    local out = C.imguiIsAnyItemHovered()
    return out
end
M.IsAnyMouseDown = M.IsAnyMouseDown or function()
    jit.off(true)
    local out = C.imguiIsAnyMouseDown()
    return out
end
M.IsClippedEx = M.IsClippedEx or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsClippedEx(i1, i2)
    return out
end
M.IsDragDropActive = M.IsDragDropActive or function()
    jit.off(true)
    local out = C.imguiIsDragDropActive()
    return out
end
M.IsDragDropPayloadBeingAccepted = M.IsDragDropPayloadBeingAccepted or function()
    jit.off(true)
    local out = C.imguiIsDragDropPayloadBeingAccepted()
    return out
end
M.IsGamepadKey = M.IsGamepadKey or function(i1)
    jit.off(true)
    local out = C.imguiIsGamepadKey(i1)
    return out
end
M.IsItemActivated = M.IsItemActivated or function()
    jit.off(true)
    local out = C.imguiIsItemActivated()
    return out
end
M.IsItemActive = M.IsItemActive or function()
    jit.off(true)
    local out = C.imguiIsItemActive()
    return out
end
M.IsItemClicked = M.IsItemClicked or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiIsItemClicked(i1)
    return out
end
M.IsItemDeactivated = M.IsItemDeactivated or function()
    jit.off(true)
    local out = C.imguiIsItemDeactivated()
    return out
end
M.IsItemDeactivatedAfterEdit = M.IsItemDeactivatedAfterEdit or function()
    jit.off(true)
    local out = C.imguiIsItemDeactivatedAfterEdit()
    return out
end
M.IsItemEdited = M.IsItemEdited or function()
    jit.off(true)
    local out = C.imguiIsItemEdited()
    return out
end
M.IsItemFocused = M.IsItemFocused or function()
    jit.off(true)
    local out = C.imguiIsItemFocused()
    return out
end
M.IsItemHovered = M.IsItemHovered or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiIsItemHovered(i1)
    return out
end
M.IsItemToggledOpen = M.IsItemToggledOpen or function()
    jit.off(true)
    local out = C.imguiIsItemToggledOpen()
    return out
end
M.IsItemToggledSelection = M.IsItemToggledSelection or function()
    jit.off(true)
    local out = C.imguiIsItemToggledSelection()
    return out
end
M.IsItemVisible = M.IsItemVisible or function()
    jit.off(true)
    local out = C.imguiIsItemVisible()
    return out
end
M.IsKeyChordPressed_Nil = M.IsKeyChordPressed_Nil or function(i1)
    jit.off(true)
    local out = C.imguiIsKeyChordPressed_Nil(i1)
    return out
end
M.IsKeyChordPressed_InputFlags = M.IsKeyChordPressed_InputFlags or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiIsKeyChordPressed_InputFlags(i1, i2, i3)
    return out
end
M.IsKeyDown_Nil = M.IsKeyDown_Nil or function(i1)
    jit.off(true)
    local out = C.imguiIsKeyDown_Nil(i1)
    return out
end
M.IsKeyDown_ID = M.IsKeyDown_ID or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsKeyDown_ID(i1, i2)
    return out
end
M.IsKeyPressed_Bool = M.IsKeyPressed_Bool or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = true
    end
    local out = C.imguiIsKeyPressed_Bool(i1, i2)
    return out
end
M.IsKeyPressed_InputFlags = M.IsKeyPressed_InputFlags or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiIsKeyPressed_InputFlags(i1, i2, i3)
    return out
end
M.IsKeyReleased_Nil = M.IsKeyReleased_Nil or function(i1)
    jit.off(true)
    local out = C.imguiIsKeyReleased_Nil(i1)
    return out
end
M.IsKeyReleased_ID = M.IsKeyReleased_ID or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsKeyReleased_ID(i1, i2)
    return out
end
M.IsKeyboardKey = M.IsKeyboardKey or function(i1)
    jit.off(true)
    local out = C.imguiIsKeyboardKey(i1)
    return out
end
M.IsLegacyKey = M.IsLegacyKey or function(i1)
    jit.off(true)
    local out = C.imguiIsLegacyKey(i1)
    return out
end
M.IsModKey = M.IsModKey or function(i1)
    jit.off(true)
    local out = C.imguiIsModKey(i1)
    return out
end
M.IsMouseClicked_Bool = M.IsMouseClicked_Bool or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = false
    end
    local out = C.imguiIsMouseClicked_Bool(i1, i2)
    return out
end
M.IsMouseClicked_InputFlags = M.IsMouseClicked_InputFlags or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiIsMouseClicked_InputFlags(i1, i2, i3)
    return out
end
M.IsMouseDoubleClicked_Nil = M.IsMouseDoubleClicked_Nil or function(i1)
    jit.off(true)
    local out = C.imguiIsMouseDoubleClicked_Nil(i1)
    return out
end
M.IsMouseDoubleClicked_ID = M.IsMouseDoubleClicked_ID or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsMouseDoubleClicked_ID(i1, i2)
    return out
end
M.IsMouseDown_Nil = M.IsMouseDown_Nil or function(i1)
    jit.off(true)
    local out = C.imguiIsMouseDown_Nil(i1)
    return out
end
M.IsMouseDown_ID = M.IsMouseDown_ID or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsMouseDown_ID(i1, i2)
    return out
end
M.IsMouseDragPastThreshold = M.IsMouseDragPastThreshold or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = -1.0
    end
    local out = C.imguiIsMouseDragPastThreshold(i1, i2)
    return out
end
M.IsMouseDragging = M.IsMouseDragging or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = -1.0
    end
    local out = C.imguiIsMouseDragging(i1, i2)
    return out
end
M.IsMouseHoveringRect = M.IsMouseHoveringRect or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = true
    end
    local out = C.imguiIsMouseHoveringRect(i1, i2, i3)
    return out
end
M.IsMouseKey = M.IsMouseKey or function(i1)
    jit.off(true)
    local out = C.imguiIsMouseKey(i1)
    return out
end
M.IsMousePosValid = M.IsMousePosValid or function(i1)
    jit.off(true)
    local out = C.imguiIsMousePosValid(i1)
    return out
end
M.IsMouseReleased_Nil = M.IsMouseReleased_Nil or function(i1)
    jit.off(true)
    local out = C.imguiIsMouseReleased_Nil(i1)
    return out
end
M.IsMouseReleased_ID = M.IsMouseReleased_ID or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsMouseReleased_ID(i1, i2)
    return out
end
M.IsNamedKey = M.IsNamedKey or function(i1)
    jit.off(true)
    local out = C.imguiIsNamedKey(i1)
    return out
end
M.IsNamedKeyOrMod = M.IsNamedKeyOrMod or function(i1)
    jit.off(true)
    local out = C.imguiIsNamedKeyOrMod(i1)
    return out
end
M.IsPopupOpen_Str = M.IsPopupOpen_Str or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiIsPopupOpen_Str(i1, i2)
    return out
end
M.IsPopupOpen_ID = M.IsPopupOpen_ID or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsPopupOpen_ID(i1, i2)
    return out
end
M.IsRectVisible_Nil = M.IsRectVisible_Nil or function(i1)
    jit.off(true)
    local out = C.imguiIsRectVisible_Nil(i1)
    return out
end
M.IsRectVisible_Vec2 = M.IsRectVisible_Vec2 or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsRectVisible_Vec2(i1, i2)
    return out
end
M.IsWindowAbove = M.IsWindowAbove or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsWindowAbove(i1, i2)
    return out
end
M.IsWindowAppearing = M.IsWindowAppearing or function()
    jit.off(true)
    local out = C.imguiIsWindowAppearing()
    return out
end
M.IsWindowChildOf = M.IsWindowChildOf or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiIsWindowChildOf(i1, i2, i3, i4)
    return out
end
M.IsWindowCollapsed = M.IsWindowCollapsed or function()
    jit.off(true)
    local out = C.imguiIsWindowCollapsed()
    return out
end
M.IsWindowContentHoverable = M.IsWindowContentHoverable or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiIsWindowContentHoverable(i1, i2)
    return out
end
M.IsWindowDocked = M.IsWindowDocked or function()
    jit.off(true)
    local out = C.imguiIsWindowDocked()
    return out
end
M.IsWindowFocused = M.IsWindowFocused or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiIsWindowFocused(i1)
    return out
end
M.IsWindowHovered = M.IsWindowHovered or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiIsWindowHovered(i1)
    return out
end
M.IsWindowNavFocusable = M.IsWindowNavFocusable or function(i1)
    jit.off(true)
    local out = C.imguiIsWindowNavFocusable(i1)
    return out
end
M.IsWindowWithinBeginStackOf = M.IsWindowWithinBeginStackOf or function(i1, i2)
    jit.off(true)
    local out = C.imguiIsWindowWithinBeginStackOf(i1, i2)
    return out
end
M.ItemAdd = M.ItemAdd or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiItemAdd(i1, i2, i3, i4)
    return out
end
M.ItemHoverable = M.ItemHoverable or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiItemHoverable(i1, i2, i3)
    return out
end
M.ItemSize_Vec2 = M.ItemSize_Vec2 or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = -1.0
    end
    local out = C.imguiItemSize_Vec2(i1, i2)
    return out
end
M.ItemSize_Rect = M.ItemSize_Rect or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = -1.0
    end
    local out = C.imguiItemSize_Rect(i1, i2)
    return out
end
M.KeepAliveID = M.KeepAliveID or function(i1)
    jit.off(true)
    local out = C.imguiKeepAliveID(i1)
    return out
end
M.LabelText = M.LabelText or function(i1, i2, ...)
    jit.off(true)
    local out = C.imguiLabelText(i1, i2, ...)
    return out
end
M.ListBox_Str_arr = M.ListBox_Str_arr or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = -1
    end
    local out = C.imguiListBox_Str_arr(i1, i2, i3, i4, i5)
    return out
end
M.ListBox_FnStrPtr = M.ListBox_FnStrPtr or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i6 == nil then
        i6 = -1
    end
    local out = C.imguiListBox_FnStrPtr(i1, i2, i3, i4, i5, i6)
    return out
end
M.LoadIniSettingsFromDisk = M.LoadIniSettingsFromDisk or function(i1)
    jit.off(true)
    local out = C.imguiLoadIniSettingsFromDisk(i1)
    return out
end
M.LoadIniSettingsFromMemory = M.LoadIniSettingsFromMemory or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiLoadIniSettingsFromMemory(i1, i2)
    return out
end
M.LocalizeGetMsg = M.LocalizeGetMsg or function(i1)
    jit.off(true)
    local out = C.imguiLocalizeGetMsg(i1)
    return out
end
M.LocalizeRegisterEntries = M.LocalizeRegisterEntries or function(i1, i2)
    jit.off(true)
    local out = C.imguiLocalizeRegisterEntries(i1, i2)
    return out
end
M.LogBegin = M.LogBegin or function(i1, i2)
    jit.off(true)
    local out = C.imguiLogBegin(i1, i2)
    return out
end
M.LogButtons = M.LogButtons or function()
    jit.off(true)
    local out = C.imguiLogButtons()
    return out
end
M.LogFinish = M.LogFinish or function()
    jit.off(true)
    local out = C.imguiLogFinish()
    return out
end
M.LogRenderedText = M.LogRenderedText or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiLogRenderedText(i1, i2, i3)
    return out
end
M.LogSetNextTextDecoration = M.LogSetNextTextDecoration or function(i1, i2)
    jit.off(true)
    local out = C.imguiLogSetNextTextDecoration(i1, i2)
    return out
end
M.LogText = M.LogText or function(i1, ...)
    jit.off(true)
    local out = C.imguiLogText(i1, ...)
    return out
end
M.LogToBuffer = M.LogToBuffer or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiLogToBuffer(i1)
    return out
end
M.LogToClipboard = M.LogToClipboard or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiLogToClipboard(i1)
    return out
end
M.LogToFile = M.LogToFile or function(i1, i2)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiLogToFile(i1, i2)
    return out
end
M.LogToTTY = M.LogToTTY or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiLogToTTY(i1)
    return out
end
M.MarkIniSettingsDirty_Nil = M.MarkIniSettingsDirty_Nil or function()
    jit.off(true)
    local out = C.imguiMarkIniSettingsDirty_Nil()
    return out
end
M.MarkIniSettingsDirty_WindowPtr = M.MarkIniSettingsDirty_WindowPtr or function(i1)
    jit.off(true)
    local out = C.imguiMarkIniSettingsDirty_WindowPtr(i1)
    return out
end
M.MarkItemEdited = M.MarkItemEdited or function(i1)
    jit.off(true)
    local out = C.imguiMarkItemEdited(i1)
    return out
end
M.MemAlloc = M.MemAlloc or function(i1)
    jit.off(true)
    local out = C.imguiMemAlloc(i1)
    return out
end
M.MemFree = M.MemFree or function(i1)
    jit.off(true)
    local out = C.imguiMemFree(i1)
    return out
end
M.MenuItem_Bool = M.MenuItem_Bool or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = false
    end
    if i4 == nil then
        i4 = true
    end
    local out = C.imguiMenuItem_Bool(i1, i2, i3, i4)
    return out
end
M.MenuItem_BoolPtr = M.MenuItem_BoolPtr or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = true
    end
    local out = C.imguiMenuItem_BoolPtr(i1, i2, i3, i4)
    return out
end
M.MenuItemEx = M.MenuItemEx or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i4 == nil then
        i4 = false
    end
    if i5 == nil then
        i5 = true
    end
    local out = C.imguiMenuItemEx(i1, i2, i3, i4, i5)
    return out
end
M.MouseButtonToKey = M.MouseButtonToKey or function(i1)
    jit.off(true)
    local out = C.imguiMouseButtonToKey(i1)
    return out
end
M.MultiSelectAddSetAll = M.MultiSelectAddSetAll or function(i1, i2)
    jit.off(true)
    local out = C.imguiMultiSelectAddSetAll(i1, i2)
    return out
end
M.MultiSelectAddSetRange = M.MultiSelectAddSetRange or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiMultiSelectAddSetRange(i1, i2, i3, i4, i5)
    return out
end
M.MultiSelectItemFooter = M.MultiSelectItemFooter or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiMultiSelectItemFooter(i1, i2, i3)
    return out
end
M.MultiSelectItemHeader = M.MultiSelectItemHeader or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiMultiSelectItemHeader(i1, i2, i3)
    return out
end
M.NavClearPreferredPosForAxis = M.NavClearPreferredPosForAxis or function(i1)
    jit.off(true)
    local out = C.imguiNavClearPreferredPosForAxis(i1)
    return out
end
M.NavHighlightActivated = M.NavHighlightActivated or function(i1)
    jit.off(true)
    local out = C.imguiNavHighlightActivated(i1)
    return out
end
M.NavInitRequestApplyResult = M.NavInitRequestApplyResult or function()
    jit.off(true)
    local out = C.imguiNavInitRequestApplyResult()
    return out
end
M.NavInitWindow = M.NavInitWindow or function(i1, i2)
    jit.off(true)
    local out = C.imguiNavInitWindow(i1, i2)
    return out
end
M.NavMoveRequestApplyResult = M.NavMoveRequestApplyResult or function()
    jit.off(true)
    local out = C.imguiNavMoveRequestApplyResult()
    return out
end
M.NavMoveRequestButNoResultYet = M.NavMoveRequestButNoResultYet or function()
    jit.off(true)
    local out = C.imguiNavMoveRequestButNoResultYet()
    return out
end
M.NavMoveRequestCancel = M.NavMoveRequestCancel or function()
    jit.off(true)
    local out = C.imguiNavMoveRequestCancel()
    return out
end
M.NavMoveRequestForward = M.NavMoveRequestForward or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiNavMoveRequestForward(i1, i2, i3, i4)
    return out
end
M.NavMoveRequestResolveWithLastItem = M.NavMoveRequestResolveWithLastItem or function(i1)
    jit.off(true)
    local out = C.imguiNavMoveRequestResolveWithLastItem(i1)
    return out
end
M.NavMoveRequestResolveWithPastTreeNode = M.NavMoveRequestResolveWithPastTreeNode or function(i1, i2)
    jit.off(true)
    local out = C.imguiNavMoveRequestResolveWithPastTreeNode(i1, i2)
    return out
end
M.NavMoveRequestSubmit = M.NavMoveRequestSubmit or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiNavMoveRequestSubmit(i1, i2, i3, i4)
    return out
end
M.NavMoveRequestTryWrapping = M.NavMoveRequestTryWrapping or function(i1, i2)
    jit.off(true)
    local out = C.imguiNavMoveRequestTryWrapping(i1, i2)
    return out
end
M.NavRestoreHighlightAfterMove = M.NavRestoreHighlightAfterMove or function()
    jit.off(true)
    local out = C.imguiNavRestoreHighlightAfterMove()
    return out
end
M.NavUpdateCurrentWindowIsScrollPushableX = M.NavUpdateCurrentWindowIsScrollPushableX or function()
    jit.off(true)
    local out = C.imguiNavUpdateCurrentWindowIsScrollPushableX()
    return out
end
M.NewFrame = M.NewFrame or function()
    jit.off(true)
    local out = C.imguiNewFrame()
    return out
end
M.NewLine = M.NewLine or function()
    jit.off(true)
    local out = C.imguiNewLine()
    return out
end
M.NextColumn = M.NextColumn or function()
    jit.off(true)
    local out = C.imguiNextColumn()
    return out
end
M.OpenPopup_Str = M.OpenPopup_Str or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiOpenPopup_Str(i1, i2)
    return out
end
M.OpenPopup_ID = M.OpenPopup_ID or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiOpenPopup_ID(i1, i2)
    return out
end
M.OpenPopupEx = M.OpenPopupEx or function(i1, i2)
    jit.off(true)
    local out = C.imguiOpenPopupEx(i1, i2)
    return out
end
M.OpenPopupOnItemClick = M.OpenPopupOnItemClick or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 1
    end
    local out = C.imguiOpenPopupOnItemClick(i1, i2)
    return out
end
M.PlotEx = M.PlotEx or function(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    jit.off(true)
    local out = C.imguiPlotEx(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    return out
end
M.PlotHistogram_FloatPtr = M.PlotHistogram_FloatPtr or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    if i6 == nil then
        i6 = FLT_MAX
    end
    if i7 == nil then
        i7 = FLT_MAX
    end
    if i8 == nil then
        i8 = M.ImVec2_Float(0, 0)
    end
    if i9 == nil then
        i9 = ffi.sizeof("float")
    end
    local out = C.imguiPlotHistogram_FloatPtr(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.PlotHistogram_FnFloatPtr = M.PlotHistogram_FnFloatPtr or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i5 == nil then
        i5 = 0
    end
    if i7 == nil then
        i7 = FLT_MAX
    end
    if i8 == nil then
        i8 = FLT_MAX
    end
    if i9 == nil then
        i9 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiPlotHistogram_FnFloatPtr(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.PlotLines_FloatPtr = M.PlotLines_FloatPtr or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    if i6 == nil then
        i6 = FLT_MAX
    end
    if i7 == nil then
        i7 = FLT_MAX
    end
    if i8 == nil then
        i8 = M.ImVec2_Float(0, 0)
    end
    if i9 == nil then
        i9 = ffi.sizeof("float")
    end
    local out = C.imguiPlotLines_FloatPtr(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.PlotLines_FnFloatPtr = M.PlotLines_FnFloatPtr or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    if i5 == nil then
        i5 = 0
    end
    if i7 == nil then
        i7 = FLT_MAX
    end
    if i8 == nil then
        i8 = FLT_MAX
    end
    if i9 == nil then
        i9 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiPlotLines_FnFloatPtr(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.PopClipRect = M.PopClipRect or function()
    jit.off(true)
    local out = C.imguiPopClipRect()
    return out
end
M.PopColumnsBackground = M.PopColumnsBackground or function()
    jit.off(true)
    local out = C.imguiPopColumnsBackground()
    return out
end
M.PopFocusScope = M.PopFocusScope or function()
    jit.off(true)
    local out = C.imguiPopFocusScope()
    return out
end
M.PopFont = M.PopFont or function()
    jit.off(true)
    local out = C.imguiPopFont()
    return out
end
M.PopID = M.PopID or function()
    jit.off(true)
    local out = C.imguiPopID()
    return out
end
M.PopItemFlag = M.PopItemFlag or function()
    jit.off(true)
    local out = C.imguiPopItemFlag()
    return out
end
M.PopItemWidth = M.PopItemWidth or function()
    jit.off(true)
    local out = C.imguiPopItemWidth()
    return out
end
M.PopStyleColor = M.PopStyleColor or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 1
    end
    local out = C.imguiPopStyleColor(i1)
    return out
end
M.PopStyleVar = M.PopStyleVar or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 1
    end
    local out = C.imguiPopStyleVar(i1)
    return out
end
M.PopTextWrapPos = M.PopTextWrapPos or function()
    jit.off(true)
    local out = C.imguiPopTextWrapPos()
    return out
end
M.ProgressBar = M.ProgressBar or function(i1, i2, i3)
    jit.off(true)
    if i2 == nil then
        i2 = M.ImVec2_Float(-FLT_MIN, 0)
    end
    local out = C.imguiProgressBar(i1, i2, i3)
    return out
end
M.PushClipRect = M.PushClipRect or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiPushClipRect(i1, i2, i3)
    return out
end
M.PushColumnClipRect = M.PushColumnClipRect or function(i1)
    jit.off(true)
    local out = C.imguiPushColumnClipRect(i1)
    return out
end
M.PushColumnsBackground = M.PushColumnsBackground or function()
    jit.off(true)
    local out = C.imguiPushColumnsBackground()
    return out
end
M.PushFocusScope = M.PushFocusScope or function(i1)
    jit.off(true)
    local out = C.imguiPushFocusScope(i1)
    return out
end
M.PushFont = M.PushFont or function(i1)
    jit.off(true)
    local out = C.imguiPushFont(i1)
    return out
end
M.PushID_Str = M.PushID_Str or function(i1)
    jit.off(true)
    local out = C.imguiPushID_Str(i1)
    return out
end
M.PushID_StrStr = M.PushID_StrStr or function(i1, i2)
    jit.off(true)
    local out = C.imguiPushID_StrStr(i1, i2)
    return out
end
M.PushID_Ptr = M.PushID_Ptr or function(i1)
    jit.off(true)
    local out = C.imguiPushID_Ptr(i1)
    return out
end
M.PushID_Int = M.PushID_Int or function(i1)
    jit.off(true)
    local out = C.imguiPushID_Int(i1)
    return out
end
M.PushItemFlag = M.PushItemFlag or function(i1, i2)
    jit.off(true)
    local out = C.imguiPushItemFlag(i1, i2)
    return out
end
M.PushItemWidth = M.PushItemWidth or function(i1)
    jit.off(true)
    local out = C.imguiPushItemWidth(i1)
    return out
end
M.PushMultiItemsWidths = M.PushMultiItemsWidths or function(i1, i2)
    jit.off(true)
    local out = C.imguiPushMultiItemsWidths(i1, i2)
    return out
end
M.PushOverrideID = M.PushOverrideID or function(i1)
    jit.off(true)
    local out = C.imguiPushOverrideID(i1)
    return out
end
M.PushStyleColor_U32 = M.PushStyleColor_U32 or function(i1, i2)
    jit.off(true)
    local out = C.imguiPushStyleColor_U32(i1, i2)
    return out
end
M.PushStyleColor_Vec4 = M.PushStyleColor_Vec4 or function(i1, i2)
    jit.off(true)
    local out = C.imguiPushStyleColor_Vec4(i1, i2)
    return out
end
M.PushStyleVar_Float = M.PushStyleVar_Float or function(i1, i2)
    jit.off(true)
    local out = C.imguiPushStyleVar_Float(i1, i2)
    return out
end
M.PushStyleVar_Vec2 = M.PushStyleVar_Vec2 or function(i1, i2)
    jit.off(true)
    local out = C.imguiPushStyleVar_Vec2(i1, i2)
    return out
end
M.PushTextWrapPos = M.PushTextWrapPos or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0.0
    end
    local out = C.imguiPushTextWrapPos(i1)
    return out
end
M.RadioButton_Bool = M.RadioButton_Bool or function(i1, i2)
    jit.off(true)
    local out = C.imguiRadioButton_Bool(i1, i2)
    return out
end
M.RadioButton_IntPtr = M.RadioButton_IntPtr or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiRadioButton_IntPtr(i1, i2, i3)
    return out
end
M.RemoveContextHook = M.RemoveContextHook or function(i1, i2)
    jit.off(true)
    local out = C.imguiRemoveContextHook(i1, i2)
    return out
end
M.RemoveSettingsHandler = M.RemoveSettingsHandler or function(i1)
    jit.off(true)
    local out = C.imguiRemoveSettingsHandler(i1)
    return out
end
M.Render = M.Render or function()
    jit.off(true)
    local out = C.imguiRender()
    return out
end
M.RenderArrow = M.RenderArrow or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i5 == nil then
        i5 = 1.0
    end
    local out = C.imguiRenderArrow(i1, i2, i3, i4, i5)
    return out
end
M.RenderArrowDockMenu = M.RenderArrowDockMenu or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiRenderArrowDockMenu(i1, i2, i3, i4)
    return out
end
M.RenderArrowPointingAt = M.RenderArrowPointingAt or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiRenderArrowPointingAt(i1, i2, i3, i4, i5)
    return out
end
M.RenderBullet = M.RenderBullet or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiRenderBullet(i1, i2, i3)
    return out
end
M.RenderCheckMark = M.RenderCheckMark or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiRenderCheckMark(i1, i2, i3, i4)
    return out
end
M.RenderColorRectWithAlphaCheckerboard = M.RenderColorRectWithAlphaCheckerboard or
                                             function(i1, i2, i3, i4, i5, i6, i7, i8)
        jit.off(true)
        if i7 == nil then
            i7 = 0.0
        end
        if i8 == nil then
            i8 = 0
        end
        local out = C.imguiRenderColorRectWithAlphaCheckerboard(i1, i2, i3, i4, i5, i6, i7, i8)
        return out
    end
M.RenderDragDropTargetRect = M.RenderDragDropTargetRect or function(i1, i2)
    jit.off(true)
    local out = C.imguiRenderDragDropTargetRect(i1, i2)
    return out
end
M.RenderFrame = M.RenderFrame or function(i1, i2, i3, i4, i5)
    jit.off(true)
    if i4 == nil then
        i4 = true
    end
    if i5 == nil then
        i5 = 0.0
    end
    local out = C.imguiRenderFrame(i1, i2, i3, i4, i5)
    return out
end
M.RenderFrameBorder = M.RenderFrameBorder or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0.0
    end
    local out = C.imguiRenderFrameBorder(i1, i2, i3)
    return out
end
M.RenderMouseCursor = M.RenderMouseCursor or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.imguiRenderMouseCursor(i1, i2, i3, i4, i5, i6)
    return out
end
M.RenderNavHighlight = M.RenderNavHighlight or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiRenderNavHighlight(i1, i2, i3)
    return out
end
M.RenderPlatformWindowsDefault = M.RenderPlatformWindowsDefault or function(i1, i2)
    jit.off(true)
    local out = C.imguiRenderPlatformWindowsDefault(i1, i2)
    return out
end
M.RenderRectFilledRangeH = M.RenderRectFilledRangeH or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.imguiRenderRectFilledRangeH(i1, i2, i3, i4, i5, i6)
    return out
end
M.RenderRectFilledWithHole = M.RenderRectFilledWithHole or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiRenderRectFilledWithHole(i1, i2, i3, i4, i5)
    return out
end
M.RenderText = M.RenderText or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = true
    end
    local out = C.imguiRenderText(i1, i2, i3, i4)
    return out
end
M.RenderTextClipped = M.RenderTextClipped or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i6 == nil then
        i6 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiRenderTextClipped(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.RenderTextClippedEx = M.RenderTextClippedEx or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    if i7 == nil then
        i7 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiRenderTextClippedEx(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.RenderTextEllipsis = M.RenderTextEllipsis or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    local out = C.imguiRenderTextEllipsis(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.RenderTextWrapped = M.RenderTextWrapped or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiRenderTextWrapped(i1, i2, i3, i4)
    return out
end
M.ResetMouseDragDelta = M.ResetMouseDragDelta or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiResetMouseDragDelta(i1)
    return out
end
M.SameLine = M.SameLine or function(i1, i2)
    jit.off(true)
    if i1 == nil then
        i1 = 0.0
    end
    if i2 == nil then
        i2 = -1.0
    end
    local out = C.imguiSameLine(i1, i2)
    return out
end
M.SaveIniSettingsToDisk = M.SaveIniSettingsToDisk or function(i1)
    jit.off(true)
    local out = C.imguiSaveIniSettingsToDisk(i1)
    return out
end
M.SaveIniSettingsToMemory = M.SaveIniSettingsToMemory or function()
    jit.off(true)
    local o1 = ffi.new("size_t[1]")
    local out = C.imguiSaveIniSettingsToMemory(o1)
    return o1[0], out
end
M.ScaleWindowsInViewport = M.ScaleWindowsInViewport or function(i1, i2)
    jit.off(true)
    local out = C.imguiScaleWindowsInViewport(i1, i2)
    return out
end
M.ScrollToBringRectIntoView = M.ScrollToBringRectIntoView or function(i1, i2)
    jit.off(true)
    local out = C.imguiScrollToBringRectIntoView(i1, i2)
    return out
end
M.ScrollToItem = M.ScrollToItem or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiScrollToItem(i1)
    return out
end
M.ScrollToRect = M.ScrollToRect or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiScrollToRect(i1, i2, i3)
    return out
end
M.ScrollToRectEx = M.ScrollToRectEx or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local o1 = M.ImVec2_Nil()
    local out = C.imguiScrollToRectEx(o1, i1, i2, i3)
    return o1, out
end
M.Scrollbar = M.Scrollbar or function(i1)
    jit.off(true)
    local out = C.imguiScrollbar(i1)
    return out
end
M.ScrollbarEx = M.ScrollbarEx or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    local out = C.imguiScrollbarEx(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.Selectable_Bool = M.Selectable_Bool or function(i1, i2, i3, i4)
    jit.off(true)
    if i2 == nil then
        i2 = false
    end
    if i3 == nil then
        i3 = 0
    end
    if i4 == nil then
        i4 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiSelectable_Bool(i1, i2, i3, i4)
    return out
end
M.Selectable_BoolPtr = M.Selectable_BoolPtr or function(i1, i2, i3, i4)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    if i4 == nil then
        i4 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiSelectable_BoolPtr(i1, i2, i3, i4)
    return out
end
M.Separator = M.Separator or function()
    jit.off(true)
    local out = C.imguiSeparator()
    return out
end
M.SeparatorEx = M.SeparatorEx or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 1.0
    end
    local out = C.imguiSeparatorEx(i1, i2)
    return out
end
M.SeparatorText = M.SeparatorText or function(i1)
    jit.off(true)
    local out = C.imguiSeparatorText(i1)
    return out
end
M.SeparatorTextEx = M.SeparatorTextEx or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiSeparatorTextEx(i1, i2, i3, i4)
    return out
end
M.SetActiveID = M.SetActiveID or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetActiveID(i1, i2)
    return out
end
M.SetActiveIdUsingAllKeyboardKeys = M.SetActiveIdUsingAllKeyboardKeys or function()
    jit.off(true)
    local out = C.imguiSetActiveIdUsingAllKeyboardKeys()
    return out
end
M.SetAllocatorFunctions = M.SetAllocatorFunctions or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiSetAllocatorFunctions(i1, i2, i3)
    return out
end
M.SetClipboardText = M.SetClipboardText or function(i1)
    jit.off(true)
    local out = C.imguiSetClipboardText(i1)
    return out
end
M.SetColorEditOptions = M.SetColorEditOptions or function(i1)
    jit.off(true)
    local out = C.imguiSetColorEditOptions(i1)
    return out
end
M.SetColumnOffset = M.SetColumnOffset or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetColumnOffset(i1, i2)
    return out
end
M.SetColumnWidth = M.SetColumnWidth or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetColumnWidth(i1, i2)
    return out
end
M.SetCurrentContext = M.SetCurrentContext or function(i1)
    jit.off(true)
    local out = C.imguiSetCurrentContext(i1)
    return out
end
M.SetCurrentFont = M.SetCurrentFont or function(i1)
    jit.off(true)
    local out = C.imguiSetCurrentFont(i1)
    return out
end
M.SetCurrentViewport = M.SetCurrentViewport or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetCurrentViewport(i1, i2)
    return out
end
M.SetCursorPos = M.SetCursorPos or function(i1)
    jit.off(true)
    local out = C.imguiSetCursorPos(i1)
    return out
end
M.SetCursorPosX = M.SetCursorPosX or function(i1)
    jit.off(true)
    local out = C.imguiSetCursorPosX(i1)
    return out
end
M.SetCursorPosY = M.SetCursorPosY or function(i1)
    jit.off(true)
    local out = C.imguiSetCursorPosY(i1)
    return out
end
M.SetCursorScreenPos = M.SetCursorScreenPos or function(i1)
    jit.off(true)
    local out = C.imguiSetCursorScreenPos(i1)
    return out
end
M.SetDragDropPayload = M.SetDragDropPayload or function(i1, i2, i3, i4)
    jit.off(true)
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiSetDragDropPayload(i1, i2, i3, i4)
    return out
end
M.SetFocusID = M.SetFocusID or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetFocusID(i1, i2)
    return out
end
M.SetHoveredID = M.SetHoveredID or function(i1)
    jit.off(true)
    local out = C.imguiSetHoveredID(i1)
    return out
end
M.SetItemDefaultFocus = M.SetItemDefaultFocus or function()
    jit.off(true)
    local out = C.imguiSetItemDefaultFocus()
    return out
end
M.SetItemKeyOwner_Nil = M.SetItemKeyOwner_Nil or function(i1)
    jit.off(true)
    local out = C.imguiSetItemKeyOwner_Nil(i1)
    return out
end
M.SetItemKeyOwner_InputFlags = M.SetItemKeyOwner_InputFlags or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetItemKeyOwner_InputFlags(i1, i2)
    return out
end
M.SetItemTooltip = M.SetItemTooltip or function(i1, ...)
    jit.off(true)
    local out = C.imguiSetItemTooltip(i1, ...)
    return out
end
M.SetKeyOwner = M.SetKeyOwner or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetKeyOwner(i1, i2, i3)
    return out
end
M.SetKeyOwnersForKeyChord = M.SetKeyOwnersForKeyChord or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetKeyOwnersForKeyChord(i1, i2, i3)
    return out
end
M.SetKeyboardFocusHere = M.SetKeyboardFocusHere or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    local out = C.imguiSetKeyboardFocusHere(i1)
    return out
end
M.SetLastItemData = M.SetLastItemData or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiSetLastItemData(i1, i2, i3, i4)
    return out
end
M.SetMouseCursor = M.SetMouseCursor or function(i1)
    jit.off(true)
    local out = C.imguiSetMouseCursor(i1)
    return out
end
M.SetNavFocusScope = M.SetNavFocusScope or function(i1)
    jit.off(true)
    local out = C.imguiSetNavFocusScope(i1)
    return out
end
M.SetNavID = M.SetNavID or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiSetNavID(i1, i2, i3, i4)
    return out
end
M.SetNavWindow = M.SetNavWindow or function(i1)
    jit.off(true)
    local out = C.imguiSetNavWindow(i1)
    return out
end
M.SetNextFrameWantCaptureKeyboard = M.SetNextFrameWantCaptureKeyboard or function(i1)
    jit.off(true)
    local out = C.imguiSetNextFrameWantCaptureKeyboard(i1)
    return out
end
M.SetNextFrameWantCaptureMouse = M.SetNextFrameWantCaptureMouse or function(i1)
    jit.off(true)
    local out = C.imguiSetNextFrameWantCaptureMouse(i1)
    return out
end
M.SetNextItemAllowOverlap = M.SetNextItemAllowOverlap or function()
    jit.off(true)
    local out = C.imguiSetNextItemAllowOverlap()
    return out
end
M.SetNextItemOpen = M.SetNextItemOpen or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetNextItemOpen(i1, i2)
    return out
end
M.SetNextItemRefVal = M.SetNextItemRefVal or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetNextItemRefVal(i1, i2)
    return out
end
M.SetNextItemSelectionUserData = M.SetNextItemSelectionUserData or function(i1)
    jit.off(true)
    local out = C.imguiSetNextItemSelectionUserData(i1)
    return out
end
M.SetNextItemShortcut = M.SetNextItemShortcut or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetNextItemShortcut(i1, i2)
    return out
end
M.SetNextItemStorageID = M.SetNextItemStorageID or function(i1)
    jit.off(true)
    local out = C.imguiSetNextItemStorageID(i1)
    return out
end
M.SetNextItemWidth = M.SetNextItemWidth or function(i1)
    jit.off(true)
    local out = C.imguiSetNextItemWidth(i1)
    return out
end
M.SetNextWindowBgAlpha = M.SetNextWindowBgAlpha or function(i1)
    jit.off(true)
    local out = C.imguiSetNextWindowBgAlpha(i1)
    return out
end
M.SetNextWindowClass = M.SetNextWindowClass or function(i1)
    jit.off(true)
    local out = C.imguiSetNextWindowClass(i1)
    return out
end
M.SetNextWindowCollapsed = M.SetNextWindowCollapsed or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetNextWindowCollapsed(i1, i2)
    return out
end
M.SetNextWindowContentSize = M.SetNextWindowContentSize or function(i1)
    jit.off(true)
    local out = C.imguiSetNextWindowContentSize(i1)
    return out
end
M.SetNextWindowDockID = M.SetNextWindowDockID or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetNextWindowDockID(i1, i2)
    return out
end
M.SetNextWindowFocus = M.SetNextWindowFocus or function()
    jit.off(true)
    local out = C.imguiSetNextWindowFocus()
    return out
end
M.SetNextWindowPos = M.SetNextWindowPos or function(i1, i2, i3)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    if i3 == nil then
        i3 = M.ImVec2_Float(0, 0)
    end
    local out = C.imguiSetNextWindowPos(i1, i2, i3)
    return out
end
M.SetNextWindowRefreshPolicy = M.SetNextWindowRefreshPolicy or function(i1)
    jit.off(true)
    local out = C.imguiSetNextWindowRefreshPolicy(i1)
    return out
end
M.SetNextWindowScroll = M.SetNextWindowScroll or function(i1)
    jit.off(true)
    local out = C.imguiSetNextWindowScroll(i1)
    return out
end
M.SetNextWindowSize = M.SetNextWindowSize or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetNextWindowSize(i1, i2)
    return out
end
M.SetNextWindowSizeConstraints = M.SetNextWindowSizeConstraints or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiSetNextWindowSizeConstraints(i1, i2, i3, i4)
    return out
end
M.SetNextWindowViewport = M.SetNextWindowViewport or function(i1)
    jit.off(true)
    local out = C.imguiSetNextWindowViewport(i1)
    return out
end
M.SetScrollFromPosX_Float = M.SetScrollFromPosX_Float or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0.5
    end
    local out = C.imguiSetScrollFromPosX_Float(i1, i2)
    return out
end
M.SetScrollFromPosX_WindowPtr = M.SetScrollFromPosX_WindowPtr or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiSetScrollFromPosX_WindowPtr(i1, i2, i3)
    return out
end
M.SetScrollFromPosY_Float = M.SetScrollFromPosY_Float or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0.5
    end
    local out = C.imguiSetScrollFromPosY_Float(i1, i2)
    return out
end
M.SetScrollFromPosY_WindowPtr = M.SetScrollFromPosY_WindowPtr or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiSetScrollFromPosY_WindowPtr(i1, i2, i3)
    return out
end
M.SetScrollHereX = M.SetScrollHereX or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0.5
    end
    local out = C.imguiSetScrollHereX(i1)
    return out
end
M.SetScrollHereY = M.SetScrollHereY or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0.5
    end
    local out = C.imguiSetScrollHereY(i1)
    return out
end
M.SetScrollX_Float = M.SetScrollX_Float or function(i1)
    jit.off(true)
    local out = C.imguiSetScrollX_Float(i1)
    return out
end
M.SetScrollX_WindowPtr = M.SetScrollX_WindowPtr or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetScrollX_WindowPtr(i1, i2)
    return out
end
M.SetScrollY_Float = M.SetScrollY_Float or function(i1)
    jit.off(true)
    local out = C.imguiSetScrollY_Float(i1)
    return out
end
M.SetScrollY_WindowPtr = M.SetScrollY_WindowPtr or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetScrollY_WindowPtr(i1, i2)
    return out
end
M.SetShortcutRouting = M.SetShortcutRouting or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiSetShortcutRouting(i1, i2, i3)
    return out
end
M.SetStateStorage = M.SetStateStorage or function(i1)
    jit.off(true)
    local out = C.imguiSetStateStorage(i1)
    return out
end
M.SetTabItemClosed = M.SetTabItemClosed or function(i1)
    jit.off(true)
    local out = C.imguiSetTabItemClosed(i1)
    return out
end
M.SetTooltip = M.SetTooltip or function(i1, ...)
    jit.off(true)
    local out = C.imguiSetTooltip(i1, ...)
    return out
end
M.SetWindowClipRectBeforeSetChannel = M.SetWindowClipRectBeforeSetChannel or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetWindowClipRectBeforeSetChannel(i1, i2)
    return out
end
M.SetWindowCollapsed_Bool = M.SetWindowCollapsed_Bool or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetWindowCollapsed_Bool(i1, i2)
    return out
end
M.SetWindowCollapsed_Str = M.SetWindowCollapsed_Str or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetWindowCollapsed_Str(i1, i2, i3)
    return out
end
M.SetWindowCollapsed_WindowPtr = M.SetWindowCollapsed_WindowPtr or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetWindowCollapsed_WindowPtr(i1, i2, i3)
    return out
end
M.SetWindowDock = M.SetWindowDock or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiSetWindowDock(i1, i2, i3)
    return out
end
M.SetWindowFocus_Nil = M.SetWindowFocus_Nil or function()
    jit.off(true)
    local out = C.imguiSetWindowFocus_Nil()
    return out
end
M.SetWindowFocus_Str = M.SetWindowFocus_Str or function(i1)
    jit.off(true)
    local out = C.imguiSetWindowFocus_Str(i1)
    return out
end
M.SetWindowFontScale = M.SetWindowFontScale or function(i1)
    jit.off(true)
    local out = C.imguiSetWindowFontScale(i1)
    return out
end
M.SetWindowHiddenAndSkipItemsForCurrentFrame = M.SetWindowHiddenAndSkipItemsForCurrentFrame or function(i1)
    jit.off(true)
    local out = C.imguiSetWindowHiddenAndSkipItemsForCurrentFrame(i1)
    return out
end
M.SetWindowHitTestHole = M.SetWindowHitTestHole or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiSetWindowHitTestHole(i1, i2, i3)
    return out
end
M.SetWindowParentWindowForFocusRoute = M.SetWindowParentWindowForFocusRoute or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetWindowParentWindowForFocusRoute(i1, i2)
    return out
end
M.SetWindowPos_Vec2 = M.SetWindowPos_Vec2 or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetWindowPos_Vec2(i1, i2)
    return out
end
M.SetWindowPos_Str = M.SetWindowPos_Str or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetWindowPos_Str(i1, i2, i3)
    return out
end
M.SetWindowPos_WindowPtr = M.SetWindowPos_WindowPtr or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetWindowPos_WindowPtr(i1, i2, i3)
    return out
end
M.SetWindowSize_Vec2 = M.SetWindowSize_Vec2 or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiSetWindowSize_Vec2(i1, i2)
    return out
end
M.SetWindowSize_Str = M.SetWindowSize_Str or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetWindowSize_Str(i1, i2, i3)
    return out
end
M.SetWindowSize_WindowPtr = M.SetWindowSize_WindowPtr or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiSetWindowSize_WindowPtr(i1, i2, i3)
    return out
end
M.SetWindowViewport = M.SetWindowViewport or function(i1, i2)
    jit.off(true)
    local out = C.imguiSetWindowViewport(i1, i2)
    return out
end
M.ShadeVertsLinearColorGradientKeepAlpha = M.ShadeVertsLinearColorGradientKeepAlpha or
                                               function(i1, i2, i3, i4, i5, i6, i7)
        jit.off(true)
        local out = C.imguiShadeVertsLinearColorGradientKeepAlpha(i1, i2, i3, i4, i5, i6, i7)
        return out
    end
M.ShadeVertsLinearUV = M.ShadeVertsLinearUV or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    local out = C.imguiShadeVertsLinearUV(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.ShadeVertsTransformPos = M.ShadeVertsTransformPos or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    local out = C.imguiShadeVertsTransformPos(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.Shortcut_Nil = M.Shortcut_Nil or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiShortcut_Nil(i1, i2)
    return out
end
M.Shortcut_ID = M.Shortcut_ID or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiShortcut_ID(i1, i2, i3)
    return out
end
M.ShowAboutWindow = M.ShowAboutWindow or function(i1)
    jit.off(true)
    local out = C.imguiShowAboutWindow(i1)
    return out
end
M.ShowDebugLogWindow = M.ShowDebugLogWindow or function(i1)
    jit.off(true)
    local out = C.imguiShowDebugLogWindow(i1)
    return out
end
M.ShowDemoWindow = M.ShowDemoWindow or function(i1)
    jit.off(true)
    local out = C.imguiShowDemoWindow(i1)
    return out
end
M.ShowFontAtlas = M.ShowFontAtlas or function(i1)
    jit.off(true)
    local out = C.imguiShowFontAtlas(i1)
    return out
end
M.ShowFontSelector = M.ShowFontSelector or function(i1)
    jit.off(true)
    local out = C.imguiShowFontSelector(i1)
    return out
end
M.ShowIDStackToolWindow = M.ShowIDStackToolWindow or function(i1)
    jit.off(true)
    local out = C.imguiShowIDStackToolWindow(i1)
    return out
end
M.ShowMetricsWindow = M.ShowMetricsWindow or function(i1)
    jit.off(true)
    local out = C.imguiShowMetricsWindow(i1)
    return out
end
M.ShowStyleEditor = M.ShowStyleEditor or function(i1)
    jit.off(true)
    local out = C.imguiShowStyleEditor(i1)
    return out
end
M.ShowStyleSelector = M.ShowStyleSelector or function(i1)
    jit.off(true)
    local out = C.imguiShowStyleSelector(i1)
    return out
end
M.ShowUserGuide = M.ShowUserGuide or function()
    jit.off(true)
    local out = C.imguiShowUserGuide()
    return out
end
M.ShrinkWidths = M.ShrinkWidths or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiShrinkWidths(i1, i2, i3)
    return out
end
M.Shutdown = M.Shutdown or function()
    jit.off(true)
    local out = C.imguiShutdown()
    return out
end
M.SliderAngle = M.SliderAngle or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i3 == nil then
        i3 = -360.0
    end
    if i4 == nil then
        i4 = 360.0
    end
    if i5 == nil then
        i5 = "%.0f deg"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderAngle(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderBehavior = M.SliderBehavior or function(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    jit.off(true)
    local out = C.imguiSliderBehavior(i1, i2, i3, i4, i5, i6, i7, i8, i9)
    return out
end
M.SliderFloat = M.SliderFloat or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%.3f"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderFloat(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderFloat2 = M.SliderFloat2 or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%.3f"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderFloat2(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderFloat3 = M.SliderFloat3 or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%.3f"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderFloat3(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderFloat4 = M.SliderFloat4 or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%.3f"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderFloat4(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderInt = M.SliderInt or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%d"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderInt(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderInt2 = M.SliderInt2 or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%d"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderInt2(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderInt3 = M.SliderInt3 or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%d"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderInt3(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderInt4 = M.SliderInt4 or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    if i5 == nil then
        i5 = "%d"
    end
    if i6 == nil then
        i6 = 0
    end
    local out = C.imguiSliderInt4(i1, i2, i3, i4, i5, i6)
    return out
end
M.SliderScalar = M.SliderScalar or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiSliderScalar(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.SliderScalarN = M.SliderScalarN or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    if i8 == nil then
        i8 = 0
    end
    local out = C.imguiSliderScalarN(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.SmallButton = M.SmallButton or function(i1)
    jit.off(true)
    local out = C.imguiSmallButton(i1)
    return out
end
M.Spacing = M.Spacing or function()
    jit.off(true)
    local out = C.imguiSpacing()
    return out
end
M.SplitterBehavior = M.SplitterBehavior or function(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    jit.off(true)
    if i8 == nil then
        i8 = 0.0
    end
    if i9 == nil then
        i9 = 0.0
    end
    if i10 == nil then
        i10 = 0
    end
    local out = C.imguiSplitterBehavior(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    return out
end
M.StartMouseMovingWindow = M.StartMouseMovingWindow or function(i1)
    jit.off(true)
    local out = C.imguiStartMouseMovingWindow(i1)
    return out
end
M.StartMouseMovingWindowOrNode = M.StartMouseMovingWindowOrNode or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiStartMouseMovingWindowOrNode(i1, i2, i3)
    return out
end
M.StyleColorsClassic = M.StyleColorsClassic or function(i1)
    jit.off(true)
    local out = C.imguiStyleColorsClassic(i1)
    return out
end
M.StyleColorsDark = M.StyleColorsDark or function(i1)
    jit.off(true)
    local out = C.imguiStyleColorsDark(i1)
    return out
end
M.StyleColorsLight = M.StyleColorsLight or function(i1)
    jit.off(true)
    local out = C.imguiStyleColorsLight(i1)
    return out
end
M.TabBarAddTab = M.TabBarAddTab or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiTabBarAddTab(i1, i2, i3)
    return out
end
M.TabBarCloseTab = M.TabBarCloseTab or function(i1, i2)
    jit.off(true)
    local out = C.imguiTabBarCloseTab(i1, i2)
    return out
end
M.TabBarFindMostRecentlySelectedTabForActiveWindow = M.TabBarFindMostRecentlySelectedTabForActiveWindow or function(i1)
    jit.off(true)
    local out = C.imguiTabBarFindMostRecentlySelectedTabForActiveWindow(i1)
    return out
end
M.TabBarFindTabByID = M.TabBarFindTabByID or function(i1, i2)
    jit.off(true)
    local out = C.imguiTabBarFindTabByID(i1, i2)
    return out
end
M.TabBarFindTabByOrder = M.TabBarFindTabByOrder or function(i1, i2)
    jit.off(true)
    local out = C.imguiTabBarFindTabByOrder(i1, i2)
    return out
end
M.TabBarGetCurrentTab = M.TabBarGetCurrentTab or function(i1)
    jit.off(true)
    local out = C.imguiTabBarGetCurrentTab(i1)
    return out
end
M.TabBarGetTabName = M.TabBarGetTabName or function(i1, i2)
    jit.off(true)
    local out = C.imguiTabBarGetTabName(i1, i2)
    return out
end
M.TabBarGetTabOrder = M.TabBarGetTabOrder or function(i1, i2)
    jit.off(true)
    local out = C.imguiTabBarGetTabOrder(i1, i2)
    return out
end
M.TabBarProcessReorder = M.TabBarProcessReorder or function(i1)
    jit.off(true)
    local out = C.imguiTabBarProcessReorder(i1)
    return out
end
M.TabBarQueueFocus = M.TabBarQueueFocus or function(i1, i2)
    jit.off(true)
    local out = C.imguiTabBarQueueFocus(i1, i2)
    return out
end
M.TabBarQueueReorder = M.TabBarQueueReorder or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiTabBarQueueReorder(i1, i2, i3)
    return out
end
M.TabBarQueueReorderFromMousePos = M.TabBarQueueReorderFromMousePos or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiTabBarQueueReorderFromMousePos(i1, i2, i3)
    return out
end
M.TabBarRemoveTab = M.TabBarRemoveTab or function(i1, i2)
    jit.off(true)
    local out = C.imguiTabBarRemoveTab(i1, i2)
    return out
end
M.TabItemBackground = M.TabItemBackground or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiTabItemBackground(i1, i2, i3, i4)
    return out
end
M.TabItemButton = M.TabItemButton or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiTabItemButton(i1, i2)
    return out
end
M.TabItemCalcSize_Str = M.TabItemCalcSize_Str or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiTabItemCalcSize_Str(o1, i1, i2)
    return o1, out
end
M.TabItemCalcSize_WindowPtr = M.TabItemCalcSize_WindowPtr or function(i1)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiTabItemCalcSize_WindowPtr(o1, i1)
    return o1, out
end
M.TabItemEx = M.TabItemEx or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiTabItemEx(i1, i2, i3, i4, i5)
    return out
end
M.TabItemLabelAndCloseButton = M.TabItemLabelAndCloseButton or function(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    jit.off(true)
    local out = C.imguiTabItemLabelAndCloseButton(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
    return out
end
M.TableAngledHeadersRow = M.TableAngledHeadersRow or function()
    jit.off(true)
    local out = C.imguiTableAngledHeadersRow()
    return out
end
M.TableAngledHeadersRowEx = M.TableAngledHeadersRowEx or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiTableAngledHeadersRowEx(i1, i2, i3, i4, i5)
    return out
end
M.TableBeginApplyRequests = M.TableBeginApplyRequests or function(i1)
    jit.off(true)
    local out = C.imguiTableBeginApplyRequests(i1)
    return out
end
M.TableBeginCell = M.TableBeginCell or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableBeginCell(i1, i2)
    return out
end
M.TableBeginContextMenuPopup = M.TableBeginContextMenuPopup or function(i1)
    jit.off(true)
    local out = C.imguiTableBeginContextMenuPopup(i1)
    return out
end
M.TableBeginInitMemory = M.TableBeginInitMemory or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableBeginInitMemory(i1, i2)
    return out
end
M.TableBeginRow = M.TableBeginRow or function(i1)
    jit.off(true)
    local out = C.imguiTableBeginRow(i1)
    return out
end
M.TableDrawBorders = M.TableDrawBorders or function(i1)
    jit.off(true)
    local out = C.imguiTableDrawBorders(i1)
    return out
end
M.TableDrawDefaultContextMenu = M.TableDrawDefaultContextMenu or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableDrawDefaultContextMenu(i1, i2)
    return out
end
M.TableEndCell = M.TableEndCell or function(i1)
    jit.off(true)
    local out = C.imguiTableEndCell(i1)
    return out
end
M.TableEndRow = M.TableEndRow or function(i1)
    jit.off(true)
    local out = C.imguiTableEndRow(i1)
    return out
end
M.TableFindByID = M.TableFindByID or function(i1)
    jit.off(true)
    local out = C.imguiTableFindByID(i1)
    return out
end
M.TableFixColumnSortDirection = M.TableFixColumnSortDirection or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableFixColumnSortDirection(i1, i2)
    return out
end
M.TableGcCompactSettings = M.TableGcCompactSettings or function()
    jit.off(true)
    local out = C.imguiTableGcCompactSettings()
    return out
end
M.TableGcCompactTransientBuffers_TablePtr = M.TableGcCompactTransientBuffers_TablePtr or function(i1)
    jit.off(true)
    local out = C.imguiTableGcCompactTransientBuffers_TablePtr(i1)
    return out
end
M.TableGcCompactTransientBuffers_TableTempDataPtr = M.TableGcCompactTransientBuffers_TableTempDataPtr or function(i1)
    jit.off(true)
    local out = C.imguiTableGcCompactTransientBuffers_TableTempDataPtr(i1)
    return out
end
M.TableGetBoundSettings = M.TableGetBoundSettings or function(i1)
    jit.off(true)
    local out = C.imguiTableGetBoundSettings(i1)
    return out
end
M.TableGetCellBgRect = M.TableGetCellBgRect or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiTableGetCellBgRect(i1, i2, i3)
    return out
end
M.TableGetColumnCount = M.TableGetColumnCount or function()
    jit.off(true)
    local out = C.imguiTableGetColumnCount()
    return out
end
M.TableGetColumnFlags = M.TableGetColumnFlags or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiTableGetColumnFlags(i1)
    return out
end
M.TableGetColumnIndex = M.TableGetColumnIndex or function()
    jit.off(true)
    local out = C.imguiTableGetColumnIndex()
    return out
end
M.TableGetColumnName_Int = M.TableGetColumnName_Int or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiTableGetColumnName_Int(i1)
    return out
end
M.TableGetColumnName_TablePtr = M.TableGetColumnName_TablePtr or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableGetColumnName_TablePtr(i1, i2)
    return out
end
M.TableGetColumnNextSortDirection = M.TableGetColumnNextSortDirection or function(i1)
    jit.off(true)
    local out = C.imguiTableGetColumnNextSortDirection(i1)
    return out
end
M.TableGetColumnResizeID = M.TableGetColumnResizeID or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiTableGetColumnResizeID(i1, i2, i3)
    return out
end
M.TableGetColumnWidthAuto = M.TableGetColumnWidthAuto or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableGetColumnWidthAuto(i1, i2)
    return out
end
M.TableGetHeaderAngledMaxLabelWidth = M.TableGetHeaderAngledMaxLabelWidth or function()
    jit.off(true)
    local out = C.imguiTableGetHeaderAngledMaxLabelWidth()
    return out
end
M.TableGetHeaderRowHeight = M.TableGetHeaderRowHeight or function()
    jit.off(true)
    local out = C.imguiTableGetHeaderRowHeight()
    return out
end
M.TableGetHoveredColumn = M.TableGetHoveredColumn or function()
    jit.off(true)
    local out = C.imguiTableGetHoveredColumn()
    return out
end
M.TableGetHoveredRow = M.TableGetHoveredRow or function()
    jit.off(true)
    local out = C.imguiTableGetHoveredRow()
    return out
end
M.TableGetInstanceData = M.TableGetInstanceData or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableGetInstanceData(i1, i2)
    return out
end
M.TableGetInstanceID = M.TableGetInstanceID or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableGetInstanceID(i1, i2)
    return out
end
M.TableGetMaxColumnWidth = M.TableGetMaxColumnWidth or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableGetMaxColumnWidth(i1, i2)
    return out
end
M.TableGetRowIndex = M.TableGetRowIndex or function()
    jit.off(true)
    local out = C.imguiTableGetRowIndex()
    return out
end
M.TableGetSortSpecs = M.TableGetSortSpecs or function()
    jit.off(true)
    local out = C.imguiTableGetSortSpecs()
    return out
end
M.TableHeader = M.TableHeader or function(i1)
    jit.off(true)
    local out = C.imguiTableHeader(i1)
    return out
end
M.TableHeadersRow = M.TableHeadersRow or function()
    jit.off(true)
    local out = C.imguiTableHeadersRow()
    return out
end
M.TableLoadSettings = M.TableLoadSettings or function(i1)
    jit.off(true)
    local out = C.imguiTableLoadSettings(i1)
    return out
end
M.TableMergeDrawChannels = M.TableMergeDrawChannels or function(i1)
    jit.off(true)
    local out = C.imguiTableMergeDrawChannels(i1)
    return out
end
M.TableNextColumn = M.TableNextColumn or function()
    jit.off(true)
    local out = C.imguiTableNextColumn()
    return out
end
M.TableNextRow = M.TableNextRow or function(i1, i2)
    jit.off(true)
    if i1 == nil then
        i1 = 0
    end
    if i2 == nil then
        i2 = 0.0
    end
    local out = C.imguiTableNextRow(i1, i2)
    return out
end
M.TableOpenContextMenu = M.TableOpenContextMenu or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = -1
    end
    local out = C.imguiTableOpenContextMenu(i1)
    return out
end
M.TablePopBackgroundChannel = M.TablePopBackgroundChannel or function()
    jit.off(true)
    local out = C.imguiTablePopBackgroundChannel()
    return out
end
M.TablePushBackgroundChannel = M.TablePushBackgroundChannel or function()
    jit.off(true)
    local out = C.imguiTablePushBackgroundChannel()
    return out
end
M.TableRemove = M.TableRemove or function(i1)
    jit.off(true)
    local out = C.imguiTableRemove(i1)
    return out
end
M.TableResetSettings = M.TableResetSettings or function(i1)
    jit.off(true)
    local out = C.imguiTableResetSettings(i1)
    return out
end
M.TableSaveSettings = M.TableSaveSettings or function(i1)
    jit.off(true)
    local out = C.imguiTableSaveSettings(i1)
    return out
end
M.TableSetBgColor = M.TableSetBgColor or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = -1
    end
    local out = C.imguiTableSetBgColor(i1, i2, i3)
    return out
end
M.TableSetColumnEnabled = M.TableSetColumnEnabled or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableSetColumnEnabled(i1, i2)
    return out
end
M.TableSetColumnIndex = M.TableSetColumnIndex or function(i1)
    jit.off(true)
    local out = C.imguiTableSetColumnIndex(i1)
    return out
end
M.TableSetColumnSortDirection = M.TableSetColumnSortDirection or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiTableSetColumnSortDirection(i1, i2, i3)
    return out
end
M.TableSetColumnWidth = M.TableSetColumnWidth or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableSetColumnWidth(i1, i2)
    return out
end
M.TableSetColumnWidthAutoAll = M.TableSetColumnWidthAutoAll or function(i1)
    jit.off(true)
    local out = C.imguiTableSetColumnWidthAutoAll(i1)
    return out
end
M.TableSetColumnWidthAutoSingle = M.TableSetColumnWidthAutoSingle or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableSetColumnWidthAutoSingle(i1, i2)
    return out
end
M.TableSettingsAddSettingsHandler = M.TableSettingsAddSettingsHandler or function()
    jit.off(true)
    local out = C.imguiTableSettingsAddSettingsHandler()
    return out
end
M.TableSettingsCreate = M.TableSettingsCreate or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableSettingsCreate(i1, i2)
    return out
end
M.TableSettingsFindByID = M.TableSettingsFindByID or function(i1)
    jit.off(true)
    local out = C.imguiTableSettingsFindByID(i1)
    return out
end
M.TableSetupColumn = M.TableSetupColumn or function(i1, i2, i3, i4)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    if i3 == nil then
        i3 = 0.0
    end
    if i4 == nil then
        i4 = 0
    end
    local out = C.imguiTableSetupColumn(i1, i2, i3, i4)
    return out
end
M.TableSetupDrawChannels = M.TableSetupDrawChannels or function(i1)
    jit.off(true)
    local out = C.imguiTableSetupDrawChannels(i1)
    return out
end
M.TableSetupScrollFreeze = M.TableSetupScrollFreeze or function(i1, i2)
    jit.off(true)
    local out = C.imguiTableSetupScrollFreeze(i1, i2)
    return out
end
M.TableSortSpecsBuild = M.TableSortSpecsBuild or function(i1)
    jit.off(true)
    local out = C.imguiTableSortSpecsBuild(i1)
    return out
end
M.TableSortSpecsSanitize = M.TableSortSpecsSanitize or function(i1)
    jit.off(true)
    local out = C.imguiTableSortSpecsSanitize(i1)
    return out
end
M.TableUpdateBorders = M.TableUpdateBorders or function(i1)
    jit.off(true)
    local out = C.imguiTableUpdateBorders(i1)
    return out
end
M.TableUpdateColumnsWeightFromWidth = M.TableUpdateColumnsWeightFromWidth or function(i1)
    jit.off(true)
    local out = C.imguiTableUpdateColumnsWeightFromWidth(i1)
    return out
end
M.TableUpdateLayout = M.TableUpdateLayout or function(i1)
    jit.off(true)
    local out = C.imguiTableUpdateLayout(i1)
    return out
end
M.TeleportMousePos = M.TeleportMousePos or function(i1)
    jit.off(true)
    local out = C.imguiTeleportMousePos(i1)
    return out
end
M.TempInputIsActive = M.TempInputIsActive or function(i1)
    jit.off(true)
    local out = C.imguiTempInputIsActive(i1)
    return out
end
M.TempInputScalar = M.TempInputScalar or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    local out = C.imguiTempInputScalar(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.TempInputText = M.TempInputText or function(i1, i2, i3, i4, i5, i6)
    jit.off(true)
    local out = C.imguiTempInputText(i1, i2, i3, i4, i5, i6)
    return out
end
M.TestKeyOwner = M.TestKeyOwner or function(i1, i2)
    jit.off(true)
    local out = C.imguiTestKeyOwner(i1, i2)
    return out
end
M.TestShortcutRouting = M.TestShortcutRouting or function(i1, i2)
    jit.off(true)
    local out = C.imguiTestShortcutRouting(i1, i2)
    return out
end
M.Text = M.Text or function(i1, ...)
    jit.off(true)
    local out = C.imguiText(i1, ...)
    return out
end
M.TextColored = M.TextColored or function(i1, i2, ...)
    jit.off(true)
    local out = C.imguiTextColored(i1, i2, ...)
    return out
end
M.TextDisabled = M.TextDisabled or function(i1, ...)
    jit.off(true)
    local out = C.imguiTextDisabled(i1, ...)
    return out
end
M.TextEx = M.TextEx or function(i1, i2, i3)
    jit.off(true)
    if i3 == nil then
        i3 = 0
    end
    local out = C.imguiTextEx(i1, i2, i3)
    return out
end
M.TextLink = M.TextLink or function(i1)
    jit.off(true)
    local out = C.imguiTextLink(i1)
    return out
end
M.TextLinkOpenURL = M.TextLinkOpenURL or function(i1, i2)
    jit.off(true)
    local out = C.imguiTextLinkOpenURL(i1, i2)
    return out
end
M.TextUnformatted = M.TextUnformatted or function(i1, i2)
    jit.off(true)
    local out = C.imguiTextUnformatted(i1, i2)
    return out
end
M.TextWrapped = M.TextWrapped or function(i1, ...)
    jit.off(true)
    local out = C.imguiTextWrapped(i1, ...)
    return out
end
M.TranslateWindowsInViewport = M.TranslateWindowsInViewport or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiTranslateWindowsInViewport(i1, i2, i3)
    return out
end
M.TreeNode_Str = M.TreeNode_Str or function(i1)
    jit.off(true)
    local out = C.imguiTreeNode_Str(i1)
    return out
end
M.TreeNode_StrStr = M.TreeNode_StrStr or function(i1, i2, ...)
    jit.off(true)
    local out = C.imguiTreeNode_StrStr(i1, i2, ...)
    return out
end
M.TreeNode_Ptr = M.TreeNode_Ptr or function(i1, i2, ...)
    jit.off(true)
    local out = C.imguiTreeNode_Ptr(i1, i2, ...)
    return out
end
M.TreeNodeBehavior = M.TreeNodeBehavior or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiTreeNodeBehavior(i1, i2, i3, i4)
    return out
end
M.TreeNodeEx_Str = M.TreeNodeEx_Str or function(i1, i2)
    jit.off(true)
    if i2 == nil then
        i2 = 0
    end
    local out = C.imguiTreeNodeEx_Str(i1, i2)
    return out
end
M.TreeNodeEx_StrStr = M.TreeNodeEx_StrStr or function(i1, i2, i3, ...)
    jit.off(true)
    local out = C.imguiTreeNodeEx_StrStr(i1, i2, i3, ...)
    return out
end
M.TreeNodeEx_Ptr = M.TreeNodeEx_Ptr or function(i1, i2, i3, ...)
    jit.off(true)
    local out = C.imguiTreeNodeEx_Ptr(i1, i2, i3, ...)
    return out
end
M.TreeNodeGetOpen = M.TreeNodeGetOpen or function(i1)
    jit.off(true)
    local out = C.imguiTreeNodeGetOpen(i1)
    return out
end
M.TreeNodeSetOpen = M.TreeNodeSetOpen or function(i1, i2)
    jit.off(true)
    local out = C.imguiTreeNodeSetOpen(i1, i2)
    return out
end
M.TreeNodeUpdateNextOpen = M.TreeNodeUpdateNextOpen or function(i1, i2)
    jit.off(true)
    local out = C.imguiTreeNodeUpdateNextOpen(i1, i2)
    return out
end
M.TreePop = M.TreePop or function()
    jit.off(true)
    local out = C.imguiTreePop()
    return out
end
M.TreePush_Str = M.TreePush_Str or function(i1)
    jit.off(true)
    local out = C.imguiTreePush_Str(i1)
    return out
end
M.TreePush_Ptr = M.TreePush_Ptr or function(i1)
    jit.off(true)
    local out = C.imguiTreePush_Ptr(i1)
    return out
end
M.TreePushOverrideID = M.TreePushOverrideID or function(i1)
    jit.off(true)
    local out = C.imguiTreePushOverrideID(i1)
    return out
end
M.TypingSelectFindBestLeadingMatch = M.TypingSelectFindBestLeadingMatch or function(i1, i2, i3, i4)
    jit.off(true)
    local out = C.imguiTypingSelectFindBestLeadingMatch(i1, i2, i3, i4)
    return out
end
M.TypingSelectFindMatch = M.TypingSelectFindMatch or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiTypingSelectFindMatch(i1, i2, i3, i4, i5)
    return out
end
M.TypingSelectFindNextSingleCharMatch = M.TypingSelectFindNextSingleCharMatch or function(i1, i2, i3, i4, i5)
    jit.off(true)
    local out = C.imguiTypingSelectFindNextSingleCharMatch(i1, i2, i3, i4, i5)
    return out
end
M.Unindent = M.Unindent or function(i1)
    jit.off(true)
    if i1 == nil then
        i1 = 0.0
    end
    local out = C.imguiUnindent(i1)
    return out
end
M.UpdateHoveredWindowAndCaptureFlags = M.UpdateHoveredWindowAndCaptureFlags or function()
    jit.off(true)
    local out = C.imguiUpdateHoveredWindowAndCaptureFlags()
    return out
end
M.UpdateInputEvents = M.UpdateInputEvents or function(i1)
    jit.off(true)
    local out = C.imguiUpdateInputEvents(i1)
    return out
end
M.UpdateMouseMovingWindowEndFrame = M.UpdateMouseMovingWindowEndFrame or function()
    jit.off(true)
    local out = C.imguiUpdateMouseMovingWindowEndFrame()
    return out
end
M.UpdateMouseMovingWindowNewFrame = M.UpdateMouseMovingWindowNewFrame or function()
    jit.off(true)
    local out = C.imguiUpdateMouseMovingWindowNewFrame()
    return out
end
M.UpdatePlatformWindows = M.UpdatePlatformWindows or function()
    jit.off(true)
    local out = C.imguiUpdatePlatformWindows()
    return out
end
M.UpdateWindowParentAndRootLinks = M.UpdateWindowParentAndRootLinks or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiUpdateWindowParentAndRootLinks(i1, i2, i3)
    return out
end
M.UpdateWindowSkipRefresh = M.UpdateWindowSkipRefresh or function(i1)
    jit.off(true)
    local out = C.imguiUpdateWindowSkipRefresh(i1)
    return out
end
M.VSliderFloat = M.VSliderFloat or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i6 == nil then
        i6 = "%.3f"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiVSliderFloat(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.VSliderInt = M.VSliderInt or function(i1, i2, i3, i4, i5, i6, i7)
    jit.off(true)
    if i6 == nil then
        i6 = "%d"
    end
    if i7 == nil then
        i7 = 0
    end
    local out = C.imguiVSliderInt(i1, i2, i3, i4, i5, i6, i7)
    return out
end
M.VSliderScalar = M.VSliderScalar or function(i1, i2, i3, i4, i5, i6, i7, i8)
    jit.off(true)
    if i8 == nil then
        i8 = 0
    end
    local out = C.imguiVSliderScalar(i1, i2, i3, i4, i5, i6, i7, i8)
    return out
end
M.Value_Bool = M.Value_Bool or function(i1, i2)
    jit.off(true)
    local out = C.imguiValue_Bool(i1, i2)
    return out
end
M.Value_Int = M.Value_Int or function(i1, i2)
    jit.off(true)
    local out = C.imguiValue_Int(i1, i2)
    return out
end
M.Value_Uint = M.Value_Uint or function(i1, i2)
    jit.off(true)
    local out = C.imguiValue_Uint(i1, i2)
    return out
end
M.Value_Float = M.Value_Float or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiValue_Float(i1, i2, i3)
    return out
end
M.WindowPosAbsToRel = M.WindowPosAbsToRel or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiWindowPosAbsToRel(o1, i1, i2)
    return o1, out
end
M.WindowPosRelToAbs = M.WindowPosRelToAbs or function(i1, i2)
    jit.off(true)
    local o1 = M.ImVec2_Nil()
    local out = C.imguiWindowPosRelToAbs(o1, i1, i2)
    return o1, out
end
M.WindowRectAbsToRel = M.WindowRectAbsToRel or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiWindowRectAbsToRel(i1, i2, i3)
    return out
end
M.WindowRectRelToAbs = M.WindowRectRelToAbs or function(i1, i2, i3)
    jit.off(true)
    local out = C.imguiWindowRectRelToAbs(i1, i2, i3)
    return out
end
-- require(path .. "love")
-- require(path .. "shortcuts")

-- remove access to M._common
M._common = nil

return M
