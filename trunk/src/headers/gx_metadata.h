#ifndef GX_METADATA_H
#define GX_METADATA_H

#include <string>
#include "engine.h"

namespace gx_engine {

class MetadataExtractor {
public:
    explicit MetadataExtractor(ParamMap& params);
    std::string getBankName() const;
    std::string getPreset() const;
    void writeMetadataFile(const std::string& audioFilePath);
    void writeCommentTag(const std::string& audioFilePath);
private:
    ParamMap& paramMap;
};

} // namespace gx_engine

#endif
