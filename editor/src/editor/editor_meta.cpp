#include "editor_meta.h"
#include "engine/pal_log.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <fstream>
#include <ios>

namespace editor {

EditorMeta* EditorMeta::_config = nullptr;

bool EditorMeta::load()
{
    // load configs
    std::ifstream ifs;
    ifs.open("./resources/editor_meta.json");
    if (!ifs.is_open()) {
        engine::PalLog::get().addLog(engine::LogLevel::info, "config file not exist !");
        return true;
    }
    ifs.seekg(0, std::ios::end);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    _config_buf = new char[size + 1];
    ifs.read(_config_buf, size);
    ifs.close();
    _config_buf[size] = '\0';
    _doc.ParseInsitu(_config_buf);
    if (_doc.HasParseError()) {
        engine::PalLog::get().addLog(engine::LogLevel::error, "parse config failed: %d %s", _doc.GetParseError(), rapidjson::GetParseError_En(_doc.GetParseError()));
    }

    return true;
}

void EditorMeta::destroy()
{
    if (_config) {
        delete _config;
        _config = nullptr;
    }
}

const char* EditorMeta::getMapName(int index)
{
    char buf[128];
    sprintf(buf, "/maps/%d/name", index);
    const auto& v = rapidjson::Pointer(buf, strlen(buf)).GetWithDefault(_doc, "");
    return v.IsString() ? v.GetString() : "";
}

void EditorMeta::setMapName(int index, const char* name)
{
    char buf[128];
    sprintf(buf, "/maps/%d/name", index);
    rapidjson::Pointer(buf, strlen(buf)).Set(_doc, name);
}

bool EditorMeta::sync()
{
    std::ofstream ofs;
    ofs.open("./resources/editor_meta.json", std::ios::trunc);
    if (!ofs.is_open()) {
        engine::PalLog::get().addLog(engine::LogLevel::error, "load editor_meta.json failed !");
        return false;
    }
    // open file
    rapidjson::StringBuffer buf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
    _doc.Accept(writer);
    const char* str = buf.GetString();
    ofs.write(str, strlen(str));
    ofs.close();
    return true;
}

}