#include "Config.h"

void yamlSerialize(Config& cfg, std::string fileName) {
    YAML::Emitter out;

    out << YAML::BeginMap;

    out << YAML::Key << "Aircraft amount factor" << YAML::Value << cfg.n_airports;
    out << YAML::Key << "Towers amount factor" << YAML::Value << cfg.n_towers;
    out << YAML::Key << "Airports amount factor" << YAML::Value << cfg.n_aircrafts;
    out << YAML::Key << "Map width" << YAML::Value << cfg.map_width;
    out << YAML::Key << "Map height" << YAML::Value << cfg.map_height;
    out << YAML::Key << "Map scale" << YAML::Value << cfg.scale;
    out << YAML::Key << "Map CPU generation enabled" << YAML::Value << cfg.CPU;
    out << YAML::Key << "Map persistence" << YAML::Value << cfg.persistence;
    out << YAML::Key << "Map frequency" << YAML::Value << cfg.frequency;
    out << YAML::Key << "Map amplitude" << YAML::Value << cfg.amplitude;
    out << YAML::Key << "Map octaves" << YAML::Value << cfg.octaves;
    out << YAML::Key << "Map randomseed" << YAML::Value << cfg.randomseed;
    out << YAML::Key << "Aircrafts desired minimum NPM" << YAML::Value << cfg.aircraftsMinNPM;
    out << YAML::Key << "Aircrafts desired maximum NPM" << YAML::Value << cfg.aircraftsMaxNPM;
    out << YAML::Key << "Static Objects desired minimum NPM" << YAML::Value << cfg.staticObjectsMinNPM;
    out << YAML::Key << "Static Objects desired maximum NPM" << YAML::Value << cfg.staticObjectsMaxNPM;
    out << YAML::Key << "Aircrafts collision detection distance" << YAML::Value << cfg.collsiionDetectionDistance;
    out << YAML::Key << "Aircrafts height collision response" << YAML::Value << cfg.collisionResponseHeight;
    out << YAML::Key << "Airports minimum distance" << YAML::Value << cfg.airportsCollisionDistance;
    out << YAML::Key << "Towers minimum distance" << YAML::Value << cfg.towersCollisionDistance;

    out << YAML::EndSeq;

    std::ofstream fout(fileName);
    fout << out.c_str();
}

void yamlDeserialize(Config& cfg, std::string path) {
    std::ifstream stream(path);
    std::stringstream strStream;
    strStream << stream.rdbuf();
    YAML::Node config = YAML::Load(strStream);

    cfg.n_airports = config["Aircraft amount factor"].as<float>();
    cfg.n_towers = config["Towers amount factor"].as<float>();
    cfg.n_aircrafts = config["Airports amount factor"].as<float>();
    cfg.map_width = config["Map width"].as<float>();
    cfg.map_height = config["Map height"].as<float>();
    cfg.scale = config["Map scale"].as<float>();
    cfg.CPU = config["Map CPU generation enabled"].as<bool>();
    cfg.persistence = config["Map persistence"].as<double>();
    cfg.frequency = config["Map frequency"].as<double>();
    cfg.amplitude = config["Map amplitude"].as<double>();
    cfg.octaves = config["Map octaves"].as<uint8_t>();
    cfg.randomseed = config["Map randomseed"].as<uint32_t>();
    cfg.aircraftsMinNPM = config["Aircrafts desired minimum NPM"].as<float>();
    cfg.aircraftsMaxNPM = config["Aircrafts desired maximum NPM"].as<float>();
    cfg.staticObjectsMinNPM = config["Static Objects desired minimum NPM"].as<float>();
    cfg.staticObjectsMaxNPM = config["Static Objects desired maximum NPM"].as<float>();
    cfg.collsiionDetectionDistance = config["Aircrafts collision detection distance"].as<float>();
    cfg.collisionResponseHeight = config["Aircrafts height collision response"].as<float>();
    cfg.airportsCollisionDistance = config["Airports minimum distance"].as<float>();
    cfg.towersCollisionDistance = config["Towers minimum distance"].as<float>();

}

bool fileExist(std::string name, std::string ext) {
    std::string fileName = name + ext;
    const std::filesystem::path directory = std::filesystem::current_path();
    if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().filename().string() == fileName) {
                return true;
            }
        }
    }
    return false;
}

void loadConfig(Config& cfg, std::string fileName) {
    std::string path = std::filesystem::current_path().string();
    path += "\\" + fileName;
    yamlDeserialize(cfg, path);
}

void saveConfig(Config& cfg, std::string fileName) {
    
    yamlSerialize(cfg, fileName);
}