#pragma once
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

namespace editor {
/**
 * @brief editor config class
 *
 */
class EditorMeta {
private:
    EditorMeta() = default;

public:
    ~EditorMeta()
    {
        if (_config_buf) {
            delete[] _config_buf;
            _config_buf = nullptr;
        }
    }
    bool load();
    static void destroy();

    static EditorMeta& get()
    {
        if (!_config) {
            _config = new EditorMeta();
        }
        return *_config;
    }

    const char* getMapName(int index);
    void setMapName(int index, const char* name);

    /**
     * @brief sync and save
     *
     */
    bool sync();

private:
    rapidjson::Document _doc;
    char* _config_buf = nullptr;
    static EditorMeta* _config;
};
}