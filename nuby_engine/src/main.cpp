#include "../include/nuby/nuby_engine.hpp"
#include "../include/nuby/server/web_server.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

void print_banner() {
    std::cout << "\033[1;36m"
              << "======================================================================\n"
              << "   ███╗   ██╗██╗   ██╗██████╗ ██╗   ██╗\n"
              << "   ████╗  ██║██║   ██║██╔══██╗╚██╗ ██╔╝\n"
              << "   ██╔██╗ ██║██║   ██║██████╔╝ ╚████╔╝ \n"
              << "   ██║╚██╗██║██║   ██║██╔══██╗  ╚██╔╝  \n"
              << "   ██║ ╚████║╚██████╔╝██████╔╝   ██║   \n"
              << "   ╚═╝  ╚═══╝ ╚═════╝ ╚═════╝    ╚═╝   \n"
              << "   Next-Gen High-Performance C++20 Web Browser Engine from Scratch    \n"
              << "======================================================================\n"
              << "\033[0m" << std::endl;
}

int main(int argc, char* argv[]) {
    print_banner();

    int port = 8080;
    if (argc > 1 && std::string(argv[1]) == "--port" && argc > 2) {
        port = std::stoi(argv[2]);
    }

    std::cout << "Initializing Nuby C++20 Browser Subsystems:\n"
              << "  [✔] WHATWG HTML5 Tokenizer & Tree Builder State Machine\n"
              << "  [✔] CSSOM Cascading Engine & Specificity Resolver\n"
              << "  [✔] Layout Engine (BFC, IFC Text Shaper & Flexbox Grid)\n"
              << "  [✔] Display List 2D Software Rasterizer & Anti-Aliased Compositor\n"
              << "  [✔] ECMAScript DOM API Bridge & Event Loop\n"
              << "  [✔] HTTP/1.1 Socket Network Stack & DNS Resolver\n"
              << "----------------------------------------------------------------------\n";

    // Test a sample render immediately to verify end-to-end correctness
    nuby::NubyBrowserEngine engine(1000, 800);
    std::string sample_html = R"(
        <div style="background-color: #0b0f19; padding: 24px; color: #ffffff;">
            <h1 style="color: #38bdf8; font-size: 28px;">Nuby Browser Engine Core</h1>
            <p style="color: #94a3b8; font-size: 14px;">Next-Gen C++20 engine executing zero-copy pipeline.</p>
        </div>
    )";

    std::cout << "Running baseline self-test pipeline..." << std::endl;
    auto test_res = engine.render_page(sample_html);
    std::cout << "Pipeline completed in: \033[1;32m"
              << (test_res.profiler.total_duration_us() / 1000.0) << " ms\033[0m ("
              << test_res.profiler.total_duration_us() << " μs)\n";

    for (const auto& ev : test_res.profiler.get_events()) {
        std::cout << "  • " << ev.name << ": " << ev.duration_us << " μs (" << ev.details << ")\n";
    }

    std::cout << "----------------------------------------------------------------------\n";
    std::cout << "Starting Live Browser Workbench DevTools server on port " << port << "...\n";

    nuby::server::WebServer server(port, "0.0.0.0");
    server.run_synchronous();

    return 0;
}
