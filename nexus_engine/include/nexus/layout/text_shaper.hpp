#pragma once

#include "../core/types.hpp"
#include "../core/string_utils.hpp"
#include <string>
#include <vector>
#include <cmath>

namespace nexus::layout {

struct GlyphMetric {
    float advance_width{8.0f};
    float height{16.0f};
    float bearing_x{0.0f};
    float bearing_y{12.0f};
};

struct TextRun {
    std::string text;
    core::RectF rect;
    float font_size{16.0f};
    int font_weight{400};
    core::Color color{0, 0, 0, 255};
};

class TextShaper {
public:
    static float estimate_char_width(char c, float font_size, int font_weight) {
        float base_width = font_size * 0.55f;
        if (c == ' ' || c == '.' || c == ',' || c == ':' || c == ';' || c == 'i' || c == 'l' || c == '!') {
            base_width = font_size * 0.28f;
        } else if (c == 'm' || c == 'w' || c == 'M' || c == 'W' || c == '@' || c == '%') {
            base_width = font_size * 0.85f;
        } else if (std::isupper(static_cast<unsigned char>(c))) {
            base_width = font_size * 0.65f;
        }
        if (font_weight >= 700) {
            base_width *= 1.12f;
        }
        return base_width;
    }

    static float measure_text_width(const std::string& text, float font_size, int font_weight) {
        float width = 0.0f;
        for (char c : text) {
            width += estimate_char_width(c, font_size, font_weight);
        }
        return width;
    }

    static std::vector<TextRun> wrap_text(const std::string& text, float max_width, float font_size, int font_weight, const core::Color& color, float line_height) {
        std::vector<TextRun> runs;
        if (text.empty()) return runs;

        auto words = core::StringUtils::split_whitespace(text);
        if (words.empty()) return runs;

        float current_x = 0.0f;
        float current_y = 0.0f;
        std::string current_line;
        float space_width = estimate_char_width(' ', font_size, font_weight);

        for (size_t i = 0; i < words.size(); ++i) {
            const auto& word = words[i];
            float word_width = measure_text_width(word, font_size, font_weight);

            if (!current_line.empty() && (current_x + space_width + word_width) > max_width && max_width > 50.0f) {
                // Emit current line
                TextRun run;
                run.text = current_line;
                run.rect = core::RectF(0.0f, current_y, current_x, line_height);
                run.font_size = font_size;
                run.font_weight = font_weight;
                run.color = color;
                runs.push_back(run);

                current_line = word;
                current_x = word_width;
                current_y += line_height;
            } else {
                if (!current_line.empty()) {
                    current_line += " ";
                    current_x += space_width;
                }
                current_line += word;
                current_x += word_width;
            }
        }

        if (!current_line.empty()) {
            TextRun run;
            run.text = current_line;
            run.rect = core::RectF(0.0f, current_y, current_x, line_height);
            run.font_size = font_size;
            run.font_weight = font_weight;
            run.color = color;
            runs.push_back(run);
        }

        return runs;
    }
};

} // namespace nexus::layout
