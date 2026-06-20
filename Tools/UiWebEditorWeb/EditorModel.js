// 模块: Tools UiWebEditorWeb
// 文件: Tools/UiWebEditorWeb/EditorModel.js

(function RegisterUiWebEditorModel(root, factory) {
    const api = factory();
    if (typeof module !== "undefined" && module.exports) {
        module.exports = api;
    }
    if (root) {
        root.UiWebEditorModel = api;
    }
})(typeof globalThis !== "undefined" ? globalThis : this, function CreateUiWebEditorModel() {
    const SCHEMA_ID = "YUFS";
    const SCHEMA_VERSION = 1;
    const RESOURCE_KINDS = ["Sprite", "Font", "Localization", "Audio", "Custom"];
    const DEFAULT_VIEWPORT_RECT = { x: 0, y: 0, width: 960, height: 540 };
    const COMPONENT_TYPES = ["Container", "Text", "Image", "Button", "Slider"];
    const COMPONENT_RECORD_KEYS = ["common", "container", "text", "image", "button", "slider"];
    const LAYOUT_TYPES = ["Absolute", "Stack", "Grid", "Overlay", "ScrollViewport"];
    const TEXT_CONTENT_MODES = ["Plain", "LocalizationKey"];
    const TEXT_FONT_STYLES = ["Normal", "Bold", "Italic", "BoldItalic"];
    const TEXT_HORIZONTAL_ALIGNMENTS = ["Left", "Center", "Right"];
    const TEXT_VERTICAL_ALIGNMENTS = ["Top", "Middle", "Bottom"];
    const TEXT_WRAP_MODES = ["None", "Character"];
    const TEXT_OVERFLOW_MODES = ["Clip"];
    const IMAGE_TYPES = ["Simple", "Sliced"];
    const SLIDER_AXES = ["Horizontal", "Vertical"];
    const HIT_TEST_ROUTES = ["None", "Self", "Children"];
    const COMPONENT_BACKLOG = [
        { component: "Toggle", status: "NeedsNativeRuntime", reason: "Toggle native parity is not available yet" },
        { component: "Progress", status: "NeedsNativeRuntime", reason: "Progress native parity is not available yet" },
        { component: "TextAutoSize", status: "Backlog", reason: "Text autosize requires native text measurement support" },
        { component: "TextEllipsis", status: "Backlog", reason: "Text ellipsis requires native overflow shaping support" },
        { component: "RichText", status: "Backlog", reason: "Rich text and link metadata require native parser support" },
        { component: "CameraPerspective", status: "NeedsNativeRuntime", reason: "Camera and perspective fields are outside current runtime UI data" },
        { component: "NativeSchemaValidator", status: "NeedsNativeRuntime", reason: "Component-specific native schema validator is not implemented" }
    ];

    function Clone(value) {
        return JSON.parse(JSON.stringify(value));
    }

    function ToNumber(value, fallback) {
        const number_value = Number(value);
        if (Number.isFinite(number_value)) {
            return number_value;
        }
        return fallback;
    }

    function ToBool(value, fallback) {
        if (typeof value === "boolean") {
            return value;
        }
        return fallback;
    }

    function ToString(value, fallback) {
        if (typeof value === "string") {
            return value;
        }
        return fallback;
    }

    function NormalizeOption(value, fallback, options) {
        const text = ToString(value, fallback);
        if (options.includes(text)) {
            return text;
        }
        return fallback;
    }

    function NormalizeKey(value, fallback) {
        const key = Math.trunc(ToNumber(value, fallback));
        if (key < 0) {
            return 0;
        }
        return key;
    }

    function NormalizePositiveNumber(value, fallback) {
        const number_value = ToNumber(value, fallback);
        if (number_value > 0) {
            return number_value;
        }
        return fallback;
    }

    function NormalizeColor(value, fallback) {
        const text = ToString(value, fallback);
        const color_pattern = /^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$/;
        if (color_pattern.test(text)) {
            return text;
        }
        return fallback;
    }

    function GetArray(value) {
        if (Array.isArray(value)) {
            return value;
        }
        return [];
    }

    function CreateVector2(x, y) {
        return {
            x: x,
            y: y
        };
    }

    function CreateThickness(left, top, right, bottom) {
        return {
            left: left,
            top: top,
            right: right,
            bottom: bottom
        };
    }

    function CreateRect(x, y, width, height) {
        return {
            x: x,
            y: y,
            width: width,
            height: height
        };
    }

    function CreateFullStretchRectTransform() {
        return {
            anchorMin: CreateVector2(0, 0),
            anchorMax: CreateVector2(1, 1),
            pivot: CreateVector2(0.5, 0.5),
            offsetMin: CreateVector2(0, 0),
            offsetMax: CreateVector2(0, 0),
            margin: CreateThickness(0, 0, 0, 0),
            padding: CreateThickness(0, 0, 0, 0),
            dpiScale: 1
        };
    }

    function NormalizeVector2(value, fallback) {
        const source = value || fallback || {};
        return {
            x: ToNumber(source.x, 0),
            y: ToNumber(source.y, 0)
        };
    }

    function NormalizeThickness(value, fallback) {
        const source = value || fallback || {};
        return {
            left: ToNumber(source.left, 0),
            top: ToNumber(source.top, 0),
            right: ToNumber(source.right, 0),
            bottom: ToNumber(source.bottom, 0)
        };
    }

    function NormalizeRect(value, fallback) {
        const source = value || fallback || DEFAULT_VIEWPORT_RECT;
        return {
            x: ToNumber(source.x, DEFAULT_VIEWPORT_RECT.x),
            y: ToNumber(source.y, DEFAULT_VIEWPORT_RECT.y),
            width: Math.max(1, ToNumber(source.width, DEFAULT_VIEWPORT_RECT.width)),
            height: Math.max(1, ToNumber(source.height, DEFAULT_VIEWPORT_RECT.height))
        };
    }

    function NormalizeRectTransform(value) {
        const full_stretch = CreateFullStretchRectTransform();
        const source = value || full_stretch;
        return {
            anchorMin: NormalizeVector2(source.anchorMin, full_stretch.anchorMin),
            anchorMax: NormalizeVector2(source.anchorMax, full_stretch.anchorMax),
            pivot: NormalizeVector2(source.pivot, full_stretch.pivot),
            offsetMin: NormalizeVector2(source.offsetMin, full_stretch.offsetMin),
            offsetMax: NormalizeVector2(source.offsetMax, full_stretch.offsetMax),
            margin: NormalizeThickness(source.margin, full_stretch.margin),
            padding: NormalizeThickness(source.padding, full_stretch.padding),
            dpiScale: Math.max(0.001, ToNumber(source.dpiScale, full_stretch.dpiScale))
        };
    }

    function NormalizeComponentType(value) {
        const component = String(value || "Container");
        if (COMPONENT_TYPES.includes(component)) {
            return component;
        }
        const backlog = COMPONENT_BACKLOG.find(function MatchBacklog(record) {
            return record.component === component;
        });
        if (backlog) {
            return backlog.component;
        }
        return component;
    }

    function GetComponentRecordKey(component) {
        if (component === "Container") {
            return "container";
        }
        if (component === "Text") {
            return "text";
        }
        if (component === "Image") {
            return "image";
        }
        if (component === "Button") {
            return "button";
        }
        if (component === "Slider") {
            return "slider";
        }
        return "";
    }

    function CreateDefaultCommonComponent() {
        return {
            layoutType: "Absolute",
            styleKey: 0,
            themeKey: 0,
            tokenKey: 0,
            resourceKey: 0,
            eventKey: 0,
            hitTestRoute: "Self",
            clipChildren: false
        };
    }

    function CreateDefaultTextComponent(label) {
        return {
            contentMode: "Plain",
            content: String(label || "Text"),
            localizationKey: "",
            fontResourceKey: 0,
            fallbackFontResourceKey: 0,
            fontSize: 18,
            fontStyle: "Normal",
            horizontalAlignment: "Left",
            verticalAlignment: "Middle",
            wrap: "Character",
            overflow: "Clip",
            lineHeight: 1.2,
            tint: "#111827",
            outline: {
                enabled: false,
                color: "#000000",
                width: 0
            },
            shadow: {
                enabled: false,
                color: "#000000",
                offset: CreateVector2(0, -1)
            },
            materialKey: 0,
            styleKey: 0,
            scissor: false,
            raycastTarget: false
        };
    }

    function CreateDefaultImageComponent() {
        return {
            spriteResourceKey: 0,
            atlasKey: 0,
            materialKey: 0,
            styleKey: 0,
            imageType: "Simple",
            tint: "#ffffff",
            preserveAspect: false,
            nineSlice: CreateThickness(0, 0, 0, 0),
            scissor: false,
            raycastTarget: false
        };
    }

    function CreateDefaultButtonComponent(label) {
        return {
            label: String(label || "Button"),
            textStyleKey: 0,
            imageStyleKey: 0,
            normalStyleKey: 0,
            hoverStyleKey: 0,
            pressedStyleKey: 0,
            disabledStyleKey: 0,
            selectedStyleKey: 0,
            normalSpriteKey: 0,
            hoverSpriteKey: 0,
            pressedSpriteKey: 0,
            disabledSpriteKey: 0,
            selectedSpriteKey: 0,
            submitEventKey: 0,
            cancelEventKey: 0,
            soundResourceKey: 0,
            actionKey: "Submit",
            pointerActivation: true,
            keyboardActivation: true,
            gamepadActivation: true,
            raycastTarget: true
        };
    }

    function CreateDefaultSliderComponent() {
        return {
            minValue: 0,
            maxValue: 1,
            value: 0.5,
            step: 0.1,
            axis: "Horizontal",
            trackStyleKey: 0,
            fillStyleKey: 0,
            handleStyleKey: 0,
            trackSpriteKey: 0,
            fillSpriteKey: 0,
            handleSpriteKey: 0,
            changeEventKey: 0,
            increaseActionKey: "Increase",
            decreaseActionKey: "Decrease",
            pointerDrag: true,
            keyboardAdjust: true,
            gamepadAdjust: true,
            raycastTarget: true
        };
    }

    function CreateDefaultContainerComponent() {
        return {
            layoutType: "Absolute",
            styleKey: 0,
            scissor: false,
            raycastTarget: false
        };
    }

    function NormalizeCommonComponent(value) {
        const source = value || {};
        return {
            layoutType: NormalizeOption(source.layoutType, "Absolute", LAYOUT_TYPES),
            styleKey: NormalizeKey(source.styleKey, 0),
            themeKey: NormalizeKey(source.themeKey, 0),
            tokenKey: NormalizeKey(source.tokenKey, 0),
            resourceKey: NormalizeKey(source.resourceKey, 0),
            eventKey: NormalizeKey(source.eventKey, 0),
            hitTestRoute: NormalizeOption(source.hitTestRoute, "Self", HIT_TEST_ROUTES),
            clipChildren: ToBool(source.clipChildren, false)
        };
    }

    function NormalizeContainerComponent(value) {
        const source = value || {};
        return {
            layoutType: NormalizeOption(source.layoutType, "Absolute", LAYOUT_TYPES),
            styleKey: NormalizeKey(source.styleKey, 0),
            scissor: ToBool(source.scissor, false),
            raycastTarget: ToBool(source.raycastTarget, false)
        };
    }

    function NormalizeTextComponent(value, legacy_text) {
        const source = value || {};
        const fallback_text = String(legacy_text || "Text");
        const outline = source.outline || {};
        const shadow = source.shadow || {};
        return {
            contentMode: NormalizeOption(source.contentMode, "Plain", TEXT_CONTENT_MODES),
            content: ToString(source.content, fallback_text),
            localizationKey: ToString(source.localizationKey, ""),
            fontResourceKey: NormalizeKey(source.fontResourceKey, 0),
            fallbackFontResourceKey: NormalizeKey(source.fallbackFontResourceKey, 0),
            fontSize: NormalizePositiveNumber(source.fontSize, 18),
            fontStyle: NormalizeOption(source.fontStyle, "Normal", TEXT_FONT_STYLES),
            horizontalAlignment: NormalizeOption(source.horizontalAlignment, "Left", TEXT_HORIZONTAL_ALIGNMENTS),
            verticalAlignment: NormalizeOption(source.verticalAlignment, "Middle", TEXT_VERTICAL_ALIGNMENTS),
            wrap: NormalizeOption(source.wrap, "Character", TEXT_WRAP_MODES),
            overflow: NormalizeOption(source.overflow, "Clip", TEXT_OVERFLOW_MODES),
            lineHeight: NormalizePositiveNumber(source.lineHeight, 1.2),
            tint: NormalizeColor(source.tint, "#111827"),
            outline: {
                enabled: ToBool(outline.enabled, false),
                color: NormalizeColor(outline.color, "#000000"),
                width: Math.max(0, ToNumber(outline.width, 0))
            },
            shadow: {
                enabled: ToBool(shadow.enabled, false),
                color: NormalizeColor(shadow.color, "#000000"),
                offset: NormalizeVector2(shadow.offset, CreateVector2(0, -1))
            },
            materialKey: NormalizeKey(source.materialKey, 0),
            styleKey: NormalizeKey(source.styleKey, 0),
            scissor: ToBool(source.scissor, false),
            raycastTarget: ToBool(source.raycastTarget, false)
        };
    }

    function NormalizeImageComponent(value) {
        const source = value || {};
        return {
            spriteResourceKey: NormalizeKey(source.spriteResourceKey, 0),
            atlasKey: NormalizeKey(source.atlasKey, 0),
            materialKey: NormalizeKey(source.materialKey, 0),
            styleKey: NormalizeKey(source.styleKey, 0),
            imageType: NormalizeOption(source.imageType, "Simple", IMAGE_TYPES),
            tint: NormalizeColor(source.tint, "#ffffff"),
            preserveAspect: ToBool(source.preserveAspect, false),
            nineSlice: NormalizeThickness(source.nineSlice, CreateThickness(0, 0, 0, 0)),
            scissor: ToBool(source.scissor, false),
            raycastTarget: ToBool(source.raycastTarget, false)
        };
    }

    function NormalizeButtonComponent(value, legacy_text) {
        const source = value || {};
        const fallback_label = String(legacy_text || "Button");
        return {
            label: ToString(source.label, fallback_label),
            textStyleKey: NormalizeKey(source.textStyleKey, 0),
            imageStyleKey: NormalizeKey(source.imageStyleKey, 0),
            normalStyleKey: NormalizeKey(source.normalStyleKey, 0),
            hoverStyleKey: NormalizeKey(source.hoverStyleKey, 0),
            pressedStyleKey: NormalizeKey(source.pressedStyleKey, 0),
            disabledStyleKey: NormalizeKey(source.disabledStyleKey, 0),
            selectedStyleKey: NormalizeKey(source.selectedStyleKey, 0),
            normalSpriteKey: NormalizeKey(source.normalSpriteKey, 0),
            hoverSpriteKey: NormalizeKey(source.hoverSpriteKey, 0),
            pressedSpriteKey: NormalizeKey(source.pressedSpriteKey, 0),
            disabledSpriteKey: NormalizeKey(source.disabledSpriteKey, 0),
            selectedSpriteKey: NormalizeKey(source.selectedSpriteKey, 0),
            submitEventKey: NormalizeKey(source.submitEventKey, 0),
            cancelEventKey: NormalizeKey(source.cancelEventKey, 0),
            soundResourceKey: NormalizeKey(source.soundResourceKey, 0),
            actionKey: ToString(source.actionKey, "Submit"),
            pointerActivation: ToBool(source.pointerActivation, true),
            keyboardActivation: ToBool(source.keyboardActivation, true),
            gamepadActivation: ToBool(source.gamepadActivation, true),
            raycastTarget: ToBool(source.raycastTarget, true)
        };
    }

    function NormalizeSliderComponent(value) {
        const source = value || {};
        const min_value = ToNumber(source.minValue, 0);
        let max_value = ToNumber(source.maxValue, 1);
        if (max_value <= min_value) {
            max_value = min_value + 1;
        }
        let current_value = ToNumber(source.value, min_value);
        if (current_value < min_value) {
            current_value = min_value;
        }
        if (current_value > max_value) {
            current_value = max_value;
        }
        return {
            minValue: min_value,
            maxValue: max_value,
            value: current_value,
            step: NormalizePositiveNumber(source.step, 0.1),
            axis: NormalizeOption(source.axis, "Horizontal", SLIDER_AXES),
            trackStyleKey: NormalizeKey(source.trackStyleKey, 0),
            fillStyleKey: NormalizeKey(source.fillStyleKey, 0),
            handleStyleKey: NormalizeKey(source.handleStyleKey, 0),
            trackSpriteKey: NormalizeKey(source.trackSpriteKey, 0),
            fillSpriteKey: NormalizeKey(source.fillSpriteKey, 0),
            handleSpriteKey: NormalizeKey(source.handleSpriteKey, 0),
            changeEventKey: NormalizeKey(source.changeEventKey, 0),
            increaseActionKey: ToString(source.increaseActionKey, "Increase"),
            decreaseActionKey: ToString(source.decreaseActionKey, "Decrease"),
            pointerDrag: ToBool(source.pointerDrag, true),
            keyboardAdjust: ToBool(source.keyboardAdjust, true),
            gamepadAdjust: ToBool(source.gamepadAdjust, true),
            raycastTarget: ToBool(source.raycastTarget, true)
        };
    }

    function CreateDefaultComponentRecords(component, label) {
        const records = {
            common: CreateDefaultCommonComponent(),
            container: CreateDefaultContainerComponent(),
            text: CreateDefaultTextComponent(label),
            image: CreateDefaultImageComponent(),
            button: CreateDefaultButtonComponent(label),
            slider: CreateDefaultSliderComponent()
        };
        if (component === "Container") {
            records.common.layoutType = "Stack";
            records.container.layoutType = "Stack";
        }
        return records;
    }

    function NormalizeComponents(value, component, legacy_text) {
        const source = value || {};
        return {
            common: NormalizeCommonComponent(source.common),
            container: NormalizeContainerComponent(source.container),
            text: NormalizeTextComponent(source.text, legacy_text),
            image: NormalizeImageComponent(source.image),
            button: NormalizeButtonComponent(source.button, legacy_text),
            slider: NormalizeSliderComponent(source.slider)
        };
    }

    function ResolveRectTransform(parent_rect, transform) {
        const parent = NormalizeRect(parent_rect, DEFAULT_VIEWPORT_RECT);
        const rect_transform = NormalizeRectTransform(transform);
        const dpi_scale = rect_transform.dpiScale;
        const margin = rect_transform.margin;
        const padding = rect_transform.padding;
        const left_anchor = parent.x + parent.width * rect_transform.anchorMin.x;
        const bottom_anchor = parent.y + parent.height * rect_transform.anchorMin.y;
        const right_anchor = parent.x + parent.width * rect_transform.anchorMax.x;
        const top_anchor = parent.y + parent.height * rect_transform.anchorMax.y;
        const left = left_anchor + rect_transform.offsetMin.x * dpi_scale + margin.left * dpi_scale;
        const bottom = bottom_anchor + rect_transform.offsetMin.y * dpi_scale + margin.bottom * dpi_scale;
        const right = right_anchor + rect_transform.offsetMax.x * dpi_scale - margin.right * dpi_scale;
        const top = top_anchor + rect_transform.offsetMax.y * dpi_scale - margin.top * dpi_scale;
        const width = right - left;
        const height = top - bottom;
        const content_left = left + padding.left * dpi_scale;
        const content_bottom = bottom + padding.bottom * dpi_scale;
        const content_width = width - (padding.left + padding.right) * dpi_scale;
        const content_height = height - (padding.top + padding.bottom) * dpi_scale;
        return {
            status: width >= 0 && height >= 0 && content_width >= 0 && content_height >= 0 ? "Success" : "InvalidOutputRect",
            rect: CreateRect(left, bottom, width, height),
            contentRect: CreateRect(content_left, content_bottom, content_width, content_height),
            pivotPoint: CreateVector2(left + width * rect_transform.pivot.x, bottom + height * rect_transform.pivot.y)
        };
    }

    function EngineRectToCanvasRect(engine_rect, viewport) {
        const view = NormalizeEditorViewport(viewport);
        const rect = NormalizeRect(engine_rect, DEFAULT_VIEWPORT_RECT);
        const view_top = view.runtimeRect.y + view.runtimeRect.height;
        return {
            left: (rect.x - view.runtimeRect.x) * view.scale + view.panX,
            top: (view_top - rect.y - rect.height) * view.scale + view.panY,
            width: rect.width * view.scale,
            height: rect.height * view.scale
        };
    }

    function CanvasRectToEngineRect(canvas_rect, viewport) {
        const view = NormalizeEditorViewport(viewport);
        const rect = canvas_rect || {};
        const left = ToNumber(rect.left, 0);
        const top = ToNumber(rect.top, 0);
        const width = Math.max(1, ToNumber(rect.width, 1));
        const height = Math.max(1, ToNumber(rect.height, 1));
        const engine_width = width / view.scale;
        const engine_height = height / view.scale;
        const view_top = view.runtimeRect.y + view.runtimeRect.height;
        return {
            x: view.runtimeRect.x + (left - view.panX) / view.scale,
            y: view_top - ((top - view.panY) / view.scale) - engine_height,
            width: engine_width,
            height: engine_height
        };
    }

    function ApplyEngineRectToRectTransform(parent_rect, transform, engine_rect) {
        const parent = NormalizeRect(parent_rect, DEFAULT_VIEWPORT_RECT);
        const rect_transform = NormalizeRectTransform(transform);
        const rect = NormalizeRect(engine_rect, DEFAULT_VIEWPORT_RECT);
        const dpi_scale = rect_transform.dpiScale;
        const margin = rect_transform.margin;
        const left_anchor = parent.x + parent.width * rect_transform.anchorMin.x;
        const bottom_anchor = parent.y + parent.height * rect_transform.anchorMin.y;
        const right_anchor = parent.x + parent.width * rect_transform.anchorMax.x;
        const top_anchor = parent.y + parent.height * rect_transform.anchorMax.y;
        const right = rect.x + rect.width;
        const top = rect.y + rect.height;
        rect_transform.offsetMin = {
            x: (rect.x - left_anchor - margin.left * dpi_scale) / dpi_scale,
            y: (rect.y - bottom_anchor - margin.bottom * dpi_scale) / dpi_scale
        };
        rect_transform.offsetMax = {
            x: (right - right_anchor + margin.right * dpi_scale) / dpi_scale,
            y: (top - top_anchor + margin.top * dpi_scale) / dpi_scale
        };
        return rect_transform;
    }

    function CreateFixedRectTransformFromCanvas(canvas_rect, viewport) {
        const engine_rect = CanvasRectToEngineRect(canvas_rect, viewport);
        const parent_rect = NormalizeEditorViewport(viewport).runtimeRect;
        return ApplyEngineRectToRectTransform(parent_rect, CreateFullStretchRectTransform(), engine_rect);
    }

    function NormalizeEditorViewport(value) {
        const source = value || {};
        return {
            runtimeRect: NormalizeRect(source.runtimeRect, DEFAULT_VIEWPORT_RECT),
            scale: Math.max(0.001, ToNumber(source.scale, 1)),
            panX: ToNumber(source.panX, 0),
            panY: ToNumber(source.panY, 0)
        };
    }

    function CreateDefaultDocument() {
        const viewport = NormalizeEditorViewport({});
        const root_components = CreateDefaultComponentRecords("Container", "Root");
        const header_components = CreateDefaultComponentRecords("Text", "Title");
        const button_components = CreateDefaultComponentRecords("Button", "Action");
        root_components.common.styleKey = 1101;
        header_components.common.styleKey = 1102;
        header_components.common.resourceKey = 2101;
        header_components.text.fontResourceKey = 2101;
        header_components.text.styleKey = 1102;
        button_components.common.styleKey = 1103;
        button_components.common.eventKey = 4101;
        button_components.button.normalStyleKey = 1103;
        button_components.button.submitEventKey = 4101;
        return {
            schema: {
                schemaId: SCHEMA_ID,
                schemaVersion: SCHEMA_VERSION,
                layoutId: 1001,
                rootNodeId: 1
            },
            nodes: [
                {
                    nodeId: 1,
                    parentId: 0,
                    name: "Root",
                    component: "Container",
                    components: root_components,
                    rectTransform: CreateFullStretchRectTransform(),
                    siblingOrder: 0,
                    layer: 0,
                    visible: true,
                    enabled: true,
                    hitTestable: false
                },
                {
                    nodeId: 2,
                    parentId: 1,
                    name: "Header",
                    component: "Text",
                    components: header_components,
                    rectTransform: CreateFixedRectTransformFromCanvas(
                        { left: 32, top: 28, width: 320, height: 48 },
                        viewport),
                    siblingOrder: 0,
                    layer: 1,
                    visible: true,
                    enabled: true,
                    hitTestable: false
                },
                {
                    nodeId: 3,
                    parentId: 1,
                    name: "PreviewButton",
                    component: "Button",
                    components: button_components,
                    rectTransform: CreateFixedRectTransformFromCanvas(
                        { left: 32, top: 108, width: 220, height: 48 },
                        viewport),
                    siblingOrder: 1,
                    layer: 1,
                    visible: true,
                    enabled: true,
                    hitTestable: true
                }
            ],
            layouts: [
                {
                    containerNodeId: 1,
                    type: "Stack",
                    direction: "Vertical",
                    gridColumnCount: 1,
                    spacing: 12,
                    padding: 24
                }
            ],
            styleRefs: [
                {
                    nodeId: 1,
                    styleKey: 1101,
                    themeKey: 1,
                    tokenKey: 1,
                    valueKind: "ColorRgba8",
                    value: "#f8fafc",
                    overridesTheme: true
                },
                {
                    nodeId: 2,
                    styleKey: 1102,
                    themeKey: 1,
                    tokenKey: 2,
                    valueKind: "ColorRgba8",
                    value: "#111827",
                    overridesTheme: true
                },
                {
                    nodeId: 3,
                    styleKey: 1103,
                    themeKey: 1,
                    tokenKey: 3,
                    valueKind: "ColorRgba8",
                    value: "#2563eb",
                    overridesTheme: true
                }
            ],
            resourceRefs: [
                {
                    nodeId: 2,
                    kind: "Font",
                    resourceKey: 2101,
                    label: "DefaultSans"
                }
            ],
            eventBindings: [
                {
                    nodeId: 3,
                    bindingKey: 3101,
                    eventKey: 4101,
                    command: "Submit"
                }
            ],
            theme: {
                tokens: [
                    { tokenKey: 1, name: "Surface", role: "Color", valueKind: "ColorRgba8", value: "#f8fafc" },
                    { tokenKey: 2, name: "TextPrimary", role: "Color", valueKind: "ColorRgba8", value: "#111827" },
                    { tokenKey: 3, name: "Accent", role: "Color", valueKind: "ColorRgba8", value: "#2563eb" },
                    { tokenKey: 4, name: "Gap", role: "Number", valueKind: "Number1000", value: 12000 }
                ],
                themes: [
                    { themeKey: 1, name: "Default", tokenKeys: [1, 2, 3, 4], default: true }
                ]
            },
            statePreview: [
                {
                    inputKey: 5101,
                    nodeId: 3,
                    name: "Pressed",
                    valueKind: "Bool",
                    value: false,
                    affectsVisibility: false,
                    affectsEnabled: true,
                    affectsResource: false
                }
            ],
            editor: {
                selectedNodeId: 1,
                dirty: false,
                viewport: viewport
            }
        };
    }

    function MigrateLegacyRect(rect) {
        const viewport = NormalizeEditorViewport({});
        const canvas_rect = {
            left: ToNumber(rect.x, 0),
            top: ToNumber(rect.y, 0),
            width: Math.max(1, ToNumber(rect.width, 120)),
            height: Math.max(1, ToNumber(rect.height, 48))
        };
        return CreateFixedRectTransformFromCanvas(canvas_rect, viewport);
    }

    function NormalizeNode(node, index) {
        const node_id = ToNumber(node.nodeId, index + 1);
        const rect_transform = node.rectTransform ? node.rectTransform : MigrateLegacyRect(node.rect || {});
        const component = NormalizeComponentType(node.component);
        const legacy_text = ToString(node.text, "");
        return {
            nodeId: node_id,
            parentId: ToNumber(node.parentId, 0),
            name: String(node.name || "Node" + node_id),
            component: component,
            components: NormalizeComponents(node.components, component, legacy_text),
            rectTransform: NormalizeRectTransform(rect_transform),
            siblingOrder: ToNumber(node.siblingOrder, index),
            layer: ToNumber(node.layer, 0),
            visible: ToBool(node.visible, true),
            enabled: ToBool(node.enabled, true),
            hitTestable: ToBool(node.hitTestable, true)
        };
    }

    function NormalizeDocument(source) {
        const base = source && typeof source === "object" ? source : CreateDefaultDocument();
        const document = Clone(base);
        const schema = document.schema || {};
        document.schema = {
            schemaId: schema.schemaId || SCHEMA_ID,
            schemaVersion: ToNumber(schema.schemaVersion, SCHEMA_VERSION),
            layoutId: ToNumber(schema.layoutId, 0),
            rootNodeId: ToNumber(schema.rootNodeId, 0)
        };
        document.nodes = GetArray(document.nodes).map(NormalizeNode);
        document.layouts = GetArray(document.layouts);
        document.styleRefs = GetArray(document.styleRefs);
        document.resourceRefs = GetArray(document.resourceRefs);
        document.eventBindings = GetArray(document.eventBindings);
        document.statePreview = GetArray(document.statePreview);
        document.theme = document.theme || { tokens: [], themes: [] };
        document.theme.tokens = GetArray(document.theme.tokens);
        document.theme.themes = GetArray(document.theme.themes);
        document.editor = document.editor || {};
        document.editor.selectedNodeId = ToNumber(document.editor.selectedNodeId, document.schema.rootNodeId);
        document.editor.dirty = ToBool(document.editor.dirty, false);
        document.editor.viewport = NormalizeEditorViewport(document.editor.viewport);
        NormalizeAllSiblingOrders(document);
        return document;
    }

    function FindNode(document, node_id) {
        return GetArray(document.nodes).find(function MatchNode(node) {
            return node.nodeId === node_id;
        }) || null;
    }

    function GetNextNodeId(document) {
        let next_id = 1;
        GetArray(document.nodes).forEach(function VisitNode(node) {
            if (node.nodeId >= next_id) {
                next_id = node.nodeId + 1;
            }
        });
        return next_id;
    }

    function CompareSiblingNodes(left, right) {
        if (left.siblingOrder !== right.siblingOrder) {
            return left.siblingOrder - right.siblingOrder;
        }
        return left.nodeId - right.nodeId;
    }

    function GetChildNodes(document, parent_id) {
        return GetArray(document.nodes).filter(function MatchParent(node) {
            return node.parentId === parent_id;
        }).sort(CompareSiblingNodes);
    }

    function NormalizeSiblingOrdersForParent(document, parent_id) {
        const children = GetChildNodes(document, parent_id);
        children.forEach(function AssignOrder(node, index) {
            node.siblingOrder = index;
        });
    }

    function NormalizeAllSiblingOrders(document) {
        const parent_ids = new Set();
        GetArray(document.nodes).forEach(function RegisterParent(node) {
            parent_ids.add(node.parentId);
        });
        parent_ids.forEach(function NormalizeParent(parent_id) {
            NormalizeSiblingOrdersForParent(document, parent_id);
        });
    }

    function IsDescendantNode(document, ancestor_id, node_id) {
        let current_id = node_id;
        const visited = new Set();
        while (current_id > 0) {
            if (visited.has(current_id)) {
                return false;
            }
            visited.add(current_id);
            const current = FindNode(document, current_id);
            if (!current) {
                return false;
            }
            if (current.parentId === ancestor_id) {
                return true;
            }
            current_id = current.parentId;
        }
        return false;
    }

    function PushIssue(issues, kind, message, node_id) {
        issues.push({
            kind: kind,
            message: message,
            nodeId: node_id || 0
        });
    }

    function IsUnitRange(value) {
        if (value < 0) {
            return false;
        }
        return value <= 1;
    }

    function IsNonNegative(value) {
        return value >= 0;
    }

    function ValidateRectTransform(transform) {
        const rect_transform = NormalizeRectTransform(transform);
        if (!IsUnitRange(rect_transform.anchorMin.x) || !IsUnitRange(rect_transform.anchorMin.y)) {
            return "InvalidAnchor";
        }
        if (!IsUnitRange(rect_transform.anchorMax.x) || !IsUnitRange(rect_transform.anchorMax.y)) {
            return "InvalidAnchor";
        }
        if (rect_transform.anchorMin.x > rect_transform.anchorMax.x || rect_transform.anchorMin.y > rect_transform.anchorMax.y) {
            return "InvalidAnchor";
        }
        if (!IsUnitRange(rect_transform.pivot.x) || !IsUnitRange(rect_transform.pivot.y)) {
            return "InvalidPivot";
        }
        if (rect_transform.dpiScale <= 0) {
            return "InvalidDpiScale";
        }
        const margin = rect_transform.margin;
        if (!IsNonNegative(margin.left) || !IsNonNegative(margin.top) ||
            !IsNonNegative(margin.right) || !IsNonNegative(margin.bottom)) {
            return "InvalidMargin";
        }
        const padding = rect_transform.padding;
        if (!IsNonNegative(padding.left) || !IsNonNegative(padding.top) ||
            !IsNonNegative(padding.right) || !IsNonNegative(padding.bottom)) {
            return "InvalidPadding";
        }
        return "Success";
    }

    function ValidateNodeComponents(issues, node) {
        const backlog = COMPONENT_BACKLOG.find(function MatchBacklog(record) {
            return record.component === node.component;
        });
        if (backlog) {
            PushIssue(issues, backlog.status, backlog.reason, node.nodeId);
            return;
        }
        if (!COMPONENT_TYPES.includes(node.component)) {
            PushIssue(issues, "UnsupportedComponent", "Component type is not supported by runtime export", node.nodeId);
            return;
        }
        const component_key = GetComponentRecordKey(node.component);
        if (!component_key || !node.components[component_key]) {
            PushIssue(issues, "MissingComponentRecord", "Node component record is missing", node.nodeId);
            return;
        }
        if (!COMPONENT_RECORD_KEYS.every(function MatchRecordKey(key) {
            return Object.prototype.hasOwnProperty.call(node.components, key);
        })) {
            PushIssue(issues, "MissingComponentRecord", "Common component records are incomplete", node.nodeId);
            return;
        }
        if (node.component === "Slider") {
            const slider = node.components.slider;
            if (slider.maxValue <= slider.minValue) {
                PushIssue(issues, "InvalidComponentRecord", "Slider max value must be greater than min value", node.nodeId);
                return;
            }
            if (slider.step <= 0) {
                PushIssue(issues, "InvalidComponentRecord", "Slider step must be positive", node.nodeId);
                return;
            }
        }
        if (node.component === "Text" && node.components.text.overflow !== "Clip") {
            PushIssue(issues, "NeedsNativeRuntime", "Text overflow mode requires native runtime support", node.nodeId);
            return;
        }
    }

    function CountComponents(nodes) {
        const counts = {};
        COMPONENT_TYPES.forEach(function SetInitialCount(component) {
            counts[component] = 0;
        });
        nodes.forEach(function CountNodeComponent(node) {
            if (!Object.prototype.hasOwnProperty.call(counts, node.component)) {
                counts.Unsupported = (counts.Unsupported || 0) + 1;
                return;
            }
            counts[node.component] += 1;
        });
        return counts;
    }

    function CreateFailedRectResult(status) {
        return {
            status: status,
            rect: CreateRect(0, 0, 0, 0),
            contentRect: CreateRect(0, 0, 0, 0),
            pivotPoint: CreateVector2(0, 0)
        };
    }

    function ResolveDocumentRects(source) {
        const document = NormalizeDocument(source);
        const viewport = document.editor.viewport;
        const resolved = new Map();
        const visiting = new Set();
        const nodes_by_id = new Map();
        document.nodes.forEach(function RegisterNode(node) {
            if (nodes_by_id.has(node.nodeId)) {
                return;
            }
            nodes_by_id.set(node.nodeId, node);
        });

        function ResolveNode(node) {
            const cached = resolved.get(node.nodeId);
            if (cached) {
                return cached;
            }
            if (visiting.has(node.nodeId)) {
                const cyclic_result = CreateFailedRectResult("CyclicParentNode");
                resolved.set(node.nodeId, cyclic_result);
                return cyclic_result;
            }

            visiting.add(node.nodeId);
            let parent_rect = viewport.runtimeRect;
            if (node.parentId > 0) {
                const parent = nodes_by_id.get(node.parentId);
                if (!parent) {
                    const missing_result = CreateFailedRectResult("MissingParentNode");
                    resolved.set(node.nodeId, missing_result);
                    visiting.delete(node.nodeId);
                    return missing_result;
                }
                const parent_result = ResolveNode(parent);
                if (parent_result.status !== "Success") {
                    const parent_failed_result = CreateFailedRectResult(parent_result.status);
                    resolved.set(node.nodeId, parent_failed_result);
                    visiting.delete(node.nodeId);
                    return parent_failed_result;
                }
                parent_rect = parent_result.rect;
            }

            const result = ResolveRectTransform(parent_rect, node.rectTransform);
            resolved.set(node.nodeId, result);
            visiting.delete(node.nodeId);
            return result;
        }

        document.nodes.forEach(function ResolveNodeEntry(node) {
            ResolveNode(node);
        });
        return resolved;
    }

    function ValidateDocument(source) {
        const document = NormalizeDocument(source);
        const issues = [];
        if (document.schema.schemaId !== SCHEMA_ID) {
            PushIssue(issues, "InvalidHeader", "Schema id must be YUFS", 0);
        }
        if (document.schema.schemaVersion !== SCHEMA_VERSION) {
            PushIssue(issues, "InvalidHeader", "Schema version must be 1", 0);
        }
        if (document.nodes.length === 0) {
            PushIssue(issues, "MissingRootNode", "Document must contain nodes", 0);
        }

        const node_counts = new Map();
        document.nodes.forEach(function CountNode(node) {
            const count = node_counts.get(node.nodeId) || 0;
            node_counts.set(node.nodeId, count + 1);
        });

        const resolved = ResolveDocumentRects(document);
        document.nodes.forEach(function CheckNode(node) {
            const duplicate_count = node_counts.get(node.nodeId) || 0;
            const transform_status = ValidateRectTransform(node.rectTransform);
            const rect_result = resolved.get(node.nodeId);
            if (duplicate_count > 1) {
                PushIssue(issues, "DuplicateNodeId", "Node id is duplicated", node.nodeId);
            }
            if (node.nodeId <= 0) {
                PushIssue(issues, "InvalidNodeRecord", "Node id must be positive", node.nodeId);
            }
            if (node.parentId > 0 && !FindNode(document, node.parentId)) {
                PushIssue(issues, "MissingParentNode", "Parent node does not exist", node.nodeId);
            }
            if (transform_status !== "Success") {
                PushIssue(issues, transform_status, "RectTransform field is invalid", node.nodeId);
            }
            if (rect_result && rect_result.status !== "Success") {
                PushIssue(issues, rect_result.status, "RectTransform cannot resolve to a valid runtime rect", node.nodeId);
            }
            ValidateNodeComponents(issues, node);
        });

        const root = FindNode(document, document.schema.rootNodeId);
        if (!root) {
            PushIssue(issues, "MissingRootNode", "Root node does not exist", document.schema.rootNodeId);
        }

        document.layouts.forEach(function CheckLayout(layout) {
            if (!FindNode(document, ToNumber(layout.containerNodeId, 0))) {
                PushIssue(issues, "MissingLayoutContainerNode", "Layout container node does not exist", 0);
            }
        });

        document.styleRefs.forEach(function CheckStyle(style_ref) {
            if (!FindNode(document, ToNumber(style_ref.nodeId, 0))) {
                PushIssue(issues, "MissingStyleRefNode", "Style ref node does not exist", 0);
            }
        });

        document.resourceRefs.forEach(function CheckResource(resource_ref) {
            const kind = String(resource_ref.kind || "Invalid");
            if (!FindNode(document, ToNumber(resource_ref.nodeId, 0))) {
                PushIssue(issues, "MissingResourceRefNode", "Resource ref node does not exist", 0);
            }
            if (!RESOURCE_KINDS.includes(kind)) {
                PushIssue(issues, "InvalidResourceRef", "Resource kind is not supported", ToNumber(resource_ref.nodeId, 0));
            }
        });

        document.eventBindings.forEach(function CheckBinding(binding) {
            if (!FindNode(document, ToNumber(binding.nodeId, 0))) {
                PushIssue(issues, "MissingEventBindingNode", "Event binding node does not exist", 0);
            }
            if (ToNumber(binding.bindingKey, 0) === 0 || ToNumber(binding.eventKey, 0) === 0) {
                PushIssue(issues, "MissingEventBindingKey", "Event binding requires stable keys", ToNumber(binding.nodeId, 0));
            }
        });

        const status = issues.length === 0 ? "Success" : "IssuesFound";
        return {
            status: status,
            issues: issues,
            summary: {
                nodeCount: document.nodes.length,
                layoutCount: document.layouts.length,
                styleRefCount: document.styleRefs.length,
                resourceRefCount: document.resourceRefs.length,
                eventBindingCount: document.eventBindings.length,
                componentCounts: CountComponents(document.nodes),
                issueCount: issues.length
            }
        };
    }

    function GetNodeDepth(document, node) {
        let depth = 0;
        let parent_id = node.parentId;
        const visited = new Set();
        while (parent_id > 0) {
            if (visited.has(parent_id)) {
                return depth;
            }
            visited.add(parent_id);
            const parent = FindNode(document, parent_id);
            if (!parent) {
                return depth;
            }
            depth += 1;
            parent_id = parent.parentId;
        }
        return depth;
    }

    function BuildHierarchy(document) {
        const normalized = NormalizeDocument(document);
        const rows = [];
        const visited = new Set();

        function PushHierarchyNode(node, depth) {
            if (visited.has(node.nodeId)) {
                return;
            }
            visited.add(node.nodeId);
            rows.push({
                nodeId: node.nodeId,
                parentId: node.parentId,
                name: node.name,
                component: node.component,
                depth: depth,
                selected: normalized.editor.selectedNodeId === node.nodeId,
                visible: node.visible,
                enabled: node.enabled,
                canDrag: node.nodeId !== normalized.schema.rootNodeId
            });
            const children = GetChildNodes(normalized, node.nodeId);
            children.forEach(function PushChild(child) {
                PushHierarchyNode(child, depth + 1);
            });
        }

        const root = FindNode(normalized, normalized.schema.rootNodeId);
        if (root) {
            PushHierarchyNode(root, 0);
        }

        normalized.nodes.slice().sort(CompareSiblingNodes).forEach(function PushUnvisited(node) {
            if (visited.has(node.nodeId)) {
                return;
            }
            PushHierarchyNode(node, GetNodeDepth(normalized, node));
        });
        return rows;
    }

    function GetComponentPreviewText(node) {
        if (node.component === "Text") {
            return node.components.text.content;
        }
        if (node.component === "Button") {
            return node.components.button.label;
        }
        if (node.component === "Image") {
            return "Sprite " + String(node.components.image.spriteResourceKey);
        }
        if (node.component === "Slider") {
            return String(node.components.slider.value);
        }
        return node.name;
    }

    function GetComponentPreviewTint(node) {
        if (node.component === "Text") {
            return node.components.text.tint;
        }
        if (node.component === "Image") {
            return node.components.image.tint;
        }
        return "";
    }

    function BuildCanvasItems(document) {
        const normalized = NormalizeDocument(document);
        const resolved = ResolveDocumentRects(normalized);
        return normalized.nodes.filter(function KeepVisible(node) {
            return node.visible;
        }).sort(function CompareCanvas(left, right) {
            if (left.layer !== right.layer) {
                return left.layer - right.layer;
            }
            return left.siblingOrder - right.siblingOrder;
        }).map(function MapCanvas(node) {
            const rect_result = resolved.get(node.nodeId);
            const runtime_rect = rect_result ? rect_result.rect : CreateRect(0, 0, 1, 1);
            return {
                nodeId: node.nodeId,
                name: node.name,
                component: node.component,
                runtimeRect: Clone(runtime_rect),
                canvasRect: EngineRectToCanvasRect(runtime_rect, normalized.editor.viewport),
                selected: normalized.editor.selectedNodeId === node.nodeId,
                enabled: node.enabled,
                hitTestable: node.hitTestable,
                previewText: GetComponentPreviewText(node),
                previewTint: GetComponentPreviewTint(node)
            };
        });
    }

    function BuildInspector(document) {
        const normalized = NormalizeDocument(document);
        const selected_node = FindNode(normalized, normalized.editor.selectedNodeId);
        if (!selected_node) {
            return null;
        }
        return {
            node: Clone(selected_node),
            componentTypes: COMPONENT_TYPES.slice(),
            componentBacklog: Clone(COMPONENT_BACKLOG),
            parentOptions: GetValidParentOptions(normalized, selected_node.nodeId),
            styleRefs: normalized.styleRefs.filter(function MatchStyle(style_ref) {
                return ToNumber(style_ref.nodeId, 0) === selected_node.nodeId;
            }),
            resourceRefs: normalized.resourceRefs.filter(function MatchResource(resource_ref) {
                return ToNumber(resource_ref.nodeId, 0) === selected_node.nodeId;
            }),
            eventBindings: normalized.eventBindings.filter(function MatchBinding(binding) {
                return ToNumber(binding.nodeId, 0) === selected_node.nodeId;
            })
        };
    }

    function CanMoveNode(source, node_id, parent_id) {
        const document = NormalizeDocument(source);
        const node = FindNode(document, node_id);
        if (!node) {
            return false;
        }
        if (node.nodeId === document.schema.rootNodeId) {
            return false;
        }
        const parent = FindNode(document, parent_id);
        if (!parent) {
            return false;
        }
        if (node.nodeId === parent.nodeId) {
            return false;
        }
        if (IsDescendantNode(document, node.nodeId, parent.nodeId)) {
            return false;
        }
        return true;
    }

    function SetSubtreeLayer(document, node_id, layer) {
        const node = FindNode(document, node_id);
        if (!node) {
            return;
        }
        node.layer = layer;
        const children = GetChildNodes(document, node_id);
        children.forEach(function UpdateChildLayer(child) {
            SetSubtreeLayer(document, child.nodeId, layer + 1);
        });
    }

    function ClampSiblingIndex(value, max_value) {
        const index = Math.trunc(ToNumber(value, max_value));
        if (index < 0) {
            return 0;
        }
        if (index > max_value) {
            return max_value;
        }
        return index;
    }

    function MoveNode(source, node_id, parent_id, sibling_index) {
        const document = NormalizeDocument(source);
        if (!CanMoveNode(document, node_id, parent_id)) {
            return document;
        }

        const node = FindNode(document, node_id);
        const parent = FindNode(document, parent_id);
        const old_parent_id = node.parentId;
        const before_result = ResolveDocumentRects(document).get(node_id);
        node.parentId = parent.nodeId;
        NormalizeSiblingOrdersForParent(document, old_parent_id);

        const siblings = GetChildNodes(document, parent.nodeId).filter(function KeepOtherSibling(sibling) {
            return sibling.nodeId !== node.nodeId;
        });
        const next_index = ClampSiblingIndex(sibling_index, siblings.length);
        siblings.splice(next_index, 0, node);
        siblings.forEach(function AssignOrder(sibling, index) {
            sibling.siblingOrder = index;
        });

        const parent_result = ResolveDocumentRects(document).get(parent.nodeId);
        if (before_result && before_result.status === "Success" && parent_result && parent_result.status === "Success") {
            node.rectTransform = ApplyEngineRectToRectTransform(parent_result.rect, node.rectTransform, before_result.rect);
        }
        SetSubtreeLayer(document, node.nodeId, parent.layer + 1);
        document.editor.selectedNodeId = node.nodeId;
        document.editor.dirty = true;
        return document;
    }

    function GetValidParentOptions(source, node_id) {
        const document = NormalizeDocument(source);
        const node = FindNode(document, node_id);
        if (!node) {
            return [];
        }
        if (node.nodeId === document.schema.rootNodeId) {
            return [];
        }
        return BuildHierarchy(document).filter(function KeepParentOption(item) {
            if (item.nodeId === node.nodeId) {
                return false;
            }
            if (IsDescendantNode(document, node.nodeId, item.nodeId)) {
                return false;
            }
            return true;
        }).map(function MapParentOption(item) {
            return {
                nodeId: item.nodeId,
                name: item.name,
                depth: item.depth
            };
        });
    }

    function MergeRectTransform(current, patch) {
        const base = NormalizeRectTransform(current);
        const update = patch || {};
        return NormalizeRectTransform({
            anchorMin: Object.assign({}, base.anchorMin, update.anchorMin || {}),
            anchorMax: Object.assign({}, base.anchorMax, update.anchorMax || {}),
            pivot: Object.assign({}, base.pivot, update.pivot || {}),
            offsetMin: Object.assign({}, base.offsetMin, update.offsetMin || {}),
            offsetMax: Object.assign({}, base.offsetMax, update.offsetMax || {}),
            margin: Object.assign({}, base.margin, update.margin || {}),
            padding: Object.assign({}, base.padding, update.padding || {}),
            dpiScale: ToNumber(update.dpiScale, base.dpiScale)
        });
    }

    function UpdateNode(source, node_id, patch) {
        const document = NormalizeDocument(source);
        document.nodes = document.nodes.map(function MapNode(node) {
            if (node.nodeId !== node_id) {
                return node;
            }
            const next_node = Object.assign({}, node, patch);
            if (patch.rectTransform) {
                next_node.rectTransform = MergeRectTransform(node.rectTransform, patch.rectTransform);
            }
            return next_node;
        });
        document.editor.selectedNodeId = node_id;
        document.editor.dirty = true;
        return document;
    }

    function GetParentRect(document, node_id) {
        const node = FindNode(document, node_id);
        if (!node) {
            return document.editor.viewport.runtimeRect;
        }
        if (node.parentId === 0) {
            return document.editor.viewport.runtimeRect;
        }
        const resolved = ResolveDocumentRects(document);
        const parent_result = resolved.get(node.parentId);
        if (!parent_result) {
            return document.editor.viewport.runtimeRect;
        }
        return parent_result.rect;
    }

    function UpdateNodeFromCanvasRect(source, node_id, canvas_rect) {
        const document = NormalizeDocument(source);
        const node = FindNode(document, node_id);
        if (!node) {
            return document;
        }
        const engine_rect = CanvasRectToEngineRect(canvas_rect, document.editor.viewport);
        const parent_rect = GetParentRect(document, node_id);
        const rect_transform = ApplyEngineRectToRectTransform(parent_rect, node.rectTransform, engine_rect);
        return UpdateNode(document, node_id, { rectTransform: rect_transform });
    }

    function GetDefaultNodeSize(component) {
        if (component === "Image") {
            return { width: 160, height: 120 };
        }
        if (component === "Slider") {
            return { width: 240, height: 40 };
        }
        return { width: 180, height: 44 };
    }

    function GetDefaultHitTestable(component) {
        if (component === "Button") {
            return true;
        }
        if (component === "Slider") {
            return true;
        }
        return false;
    }

    function AddNode(source, component) {
        const document = NormalizeDocument(source);
        const component_type = NormalizeComponentType(component);
        const selected = FindNode(document, document.editor.selectedNodeId);
        const parent_id = selected ? selected.nodeId : document.schema.rootNodeId;
        const parent_rect = selected ? ResolveDocumentRects(document).get(selected.nodeId).rect : document.editor.viewport.runtimeRect;
        const node_id = GetNextNodeId(document);
        const default_size = GetDefaultNodeSize(component_type);
        const sibling_order = document.nodes.filter(function MatchParent(node) {
            return node.parentId === parent_id;
        }).length;
        const engine_rect = {
            x: parent_rect.x + 48 + sibling_order * 24,
            y: parent_rect.y + parent_rect.height - 108 - sibling_order * 52,
            width: default_size.width,
            height: default_size.height
        };
        const rect_transform = ApplyEngineRectToRectTransform(parent_rect, CreateFullStretchRectTransform(), engine_rect);
        const label = String(component_type || "Container") + node_id;
        const new_node = {
            nodeId: node_id,
            parentId: parent_id,
            name: label,
            component: component_type,
            components: CreateDefaultComponentRecords(component_type, label),
            rectTransform: rect_transform,
            siblingOrder: sibling_order,
            layer: selected ? selected.layer + 1 : 1,
            visible: true,
            enabled: true,
            hitTestable: GetDefaultHitTestable(component_type)
        };
        document.nodes.push(new_node);
        document.editor.selectedNodeId = node_id;
        document.editor.dirty = true;
        return document;
    }

    function RemoveNode(source, node_id) {
        const document = NormalizeDocument(source);
        if (node_id === document.schema.rootNodeId) {
            return document;
        }
        const removed_ids = new Set([node_id]);
        let changed = true;
        while (changed) {
            changed = false;
            document.nodes.forEach(function VisitNode(node) {
                if (removed_ids.has(node.parentId) && !removed_ids.has(node.nodeId)) {
                    removed_ids.add(node.nodeId);
                    changed = true;
                }
            });
        }
        document.nodes = document.nodes.filter(function KeepNode(node) {
            return !removed_ids.has(node.nodeId);
        });
        document.layouts = document.layouts.filter(function KeepLayout(layout) {
            return !removed_ids.has(ToNumber(layout.containerNodeId, 0));
        });
        document.styleRefs = document.styleRefs.filter(function KeepStyle(style_ref) {
            return !removed_ids.has(ToNumber(style_ref.nodeId, 0));
        });
        document.resourceRefs = document.resourceRefs.filter(function KeepResource(resource_ref) {
            return !removed_ids.has(ToNumber(resource_ref.nodeId, 0));
        });
        document.eventBindings = document.eventBindings.filter(function KeepBinding(binding) {
            return !removed_ids.has(ToNumber(binding.nodeId, 0));
        });
        document.editor.selectedNodeId = document.schema.rootNodeId;
        document.editor.dirty = true;
        return document;
    }

    function BuildRuntimeComponents(node) {
        const component_key = GetComponentRecordKey(node.component);
        const records = {
            common: Clone(node.components.common)
        };
        if (component_key && node.components[component_key]) {
            records[component_key] = Clone(node.components[component_key]);
        }
        return records;
    }

    function BuildRuntimeDocument(source) {
        const document = NormalizeDocument(source);
        const runtime_nodes = document.nodes.map(function MapRuntimeNode(node) {
            return {
                nodeId: node.nodeId,
                parentId: node.parentId,
                name: node.name,
                component: node.component,
                components: BuildRuntimeComponents(node),
                rectTransform: Clone(node.rectTransform),
                siblingOrder: node.siblingOrder,
                layer: node.layer,
                visible: node.visible,
                enabled: node.enabled,
                hitTestable: node.hitTestable
            };
        });
        const runtime_document = {
            schema: document.schema,
            nodes: runtime_nodes,
            layouts: document.layouts,
            styleRefs: document.styleRefs,
            resourceRefs: document.resourceRefs,
            eventBindings: document.eventBindings,
            theme: document.theme
        };
        return Clone(runtime_document);
    }

    function GetComponentMatrix() {
        return {
            implemented: COMPONENT_TYPES.map(function MapComponent(component) {
                return {
                    component: component,
                    status: "Implemented",
                    recordKey: GetComponentRecordKey(component)
                };
            }),
            records: COMPONENT_RECORD_KEYS.slice(),
            layoutTypes: LAYOUT_TYPES.slice(),
            backlog: Clone(COMPONENT_BACKLOG)
        };
    }

    function FormatJson(value) {
        return JSON.stringify(value, null, 4);
    }

    return {
        SCHEMA_ID: SCHEMA_ID,
        SCHEMA_VERSION: SCHEMA_VERSION,
        DEFAULT_VIEWPORT_RECT: DEFAULT_VIEWPORT_RECT,
        COMPONENT_TYPES: COMPONENT_TYPES.slice(),
        CreateDefaultDocument: CreateDefaultDocument,
        NormalizeDocument: NormalizeDocument,
        NormalizeRectTransform: NormalizeRectTransform,
        ResolveRectTransform: ResolveRectTransform,
        ResolveDocumentRects: ResolveDocumentRects,
        EngineRectToCanvasRect: EngineRectToCanvasRect,
        CanvasRectToEngineRect: CanvasRectToEngineRect,
        ApplyEngineRectToRectTransform: ApplyEngineRectToRectTransform,
        ValidateDocument: ValidateDocument,
        BuildHierarchy: BuildHierarchy,
        BuildCanvasItems: BuildCanvasItems,
        BuildInspector: BuildInspector,
        CanMoveNode: CanMoveNode,
        MoveNode: MoveNode,
        GetValidParentOptions: GetValidParentOptions,
        UpdateNode: UpdateNode,
        UpdateNodeFromCanvasRect: UpdateNodeFromCanvasRect,
        AddNode: AddNode,
        RemoveNode: RemoveNode,
        BuildRuntimeDocument: BuildRuntimeDocument,
        GetComponentMatrix: GetComponentMatrix,
        FormatJson: FormatJson
    };
});
