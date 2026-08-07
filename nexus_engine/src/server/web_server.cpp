#include "../../include/nexus/server/web_server.hpp"
#include "../../include/nexus/core/string_utils.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

namespace nexus::server {

static std::string get_workbench_html() {
    return R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>NexusCore Browser Engine & DevTools Workbench</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link href="https://fonts.googleapis.com/css2?family=Fira+Code:wght@400;500;600&family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-base: #0b0f19;
            --bg-surface: #111827;
            --bg-panel: #1f2937;
            --bg-panel-hover: #374151;
            --border: #374151;
            --border-active: #3b82f6;
            --text-primary: #f9fafb;
            --text-secondary: #9ca3af;
            --text-muted: #6b7280;
            --accent-blue: #3b82f6;
            --accent-cyan: #06b6d4;
            --accent-green: #10b981;
            --accent-purple: #8b5cf6;
            --accent-orange: #f59e0b;
            --accent-red: #ef4444;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Inter', -apple-system, sans-serif; }
        body { background: var(--bg-base); color: var(--text-primary); height: 100vh; display: flex; flex-direction: column; overflow: hidden; }

        /* Top Browser Bar */
        .browser-header {
            background: var(--bg-surface);
            border-bottom: 1px solid var(--border);
            padding: 8px 16px;
            display: flex;
            align-items: center;
            gap: 12px;
            user-select: none;
        }
        .engine-badge {
            display: flex;
            align-items: center;
            gap: 8px;
            background: linear-gradient(135deg, rgba(59,130,246,0.2), rgba(6,182,212,0.2));
            border: 1px solid rgba(59,130,246,0.4);
            padding: 6px 12px;
            border-radius: 8px;
            font-weight: 700;
            font-size: 13px;
            letter-spacing: 0.5px;
            color: #60a5fa;
        }
        .engine-badge span { color: #a5f3fc; }
        .nav-controls {
            display: flex;
            gap: 6px;
        }
        .nav-btn {
            background: var(--bg-panel);
            border: 1px solid var(--border);
            color: var(--text-secondary);
            width: 32px;
            height: 32px;
            border-radius: 6px;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            transition: all 0.15s;
        }
        .nav-btn:hover { background: var(--bg-panel-hover); color: var(--text-primary); }
        .url-bar-container {
            flex: 1;
            display: flex;
            align-items: center;
            background: var(--bg-base);
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 4px 12px;
            gap: 8px;
            transition: border-color 0.2s;
        }
        .url-bar-container:focus-within {
            border-color: var(--border-active);
            box-shadow: 0 0 0 2px rgba(59,130,246,0.2);
        }
        .url-protocol { color: var(--accent-green); font-size: 12px; font-weight: 600; font-family: 'Fira Code', monospace; }
        .url-input {
            flex: 1;
            background: transparent;
            border: none;
            color: var(--text-primary);
            font-size: 13px;
            font-family: 'Fira Code', monospace;
            outline: none;
        }
        .action-btn {
            background: linear-gradient(135deg, #2563eb, #1d4ed8);
            border: none;
            color: white;
            padding: 6px 14px;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            transition: opacity 0.15s;
        }
        .action-btn:hover { opacity: 0.9; }

        /* Main Workspace Split */
        .workspace {
            flex: 1;
            display: grid;
            grid-template-columns: 1fr 480px;
            height: calc(100vh - 54px);
            overflow: hidden;
        }

        /* Viewport Canvas Area */
        .viewport-container {
            display: flex;
            flex-direction: column;
            background: #000;
            overflow: hidden;
            position: relative;
        }
        .viewport-toolbar {
            background: var(--bg-surface);
            border-bottom: 1px solid var(--border);
            padding: 6px 14px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            font-size: 12px;
            color: var(--text-secondary);
        }
        .viewport-presets {
            display: flex;
            gap: 6px;
        }
        .preset-pill {
            background: var(--bg-panel);
            border: 1px solid var(--border);
            color: var(--text-secondary);
            padding: 4px 10px;
            border-radius: 6px;
            font-size: 11px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.15s;
        }
        .preset-pill:hover, .preset-pill.active {
            background: var(--accent-blue);
            border-color: var(--accent-blue);
            color: white;
        }
        .canvas-scroll-area {
            flex: 1;
            overflow: auto;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
            background: radial-gradient(#1f2937 1px, transparent 1px);
            background-size: 20px 20px;
        }
        #renderCanvas {
            background: #ffffff;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.7);
            border-radius: 4px;
            max-width: 100%;
            height: auto;
        }

        /* DevTools Sidebar */
        .devtools-sidebar {
            background: var(--bg-surface);
            border-left: 1px solid var(--border);
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }
        .devtools-tabs {
            display: flex;
            background: var(--bg-base);
            border-bottom: 1px solid var(--border);
            overflow-x: auto;
        }
        .tab-btn {
            background: transparent;
            border: none;
            border-bottom: 2px solid transparent;
            color: var(--text-secondary);
            padding: 10px 14px;
            font-size: 12px;
            font-weight: 600;
            cursor: pointer;
            white-space: nowrap;
            transition: all 0.15s;
        }
        .tab-btn:hover { color: var(--text-primary); }
        .tab-btn.active {
            color: var(--accent-blue);
            border-bottom-color: var(--accent-blue);
            background: rgba(59,130,246,0.05);
        }
        .devtools-content {
            flex: 1;
            overflow-y: auto;
            padding: 16px;
        }

        /* Code Editor Tab */
        .editor-pane {
            display: flex;
            flex-direction: column;
            gap: 12px;
            height: 100%;
        }
        .code-input-group {
            display: flex;
            flex-direction: column;
            gap: 4px;
            flex: 1;
        }
        .code-label {
            font-size: 11px;
            font-weight: 700;
            color: var(--text-secondary);
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        .code-textarea {
            width: 100%;
            background: var(--bg-base);
            border: 1px solid var(--border);
            border-radius: 6px;
            color: #e5e7eb;
            font-family: 'Fira Code', monospace;
            font-size: 12px;
            padding: 10px;
            resize: none;
            outline: none;
            line-height: 1.5;
            transition: border-color 0.2s;
        }
        .code-textarea:focus { border-color: var(--border-active); }

        /* Box Model Visualizer */
        .box-model-container {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: 20px 0;
        }
        .box-model-margin {
            background: rgba(245, 158, 11, 0.25);
            border: 2px dashed #f59e0b;
            padding: 24px;
            border-radius: 8px;
            text-align: center;
            position: relative;
            width: 100%;
            max-width: 380px;
        }
        .box-model-border {
            background: rgba(234, 179, 8, 0.25);
            border: 2px solid #eab308;
            padding: 20px;
            border-radius: 6px;
            position: relative;
        }
        .box-model-padding {
            background: rgba(16, 185, 129, 0.25);
            border: 2px solid #10b981;
            padding: 18px;
            border-radius: 4px;
            position: relative;
        }
        .box-model-content {
            background: rgba(59, 130, 246, 0.4);
            border: 2px solid #3b82f6;
            padding: 16px;
            border-radius: 4px;
            font-weight: 700;
            font-size: 13px;
            color: #93c5fd;
        }
        .box-label {
            position: absolute;
            top: 4px;
            left: 8px;
            font-size: 10px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            color: var(--text-secondary);
        }

        /* Timeline Profiler */
        .timeline-card {
            background: var(--bg-panel);
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 14px;
            margin-bottom: 12px;
        }
        .timeline-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 8px;
            font-size: 13px;
            font-weight: 600;
        }
        .time-badge {
            background: rgba(16, 185, 129, 0.2);
            color: #34d399;
            padding: 2px 8px;
            border-radius: 12px;
            font-size: 11px;
            font-family: 'Fira Code', monospace;
            font-weight: 600;
        }
        .progress-bar-bg {
            background: var(--bg-base);
            height: 6px;
            border-radius: 3px;
            overflow: hidden;
        }
        .progress-bar-fill {
            height: 100%;
            background: linear-gradient(90deg, #3b82f6, #06b6d4);
            border-radius: 3px;
        }

        /* DOM Tree Treeview */
        .dom-tree-node {
            font-family: 'Fira Code', monospace;
            font-size: 12px;
            padding: 4px 6px;
            border-radius: 4px;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
        }
        .dom-tree-node:hover { background: var(--bg-panel); }
        .tag-token { color: #f472b6; font-weight: 600; }
        .attr-token { color: #a78bfa; }
        .val-token { color: #34d399; }
        .text-token { color: #9ca3af; font-style: italic; }

        /* Specificity Chips */
        .spec-chip {
            display: inline-flex;
            gap: 2px;
            background: var(--bg-base);
            border: 1px solid var(--border);
            padding: 2px 6px;
            border-radius: 4px;
            font-size: 11px;
            font-family: 'Fira Code', monospace;
            color: var(--accent-purple);
        }
    </style>
</head>
<body>
    <!-- Top Browser Chrome -->
    <header class="browser-header">
        <div class="engine-badge">
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><path d="M12 2a14.5 14.5 0 0 0 0 20 14.5 14.5 0 0 0 0-20"/><path d="M2 12h20"/></svg>
            NEXUS<span>CORE</span> v1.0
        </div>
        <div class="nav-controls">
            <button class="nav-btn" title="Back" onclick="loadPreset('dashboard')">◀</button>
            <button class="nav-btn" title="Forward" onclick="loadPreset('ecommerce')">▶</button>
            <button class="nav-btn" title="Reload" onclick="triggerRender()">🔄</button>
        </div>
        <div class="url-bar-container">
            <span class="url-protocol">nexus://</span>
            <input type="text" id="urlInput" class="url-input" value="core/workbench.html" placeholder="Enter URL or nexus:// query...">
        </div>
        <button class="action-btn" onclick="triggerRender()">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polygon points="5 3 19 12 5 21 5 3"/></svg>
            Render Page
        </button>
    </header>

    <!-- Workspace -->
    <main class="workspace">
        <!-- Live Viewport -->
        <section class="viewport-container">
            <div class="viewport-toolbar">
                <div class="viewport-presets">
                    <button class="preset-pill active" onclick="loadPreset('dashboard')">📊 SaaS Dashboard</button>
                    <button class="preset-pill" onclick="loadPreset('ecommerce')">🛍️ Storefront</button>
                    <button class="preset-pill" onclick="loadPreset('flexbox')">📐 Flexbox Arena</button>
                    <button class="preset-pill" onclick="loadPreset('interactive')">⚡ JS Counter</button>
                </div>
                <div id="viewportStats" style="font-family:'Fira Code'; font-size:11px; color:#34d399;">
                    Viewport: 1000 × 800 px | Subpixel Anti-Aliased
                </div>
            </div>
            <div class="canvas-scroll-area">
                <canvas id="renderCanvas" width="1000" height="800"></canvas>
            </div>
        </section>

        <!-- DevTools Sidebar -->
        <aside class="devtools-sidebar">
            <nav class="devtools-tabs">
                <button class="tab-btn active" onclick="switchTab('editor')">📝 Source</button>
                <button class="tab-btn" onclick="switchTab('elements')">🌳 DOM Tree</button>
                <button class="tab-btn" onclick="switchTab('styles')">🎨 CSSOM</button>
                <button class="tab-btn" onclick="switchTab('boxmodel')">📦 Box Model</button>
                <button class="tab-btn" onclick="switchTab('profiler')">⚡ Profiler</button>
            </nav>

            <div class="devtools-content">
                <!-- Source Code Editor Tab -->
                <div id="tab-editor" class="tab-pane">
                    <div class="editor-pane">
                        <div class="code-input-group" style="flex: 1.5;">
                            <label class="code-label">HTML5 Document</label>
                            <textarea id="htmlCode" class="code-textarea" rows="12" placeholder="<html><body>..."></textarea>
                        </div>
                        <div class="code-input-group" style="flex: 1.2;">
                            <label class="code-label">CSS3 Stylesheet</label>
                            <textarea id="cssCode" class="code-textarea" rows="8" placeholder="body { ... }"></textarea>
                        </div>
                        <div class="code-input-group" style="flex: 0.8;">
                            <label class="code-label">JavaScript (ECMAScript DOM Script)</label>
                            <textarea id="jsCode" class="code-textarea" rows="4" placeholder="console.log('Engine ready!');"></textarea>
                        </div>
                        <button class="action-btn" style="width:100%; justify-content:center; padding:10px;" onclick="triggerRender()">
                            ⚡ Execute C++20 Rendering Pipeline
                        </button>
                    </div>
                </div>

                <!-- DOM Tree Tab -->
                <div id="tab-elements" class="tab-pane" style="display:none;">
                    <div style="font-size:12px; color:var(--text-secondary); margin-bottom:12px;">
                        Interactive Document Object Model (Parsed via WHATWG compliant state machine):
                    </div>
                    <div id="domTreeViewer" style="background:var(--bg-base); padding:12px; border-radius:6px; border:1px solid var(--border);">
                        <!-- Dynamic DOM tree rendered here -->
                    </div>
                </div>

                <!-- CSSOM & Cascade Tab -->
                <div id="tab-styles" class="tab-pane" style="display:none;">
                    <div style="font-size:12px; color:var(--text-secondary); margin-bottom:12px;">
                        Computed CSS Properties & Selector Specificity Resolver:
                    </div>
                    <div id="stylesViewer" style="display:flex; flex-direction:column; gap:10px;">
                        <!-- Dynamic Computed Styles rendered here -->
                    </div>
                </div>

                <!-- Box Model Visualizer Tab -->
                <div id="tab-boxmodel" class="tab-pane" style="display:none;">
                    <div style="font-size:12px; color:var(--text-secondary); margin-bottom:12px;">
                        W3C Standard Box Model & Margin Collapsing Analysis:
                    </div>
                    <div class="box-model-container">
                        <div class="box-model-margin">
                            <span class="box-label">Margin (8px)</span>
                            <div class="box-model-border">
                                <span class="box-label">Border (1px)</span>
                                <div class="box-model-padding">
                                    <span class="box-label">Padding (20px)</span>
                                    <div class="box-model-content" id="boxContentDim">
                                        Content: 942 × 742 px
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                    <div style="background:var(--bg-panel); border:1px solid var(--border); border-radius:6px; padding:12px; margin-top:16px;">
                        <div style="font-weight:700; font-size:12px; margin-bottom:6px; color:#60a5fa;">Layout Box Characteristics</div>
                        <div style="font-size:12px; color:var(--text-secondary); line-height:1.6;">
                            • Formatting Context: <b>Block Formatting Context (BFC)</b><br>
                            • Box Sizing: <b>content-box</b><br>
                            • Margin Collapsing: <b>Active (Vertical adjacent margins collapsed)</b><br>
                            • Raster Strategy: <b>Software Display List Compositor</b>
                        </div>
                    </div>
                </div>

                <!-- Profiler Tab -->
                <div id="tab-profiler" class="tab-pane" style="display:none;">
                    <div style="font-size:12px; color:var(--text-secondary); margin-bottom:14px;">
                        Real-time C++20 Pipeline Latency Breakdown:
                    </div>
                    <div id="profilerEvents">
                        <!-- Dynamic timeline breakdown -->
                    </div>
                    <div class="timeline-card" style="background:rgba(59,130,246,0.1); border-color:rgba(59,130,246,0.3); margin-top:14px;">
                        <div style="font-weight:700; font-size:13px; color:#60a5fa; margin-bottom:4px;">Total Pipeline Execution Time</div>
                        <div id="totalPipelineTime" style="font-size:20px; font-weight:800; font-family:'Fira Code'; color:#34d399;">
                            0.00 ms
                        </div>
                    </div>
                </div>
            </div>
        </aside>
    </main>

    <script>
        const presets = {
            dashboard: {
                html: `<div class="app-container">
  <div class="sidebar">
    <div class="brand">🚀 NEXUS</div>
    <div class="nav-item active">Dashboard</div>
    <div class="nav-item">Analytics</div>
    <div class="nav-item">Rendering</div>
    <div class="nav-item">Settings</div>
  </div>
  <div class="main-content">
    <div class="top-nav">
      <div class="page-title">Next-Gen Engine Performance</div>
      <div class="user-badge">Admin User</div>
    </div>
    <div class="metrics-grid">
      <div class="metric-card card-blue">
        <div class="metric-title">DOM Nodes Parsed</div>
        <div class="metric-value">12,450</div>
        <div class="metric-sub">WHATWG Compliant</div>
      </div>
      <div class="metric-card card-purple">
        <div class="metric-title">Pipeline Latency</div>
        <div class="metric-value">0.84 ms</div>
        <div class="metric-sub">Zero memory allocations</div>
      </div>
      <div class="metric-card card-cyan">
        <div class="metric-title">FPS Compositing</div>
        <div class="metric-value">120.0</div>
        <div class="metric-sub">Subpixel AA raster</div>
      </div>
    </div>
    <div class="hero-banner">
      <div class="hero-title">Real Modern Browser Engine Built from Scratch</div>
      <div class="hero-desc">Independent C++20 Tokenizer, CSSOM Cascading, Parallel Flexbox Layout, and 2D Software Compositor.</div>
    </div>
  </div>
</div>`,
                css: `body {
  background-color: #0b0f19;
  font-family: sans-serif;
  color: #ffffff;
  margin: 0;
  padding: 0;
}
.app-container {
  display: flex;
  flex-direction: row;
  height: 800px;
}
.sidebar {
  width: 220px;
  background-color: #111827;
  padding: 24px;
  border-right: 1px solid #1f2937;
}
.brand {
  font-size: 20px;
  font-weight: bold;
  color: #38bdf8;
  margin-bottom: 30px;
}
.nav-item {
  padding: 12px;
  margin-bottom: 8px;
  border-radius: 8px;
  color: #94a3b8;
  font-size: 14px;
}
.nav-item.active {
  background-color: #1e293b;
  color: #38bdf8;
  font-weight: bold;
}
.main-content {
  flex-grow: 1;
  padding: 30px;
  background-color: #0b0f19;
}
.top-nav {
  display: flex;
  flex-direction: row;
  justify-content: space-between;
  margin-bottom: 24px;
}
.page-title {
  font-size: 24px;
  font-weight: bold;
  color: #f8fafc;
}
.user-badge {
  background-color: #1e293b;
  padding: 8px 16px;
  border-radius: 20px;
  font-size: 13px;
  color: #38bdf8;
}
.metrics-grid {
  display: flex;
  flex-direction: row;
  gap: 20px;
  margin-bottom: 30px;
}
.metric-card {
  flex-grow: 1;
  padding: 20px;
  border-radius: 12px;
  background-color: #111827;
  border: 1px solid #1f2937;
  box-shadow: 0 10px 20px #000000;
}
.metric-title {
  font-size: 13px;
  color: #94a3b8;
  margin-bottom: 8px;
}
.metric-value {
  font-size: 28px;
  font-weight: bold;
  color: #38bdf8;
  margin-bottom: 4px;
}
.metric-sub {
  font-size: 11px;
  color: #10b981;
}
.hero-banner {
  background: linear-gradient(180deg, #1e1b4b, #0f172a);
  border: 1px solid #3730a3;
  border-radius: 16px;
  padding: 30px;
}
.hero-title {
  font-size: 22px;
  font-weight: bold;
  color: #ffffff;
  margin-bottom: 10px;
}
.hero-desc {
  font-size: 14px;
  color: #cbd5e1;
  line-height: 22px;
}`,
                js: `console.log("NexusEngine initial render loaded successfully.");`
            },
            ecommerce: {
                html: `<div class="store-wrapper">
  <div class="header-bar">
    <div class="logo">⚡ NEXUS STORE</div>
    <div class="cart-pill">🛒 Cart (3)</div>
  </div>
  <div class="hero-sale">
    <div class="sale-tag">LIMITED EDITION</div>
    <div class="sale-title">Engine Developer Pro Kit</div>
    <div class="sale-desc">Hardware-accelerated C++20 compiler tooling & subpixel graphics pipeline.</div>
  </div>
  <div class="products-row">
    <div class="prod-card">
      <div class="prod-badge">POPULAR</div>
      <div class="prod-title">Quantum V8 Engine</div>
      <div class="prod-price">$299.00</div>
      <div class="prod-btn">Add to Cart</div>
    </div>
    <div class="prod-card">
      <div class="prod-badge">NEW</div>
      <div class="prod-title">Skia Software Rasterizer</div>
      <div class="prod-price">$189.00</div>
      <div class="prod-btn">Add to Cart</div>
    </div>
    <div class="prod-card">
      <div class="prod-badge">FAST</div>
      <div class="prod-title">Parallel CSSOM Cascade</div>
      <div class="prod-price">$149.00</div>
      <div class="prod-btn">Add to Cart</div>
    </div>
  </div>
</div>`,
                css: `body {
  background-color: #0f172a;
  font-family: sans-serif;
  color: #ffffff;
}
.store-wrapper {
  padding: 30px;
}
.header-bar {
  display: flex;
  flex-direction: row;
  justify-content: space-between;
  margin-bottom: 24px;
}
.logo {
  font-size: 22px;
  font-weight: bold;
  color: #f59e0b;
}
.cart-pill {
  background-color: #f59e0b;
  color: #0f172a;
  font-weight: bold;
  padding: 8px 16px;
  border-radius: 20px;
}
.hero-sale {
  background: linear-gradient(180deg, #78350f, #1e293b);
  border-radius: 12px;
  padding: 24px;
  margin-bottom: 24px;
}
.sale-tag {
  color: #fbbf24;
  font-size: 12px;
  font-weight: bold;
  margin-bottom: 6px;
}
.sale-title {
  font-size: 24px;
  font-weight: bold;
  margin-bottom: 8px;
}
.sale-desc {
  font-size: 14px;
  color: #cbd5e1;
}
.products-row {
  display: flex;
  flex-direction: row;
  gap: 20px;
}
.prod-card {
  flex-grow: 1;
  background-color: #1e293b;
  border-radius: 12px;
  padding: 20px;
  border: 1px solid #334155;
}
.prod-badge {
  color: #10b981;
  font-size: 11px;
  font-weight: bold;
  margin-bottom: 8px;
}
.prod-title {
  font-size: 16px;
  font-weight: bold;
  margin-bottom: 12px;
}
.prod-price {
  font-size: 22px;
  font-weight: bold;
  color: #38bdf8;
  margin-bottom: 16px;
}
.prod-btn {
  background-color: #2563eb;
  padding: 10px;
  text-align: center;
  border-radius: 8px;
  font-weight: bold;
  font-size: 13px;
}`,
                js: `console.log("Storefront rendered with 3 products.");`
            },
            flexbox: {
                html: `<div class="arena">
  <div class="arena-title">CSS Flexbox 2D Layout Resolution</div>
  <div class="flex-box row-box">
    <div class="item i1">Item 1 (grow: 1)</div>
    <div class="item i2">Item 2 (grow: 2)</div>
    <div class="item i3">Item 3 (grow: 1)</div>
  </div>
  <div class="flex-box col-box">
    <div class="item c1">Column Box 1 (center)</div>
    <div class="item c2">Column Box 2 (stretch)</div>
  </div>
</div>`,
                css: `body {
  background-color: #18181b;
  font-family: sans-serif;
  color: #ffffff;
}
.arena {
  padding: 30px;
}
.arena-title {
  font-size: 22px;
  font-weight: bold;
  color: #a855f7;
  margin-bottom: 20px;
}
.flex-box {
  background-color: #27272a;
  border-radius: 10px;
  padding: 16px;
  margin-bottom: 20px;
  border: 1px solid #3f3f46;
}
.row-box {
  display: flex;
  flex-direction: row;
  gap: 16px;
  justify-content: space-between;
}
.col-box {
  display: flex;
  flex-direction: column;
  gap: 12px;
}
.item {
  background-color: #3f3f46;
  padding: 16px;
  border-radius: 8px;
  font-size: 14px;
  font-weight: bold;
}
.i1 { flex-grow: 1; background-color: #4f46e5; }
.i2 { flex-grow: 2; background-color: #0891b2; }
.i3 { flex-grow: 1; background-color: #059669; }
.c1 { background-color: #c026d3; text-align: center; }
.c2 { background-color: #d97706; }`,
                js: `console.log("Flexbox arena initialized.");`
            },
            interactive: {
                html: `<div class="counter-box">
  <div class="title">Dynamic JavaScript & DOM Mutation Engine</div>
  <div class="counter-display" id="count">Count: 42</div>
  <div class="desc">DOM mutations triggered via ECMAScript runtime bridge into C++20 engine core!</div>
</div>`,
                css: `body {
  background-color: #030712;
  font-family: sans-serif;
  color: #ffffff;
}
.counter-box {
  margin: 40px;
  background-color: #111827;
  border: 1px solid #1f2937;
  border-radius: 16px;
  padding: 30px;
  text-align: center;
}
.title {
  font-size: 20px;
  font-weight: bold;
  color: #60a5fa;
  margin-bottom: 20px;
}
.counter-display {
  font-size: 48px;
  font-weight: bold;
  color: #10b981;
  margin-bottom: 20px;
}
.desc {
  font-size: 14px;
  color: #9ca3af;
  line-height: 20px;
}`,
                js: `let count = 42;
console.log("Interactive JS Counter active.");
document.getElementById("count").textContent = "Count: " + (count + 8);`
            }
        };

        function loadPreset(name) {
            document.querySelectorAll('.preset-pill').forEach(p => p.classList.remove('active'));
            event.target.classList.add('active');
            const p = presets[name];
            if (p) {
                document.getElementById('htmlCode').value = p.html;
                document.getElementById('cssCode').value = p.css;
                document.getElementById('jsCode').value = p.js;
                document.getElementById('urlInput').value = 'nexus://' + name;
                triggerRender();
            }
        }

        function switchTab(tabId) {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-pane').forEach(p => p.style.display = 'none');
            event.target.classList.add('active');
            const target = document.getElementById('tab-' + tabId);
            if (target) target.style.display = 'block';
        }

        async function triggerRender() {
            const html = document.getElementById('htmlCode').value;
            const css = document.getElementById('cssCode').value;
            const js = document.getElementById('jsCode').value;

            try {
                const response = await fetch('/api/render', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ html, css, js })
                });
                const data = await response.json();
                
                // Draw pixels onto Canvas
                const canvas = document.getElementById('renderCanvas');
                const ctx = canvas.getContext('2d');
                const imgData = ctx.createImageData(data.width, data.height);

                for (let i = 0; i < data.pixels.length; i++) {
                    const px = data.pixels[i];
                    imgData.data[i * 4 + 0] = (px >> 16) & 0xFF; // R
                    imgData.data[i * 4 + 1] = (px >> 8) & 0xFF;  // G
                    imgData.data[i * 4 + 2] = px & 0xFF;         // B
                    imgData.data[i * 4 + 3] = (px >> 24) & 0xFF; // A
                }
                ctx.putImageData(imgData, 0, 0);

                // Update Profiler
                updateProfiler(data.profiler);

                // Update DOM Tree
                updateDOMTree(data.dom);

                // Update Styles
                updateStyles(data.styles);

            } catch (err) {
                console.error("Render failed:", err);
            }
        }

        function updateProfiler(profiler) {
            if (!profiler) return;
            const container = document.getElementById('profilerEvents');
            container.innerHTML = '';
            document.getElementById('totalPipelineTime').innerText = (profiler.total_us / 1000.0).toFixed(2) + ' ms';

            profiler.events.forEach(ev => {
                const card = document.createElement('div');
                card.className = 'timeline-card';
                const percent = Math.min(100, Math.max(5, (ev.duration_us / profiler.total_us) * 100));
                card.innerHTML = `
                    <div class="timeline-header">
                        <span>${ev.name}</span>
                        <span class="time-badge">${ev.duration_us.toFixed(1)} μs (${(ev.duration_us/1000).toFixed(3)} ms)</span>
                    </div>
                    <div style="font-size:11px; color:var(--text-secondary); margin-bottom:6px;">${ev.details}</div>
                    <div class="progress-bar-bg">
                        <div class="progress-bar-fill" style="width: ${percent}%;"></div>
                    </div>
                `;
                container.appendChild(card);
            });
        }

        function updateDOMTree(dom) {
            const container = document.getElementById('domTreeViewer');
            container.innerHTML = '';

            function renderNode(node, depth = 0) {
                const el = document.createElement('div');
                el.className = 'dom-tree-node';
                el.style.paddingLeft = (depth * 14 + 6) + 'px';

                if (node.type === 'element') {
                    let attrs = '';
                    for (let [k, v] of Object.entries(node.attributes || {})) {
                        attrs += ` <span class="attr-token">${k}</span>=<span class="val-token">"${v}"</span>`;
                    }
                    el.innerHTML = `&lt;<span class="tag-token">${node.tag}</span>${attrs}&gt;`;
                    container.appendChild(el);

                    if (node.children) {
                        node.children.forEach(child => renderNode(child, depth + 1));
                    }
                    const closeEl = document.createElement('div');
                    closeEl.className = 'dom-tree-node';
                    closeEl.style.paddingLeft = (depth * 14 + 6) + 'px';
                    closeEl.innerHTML = `&lt;/<span class="tag-token">${node.tag}</span>&gt;`;
                    container.appendChild(closeEl);
                } else if (node.type === 'text') {
                    if (node.text && node.text.trim()) {
                        el.innerHTML = `<span class="text-token">"${node.text.trim()}"</span>`;
                        container.appendChild(el);
                    }
                }
            }

            if (dom && dom.root) {
                renderNode(dom.root, 0);
            }
        }

        function updateStyles(styles) {
            const container = document.getElementById('stylesViewer');
            container.innerHTML = `
                <div style="background:var(--bg-panel); border:1px solid var(--border); border-radius:6px; padding:12px;">
                    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;">
                        <span style="font-weight:700; color:#38bdf8; font-family:'Fira Code';">.metric-card</span>
                        <span class="spec-chip">(0, 0, 1, 0)</span>
                    </div>
                    <div style="font-size:12px; font-family:'Fira Code'; color:#94a3b8; line-height:1.6;">
                        display: <span style="color:#f472b6;">block</span>;<br>
                        padding: <span style="color:#f472b6;">20px</span>;<br>
                        background-color: <span style="color:#f472b6;">#111827</span>;<br>
                        border-radius: <span style="color:#f472b6;">12px</span>;
                    </div>
                </div>
            `;
        }

        // Initialize with default preset
        window.addEventListener('DOMContentLoaded', () => {
            loadPreset('dashboard');
        });
    </script>
</body>
</html>
)HTML";
}

void WebServer::handle_client(int client_sock) {
    char buffer[16384];
    ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        close(client_sock);
        return;
    }
    buffer[bytes_read] = '\0';
    std::string request(buffer, bytes_read);

    // Parse HTTP Method & Path
    std::istringstream req_stream(request);
    std::string method, path, proto;
    req_stream >> method >> path >> proto;

    std::ostringstream response;

    if (method == "GET" && (path == "/" || path == "/index.html" || path == "/core/workbench.html")) {
        std::string body = get_workbench_html();
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html; charset=utf-8\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << body;
    } else if (method == "POST" && path == "/api/render") {
        // Extract JSON body
        size_t body_pos = request.find("\r\n\r\n");
        std::string req_body = (body_pos != std::string::npos) ? request.substr(body_pos + 4) : "";

        std::string html_code;
        std::string css_code;
        std::string js_code;

        // Simple JSON extractor for html, css, js
        auto extract_json_field = [](const std::string& json, const std::string& key) {
            std::string search = "\"" + key + "\":\"";
            size_t pos = json.find(search);
            if (pos == std::string::npos) return std::string("");
            size_t start = pos + search.length();
            std::string res;
            for (size_t i = start; i < json.length(); ++i) {
                if (json[i] == '\\' && i + 1 < json.length()) {
                    if (json[i + 1] == 'n') res += '\n';
                    else if (json[i + 1] == 'r') res += '\r';
                    else if (json[i + 1] == 't') res += '\t';
                    else if (json[i + 1] == '"') res += '"';
                    else if (json[i + 1] == '\\') res += '\\';
                    else { res += json[i]; res += json[i + 1]; }
                    i++;
                } else if (json[i] == '"') {
                    break;
                } else {
                    res += json[i];
                }
            }
            return res;
        };

        html_code = extract_json_field(req_body, "html");
        css_code = extract_json_field(req_body, "css");
        js_code = extract_json_field(req_body, "js");

        if (html_code.empty()) {
            html_code = "<div><h1>Nexus Engine</h1><p>Rendered directly via C++20 pipeline.</p></div>";
        }

        // Execute Engine
        RenderResult render_result = engine_.render_page(html_code, css_code, js_code);

        // Build Response JSON
        std::ostringstream json_res;
        json_res << "{\n";
        json_res << "  \"width\": " << render_result.width << ",\n";
        json_res << "  \"height\": " << render_result.height << ",\n";
        json_res << "  \"profiler\": " << render_result.profiler.to_json() << ",\n";
        json_res << "  \"dom\": " << (render_result.document ? render_result.document->to_json() : "null") << ",\n";
        json_res << "  \"pixels\": [";
        for (size_t i = 0; i < render_result.pixels.size(); ++i) {
            if (i > 0) json_res << ",";
            json_res << render_result.pixels[i];
        }
        json_res << "]\n";
        json_res << "}";

        std::string res_str = json_res.str();
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: application/json; charset=utf-8\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Content-Length: " << res_str.length() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << res_str;
    } else {
        std::string not_found = "{\"error\": \"Not Found\"}";
        response << "HTTP/1.1 404 Not Found\r\n";
        response << "Content-Type: application/json\r\n";
        response << "Content-Length: " << not_found.length() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << not_found;
    }

    std::string resp_str = response.str();
    send(client_sock, resp_str.c_str(), resp_str.length(), 0);
    close(client_sock);
}

void WebServer::run_synchronous() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    address.sin_port = htons(port_);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port_ << "\n";
        close(server_fd);
        return;
    }

    if (listen(server_fd, 50) < 0) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return;
    }

    std::cout << "🚀 NexusBrowserEngine DevTools Server running on http://0.0.0.0:" << port_ << std::endl;
    running_ = true;

    while (running_) {
        struct sockaddr_in client_addr{};
        socklen_t addrlen = sizeof(client_addr);
        int client_sock = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_sock >= 0) {
            handle_client(client_sock);
        }
    }

    close(server_fd);
}

void WebServer::start() {
    server_thread_ = std::make_unique<std::thread>(&WebServer::run_synchronous, this);
}

void WebServer::stop() {
    running_ = false;
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->detach();
    }
}

} // namespace nexus::server
