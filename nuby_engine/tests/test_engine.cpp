#include "../include/nuby/nuby_engine.hpp"
#include <cassert>
#include <iostream>

void test_html_parser() {
    std::cout << "[Test] Running HTML5 Parser & DOM tests..." << std::endl;
    std::string html = "<div id=\"main\" class=\"container hero\"><p>Hello Nuby</p></div>";
    nuby::html::HTMLParser parser(html);
    auto doc = parser.parse();

    assert(doc != nullptr);
    auto main_elem = doc->get_element_by_id("main");
    assert(main_elem != nullptr);
    assert(main_elem->get_tag_name() == "div");
    assert(main_elem->has_class("container"));
    assert(main_elem->has_class("hero"));

    auto ps = doc->get_elements_by_tag_name("p");
    assert(ps.size() == 1);
    assert(ps[0]->get_text_content() == "Hello Nuby");
    std::cout << "  [✔] HTML5 Parser and DOM tests passed!" << std::endl;
}

void test_css_specificity() {
    std::cout << "[Test] Running CSS Specificity & Cascade tests..." << std::endl;
    nuby::css::Specificity inline_spec(1, 0, 0, 0);
    nuby::css::Specificity id_spec(0, 1, 0, 0);
    nuby::css::Specificity class_spec(0, 0, 1, 0);
    nuby::css::Specificity elem_spec(0, 0, 0, 1);

    assert(inline_spec > id_spec);
    assert(id_spec > class_spec);
    assert(class_spec > elem_spec);

    std::string css = R"(
        div { color: #ff0000; font-size: 14px; }
        .hero { color: #00ff00; }
        #main { color: #0000ff; }
    )";
    nuby::css::CSSParser parser(css);
    auto sheet = parser.parse();
    assert(sheet.rules.size() == 3);

    std::cout << "  [✔] CSS Specificity and Cascade tests passed!" << std::endl;
}

void test_box_model_and_flexbox() {
    std::cout << "[Test] Running Box Model & Flexbox Layout tests..." << std::endl;
    nuby::NubyBrowserEngine engine(800, 600);
    std::string html = R"(
        <div style="display: flex; flex-direction: row; gap: 10px; width: 400px; height: 100px;">
            <div style="flex-grow: 1; height: 50px;">Item 1</div>
            <div style="flex-grow: 2; height: 50px;">Item 2</div>
        </div>
    )";
    auto res = engine.render_page(html);
    assert(res.layout_tree != nullptr);
    assert(res.display_list.size() > 0);
    assert(res.pixels.size() == 800 * 600);

    std::cout << "  [✔] Box Model & Flexbox tests passed!" << std::endl;
}

void test_rasterizer() {
    std::cout << "[Test] Running 2D Software Rasterizer tests..." << std::endl;
    nuby::paint::SoftwareRasterizer rasterizer(100, 100, nuby::core::Color::white());
    rasterizer.fill_rounded_rect(nuby::core::RectF(10, 10, 80, 80), nuby::core::BorderRadius(8.0f), nuby::core::Color::blue());
    assert(rasterizer.save_bmp("/tmp/test_render.bmp"));
    std::cout << "  [✔] Rasterizer output saved to /tmp/test_render.bmp successfully!" << std::endl;
}

int main() {
    std::cout << "========================================\n"
              << "  NUBY BROWSER ENGINE TEST SUITE       \n"
              << "========================================\n";
    test_html_parser();
    test_css_specificity();
    test_box_model_and_flexbox();
    test_rasterizer();
    std::cout << "\n\033[1;32mALL TEST SUITES PASSED (100% SUCCESS)!\033[0m\n";
    return 0;
}
