#include "gx_metadata.h"
#include <taglib/tag.h>
#include <taglib/wavfile.h>
#include <taglib/flacfile.h>
#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>

namespace gx_engine {

MetadataExtractor::MetadataExtractor(ParamMap& params)
    : paramMap(params)
{}

std::string MetadataExtractor::getBankName() const {
    std::string bank_name = "unknown";
    for (auto& entry : paramMap) {
        if (entry.first == "system.current_bank" && entry.second->isString()) {
            bank_name = entry.second->getString().get_value();
        }
    }
    return bank_name;
}

std::string MetadataExtractor::getPreset() const {
    std::string preset = "unknown";
    for (auto& entry : paramMap) {
        if (entry.first == "system.current_preset" && entry.second->isString()) {
            preset = entry.second->getString().get_value();
        }
    }
    return preset;
}
void MetadataExtractor::writeMetadataFile(const std::string& audioFilePath) {
    std::string basePath = audioFilePath.substr(0, audioFilePath.find_last_of('.'));
    std::ofstream outFile(basePath + ".txt");

    if (!outFile.is_open()) {
        std::cerr << "Unable to open metadata file: " << basePath << ".txt\n";
        return;
    }
    for (auto& kv : paramMap) {
        std::string value;
        if (kv.second->isFloat()) {
            value = std::to_string(kv.second->getFloat().get_value());
        } else if (kv.second->isInt()) {
            value = std::to_string(kv.second->getInt().get_value());
        } else if (kv.second->isBool()) {
            value = std::to_string(kv.second->getBool().get_value());
        } else if (kv.second->isFile()) {
            value = kv.second->getFile().get_display_name();
        } else if (kv.second->isString()) {
            value = kv.second->getString().get_value();
        }
        outFile << kv.first << " : " << value << "\n";
    }

    outFile.close();
    std::cout << "Metadata written to file successfully: " << basePath << ".txt\n";
}
void MetadataExtractor::writeCommentTag(const std::string& audioFilePath) {
    std::string bank = getBankName();
    std::string preset = getPreset();
    std::string strippedFile = std::filesystem::path(audioFilePath).stem().string();
    std::string software = "Guitarix";
    std::string artist = "";
    std::string album = bank + ":" + preset;
    std::string genre = bank + ":" + preset;
    std::string comment = bank + ":" + preset;
    // Date
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    char datebuf[20];
    std::strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", now);
    std::string date(datebuf);

    try {
        std::string ext = std::filesystem::path(audioFilePath).extension().string(); // includes the dot
        if (!ext.empty() && ext[0] == '.') ext.erase(0, 1); // remove leading dot
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
        if (ext == "wav" || ext == "WAV") {
            TagLib::RIFF::WAV::File file(audioFilePath.c_str());
            if (file.isValid()) {
                TagLib::Tag* tag = file.tag();
                tag->setTitle(strippedFile);
                tag->setArtist(artist);
                tag->setAlbum(album);
                tag->setGenre(genre);
                tag->setComment(comment);
                tag->setYear(std::stoi(date.substr(0,4)));
                file.save();
            }
        } else if (ext == "w64" || ext == "W64") {
           //sndfile and taglib both don't work it's probably better to write tags externally
        } else if (ext == "flac" || ext == "FLAC") {
            TagLib::FLAC::File file(audioFilePath.c_str());
            if (file.isValid()) {
                TagLib::Tag* tag = file.tag();
                tag->setTitle(strippedFile);
                tag->setArtist(artist);
                tag->setAlbum(album);
                tag->setGenre(genre);
                tag->setComment(comment);
                TagLib::Ogg::XiphComment* xiph = nullptr;
                xiph = file.xiphComment();
                if (xiph) { 
                    xiph->addField("SOFTWARE", software);
                    xiph->addField("BANK", bank);
                    xiph->addField("PRESET", preset);
                    xiph->addField("DATE", date, true);
                }
                file.save();
            }
        } else if (ext == "ogg" || ext == "OGG") {
            TagLib::Ogg::Vorbis::File file(audioFilePath.c_str());
            if (file.isValid()) {
                TagLib::Tag* tag = file.tag();
                tag->setTitle(strippedFile);
                tag->setArtist(artist);
                tag->setAlbum(album);
                tag->setGenre(genre);
                tag->setComment(comment);
                TagLib::Ogg::XiphComment* xiph = nullptr;
                xiph = file.tag();
                if (xiph) { 
                    xiph->addField("SOFTWARE", software);
                    xiph->addField("BANK", bank);
                    xiph->addField("PRESET", preset);
                    xiph->addField("DATE", date, true);
                }
                file.save();
            }
        } else {
            std::cerr << "Unsupported audio format for tagging: " << ext << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error writing comment tag: " << e.what() << "\n";
    }
}
} // namespace gx_engine
