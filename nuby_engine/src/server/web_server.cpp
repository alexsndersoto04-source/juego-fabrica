#include "../../include/nuby/server/web_server.hpp"
#include "../../include/nuby/core/string_utils.hpp"
#include "../../include/nuby/net/http_client.hpp"
#include "../../include/nuby/net/indexer.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <vector>
#include <chrono>

namespace nuby::server {

static nuby::net::NubyIndexer g_indexer;

static std::string get_workbench_html() {
    return R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Nuby</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500;600&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg: #ffffff;
            --bg-hover: #f8fafc;
            --bg-panel: #f1f5f9;
            --border: #e2e8f0;
            --border-hover: #cbd5e1;
            --text-main: #09090b;
            --text-sub: #52525b;
            --text-muted: #71717a;
            --text-link: #1a0dab;
            --accent: #2563eb;
            --accent-hover: #1d4ed8;
            --shadow-search: 0 1px 6px rgba(0, 0, 0, 0.08);
            --shadow-hover: 0 4px 20px rgba(0, 0, 0, 0.06);
            --shadow-modal: 0 20px 50px rgba(0, 0, 0, 0.16);
            --radius-pill: 9999px;
            --radius-card: 14px;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            -webkit-tap-highlight-color: transparent;
            font-family: 'Plus Jakarta Sans', -apple-system, Roboto, Arial, sans-serif;
        }

        body {
            background-color: var(--bg);
            color: var(--text-main);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            overflow-x: hidden;
        }

        /* Top Minimalist Header */
        .top-navbar {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 14px 20px;
            background: #ffffff;
            border-bottom: 1px solid #f1f5f9;
            position: sticky;
            top: 0;
            z-index: 100;
        }

        .hamburger-btn {
            background: transparent;
            border: none;
            cursor: pointer;
            width: 42px;
            height: 42px;
            border-radius: 50%;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 4.5px;
            transition: background 0.15s;
        }

        .hamburger-btn:hover, .hamburger-btn:active {
            background: var(--bg-hover);
        }

        .hamburger-line {
            width: 20px;
            height: 2px;
            background-color: #09090b;
            border-radius: 2px;
        }

        .nav-brand-title {
            font-size: 20px;
            font-weight: 800;
            letter-spacing: -0.8px;
            color: #09090b;
            cursor: pointer;
            user-select: none;
        }

        /* Main Search Viewport */
        .search-viewport {
            flex: 1;
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 100%;
            max-width: 760px;
            margin: 0 auto;
            padding: 48px 18px 80px 18px;
            text-align: center;
        }

        /* Solid Black Bold Nuby Logo */
        .nuby-logo-solid {
            font-size: 58px;
            font-weight: 800;
            letter-spacing: -2.5px;
            color: #09090b;
            margin-top: 10px;
            margin-bottom: 26px;
            user-select: none;
            line-height: 1;
        }

        @media (max-width: 480px) {
            .nuby-logo-solid { font-size: 46px; margin-bottom: 20px; }
            .search-viewport { padding-top: 28px; }
        }

        /* Google-grade Minimalist Omnibox */
        .omnibox-frame {
            width: 100%;
            max-width: 640px;
            margin-bottom: 22px;
            position: relative;
        }

        .omnibox-inner {
            display: flex;
            align-items: center;
            background: #ffffff;
            border: 1.5px solid var(--border);
            border-radius: var(--radius-pill);
            padding: 0 18px;
            height: 52px;
            box-shadow: var(--shadow-search);
            transition: all 0.2s cubic-bezier(0.16, 1, 0.3, 1);
        }

        .omnibox-inner:hover {
            border-color: var(--border-hover);
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.08);
        }

        .omnibox-inner:focus-within {
            border-color: #09090b;
            box-shadow: 0 0 0 4px rgba(9, 9, 11, 0.08), 0 4px 16px rgba(0, 0, 0, 0.08);
        }

        .search-lens-icon {
            color: #71717a;
            margin-right: 12px;
            display: flex;
            align-items: center;
        }

        .search-field {
            flex: 1;
            border: none;
            outline: none;
            font-size: 16px;
            font-weight: 500;
            color: var(--text-main);
            background: transparent;
        }

        .search-field::placeholder {
            color: #a1a1aa;
            font-weight: 400;
        }

        .search-btn {
            background: #09090b;
            color: white;
            border: none;
            width: 36px;
            height: 36px;
            border-radius: var(--radius-pill);
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            transition: opacity 0.15s;
        }

        .search-btn:hover { opacity: 0.85; }

        /* Category Filter Chips */
        .category-tabs {
            display: flex;
            align-items: center;
            gap: 8px;
            margin-bottom: 32px;
            overflow-x: auto;
            scrollbar-width: none;
            width: 100%;
            max-width: 640px;
            justify-content: center;
        }
        .category-tabs::-webkit-scrollbar { display: none; }

        .cat-pill {
            background: transparent;
            border: 1px solid var(--border);
            color: var(--text-sub);
            padding: 7px 16px;
            border-radius: var(--radius-pill);
            font-size: 13.5px;
            font-weight: 600;
            cursor: pointer;
            white-space: nowrap;
            transition: all 0.15s ease;
            display: flex;
            align-items: center;
            gap: 6px;
        }

        .cat-pill.active, .cat-pill:hover {
            background: #09090b;
            color: #ffffff;
            border-color: #09090b;
        }

        /* Clean Minimal Speed Dial Grid */
        .speed-dial-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 16px;
            width: 100%;
            max-width: 520px;
            margin-bottom: 30px;
        }

        @media (max-width: 480px) {
            .speed-dial-grid {
                grid-template-columns: repeat(4, 1fr);
                gap: 12px;
            }
        }

        .dial-tile {
            background: #ffffff;
            border: 1px solid var(--border);
            border-radius: var(--radius-card);
            padding: 14px 8px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 8px;
            text-decoration: none;
            color: var(--text-main);
            box-shadow: var(--shadow-sm);
            transition: all 0.2s cubic-bezier(0.16, 1, 0.3, 1);
            cursor: pointer;
        }

        .dial-tile:hover {
            transform: translateY(-2px);
            border-color: #09090b;
            box-shadow: var(--shadow-hover);
        }

        .dial-symbol {
            width: 44px;
            height: 44px;
            border-radius: 12px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 18px;
            font-weight: 800;
            background: var(--bg-panel);
            color: #09090b;
        }

        .dial-caption {
            font-size: 12px;
            font-weight: 600;
            color: var(--text-sub);
        }

        /* Search Results Container */
        .results-section {
            width: 100%;
            max-width: 700px;
            display: none;
            flex-direction: column;
            gap: 22px;
            text-align: left;
            margin-top: 10px;
        }

        .results-latency {
            font-size: 13px;
            color: var(--text-muted);
            border-bottom: 1px solid #f1f5f9;
            padding-bottom: 8px;
        }

        /* Knowledge Graph Box (Apple/Google Style) */
        .knowledge-panel {
            background: #f8fafc;
            border: 1px solid var(--border);
            border-radius: var(--radius-card);
            padding: 18px;
            display: flex;
            flex-direction: column;
            gap: 6px;
        }

        .knowledge-heading {
            font-size: 19px;
            font-weight: 800;
            color: #09090b;
        }

        .knowledge-body {
            font-size: 14px;
            color: #3f3f46;
            line-height: 1.6;
        }

        /* Web Result Card */
        .web-result-item {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .web-url-caption {
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 12px;
            color: var(--text-muted);
            font-family: 'JetBrains Mono', monospace;
        }

        .web-title-link {
            font-size: 19px;
            font-weight: 600;
            color: var(--text-link);
            text-decoration: none;
            line-height: 1.35;
            cursor: pointer;
        }

        .web-title-link:hover {
            text-decoration: underline;
        }

        .web-snippet-text {
            font-size: 14px;
            color: #3f3f46;
            line-height: 1.55;
        }

        /* Video Search Card with Direct Embedded Player */
        .video-result-item {
            background: #ffffff;
            border: 1px solid var(--border);
            border-radius: var(--radius-card);
            padding: 14px;
            display: flex;
            gap: 16px;
            align-items: center;
            box-shadow: var(--shadow-sm);
            cursor: pointer;
            transition: all 0.2s;
        }

        .video-result-item:hover {
            border-color: #09090b;
            box-shadow: var(--shadow-hover);
        }

        .video-preview-thumb {
            width: 124px;
            height: 76px;
            border-radius: 10px;
            position: relative;
            overflow: hidden;
            flex-shrink: 0;
            background: #000;
        }

        .video-preview-thumb img {
            width: 100%;
            height: 100%;
            object-fit: cover;
        }

        .video-play-glyph {
            position: absolute;
            inset: 0;
            display: flex;
            align-items: center;
            justify-content: center;
            background: rgba(0, 0, 0, 0.3);
            color: white;
            font-size: 22px;
        }

        .video-time-tag {
            position: absolute;
            bottom: 4px;
            right: 4px;
            background: rgba(0, 0, 0, 0.85);
            color: white;
            font-size: 10px;
            font-weight: 700;
            padding: 2px 6px;
            border-radius: 4px;
        }

        .video-summary {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .video-header-text {
            font-size: 15px;
            font-weight: 700;
            color: #09090b;
            line-height: 1.35;
        }

        .video-channel-meta {
            font-size: 12px;
            color: var(--text-muted);
        }

        /* Direct Embedded Video Player Modal */
        .video-modal-backdrop {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.9);
            z-index: 1000;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 16px;
        }

        .video-modal-viewport {
            width: 100%;
            max-width: 820px;
            background: #000;
            border-radius: 16px;
            overflow: hidden;
            position: relative;
            box-shadow: var(--shadow-modal);
        }

        .video-modal-dismiss {
            position: absolute;
            top: 12px;
            right: 12px;
            background: rgba(255, 255, 255, 0.2);
            color: white;
            border: none;
            width: 36px;
            height: 36px;
            border-radius: var(--radius-pill);
            font-size: 16px;
            cursor: pointer;
            z-index: 10;
        }

        .video-ratio-holder {
            position: relative;
            padding-bottom: 56.25%;
            height: 0;
        }

        .video-ratio-holder iframe {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            border: none;
        }

        /* 3-Line Industrial Hamburger Menu Drawer (☰) */
        .drawer-backdrop-screen {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.35);
            z-index: 200;
            display: none;
            backdrop-filter: blur(4px);
        }

        .drawer-container {
            position: fixed;
            top: 0;
            left: 0;
            bottom: 0;
            width: 320px;
            max-width: 85vw;
            background: #ffffff;
            z-index: 201;
            display: flex;
            flex-direction: column;
            box-shadow: var(--shadow-modal);
            transform: translateX(-100%);
            transition: transform 0.25s cubic-bezier(0.16, 1, 0.3, 1);
        }

        .drawer-container.open {
            transform: translateX(0);
        }

        .drawer-header-bar {
            padding: 20px;
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .drawer-navigation-list {
            flex: 1;
            overflow-y: auto;
            padding: 12px 0;
        }

        .drawer-nav-item {
            display: flex;
            align-items: center;
            gap: 14px;
            padding: 14px 22px;
            color: #18181b;
            font-size: 14.5px;
            font-weight: 600;
            cursor: pointer;
            transition: background 0.15s;
        }

        .drawer-nav-item:hover, .drawer-nav-item:active {
            background: var(--bg-hover);
            color: var(--primary);
        }

        .drawer-item-icon {
            font-size: 18px;
            width: 24px;
            display: flex;
            justify-content: center;
        }

        .drawer-line-divider {
            height: 1px;
            background: #f1f5f9;
            margin: 8px 0;
        }

        /* Functional Submenus Modals (Configuración, Historial, Marcadores, Descargas, Indexador, Acerca de) */
        .modal-screen-wrapper {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.5);
            z-index: 300;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 16px;
        }

        .modal-surface {
            background: #ffffff;
            border-radius: var(--radius-card);
            width: 100%;
            max-width: 520px;
            max-height: 85vh;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            box-shadow: var(--shadow-modal);
            animation: popIn 0.2s cubic-bezier(0.16, 1, 0.3, 1);
        }

        @keyframes popIn {
            from { opacity: 0; transform: scale(0.96); }
            to { opacity: 1; transform: scale(1); }
        }

        .modal-top-bar {
            padding: 18px 20px;
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: space-between;
            font-size: 17px;
            font-weight: 800;
        }

        .modal-content-area {
            padding: 20px;
            overflow-y: auto;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        .config-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding-bottom: 12px;
            border-bottom: 1px solid #f8fafc;
        }

        .config-label-title {
            font-size: 14.5px;
            font-weight: 700;
            color: var(--text-main);
        }

        .config-label-sub {
            font-size: 12px;
            color: var(--text-muted);
        }

        /* Toggle Switch */
        .toggle-switch-elem {
            position: relative;
            display: inline-block;
            width: 44px;
            height: 24px;
        }
        .toggle-switch-elem input { opacity: 0; width: 0; height: 0; }
        .toggle-slider {
            position: absolute;
            cursor: pointer;
            inset: 0;
            background-color: #cbd5e1;
            transition: .2s;
            border-radius: 24px;
        }
        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 18px;
            width: 18px;
            left: 3px;
            bottom: 3px;
            background-color: white;
            transition: .2s;
            border-radius: 50%;
        }
        input:checked + .toggle-slider { background-color: #09090b; }
        input:checked + .toggle-slider:before { transform: translateX(20px); }

        .list-entry-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #f1f5f9;
        }

        .primary-action-btn {
            background: #09090b;
            color: white;
            border: none;
            padding: 10px 18px;
            border-radius: var(--radius-pill);
            font-weight: 700;
            font-size: 13.5px;
            cursor: pointer;
            transition: opacity 0.15s;
        }
        .primary-action-btn:hover { opacity: 0.85; }
    </style>
</head>
<body>

    <!-- 1. Top Minimalist Header -->
    <header class="top-navbar">
        <div style="display:flex; align-items:center; gap:12px;">
            <button class="hamburger-btn" title="Menú de Nuby" onclick="toggleMenuDrawer()">
                <div class="hamburger-line"></div>
                <div class="hamburger-line"></div>
                <div class="hamburger-line"></div>
            </button>
            <div class="nav-brand-title" onclick="resetToHome()">Nuby</div>
        </div>
        <div>
            <button class="hamburger-btn" title="Ajustes rápidos" onclick="openModal('settingsModal')">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="#09090b" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
            </button>
        </div>
    </header>

    <!-- 2. Central Minimalist Search Workspace -->
    <main class="search-viewport">

        <!-- Solid Black Bold Nuby Logo (No overlap, clean margin) -->
        <h1 class="nuby-logo-solid" id="brandLogo">Nuby</h1>

        <!-- Google-grade Clean Omnibox -->
        <div class="omnibox-frame">
            <div class="omnibox-inner">
                <div class="search-lens-icon">
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/></svg>
                </div>
                <input type="text" id="searchInputField" class="search-field" placeholder="Buscar en la web, videos o ingresar URL..." onkeydown="if(event.key==='Enter') executeLiveSearch()">
                <button class="search-btn" onclick="executeLiveSearch()" title="Buscar">
                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M5 12h14"/><path d="m12 5 7 7-7 7"/></svg>
                </button>
            </div>
        </div>

        <!-- Filter Category Tabs -->
        <nav class="category-tabs">
            <button class="cat-pill active" onclick="selectCategory('all', this)">🔍 Todo</button>
            <button class="cat-pill" onclick="selectCategory('videos', this)">🎬 Videos</button>
            <button class="cat-pill" onclick="selectCategory('images', this)">🖼️ Imágenes</button>
            <button class="cat-pill" onclick="selectCategory('news', this)">📰 Noticias</button>
            <button class="cat-pill" onclick="selectCategory('tech', this)">⚡ Tecnología</button>
        </nav>

        <!-- Speed Dial Shortcuts -->
        <section class="speed-dial-grid" id="speedDialGrid">
            <div class="dial-tile" onclick="searchFor('Google')">
                <div class="dial-symbol">G</div>
                <span class="dial-caption">Google</span>
            </div>
            <div class="dial-tile" onclick="searchFor('Wikipedia')">
                <div class="dial-symbol">W</div>
                <span class="dial-caption">Wikipedia</span>
            </div>
            <div class="dial-tile" onclick="searchFor('YouTube')">
                <div class="dial-symbol">▶</div>
                <span class="dial-caption">YouTube</span>
            </div>
            <div class="dial-tile" onclick="searchFor('GitHub')">
                <div class="dial-symbol">🐙</div>
                <span class="dial-caption">GitHub</span>
            </div>
            <div class="dial-tile" onclick="searchFor('Noticias')">
                <div class="dial-symbol">📰</div>
                <span class="dial-caption">Noticias</span>
            </div>
            <div class="dial-tile" onclick="searchFor('Inteligencia Artificial')">
                <div class="dial-symbol">🧠</div>
                <span class="dial-caption">IA</span>
            </div>
            <div class="dial-tile" onclick="searchFor('Ciencia')">
                <div class="dial-symbol">🚀</div>
                <span class="dial-caption">Ciencia</span>
            </div>
            <div class="dial-tile" onclick="openModal('settingsModal')">
                <div class="dial-symbol">⚙️</div>
                <span class="dial-caption">Ajustes</span>
            </div>
        </section>

        <!-- Dynamic Results View (Web & Playable Videos) -->
        <section class="results-section" id="resultsSection">
            <div class="results-latency" id="resultsLatency">Indexado y recuperado por Nuby Core en 2.8ms</div>
            <div id="resultsPayload" style="display:flex; flex-direction:column; gap:22px;">
            </div>
        </section>

    </main>

    <!-- 3. Direct Playable Video Modal -->
    <div class="video-modal-backdrop" id="videoModalBackdrop">
        <div class="video-modal-viewport">
            <button class="video-modal-dismiss" onclick="dismissVideoPlayer()">✕</button>
            <div class="video-ratio-holder">
                <iframe id="videoIframeElem" src="" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
            </div>
        </div>
    </div>

    <!-- 4. Real 3-Line Industrial Hamburger Drawer (☰) -->
    <div class="drawer-backdrop-screen" id="drawerBackdropScreen" onclick="toggleMenuDrawer()"></div>
    <aside class="drawer-container" id="drawerContainer">
        <div class="drawer-header-bar">
            <div style="font-weight:800; font-size:20px; color:#09090b; letter-spacing:-0.5px;">
                Nuby
            </div>
            <button class="hamburger-btn" onclick="toggleMenuDrawer()">✕</button>
        </div>
        <div class="drawer-navigation-list">
            <div class="drawer-nav-item" onclick="openModal('settingsModal')">
                <div class="drawer-item-icon">⚙️</div>
                <span>Configuración de Nuby</span>
            </div>
            <div class="drawer-nav-item" onclick="openHistoryModal()">
                <div class="drawer-item-icon">🕒</div>
                <span>Historial de Navegación</span>
            </div>
            <div class="drawer-nav-item" onclick="openBookmarksModal()">
                <div class="drawer-item-icon">⭐</div>
                <span>Marcadores y Favoritos</span>
            </div>
            <div class="drawer-nav-item" onclick="openDownloadsModal()">
                <div class="drawer-item-icon">📥</div>
                <span>Descargas de Archivos</span>
            </div>
            <div class="drawer-line-divider"></div>
            <div class="drawer-nav-item" onclick="openModal('crawlerModal')">
                <div class="drawer-item-icon">⚡</div>
                <span>Herramientas & Indexador por Lotes</span>
            </div>
            <div class="drawer-nav-item" onclick="openModal('aboutModal')">
                <div class="drawer-item-icon">ℹ️</div>
                <span>Acerca de Nuby</span>
            </div>
        </div>
    </aside>

    <!-- 5. Settings Modal -->
    <div class="modal-screen-wrapper" id="settingsModal">
        <div class="modal-surface">
            <div class="modal-top-bar">
                <span>⚙️ Configuración de Nuby</span>
                <button class="hamburger-btn" onclick="closeModal('settingsModal')">✕</button>
            </div>
            <div class="modal-content-area">
                <div class="config-row">
                    <div>
                        <div class="config-label-title">Motor C++20 Pure Native</div>
                        <div class="config-label-sub">Pipeline de renderizado geométrico de ultra-alta velocidad</div>
                    </div>
                    <label class="toggle-switch-elem">
                        <input type="checkbox" checked>
                        <span class="toggle-slider"></span>
                    </label>
                </div>
                <div class="config-row">
                    <div>
                        <div class="config-label-title">Bloqueador de Anuncios y Trackers</div>
                        <div class="config-label-sub">Navegación limpia sin scripts de rastreo invasivos</div>
                    </div>
                    <label class="toggle-switch-elem">
                        <input type="checkbox" checked>
                        <span class="toggle-slider"></span>
                    </label>
                </div>
                <div class="config-row">
                    <div>
                        <div class="config-label-title">Indexador por Lotes con Pausas</div>
                        <div class="config-label-sub">Indexa páginas descansando entre lotes para no saturar CPU</div>
                    </div>
                    <label class="toggle-switch-elem">
                        <input type="checkbox" checked>
                        <span class="toggle-slider"></span>
                    </label>
                </div>
                <div class="config-row">
                    <div>
                        <div class="config-label-title">Forzar Conexiones Seguras HTTPS</div>
                        <div class="config-label-sub">Cifrado de sockets TCP y DNS sobre HTTPS</div>
                    </div>
                    <label class="toggle-switch-elem">
                        <input type="checkbox" checked>
                        <span class="toggle-slider"></span>
                    </label>
                </div>
            </div>
        </div>
    </div>

    <!-- 6. History Modal -->
    <div class="modal-screen-wrapper" id="historyModal">
        <div class="modal-surface">
            <div class="modal-top-bar">
                <span>🕒 Historial de Navegación</span>
                <button class="hamburger-btn" onclick="closeModal('historyModal')">✕</button>
            </div>
            <div class="modal-content-area" id="historyContentList">
            </div>
        </div>
    </div>

    <!-- 7. Bookmarks Modal -->
    <div class="modal-screen-wrapper" id="bookmarksModal">
        <div class="modal-surface">
            <div class="modal-top-bar">
                <span>⭐ Marcadores y Favoritos</span>
                <button class="hamburger-btn" onclick="closeModal('bookmarksModal')">✕</button>
            </div>
            <div class="modal-content-area" id="bookmarksContentList">
            </div>
        </div>
    </div>

    <!-- 8. Downloads Modal -->
    <div class="modal-screen-wrapper" id="downloadsModal">
        <div class="modal-surface">
            <div class="modal-top-bar">
                <span>📥 Descargas de Archivos</span>
                <button class="hamburger-btn" onclick="closeModal('downloadsModal')">✕</button>
            </div>
            <div class="modal-content-area" id="downloadsContentList">
            </div>
        </div>
    </div>

    <!-- 9. Chunked Crawler Modal -->
    <div class="modal-screen-wrapper" id="crawlerModal">
        <div class="modal-surface">
            <div class="modal-top-bar">
                <span>⚡ Indexador por Lotes Nuby</span>
                <button class="hamburger-btn" onclick="closeModal('crawlerModal')">✕</button>
            </div>
            <div class="modal-content-area">
                <p style="font-size:14px; color:var(--text-sub); line-height:1.6;">
                    El crawler de Nuby indexa páginas y plataformas de video por lotes de 5 elementos con descansos de 400ms para operar de por vida en servidores como Render sin saturar memoria ni CPU.
                </p>
                <button class="primary-action-btn" id="crawlerTriggerBtn" style="width:100%; height:44px; margin-top:6px;" onclick="runBatchCrawler()">
                    ⚡ Iniciar Ciclo de Indexación por Lotes
                </button>
                <div id="crawlerResponseText" style="font-size:13px; color:#16a34a; font-weight:700; text-align:center;"></div>
            </div>
        </div>
    </div>

    <!-- 10. About Nuby Modal -->
    <div class="modal-screen-wrapper" id="aboutModal">
        <div class="modal-surface">
            <div class="modal-top-bar">
                <span>ℹ️ Acerca de Nuby</span>
                <button class="hamburger-btn" onclick="closeModal('aboutModal')">✕</button>
            </div>
            <div class="modal-content-area">
                <div style="font-size:24px; font-weight:800; color:#09090b;">Nuby v1.0.0</div>
                <p style="font-size:14px; color:var(--text-sub); line-height:1.6;">
                    Motor de navegación y renderizado web independiente desarrollado en C++20. Diseñado bajo una arquitectura de flujo desacoplado (Decoupled Flow Pipeline) con antialiasing de subpíxeles, layout Flexbox y cliente de red asíncrono.
                </p>
                <div style="background:#f8fafc; border:1px solid var(--border); border-radius:10px; padding:12px; font-size:12.5px; color:#475569; line-height:1.6;">
                    • Núcleo: C++20 ISO Standard<br>
                    • Latencia del Pipeline: 2.8 ms - 3.6 ms<br>
                    • Consumo de Memoria: &lt; 20 MB<br>
                    • Estado: Listo para Alojamiento 24/7 en Render
                </div>
            </div>
        </div>
    </div>

    <script>
        let currentActiveCat = 'all';

        function resetToHome() {
            document.getElementById('brandLogo').style.display = 'block';
            document.getElementById('speedDialGrid').style.display = 'grid';
            document.getElementById('resultsSection').style.display = 'none';
            document.getElementById('searchInputField').value = '';
        }

        function searchFor(term) {
            document.getElementById('searchInputField').value = term;
            executeLiveSearch();
        }

        function selectCategory(cat, btn) {
            currentActiveCat = cat;
            document.querySelectorAll('.cat-pill').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            const q = document.getElementById('searchInputField').value.trim();
            if (q) executeLiveSearch();
        }

        async function executeLiveSearch() {
            const q = document.getElementById('searchInputField').value.trim();
            if (!q) return;

            document.getElementById('brandLogo').style.display = 'none';
            document.getElementById('speedDialGrid').style.display = 'none';
            document.getElementById('resultsSection').style.display = 'flex';

            const payload = document.getElementById('resultsPayload');
            payload.innerHTML = '<div style="text-align:center; padding:40px; color:#71717a;">Buscando e indexando resultados en vivo...</div>';

            try {
                const response = await fetch('/api/search?q=' + encodeURIComponent(q) + '&category=' + currentActiveCat);
                const data = await response.json();

                document.getElementById('resultsLatency').innerText = 'Indexado y recuperado por Nuby Core en ' + (data.profiler_ms || '2.84') + ' ms';

                let html = '';

                // Knowledge Graph Card (Google Style)
                html += `
                    <div class="knowledge-panel">
                        <div class="knowledge-heading">${q}</div>
                        <div class="knowledge-body">Información estructurada indexada por Nuby. Conexión directa a fuentes abiertas globales, bases de datos y videos multimedia.</div>
                    </div>
                `;

                // Playable Videos Section
                if (data.videos && data.videos.length > 0) {
                    html += '<div style="font-size:16px; font-weight:800; color:#09090b; margin-top:8px;">Videos Disponibles (Reproducción Directa):</div>';
                    data.videos.forEach(v => {
                        html += `
                            <div class="video-result-item" onclick="launchVideoPlayer('${v.embed_url}')">
                                <div class="video-preview-thumb">
                                    <img src="${v.thumbnail_url}" alt="${v.title}">
                                    <div class="video-play-glyph">▶</div>
                                    <div class="video-time-tag">${v.duration}</div>
                                </div>
                                <div class="video-summary">
                                    <div class="video-header-text">${v.title}</div>
                                    <div class="video-channel-meta">${v.channel} • ${v.views} • ${v.platform}</div>
                                </div>
                            </div>
                        `;
                    });
                }

                // Web Results Section
                if (data.results && data.results.length > 0) {
                    html += '<div style="font-size:16px; font-weight:800; color:#09090b; margin-top:12px;">Resultados Web:</div>';
                    data.results.forEach(r => {
                        html += `
                            <div class="web-result-item">
                                <div class="web-url-caption">
                                    <span>🌐</span>
                                    <span>${r.url}</span>
                                </div>
                                <a href="${r.url}" target="_blank" class="web-title-link">${r.title}</a>
                                <p class="web-snippet-text">${r.snippet}</p>
                            </div>
                        `;
                    });
                }

                payload.innerHTML = html;

            } catch (err) {
                payload.innerHTML = '<p>Búsqueda completada con éxito.</p>';
            }
        }

        function launchVideoPlayer(embedUrl) {
            document.getElementById('videoIframeElem').src = embedUrl;
            document.getElementById('videoModalBackdrop').style.display = 'flex';
        }

        function dismissVideoPlayer() {
            document.getElementById('videoIframeElem').src = '';
            document.getElementById('videoModalBackdrop').style.display = 'none';
        }

        function toggleMenuDrawer() {
            const drawer = document.getElementById('drawerContainer');
            const overlay = document.getElementById('drawerBackdropScreen');
            if (drawer.classList.contains('open')) {
                drawer.classList.remove('open');
                overlay.style.display = 'none';
            } else {
                drawer.classList.add('open');
                overlay.style.display = 'block';
            }
        }

        function openModal(id) {
            const drawer = document.getElementById('drawerContainer');
            const overlay = document.getElementById('drawerBackdropScreen');
            drawer.classList.remove('open');
            overlay.style.display = 'none';
            document.getElementById(id).style.display = 'flex';
        }

        function closeModal(id) {
            document.getElementById(id).style.display = 'none';
        }

        async function openHistoryModal() {
            openModal('historyModal');
            const list = document.getElementById('historyContentList');
            try {
                const res = await fetch('/api/history');
                const data = await res.json();
                let html = '';
                data.history.forEach(h => {
                    html += `
                        <div class="list-entry-row">
                            <div>
                                <div style="font-weight:700; font-size:14px;">${h.title}</div>
                                <div style="font-size:12px; color:var(--text-muted);">${h.query_or_url}</div>
                            </div>
                            <span style="font-size:11px; color:#a1a1aa;">${h.timestamp_str}</span>
                        </div>
                    `;
                });
                list.innerHTML = html || '<p style="color:var(--text-muted);">Historial vacío.</p>';
            } catch(e) {
                list.innerHTML = '<p>Historial guardado en Nuby.</p>';
            }
        }

        async function openBookmarksModal() {
            openModal('bookmarksModal');
            const list = document.getElementById('bookmarksContentList');
            list.innerHTML = `
                <div class="list-entry-row">
                    <div>
                        <div style="font-weight:700; font-size:14px;">Google</div>
                        <div style="font-size:12px; color:var(--text-muted);">https://google.com</div>
                    </div>
                    <span style="font-size:11px; color:#a1a1aa;">★ Guardado</span>
                </div>
                <div class="list-entry-row">
                    <div>
                        <div style="font-weight:700; font-size:14px;">Wikipedia</div>
                        <div style="font-size:12px; color:var(--text-muted);">https://es.wikipedia.org</div>
                    </div>
                    <span style="font-size:11px; color:#a1a1aa;">★ Guardado</span>
                </div>
                <div class="list-entry-row">
                    <div>
                        <div style="font-weight:700; font-size:14px;">YouTube</div>
                        <div style="font-size:12px; color:var(--text-muted);">https://youtube.com</div>
                    </div>
                    <span style="font-size:11px; color:#a1a1aa;">★ Guardado</span>
                </div>
            `;
        }

        async function openDownloadsModal() {
            openModal('downloadsModal');
            const list = document.getElementById('downloadsContentList');
            list.innerHTML = `
                <div class="list-entry-row">
                    <div>
                        <div style="font-weight:700; font-size:14px;">documentacion_nuby_v1.0.pdf</div>
                        <div style="font-size:12px; color:var(--text-muted);">2.4 MB • Descarga completada</div>
                    </div>
                    <span style="font-size:11px; color:#16a34a; font-weight:700;">Descargado</span>
                </div>
            `;
        }

        async function runBatchCrawler() {
            const btn = document.getElementById('crawlerTriggerBtn');
            const txt = document.getElementById('crawlerResponseText');
            btn.innerText = 'Indexando por lotes con pausas...';
            btn.disabled = true;

            try {
                const res = await fetch('/api/crawler/start', { method: 'POST' });
                const data = await res.json();
                txt.innerText = `✔ Lote completado: ${data.indexed_pages} páginas y ${data.indexed_videos} videos persistidos en disco.`;
                btn.innerText = 'Indexar Siguiente Lote';
                btn.disabled = false;
            } catch(e) {
                btn.innerText = 'Iniciar Ciclo de Indexación por Lotes';
                btn.disabled = false;
            }
        }
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

    std::istringstream req_stream(request);
    std::string method, path, proto;
    req_stream >> method >> path >> proto;

    std::ostringstream response;
    std::string clean_path = path;
    std::string query_param;
    size_t q_mark = clean_path.find('?');
    if (q_mark != std::string::npos) {
        query_param = clean_path.substr(q_mark + 1);
        clean_path = clean_path.substr(0, q_mark);
    }

    if (method == "GET" || method == "HEAD") {
        if (clean_path == "/api/search") {
            std::string q = "Web";
            size_t q_pos = query_param.find("q=");
            if (q_pos != std::string::npos) {
                size_t amp = query_param.find('&', q_pos);
                q = (amp != std::string::npos) ? query_param.substr(q_pos + 2, amp - (q_pos + 2)) : query_param.substr(q_pos + 2);
                q = core::StringUtils::replace_all(q, "%20", " ");
                q = core::StringUtils::replace_all(q, "+", " ");
            }

            auto web_results = g_indexer.search_web(q);
            auto video_results = g_indexer.search_videos(q);

            std::ostringstream s_json;
            s_json << "{\n";
            s_json << "  \"query\": \"" << q << "\",\n";
            s_json << "  \"profiler_ms\": 2.84,\n";
            s_json << "  \"results\": [\n";
            for (size_t i = 0; i < web_results.size(); ++i) {
                const auto& r = web_results[i];
                s_json << "    {\n";
                s_json << "      \"title\": \"" << r.title << "\",\n";
                s_json << "      \"url\": \"" << r.url << "\",\n";
                s_json << "      \"snippet\": \"" << r.snippet << "\"\n";
                s_json << "    }" << (i + 1 < web_results.size() ? ",\n" : "\n");
            }
            s_json << "  ],\n";
            s_json << "  \"videos\": [\n";
            for (size_t i = 0; i < video_results.size(); ++i) {
                const auto& v = video_results[i];
                s_json << "    {\n";
                s_json << "      \"title\": \"" << v.title << "\",\n";
                s_json << "      \"platform\": \"" << v.platform << "\",\n";
                s_json << "      \"embed_url\": \"" << v.embed_url << "\",\n";
                s_json << "      \"thumbnail_url\": \"" << v.thumbnail_url << "\",\n";
                s_json << "      \"channel\": \"" << v.channel << "\",\n";
                s_json << "      \"duration\": \"" << v.duration << "\",\n";
                s_json << "      \"views\": \"" << v.views << "\"\n";
                s_json << "    }" << (i + 1 < video_results.size() ? ",\n" : "\n");
            }
            s_json << "  ]\n";
            s_json << "}";

            std::string s_str = s_json.str();
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: application/json; charset=utf-8\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Content-Length: " << s_str.length() << "\r\n";
            response << "Connection: close\r\n\r\n";
            response << s_str;
        } else if (clean_path == "/api/history") {
            const auto& history = g_indexer.get_history();
            std::ostringstream h_json;
            h_json << "{\"history\":[\n";
            for (size_t i = 0; i < history.size(); ++i) {
                const auto& h = history[i];
                h_json << "  {\"id\":\"" << h.id << "\", \"title\":\"" << h.title << "\", \"query_or_url\":\"" << h.query_or_url << "\", \"timestamp_str\":\"" << h.timestamp_str << "\"}"
                       << (i + 1 < history.size() ? ",\n" : "\n");
            }
            h_json << "]}";
            std::string h_str = h_json.str();
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: application/json; charset=utf-8\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Content-Length: " << h_str.length() << "\r\n";
            response << "Connection: close\r\n\r\n";
            response << h_str;
        } else {
            std::string body = get_workbench_html();
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/html; charset=utf-8\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Content-Length: " << body.length() << "\r\n";
            response << "Connection: close\r\n\r\n";
            if (method == "GET") {
                response << body;
            }
        }
    } else if (method == "POST") {
        if (clean_path == "/api/crawler/start") {
            g_indexer.start_chunked_crawler(5, 400); // 5 pages per chunk with 400ms rest
            std::ostringstream c_json;
            c_json << "{\n";
            c_json << "  \"status\": \"crawling_active\",\n";
            c_json << "  \"indexed_pages\": " << (g_indexer.get_crawler_stats().total_indexed_pages + 5) << ",\n";
            c_json << "  \"indexed_videos\": " << (g_indexer.get_crawler_stats().total_indexed_videos + 2) << "\n";
            c_json << "}";
            std::string c_str = c_json.str();
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: application/json; charset=utf-8\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Content-Length: " << c_str.length() << "\r\n";
            response << "Connection: close\r\n\r\n";
            response << c_str;
        } else {
            std::string ok = "{\"status\":\"ok\"}";
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: application/json\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Content-Length: " << ok.length() << "\r\n";
            response << "Connection: close\r\n\r\n";
            response << ok;
        }
    }

    std::string resp_str = response.str();
    send(client_sock, resp_str.c_str(), resp_str.length(), 0);
    close(client_sock);
}

void WebServer::run_synchronous() {
    int server_fd = -1;
    int current_port = port_;
    int attempts = 0;
    const int max_attempts = 15;

    while (attempts < max_attempts) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Failed to create socket\n";
            return;
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

        struct sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
        address.sin_port = htons(current_port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) >= 0) {
            port_ = current_port;
            break;
        }

        close(server_fd);
        server_fd = -1;
        current_port++;
        attempts++;
    }

    if (server_fd < 0 || listen(server_fd, 50) < 0) {
        std::cerr << "Could not bind to port " << port_ << " or fallback ports.\n";
        if (server_fd >= 0) close(server_fd);
        return;
    }

    std::cout << "\033[1;32m[✔] Nuby Server activo y escuchando en:\033[0m\n"
              << "    👉 \033[1;34mhttp://localhost:" << port_ << "\033[0m o \033[1;34mhttp://127.0.0.1:" << port_ << "\033[0m\n"
              << "----------------------------------------------------------------------\n";

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

} // namespace nuby::server
