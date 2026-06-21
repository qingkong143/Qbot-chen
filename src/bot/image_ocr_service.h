#pragma once

#include <string>
#include <vector>

class ImageOcrService {
public:
    static ImageOcrService& get();

    std::vector<std::string> extractImageUrls(const std::string& raw_message, int max_images = 2) const;
    std::vector<std::string> extractImageFiles(const std::string& raw_message, int max_images = 2) const;
    std::string recognizeImages(const std::vector<std::string>& image_refs) const;

private:
    ImageOcrService() = default;

    std::string recognizeOne(const std::string& image_ref, int index) const;
    std::string resolveImageSource(const std::string& image_ref) const;
    std::string fetchImageUrlFromNapcat(const std::string& file_id) const;
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output);
};
