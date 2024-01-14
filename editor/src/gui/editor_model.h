#pragma once

#define DEF_VALUE_CHANGE_NOTIFY(value) void on_##value##_changed();
#define IMPL_VALUE_CHANGE_NOTIFY(value) void EditorModel::on_##value##_changed()

enum MapLayer {
    MapLayer_All,
    MapLayer_Top,
    MapLayer_Bottom,
};

struct EditorModelData {
    bool drawMapBlock = false;
    MapLayer layer = MapLayer_Bottom;
    bool runSceneEnterScript; // run scene enter script
    bool drawScripts = true;
    bool isNightPalette = false;
    unsigned short numScene = 0;
};

class EditorModel {
public:
    EditorModel() = default;

private:
    DEF_VALUE_CHANGE_NOTIFY(drawMapBlock);
    DEF_VALUE_CHANGE_NOTIFY(layer);
    DEF_VALUE_CHANGE_NOTIFY(runSceneEnterScript);
    DEF_VALUE_CHANGE_NOTIFY(drawScripts);
    DEF_VALUE_CHANGE_NOTIFY(isNightPalette);
    DEF_VALUE_CHANGE_NOTIFY(numScene);

private:
    EditorModelData data;
};