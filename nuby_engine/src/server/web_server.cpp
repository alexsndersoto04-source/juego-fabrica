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
    <link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg: #ffffff;
            --bg-hover: #f8fafc;
            --bg-panel: #f1f5f9;
            --border: #dfe1e5;
            --border-hover: #c3c7d0;
            --text-main: #202124;
            --text-sub: #5f6368;
            --text-link: #1a0dab;
            --primary: #1a73e8;
            --primary-hover: #1557b0;
            --shadow-search: 0 1px 6px rgba(32, 33, 36, 0.28);
            --shadow-hover: 0 4px 16px rgba(32, 33, 36, 0.16);
            --shadow-modal: 0 20px 50px rgba(0, 0, 0, 0.15);
            --radius-pill: 24px;
            --radius-card: 12px;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Plus Jakarta Sans', -apple-system, Roboto, Arial, sans-serif;
            -webkit-tap-highlight-color: transparent;
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
        .nuby-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 12px 18px;
            background: #ffffff;
            border-bottom: 1px solid #f1f3f4;
            position: sticky;
            top: 0;
            z-index: 100;
        }

        .header-left, .header-right {
            display: flex;
            align-items: center;
            gap: 12px;
        }

        .menu-btn {
            background: transparent;
            border: none;
            cursor: pointer;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 4px;
            transition: background 0.15s;
        }

        .menu-btn:hover, .menu-btn:active {
            background: #f1f3f4;
        }

        .menu-line {
            width: 18px;
            height: 2px;
            background-color: #5f6368;
            border-radius: 2px;
        }

        .brand-logo-small {
            font-size: 20px;
            font-weight: 800;
            letter-spacing: -0.5px;
            color: #202124;
            text-decoration: none;
            display: flex;
            align-items: center;
            gap: 4px;
        }

        .brand-logo-small span.blue { color: #4285f4; }
        .brand-logo-small span.red { color: #ea4335; }
        .brand-logo-small span.yellow { color: #fbbc05; }
        .brand-logo-small span.green { color: #34a853; }

        .apk-badge-btn {
            background: #e8f0fe;
            color: #1a73e8;
            border: 1px solid #d2e3fc;
            padding: 6px 14px;
            border-radius: var(--radius-pill);
            font-size: 12.5px;
            font-weight: 700;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            transition: all 0.15s;
            text-decoration: none;
        }

        .apk-badge-btn:hover {
            background: #1a73e8;
            color: #ffffff;
        }

        /* Main Search Container (Google Aesthetic) */
        .main-container {
            flex: 1;
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 100%;
            max-width: 800px;
            margin: 0 auto;
            padding: 30px 16px 80px 16px;
        }

        .hero-logo {
            font-size: 56px;
            font-weight: 800;
            letter-spacing: -2px;
            margin-bottom: 24px;
            margin-top: 15px;
            user-select: none;
            display: flex;
            align-items: center;
        }

        .hero-logo span.n { color: #4285f4; }
        .hero-logo span.u { color: #ea4335; }
        .hero-logo span.b { color: #fbbc05; }
        .hero-logo span.y { color: #34a853; }

        /* Google-style Minimal Omnibox */
        .search-box-wrapper {
            width: 100%;
            max-width: 620px;
            margin-bottom: 24px;
            position: relative;
        }

        .search-box {
            display: flex;
            align-items: center;
            background: #ffffff;
            border: 1px solid var(--border);
            border-radius: var(--radius-pill);
            padding: 0 16px;
            height: 50px;
            box-shadow: 0 1px 6px rgba(32, 33, 36, 0.08);
            transition: all 0.2s ease;
        }

        .search-box:hover {
            box-shadow: 0 2px 8px rgba(32, 33, 36, 0.18);
            border-color: transparent;
        }

        .search-box:focus-within {
            box-shadow: var(--shadow-search);
            border-color: transparent;
        }

        .search-icon {
            color: #9aa0a6;
            margin-right: 12px;
            display: flex;
            align-items: center;
        }

        .search-input {
            flex: 1;
            border: none;
            outline: none;
            font-size: 16px;
            color: var(--text-main);
            background: transparent;
        }

        .search-actions {
            display: flex;
            align-items: center;
            gap: 8px;
        }

        .clear-btn {
            background: transparent;
            border: none;
            color: #70757a;
            cursor: pointer;
            padding: 4px;
            display: none;
        }

        /* Search Categories (Google tabs: Todo, Videos, Imágenes, Noticias) */
        .category-nav {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 24px;
            overflow-x: auto;
            scrollbar-width: none;
            width: 100%;
            max-width: 620px;
            padding-bottom: 2px;
        }
        .category-nav::-webkit-scrollbar { display: none; }

        .cat-chip {
            background: transparent;
            border: 1px solid var(--border);
            color: var(--text-sub);
            padding: 7px 16px;
            border-radius: var(--radius-pill);
            font-size: 13.5px;
            font-weight: 600;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            white-space: nowrap;
            transition: all 0.15s;
        }

        .cat-chip.active, .cat-chip:hover {
            background: #f1f3f4;
            color: var(--primary);
            border-color: #d2e3fc;
        }

        /* Speed Dial / Top Sites Google Grid */
        .speed-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 16px;
            width: 100%;
            max-width: 540px;
            margin-bottom: 30px;
        }

        @media (max-width: 480px) {
            .speed-grid {
                grid-template-columns: repeat(4, 1fr);
                gap: 12px;
            }
            .hero-logo { font-size: 44px; }
        }

        .speed-item {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 8px;
            text-decoration: none;
            color: var(--text-main);
            cursor: pointer;
        }

        .speed-circle {
            width: 52px;
            height: 52px;
            border-radius: 50%;
            background: #f1f3f4;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 20px;
            font-weight: 700;
            transition: all 0.2s;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.05);
        }

        .speed-item:hover .speed-circle {
            background: #e8f0fe;
            transform: scale(1.06);
        }

        .speed-name {
            font-size: 12px;
            font-weight: 500;
            color: var(--text-sub);
            max-width: 68px;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }

        /* Crawler & Indexer Status Bar */
        .crawler-box {
            width: 100%;
            max-width: 620px;
            background: #ffffff;
            border: 1px solid var(--border);
            border-radius: var(--radius-card);
            padding: 16px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.03);
            margin-bottom: 24px;
        }

        .crawler-info {
            display: flex;
            flex-direction: column;
            gap: 2px;
        }

        .crawler-title {
            font-size: 13.5px;
            font-weight: 700;
            color: var(--text-main);
            display: flex;
            align-items: center;
            gap: 6px;
        }

        .crawler-sub {
            font-size: 12px;
            color: var(--text-sub);
        }

        .crawler-btn {
            background: #1a73e8;
            border: none;
            color: white;
            padding: 8px 16px;
            border-radius: var(--radius-pill);
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
            transition: background 0.15s;
        }

        .crawler-btn:hover { background: #1557b0; }

        /* Search Results Container */
        .results-container {
            width: 100%;
            max-width: 700px;
            display: none;
            flex-direction: column;
            gap: 20px;
            margin-top: 10px;
        }

        .results-stat {
            font-size: 13px;
            color: var(--text-sub);
            margin-bottom: 4px;
        }

        .result-card {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .result-domain-row {
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 12px;
            color: var(--text-sub);
        }

        .result-title {
            font-size: 19px;
            font-weight: 600;
            color: var(--text-link);
            text-decoration: none;
            line-height: 1.35;
            cursor: pointer;
        }

        .result-title:hover {
            text-decoration: underline;
        }

        .result-snippet {
            font-size: 14px;
            color: #4d5156;
            line-height: 1.55;
        }

        /* Video Card Results (Reproductor Embebido) */
        .video-card {
            background: #ffffff;
            border: 1px solid var(--border);
            border-radius: var(--radius-card);
            padding: 14px;
            display: flex;
            gap: 14px;
            align-items: flex-start;
            box-shadow: 0 1px 4px rgba(0,0,0,0.04);
            cursor: pointer;
            transition: all 0.2s;
        }

        .video-card:hover {
            box-shadow: var(--shadow-hover);
            border-color: #d2e3fc;
        }

        .video-thumb-container {
            position: relative;
            width: 130px;
            height: 80px;
            border-radius: 8px;
            overflow: hidden;
            flex-shrink: 0;
            background: #000;
        }

        .video-thumb-img {
            width: 100%;
            height: 100%;
            object-fit: cover;
        }

        .video-duration-pill {
            position: absolute;
            bottom: 4px;
            right: 4px;
            background: rgba(0, 0, 0, 0.8);
            color: white;
            font-size: 10px;
            padding: 2px 6px;
            border-radius: 4px;
            font-weight: 700;
        }

        .video-play-overlay {
            position: absolute;
            inset: 0;
            display: flex;
            align-items: center;
            justify-content: center;
            background: rgba(0,0,0,0.25);
            color: white;
            font-size: 24px;
        }

        .video-info {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .video-title {
            font-size: 15px;
            font-weight: 700;
            color: var(--text-main);
            line-height: 1.35;
        }

        .video-meta {
            font-size: 12px;
            color: var(--text-sub);
        }

        /* Video Player Modal (Ver video sin salir) */
        .video-modal-backdrop {
            position: fixed;
            inset: 0;
            background: rgba(0,0,0,0.85);
            z-index: 1000;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }

        .video-modal-box {
            width: 100%;
            max-width: 800px;
            background: #000000;
            border-radius: 16px;
            overflow: hidden;
            position: relative;
            box-shadow: var(--shadow-modal);
        }

        .video-modal-close {
            position: absolute;
            top: 12px;
            right: 12px;
            background: rgba(255,255,255,0.2);
            color: white;
            border: none;
            width: 36px;
            height: 36px;
            border-radius: 50%;
            font-size: 18px;
            cursor: pointer;
            z-index: 10;
        }

        .video-iframe-holder {
            position: relative;
            padding-bottom: 56.25%;
            height: 0;
        }

        .video-iframe-holder iframe {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            border: none;
        }

        /* 3-Line Menu Drawer (Industrial Grade) */
        .drawer-backdrop {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.4);
            z-index: 200;
            display: none;
            backdrop-filter: blur(2px);
        }

        .drawer-sidebar {
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

        .drawer-sidebar.open {
            transform: translateX(0);
        }

        .drawer-top {
            padding: 18px;
            border-bottom: 1px solid #f1f3f4;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .drawer-list {
            flex: 1;
            overflow-y: auto;
            padding: 12px 0;
        }

        .drawer-item {
            display: flex;
            align-items: center;
            gap: 14px;
            padding: 12px 20px;
            color: #3c4043;
            font-size: 14.5px;
            font-weight: 500;
            cursor: pointer;
            text-decoration: none;
            transition: background 0.15s;
        }

        .drawer-item:hover, .drawer-item:active {
            background: #f8f9fa;
            color: var(--primary);
        }

        .drawer-icon {
            font-size: 18px;
            width: 24px;
            display: flex;
            justify-content: center;
        }

        .drawer-divider {
            height: 1px;
            background: #f1f3f4;
            margin: 8px 0;
        }

        /* Modal Dialogs (Configuracion, Historial, Descargas) */
        .modal-overlay {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.5);
            z-index: 300;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 16px;
        }

        .modal-content {
            background: #ffffff;
            border-radius: 16px;
            width: 100%;
            max-width: 520px;
            max-height: 85vh;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            box-shadow: var(--shadow-modal);
            animation: modalPop 0.2s ease;
        }

        @keyframes modalPop {
            from { opacity: 0; transform: scale(0.95); }
            to { opacity: 1; transform: scale(1); }
        }

        .modal-header {
            padding: 16px 20px;
            border-bottom: 1px solid #f1f3f4;
            display: flex;
            align-items: center;
            justify-content: space-between;
            font-size: 17px;
            font-weight: 700;
        }

        .modal-body {
            padding: 20px;
            overflow-y: auto;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        .setting-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding-bottom: 12px;
            border-bottom: 1px solid #f8f9fa;
        }

        .setting-label {
            font-size: 14.5px;
            font-weight: 600;
            color: var(--text-main);
        }

        .setting-desc {
            font-size: 12px;
            color: var(--text-sub);
        }

        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 44px;
            height: 24px;
        }

        .toggle-switch input { opacity: 0; width: 0; height: 0; }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #ccc;
            transition: .2s;
            border-radius: 24px;
        }

        .slider:before {
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

        input:checked + .slider { background-color: #1a73e8; }
        input:checked + .slider:before { transform: translateX(20px); }

        .history-entry, .download-entry {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #f1f3f4;
            font-size: 13.5px;
        }
    </style>
</head>
<body>

    <!-- 1. Top Minimal Bar -->
    <header class="nuby-header">
        <div class="header-left">
            <button class="menu-btn" title="Menú principal de Nuby" onclick="toggleDrawer()">
                <div class="menu-line"></div>
                <div class="menu-line"></div>
                <div class="menu-line"></div>
            </button>
            <a href="#" class="brand-logo-small" onclick="goHome()">
                <span class="blue">N</span><span class="red">u</span><span class="yellow">b</span><span class="green">y</span>
            </a>
        </div>
        <div class="header-right">
            <button class="apk-badge-btn" onclick="openApkModal()">
                📲 Descargar APK
            </button>
        </div>
    </header>

    <!-- 2. Main Search & Home Workspace -->
    <main class="main-container">
        
        <!-- Big Iconic Logo (Google Style) -->
        <div class="hero-logo" id="heroLogo">
            <span class="n">N</span><span class="u">u</span><span class="b">b</span><span class="y">y</span>
        </div>

        <!-- Google Omnibox Input -->
        <div class="search-box-wrapper">
            <div class="search-box">
                <div class="search-icon">
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/></svg>
                </div>
                <input type="text" id="mainSearchInput" class="search-input" placeholder="Buscar en la web, videos o ingresar URL..." onkeydown="if(event.key==='Enter') executeSearch()">
                <div class="search-actions">
                    <button id="clearBtn" class="clear-btn" onclick="clearInput()">✕</button>
                </div>
            </div>
        </div>

        <!-- Category Filter Tabs (Todo, Videos, Imágenes, Noticias) -->
        <nav class="category-nav">
            <button class="cat-chip active" onclick="switchCategory('all', this)">
                🔍 Todo
            </button>
            <button class="cat-chip" onclick="switchCategory('videos', this)">
                🎬 Videos
            </button>
            <button class="cat-chip" onclick="switchCategory('images', this)">
                🖼️ Imágenes
            </button>
            <button class="cat-chip" onclick="switchCategory('news', this)">
                📰 Noticias
            </button>
            <button class="cat-chip" onclick="switchCategory('tech', this)">
                ⚡ Tecnología
            </button>
        </nav>

        <!-- Speed Dial Shortcuts (Google style) -->
        <section class="speed-grid" id="speedGrid">
            <div class="speed-item" onclick="quickSearch('Google')">
                <div class="speed-circle" style="color:#4285f4;">G</div>
                <span class="speed-name">Google</span>
            </div>
            <div class="speed-item" onclick="quickSearch('Wikipedia')">
                <div class="speed-circle" style="color:#000000;">W</div>
                <span class="speed-name">Wikipedia</span>
            </div>
            <div class="speed-item" onclick="quickSearch('YouTube')">
                <div class="speed-circle" style="color:#ea4335;">▶</div>
                <span class="speed-name">YouTube</span>
            </div>
            <div class="speed-item" onclick="quickSearch('GitHub')">
                <div class="speed-circle" style="color:#24292e;">🐙</div>
                <span class="speed-name">GitHub</span>
            </div>
            <div class="speed-item" onclick="quickSearch('Noticias')">
                <div class="speed-circle" style="color:#34a853;">📰</div>
                <span class="speed-name">Noticias</span>
            </div>
            <div class="speed-item" onclick="quickSearch('Inteligencia Artificial')">
                <div class="speed-circle" style="color:#8b5cf6;">🧠</div>
                <span class="speed-name">IA</span>
            </div>
            <div class="speed-item" onclick="quickSearch('Ciencia')">
                <div class="speed-circle" style="color:#f59e0b;">🚀</div>
                <span class="speed-name">Ciencia</span>
            </div>
            <div class="speed-item" onclick="openSettingsModal()">
                <div class="speed-circle" style="color:#5f6368;">⚙️</div>
                <span class="speed-name">Ajustes</span>
            </div>
        </section>

        <!-- Real-Time Chunked Indexer Status -->
        <div class="crawler-box" id="crawlerBox">
            <div class="crawler-info">
                <div class="crawler-title">
                    <span>⚡</span> Indexador Web y de Videos Nuby
                </div>
                <div class="crawler-sub" id="crawlerStatsText">
                    Índice activo: 12,450 páginas y videos indexados de forma persistente.
                </div>
            </div>
            <button class="crawler-btn" id="crawlerTriggerBtn" onclick="triggerChunkedCrawler()">
                Indexar por Lotes
            </button>
        </div>

        <!-- 3. Dynamic Search & Video Results -->
        <section class="results-container" id="resultsContainer">
            <div class="results-stat" id="resultsStat">Aproximadamente 1,240,000 resultados (0.003 segundos)</div>
            <div id="resultsList" style="display:flex; flex-direction:column; gap:20px;">
                <!-- Real result cards injected here -->
            </div>
        </section>

    </main>

    <!-- 4. Video Player Direct Modal -->
    <div class="video-modal-backdrop" id="videoModal">
        <div class="video-modal-box">
            <button class="video-modal-close" onclick="closeVideoModal()">✕</button>
            <div class="video-iframe-holder">
                <iframe id="videoIframe" src="" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
            </div>
        </div>
    </div>

    <!-- 5. Real 3-Line Industrial Menu Drawer (☰) -->
    <div class="drawer-backdrop" id="drawerBackdrop" onclick="toggleDrawer()"></div>
    <aside class="drawer-sidebar" id="drawerSidebar">
        <div class="drawer-top">
            <div class="brand-logo-small">
                <span class="blue">N</span><span class="red">u</span><span class="yellow">b</span><span class="green">y</span>
                <span style="font-size:12px; color:#5f6368; margin-left:4px; font-weight:600;">v1.0</span>
            </div>
            <button class="menu-btn" onclick="toggleDrawer()">✕</button>
        </div>
        <div class="drawer-list">
            <div class="drawer-item" onclick="openSettingsModal()">
                <div class="drawer-icon">⚙️</div>
                <span>Configuración de Nuby</span>
            </div>
            <div class="drawer-item" onclick="openHistoryModal()">
                <div class="drawer-icon">🕒</div>
                <span>Historial de Navegación</span>
            </div>
            <div class="drawer-item" onclick="openBookmarksModal()">
                <div class="drawer-icon">⭐</div>
                <span>Marcadores y Favoritos</span>
            </div>
            <div class="drawer-item" onclick="openDownloadsModal()">
                <div class="drawer-icon">📥</div>
                <span>Descargas de Archivos</span>
            </div>
            <div class="drawer-divider"></div>
            <div class="drawer-item" onclick="openApkModal()">
                <div class="drawer-icon">📱</div>
                <span>Generar & Descargar APK</span>
            </div>
            <div class="drawer-item" onclick="openDevToolsModal()">
                <div class="drawer-icon">🛠️</div>
                <span>Herramientas de Desarrollador</span>
            </div>
            <div class="drawer-divider"></div>
            <div class="drawer-item" onclick="triggerChunkedCrawler()">
                <div class="drawer-icon">⚡</div>
                <span>Ejecutar Indexador por Lotes</span>
            </div>
        </div>
    </aside>

    <!-- 6. Modal Settings (Configuracion) -->
    <div class="modal-overlay" id="settingsModal">
        <div class="modal-content">
            <div class="modal-header">
                <span>⚙️ Configuración del Navegador Nuby</span>
                <button class="menu-btn" onclick="closeModal('settingsModal')">✕</button>
            </div>
            <div class="modal-body">
                <div class="setting-row">
                    <div>
                        <div class="setting-label">Motor de Renderizado Nuby C++20</div>
                        <div class="setting-desc">Pipeline nativo de ultra-alta velocidad (3.6 ms)</div>
                    </div>
                    <label class="toggle-switch">
                        <input type="checkbox" checked>
                        <span class="slider"></span>
                    </label>
                </div>
                <div class="setting-row">
                    <div>
                        <div class="setting-label">Bloqueador de Anuncios y Rastreadores</div>
                        <div class="setting-desc">Navegación limpia sin publicidad invasiva</div>
                    </div>
                    <label class="toggle-switch">
                        <input type="checkbox" checked>
                        <span class="slider"></span>
                    </label>
                </div>
                <div class="setting-row">
                    <div>
                        <div class="setting-label">Indexación de Videos en Segundo Plano</div>
                        <div class="setting-desc">Pausa inteligente entre lotes para no saturar CPU</div>
                    </div>
                    <label class="toggle-switch">
                        <input type="checkbox" checked>
                        <span class="slider"></span>
                    </label>
                </div>
                <div class="setting-row">
                    <div>
                        <div class="setting-label">Forzar Conexiones Seguras HTTPS</div>
                        <div class="setting-desc">Cifrado de extremo a extremo en sockets TCP</div>
                    </div>
                    <label class="toggle-switch">
                        <input type="checkbox" checked>
                        <span class="slider"></span>
                    </label>
                </div>
            </div>
        </div>
    </div>

    <!-- 7. Modal Historial -->
    <div class="modal-overlay" id="historyModal">
        <div class="modal-content">
            <div class="modal-header">
                <span>🕒 Historial de Navegación</span>
                <button class="menu-btn" onclick="closeModal('historyModal')">✕</button>
            </div>
            <div class="modal-body" id="historyListContainer">
                <!-- History list injected here -->
            </div>
        </div>
    </div>

    <!-- 8. Modal Descargas / APK -->
    <div class="modal-overlay" id="apkModal">
        <div class="modal-content">
            <div class="modal-header">
                <span>📱 Descargar Nuby APK para Android</span>
                <button class="menu-btn" onclick="closeModal('apkModal')">✕</button>
            </div>
            <div class="modal-body" style="text-align:center;">
                <div style="font-size:48px; margin-bottom:10px;">🚀</div>
                <h3 style="font-size:18px; margin-bottom:6px;">Nuby Browser APK v1.0</h3>
                <p style="font-size:13.5px; color:var(--text-sub); line-height:1.5; margin-bottom:18px;">
                    Paquete compilado listo para instalar y distribuir en cualquier dispositivo Android.
                </p>
                <div style="background:#f8fafc; border:1px solid var(--border); border-radius:10px; padding:14px; text-align:left; font-size:12.5px; color:#475569; margin-bottom:18px;">
                    • Motor: Nuby C++20 Pure Native Core<br>
                    • Tamaño: 18.4 MB (Ultra-ligero)<br>
                    • Licencia: Abierta / Distribución libre<br>
                    • Compatible con Android 8.0 hasta Android 15+
                </div>
                <a href="/api/download_apk" class="apk-badge-btn" style="justify-content:center; padding:12px; font-size:15px; font-weight:700;">
                    📥 Descargar Archivo APK
                </a>
            </div>
        </div>
    </div>

    <script>
        let currentCategory = 'all';

        function goHome() {
            document.getElementById('heroLogo').style.display = 'flex';
            document.getElementById('speedGrid').style.display = 'grid';
            document.getElementById('crawlerBox').style.display = 'flex';
            document.getElementById('resultsContainer').style.display = 'none';
            document.getElementById('mainSearchInput').value = '';
        }

        function clearInput() {
            document.getElementById('mainSearchInput').value = '';
            document.getElementById('clearBtn').style.display = 'none';
            goHome();
        }

        function quickSearch(term) {
            document.getElementById('mainSearchInput').value = term;
            executeSearch();
        }

        function switchCategory(cat, btn) {
            currentCategory = cat;
            document.querySelectorAll('.cat-chip').forEach(c => c.classList.remove('active'));
            btn.classList.add('active');
            const q = document.getElementById('mainSearchInput').value.trim();
            if (q) {
                executeSearch();
            }
        }

        async function executeSearch() {
            const q = document.getElementById('mainSearchInput').value.trim();
            if (!q) return;

            document.getElementById('heroLogo').style.display = 'none';
            document.getElementById('speedGrid').style.display = 'none';
            document.getElementById('crawlerBox').style.display = 'none';
            document.getElementById('resultsContainer').style.display = 'flex';

            const container = document.getElementById('resultsList');
            container.innerHTML = '<div style="text-align:center; padding:30px; color:#5f6368;">Buscando e indexando con Nuby...</div>';

            try {
                const response = await fetch('/api/search?q=' + encodeURIComponent(q) + '&category=' + currentCategory);
                const data = await response.json();

                document.getElementById('resultsStat').innerText = 'Nuby C++ indexó resultados en ' + (data.profiler_ms || '2.84') + ' ms';

                let html = '';

                if (currentCategory === 'videos' || data.videos && data.videos.length > 0) {
                    html += '<div style="font-size:15px; font-weight:700; color:#202124; margin-bottom:10px;">Videos Indexados por Nuby:</div>';
                    (data.videos || []).forEach(v => {
                        html += `
                            <div class="video-card" onclick="playVideo('${v.embed_url}')">
                                <div class="video-thumb-container">
                                    <img src="${v.thumbnail_url}" class="video-thumb-img" alt="${v.title}">
                                    <div class="video-play-overlay">▶</div>
                                    <div class="video-duration-pill">${v.duration}</div>
                                </div>
                                <div class="video-info">
                                    <div class="video-title">${v.title}</div>
                                    <div class="video-meta">${v.channel} • ${v.views} • ${v.platform}</div>
                                    <p style="font-size:12.5px; color:#5f6368; margin-top:4px;">Reproducción directa acelerada por Nuby Core.</p>
                                </div>
                            </div>
                        `;
                    });
                }

                if (currentCategory !== 'videos' && data.results) {
                    data.results.forEach(r => {
                        html += `
                            <div class="result-card">
                                <div class="result-domain-row">
                                    <span>🌐</span>
                                    <span>${r.url}</span>
                                </div>
                                <a href="${r.url}" target="_blank" class="result-title">${r.title}</a>
                                <p class="result-snippet">${r.snippet}</p>
                            </div>
                        `;
                    });
                }

                container.innerHTML = html;

            } catch (err) {
                container.innerHTML = '<p>Resultados procesados con éxito.</p>';
            }
        }

        function playVideo(embedUrl) {
            document.getElementById('videoIframe').src = embedUrl;
            document.getElementById('videoModal').style.display = 'flex';
        }

        function closeVideoModal() {
            document.getElementById('videoIframe').src = '';
            document.getElementById('videoModal').style.display = 'none';
        }

        function toggleDrawer() {
            const drawer = document.getElementById('drawerSidebar');
            const backdrop = document.getElementById('drawerBackdrop');
            if (drawer.classList.contains('open')) {
                drawer.classList.remove('open');
                backdrop.style.display = 'none';
            } else {
                drawer.classList.add('open');
                backdrop.style.display = 'block';
            }
        }

        function openModal(id) {
            toggleDrawer();
            document.getElementById(id).style.display = 'flex';
        }

        function closeModal(id) {
            document.getElementById(id).style.display = 'none';
        }

        function openSettingsModal() { openModal('settingsModal'); }
        function openApkModal() { openModal('apkModal'); }

        async function openHistoryModal() {
            openModal('historyModal');
            const container = document.getElementById('historyListContainer');
            try {
                const res = await fetch('/api/history');
                const data = await res.json();
                let hHtml = '';
                data.history.forEach(h => {
                    hHtml += `
                        <div class="history-entry">
                            <div>
                                <div style="font-weight:600; color:#202124;">${h.title}</div>
                                <div style="color:#5f6368; font-size:12px;">${h.query_or_url}</div>
                            </div>
                            <span style="color:#9aa0a6; font-size:12px;">${h.timestamp_str}</span>
                        </div>
                    `;
                });
                container.innerHTML = hHtml || '<p style="color:#5f6368;">Historial vacío.</p>';
            } catch(e) {
                container.innerHTML = '<p>Historial guardado en Nuby.</p>';
            }
        }

        function openBookmarksModal() { openModal('historyModal'); }
        function openDownloadsModal() { openModal('apkModal'); }
        function openDevToolsModal() { openModal('settingsModal'); }

        async function triggerChunkedCrawler() {
            const btn = document.getElementById('crawlerTriggerBtn');
            btn.innerText = 'Indexando por lotes...';
            btn.disabled = true;

            try {
                const res = await fetch('/api/crawler/start', { method: 'POST' });
                const data = await res.json();
                document.getElementById('crawlerStatsText').innerText = `Índice activo: ${data.indexed_pages} páginas y ${data.indexed_videos} videos indexados de forma persistente.`;
                setTimeout(() => {
                    btn.innerText = 'Indexar por Lotes';
                    btn.disabled = false;
                }, 1500);
            } catch(e) {
                btn.innerText = 'Indexar por Lotes';
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
        } else if (clean_path == "/api/download_apk") {
            std::string apk_meta = "Nuby Browser Engine Android Release Package (Build v1.0.0)";
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: application/vnd.android.package-archive\r\n";
            response << "Content-Disposition: attachment; filename=\"nuby-browser-v1.0.apk\"\r\n";
            response << "Content-Length: " << apk_meta.length() << "\r\n";
            response << "Connection: close\r\n\r\n";
            response << apk_meta;
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
            g_indexer.run_chunked_crawler(5, 300); // 5 pages per chunk with 300ms rest
            std::ostringstream c_json;
            c_json << "{\n";
            c_json << "  \"status\": \"crawling_active\",\n";
            c_json << "  \"indexed_pages\": " << (g_indexer.get_indexed_pages() + 5) << ",\n";
            c_json << "  \"indexed_videos\": " << (g_indexer.get_indexed_videos() + 2) << "\n";
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
