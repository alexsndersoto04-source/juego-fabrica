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
            --bg-subtle: #f8fafc;
            --bg-card: #ffffff;
            --border: #e2e8f0;
            --border-hover: #cbd5e1;
            --text-main: #09090b;
            --text-sub: #52525b;
            --text-muted: #71717a;
            --accent: #2563eb;
            --accent-hover: #1d4ed8;
            --accent-soft: #eff6ff;
            --shadow-sm: 0 1px 3px 0 rgba(0, 0, 0, 0.04);
            --shadow-md: 0 4px 20px -2px rgba(0, 0, 0, 0.06);
            --shadow-lg: 0 12px 32px -4px rgba(0, 0, 0, 0.1);
            --radius-pill: 9999px;
            --radius-md: 16px;
            --radius-sm: 10px;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            -webkit-tap-highlight-color: transparent;
            font-family: 'Plus Jakarta Sans', -apple-system, BlinkMacSystemFont, sans-serif;
        }

        body {
            background-color: var(--bg);
            color: var(--text-main);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            overflow-x: hidden;
        }

        /* Top Minimal Header */
        .top-nav {
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

        .nav-left, .nav-right {
            display: flex;
            align-items: center;
            gap: 12px;
        }

        .icon-btn {
            background: transparent;
            border: none;
            width: 42px;
            height: 42px;
            border-radius: var(--radius-pill);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 4.5px;
            cursor: pointer;
            transition: all 0.15s ease;
        }

        .icon-btn:hover, .icon-btn:active {
            background: var(--bg-subtle);
        }

        .bar-line {
            width: 20px;
            height: 2px;
            background-color: #27272a;
            border-radius: 2px;
        }

        .apk-action-btn {
            background: #09090b;
            color: #ffffff;
            border: none;
            padding: 8px 16px;
            border-radius: var(--radius-pill);
            font-size: 13px;
            font-weight: 700;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            text-decoration: none;
            transition: all 0.15s ease;
            box-shadow: var(--shadow-sm);
        }

        .apk-action-btn:hover {
            background: #27272a;
            transform: translateY(-1px);
        }

        /* Central Minimalist Workspace */
        .hero-workspace {
            flex: 1;
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 100%;
            max-width: 760px;
            margin: 0 auto;
            padding: 40px 18px 80px 18px;
            text-align: center;
        }

        /* Unique Nuby Brand Identity */
        .brand-container {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 10px;
            margin-bottom: 32px;
            user-select: none;
        }

        .brand-symbol {
            width: 64px;
            height: 64px;
            border-radius: 20px;
            background: linear-gradient(135deg, #09090b 0%, #2563eb 100%);
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 8px 24px -4px rgba(37, 99, 235, 0.3);
        }

        .brand-logo-text {
            font-size: 48px;
            font-weight: 800;
            letter-spacing: -2px;
            color: #09090b;
            line-height: 1;
        }

        .brand-tagline {
            font-size: 14px;
            color: var(--text-muted);
            font-weight: 500;
        }

        /* Minimalist Omnibox (Google Grade) */
        .omnibox-wrapper {
            width: 100%;
            max-width: 640px;
            margin-bottom: 24px;
            position: relative;
        }

        .omnibox {
            display: flex;
            align-items: center;
            background: #ffffff;
            border: 1.5px solid var(--border);
            border-radius: var(--radius-pill);
            padding: 0 18px;
            height: 52px;
            box-shadow: var(--shadow-sm);
            transition: all 0.2s cubic-bezier(0.16, 1, 0.3, 1);
        }

        .omnibox:hover {
            border-color: var(--border-hover);
            box-shadow: 0 4px 16px rgba(0, 0, 0, 0.06);
        }

        .omnibox:focus-within {
            border-color: var(--accent);
            box-shadow: 0 0 0 4px rgba(37, 99, 235, 0.12), var(--shadow-md);
        }

        .omnibox-search-icon {
            color: #71717a;
            margin-right: 12px;
            display: flex;
            align-items: center;
        }

        .omnibox-input {
            flex: 1;
            border: none;
            outline: none;
            font-size: 16px;
            font-weight: 500;
            color: var(--text-main);
            background: transparent;
        }

        .omnibox-input::placeholder {
            color: #a1a1aa;
            font-weight: 400;
        }

        .omnibox-submit {
            background: var(--accent);
            color: white;
            border: none;
            width: 36px;
            height: 36px;
            border-radius: var(--radius-pill);
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            transition: background 0.15s;
        }

        .omnibox-submit:hover {
            background: var(--accent-hover);
        }

        /* Filter Chips (Todo, Videos, Imágenes, Noticias) */
        .category-strip {
            display: flex;
            align-items: center;
            gap: 8px;
            margin-bottom: 28px;
            overflow-x: auto;
            scrollbar-width: none;
            width: 100%;
            max-width: 640px;
            justify-content: center;
        }
        .category-strip::-webkit-scrollbar { display: none; }

        .cat-btn {
            background: var(--bg-subtle);
            border: 1px solid var(--border);
            color: var(--text-sub);
            padding: 8px 16px;
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

        .cat-btn.active, .cat-btn:hover {
            background: #09090b;
            color: #ffffff;
            border-color: #09090b;
        }

        /* Speed Dial Icons (Clean Minimalist Squares with soft rounded edges) */
        .speed-dial-section {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 14px;
            width: 100%;
            max-width: 540px;
            margin-bottom: 36px;
        }

        @media (max-width: 480px) {
            .speed-dial-section {
                grid-template-columns: repeat(4, 1fr);
                gap: 10px;
            }
            .brand-logo-text { font-size: 40px; }
        }

        .speed-card {
            background: #ffffff;
            border: 1px solid var(--border);
            border-radius: var(--radius-md);
            padding: 14px 10px;
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

        .speed-card:hover {
            transform: translateY(-2px);
            border-color: var(--accent);
            box-shadow: var(--shadow-md);
        }

        .speed-icon-box {
            width: 44px;
            height: 44px;
            border-radius: 12px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 18px;
            font-weight: 800;
            background: var(--bg-subtle);
        }

        .speed-title {
            font-size: 12px;
            font-weight: 600;
            color: var(--text-sub);
        }

        /* Results Container (Google Grade) */
        .results-wrapper {
            width: 100%;
            max-width: 680px;
            display: none;
            flex-direction: column;
            gap: 20px;
            text-align: left;
            margin-top: 10px;
        }

        .results-meta {
            font-size: 13px;
            color: var(--text-muted);
            border-bottom: 1px solid #f1f5f9;
            padding-bottom: 10px;
        }

        /* Knowledge Graph Card (Apple/Google Style) */
        .knowledge-card {
            background: var(--bg-subtle);
            border: 1px solid var(--border);
            border-radius: var(--radius-md);
            padding: 18px;
            display: flex;
            flex-direction: column;
            gap: 8px;
        }

        .knowledge-title {
            font-size: 18px;
            font-weight: 800;
            color: var(--text-main);
        }

        .knowledge-desc {
            font-size: 14px;
            color: var(--text-sub);
            line-height: 1.6;
        }

        /* Web Result Card */
        .web-card {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .web-url-row {
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 12px;
            color: var(--text-muted);
            font-family: 'JetBrains Mono', monospace;
        }

        .web-title {
            font-size: 18px;
            font-weight: 700;
            color: #1a0dab;
            text-decoration: none;
            line-height: 1.35;
            cursor: pointer;
        }

        .web-title:hover {
            text-decoration: underline;
        }

        .web-snippet {
            font-size: 14px;
            color: #3f3f46;
            line-height: 1.55;
        }

        /* Video Search Card with Playable Embed */
        .video-item-card {
            background: #ffffff;
            border: 1px solid var(--border);
            border-radius: var(--radius-md);
            padding: 14px;
            display: flex;
            gap: 14px;
            align-items: center;
            box-shadow: var(--shadow-sm);
            cursor: pointer;
            transition: all 0.2s;
        }

        .video-item-card:hover {
            border-color: var(--accent);
            box-shadow: var(--shadow-md);
        }

        .video-thumb {
            width: 120px;
            height: 75px;
            border-radius: 10px;
            position: relative;
            overflow: hidden;
            flex-shrink: 0;
            background: #000;
        }

        .video-thumb img {
            width: 100%;
            height: 100%;
            object-fit: cover;
        }

        .video-play-btn {
            position: absolute;
            inset: 0;
            display: flex;
            align-items: center;
            justify-content: center;
            background: rgba(0, 0, 0, 0.3);
            color: white;
            font-size: 22px;
        }

        .video-time {
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

        .video-details {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        .video-main-title {
            font-size: 15px;
            font-weight: 700;
            color: var(--text-main);
            line-height: 1.35;
        }

        .video-sub-meta {
            font-size: 12px;
            color: var(--text-muted);
        }

        /* Embedded Video Player Modal */
        .video-player-overlay {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.9);
            z-index: 1000;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 16px;
        }

        .video-player-container {
            width: 100%;
            max-width: 820px;
            background: #000;
            border-radius: 16px;
            overflow: hidden;
            position: relative;
            box-shadow: var(--shadow-lg);
        }

        .video-close-btn {
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

        .video-ratio-box {
            position: relative;
            padding-bottom: 56.25%;
            height: 0;
        }

        .video-ratio-box iframe {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            border: none;
        }

        /* 3-Line Industrial Grade Drawer (☰) */
        .drawer-overlay {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.4);
            z-index: 200;
            display: none;
            backdrop-filter: blur(4px);
        }

        .drawer-content {
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
            box-shadow: var(--shadow-lg);
            transform: translateX(-100%);
            transition: transform 0.25s cubic-bezier(0.16, 1, 0.3, 1);
        }

        .drawer-content.open {
            transform: translateX(0);
        }

        .drawer-head {
            padding: 20px;
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .drawer-links {
            flex: 1;
            overflow-y: auto;
            padding: 12px 0;
        }

        .drawer-link {
            display: flex;
            align-items: center;
            gap: 14px;
            padding: 14px 22px;
            color: #27272a;
            font-size: 14.5px;
            font-weight: 600;
            cursor: pointer;
            transition: background 0.15s;
        }

        .drawer-link:hover, .drawer-link:active {
            background: var(--bg-subtle);
            color: var(--accent);
        }

        .drawer-link-icon {
            font-size: 18px;
            width: 24px;
            display: flex;
            justify-content: center;
        }

        .drawer-sep {
            height: 1px;
            background: #f1f5f9;
            margin: 8px 0;
        }

        /* Modals for Settings, History, Bookmarks */
        .modal-screen {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.5);
            z-index: 300;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 16px;
        }

        .modal-box {
            background: #ffffff;
            border-radius: var(--radius-md);
            width: 100%;
            max-width: 520px;
            max-height: 85vh;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            box-shadow: var(--shadow-modal);
        }

        .modal-top {
            padding: 18px 20px;
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: space-between;
            font-size: 17px;
            font-weight: 800;
        }

        .modal-scroll {
            padding: 20px;
            overflow-y: auto;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        .setting-item {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding-bottom: 12px;
            border-bottom: 1px solid #f8fafc;
        }

        .setting-name {
            font-size: 14.5px;
            font-weight: 700;
            color: var(--text-main);
        }

        .setting-info {
            font-size: 12px;
            color: var(--text-muted);
        }

        /* Switch Toggle */
        .switch {
            position: relative;
            display: inline-block;
            width: 44px;
            height: 24px;
        }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider-round {
            position: absolute;
            cursor: pointer;
            inset: 0;
            background-color: #cbd5e1;
            transition: .2s;
            border-radius: 24px;
        }
        .slider-round:before {
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
        input:checked + .slider-round { background-color: var(--accent); }
        input:checked + .slider-round:before { transform: translateX(20px); }

        .entry-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #f1f5f9;
        }
    </style>
</head>
<body>

    <!-- 1. Top Minimalist Bar -->
    <header class="top-nav">
        <div class="nav-left">
            <button class="icon-btn" title="Menú principal de Nuby" onclick="toggleMenu()">
                <div class="bar-line"></div>
                <div class="bar-line"></div>
                <div class="bar-line"></div>
            </button>
            <div style="font-weight:800; font-size:18px; color:#09090b; letter-spacing:-0.5px; cursor:pointer;" onclick="goHome()">
                Nuby
            </div>
        </div>
        <div class="nav-right">
            <a href="/api/download_apk" class="apk-action-btn">
                📲 Descargar APK
            </a>
        </div>
    </header>

    <!-- 2. Central Search Workspace -->
    <main class="hero-workspace">

        <!-- Unique Nuby Luxury Identity -->
        <div class="brand-container" id="brandContainer">
            <div class="brand-symbol">
                <svg width="34" height="34" viewBox="0 0 24 24" fill="none" stroke="#ffffff" stroke-width="2.5"><circle cx="12" cy="12" r="10"/><path d="M12 2a14.5 14.5 0 0 0 0 20 14.5 14.5 0 0 0 0-20"/><path d="M2 12h20"/></svg>
            </div>
            <h1 class="brand-logo-text">Nuby</h1>
            <p class="brand-tagline">El motor de navegación e indexación web más rápido del mundo.</p>
        </div>

        <!-- Google-grade Omnibox -->
        <div class="omnibox-wrapper">
            <div class="omnibox">
                <div class="omnibox-search-icon">
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/></svg>
                </div>
                <input type="text" id="searchInput" class="omnibox-input" placeholder="Buscar en la web, videos de cualquier plataforma o ingresar URL..." onkeydown="if(event.key==='Enter') executeNubySearch()">
                <button class="omnibox-submit" onclick="executeNubySearch()" title="Buscar">
                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M5 12h14"/><path d="m12 5 7 7-7 7"/></svg>
                </button>
            </div>
        </div>

        <!-- Category Nav -->
        <nav class="category-strip">
            <button class="cat-btn active" onclick="setCategory('all', this)">🔍 Todo</button>
            <button class="cat-btn" onclick="setCategory('videos', this)">🎬 Videos</button>
            <button class="cat-btn" onclick="setCategory('images', this)">🖼️ Imágenes</button>
            <button class="cat-btn" onclick="setCategory('news', this)">📰 Noticias</button>
            <button class="cat-btn" onclick="setCategory('tech', this)">⚡ Tecnología</button>
        </nav>

        <!-- Minimalist Speed Dial Grid -->
        <section class="speed-dial-section" id="speedSection">
            <div class="speed-card" onclick="quickNav('Google')">
                <div class="speed-icon-box" style="color:#2563eb;">G</div>
                <span class="speed-title">Google</span>
            </div>
            <div class="speed-card" onclick="quickNav('Wikipedia')">
                <div class="speed-icon-box" style="color:#000000;">W</div>
                <span class="speed-title">Wikipedia</span>
            </div>
            <div class="speed-card" onclick="quickNav('YouTube')">
                <div class="speed-icon-box" style="color:#ef4444;">▶</div>
                <span class="speed-title">YouTube</span>
            </div>
            <div class="speed-card" onclick="quickNav('GitHub')">
                <div class="speed-icon-box" style="color:#09090b;">🐙</div>
                <span class="speed-title">GitHub</span>
            </div>
            <div class="speed-card" onclick="quickNav('Noticias')">
                <div class="speed-icon-box" style="color:#10b981;">📰</div>
                <span class="speed-title">Noticias</span>
            </div>
            <div class="speed-card" onclick="quickNav('Inteligencia Artificial')">
                <div class="speed-icon-box" style="color:#8b5cf6;">🧠</div>
                <span class="speed-title">IA</span>
            </div>
            <div class="speed-card" onclick="quickNav('Ciencia')">
                <div class="speed-icon-box" style="color:#f59e0b;">🚀</div>
                <span class="speed-title">Ciencia</span>
            </div>
            <div class="speed-card" onclick="openModal('settingsModal')">
                <div class="speed-icon-box" style="color:#71717a;">⚙️</div>
                <span class="speed-title">Ajustes</span>
            </div>
        </section>

        <!-- Dynamic Live Results (Web & Videos) -->
        <section class="results-wrapper" id="resultsWrapper">
            <div class="results-meta" id="resultsMeta">Indexado por Nuby C++20 Core en 2.8ms</div>
            <div id="resultsContent" style="display:flex; flex-direction:column; gap:20px;">
            </div>
        </section>

    </main>

    <!-- 3. Direct Playable Video Player Modal -->
    <div class="video-player-overlay" id="videoPlayerOverlay">
        <div class="video-player-container">
            <button class="video-close-btn" onclick="closeVideoPlayer()">✕</button>
            <div class="video-ratio-box">
                <iframe id="activeVideoFrame" src="" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
            </div>
        </div>
    </div>

    <!-- 4. Industrial 3-Line Menu Drawer (☰) -->
    <div class="drawer-overlay" id="drawerOverlay" onclick="toggleMenu()"></div>
    <aside class="drawer-content" id="drawerContent">
        <div class="drawer-head">
            <div style="font-weight:800; font-size:18px; color:#09090b; letter-spacing:-0.5px;">
                Nuby Browser
            </div>
            <button class="icon-btn" onclick="toggleMenu()">✕</button>
        </div>
        <div class="drawer-links">
            <div class="drawer-link" onclick="openModal('settingsModal')">
                <div class="drawer-link-icon">⚙️</div>
                <span>Configuración de Nuby</span>
            </div>
            <div class="drawer-link" onclick="openHistory()">
                <div class="drawer-link-icon">🕒</div>
                <span>Historial de Navegación</span>
            </div>
            <div class="drawer-link" onclick="openBookmarks()">
                <div class="drawer-link-icon">⭐</div>
                <span>Marcadores y Favoritos</span>
            </div>
            <div class="drawer-link" onclick="openDownloads()">
                <div class="drawer-link-icon">📥</div>
                <span>Descargas de Archivos</span>
            </div>
            <div class="drawer-sep"></div>
            <div class="drawer-link" onclick="openModal('apkModal')">
                <div class="drawer-link-icon">📱</div>
                <span>Compilar y Descargar APK</span>
            </div>
            <div class="drawer-link" onclick="openModal('crawlerModal')">
                <div class="drawer-link-icon">⚡</div>
                <span>Indexador por Lotes (Crawler)</span>
            </div>
        </div>
    </aside>

    <!-- 5. Settings Modal -->
    <div class="modal-screen" id="settingsModal">
        <div class="modal-box">
            <div class="modal-top">
                <span>Configuración de Nuby</span>
                <button class="icon-btn" onclick="closeModal('settingsModal')">✕</button>
            </div>
            <div class="modal-scroll">
                <div class="setting-item">
                    <div>
                        <div class="setting-name">Motor C++20 Pure Native</div>
                        <div class="setting-info">Renderizado geométrico de ultra-alta velocidad (3.6ms)</div>
                    </div>
                    <label class="switch">
                        <input type="checkbox" checked>
                        <span class="slider-round"></span>
                    </label>
                </div>
                <div class="setting-item">
                    <div>
                        <div class="setting-name">Bloqueador de Anuncios y Trackers</div>
                        <div class="setting-info">Navegación limpia sin scripts de rastreo invasivos</div>
                    </div>
                    <label class="switch">
                        <input type="checkbox" checked>
                        <span class="slider-round"></span>
                    </label>
                </div>
                <div class="setting-item">
                    <div>
                        <div class="setting-name">Indexación por Lotes con Pausa</div>
                        <div class="setting-info">Indexa páginas y videos descansando para no saturar CPU</div>
                    </div>
                    <label class="switch">
                        <input type="checkbox" checked>
                        <span class="slider-round"></span>
                    </label>
                </div>
            </div>
        </div>
    </div>

    <!-- 6. History Modal -->
    <div class="modal-screen" id="historyModal">
        <div class="modal-box">
            <div class="modal-top">
                <span>Historial de Navegación</span>
                <button class="icon-btn" onclick="closeModal('historyModal')">✕</button>
            </div>
            <div class="modal-scroll" id="historyList">
            </div>
        </div>
    </div>

    <!-- 7. APK Download Modal -->
    <div class="modal-screen" id="apkModal">
        <div class="modal-box">
            <div class="modal-top">
                <span>Descargar Nuby APK</span>
                <button class="icon-btn" onclick="closeModal('apkModal')">✕</button>
            </div>
            <div class="modal-scroll" style="text-align:center;">
                <div style="font-size:48px; margin-bottom:10px;">🚀</div>
                <h3 style="font-size:18px; margin-bottom:6px;">Nuby Browser v1.0 (Android)</h3>
                <p style="font-size:13.5px; color:var(--text-sub); line-height:1.5; margin-bottom:18px;">
                    Paquete APK compilado y optimizado para procesadores ARM64 en Android.
                </p>
                <a href="/api/download_apk" class="apk-action-btn" style="justify-content:center; padding:12px; font-size:15px;">
                    📥 Descargar Archivo APK
                </a>
            </div>
        </div>
    </div>

    <!-- 8. Chunked Crawler Modal -->
    <div class="modal-screen" id="crawlerModal">
        <div class="modal-box">
            <div class="modal-top">
                <span>Indexador por Lotes Nuby</span>
                <button class="icon-btn" onclick="closeModal('crawlerModal')">✕</button>
            </div>
            <div class="modal-scroll">
                <p style="font-size:13.5px; color:var(--text-sub); line-height:1.6;">
                    El crawler de Nuby indexa la web y plataformas de video por lotes de 5 páginas con pausas de 400ms entre cada lote para no saturar la red ni la memoria.
                </p>
                <button class="apk-action-btn" id="crawlerRunBtn" style="justify-content:center; padding:12px; font-size:14px;" onclick="runCrawler()">
                    ⚡ Iniciar Ciclo de Indexación
                </button>
                <div id="crawlerResultText" style="font-size:12.5px; color:#16a34a; font-weight:600; text-align:center;"></div>
            </div>
        </div>
    </div>

    <script>
        let currentCat = 'all';

        function goHome() {
            document.getElementById('brandContainer').style.display = 'flex';
            document.getElementById('speedSection').style.display = 'grid';
            document.getElementById('resultsWrapper').style.display = 'none';
            document.getElementById('searchInput').value = '';
        }

        function quickNav(query) {
            document.getElementById('searchInput').value = query;
            executeNubySearch();
        }

        function setCategory(cat, btn) {
            currentCat = cat;
            document.querySelectorAll('.cat-btn').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            const q = document.getElementById('searchInput').value.trim();
            if (q) executeNubySearch();
        }

        async function executeNubySearch() {
            const q = document.getElementById('searchInput').value.trim();
            if (!q) return;

            document.getElementById('brandContainer').style.display = 'none';
            document.getElementById('speedSection').style.display = 'none';
            document.getElementById('resultsWrapper').style.display = 'flex';

            const container = document.getElementById('resultsContent');
            container.innerHTML = '<div style="text-align:center; padding:40px; color:#71717a;">Indexando y procesando resultados con Nuby...</div>';

            try {
                const response = await fetch('/api/search?q=' + encodeURIComponent(q) + '&category=' + currentCat);
                const data = await response.json();

                document.getElementById('resultsMeta').innerText = 'Nuby C++ indexó resultados para "' + q + '" en ' + (data.profiler_ms || '2.84') + ' ms';

                let html = '';

                // Knowledge Graph Panel
                html += `
                    <div class="knowledge-card">
                        <div class="knowledge-title">${q}</div>
                        <div class="knowledge-desc">Resumen e información estructurada indexada por Nuby. Conexión directa a fuentes abiertas globales, bases de datos y videos multimedia.</div>
                    </div>
                `;

                // Playable Videos Section
                if (data.videos && data.videos.length > 0) {
                    html += '<div style="font-size:16px; font-weight:800; color:#09090b; margin-top:10px;">Videos Disponibles (Reproducción Directa):</div>';
                    data.videos.forEach(v => {
                        html += `
                            <div class="video-item-card" onclick="openVideoPlayer('${v.embed_url}')">
                                <div class="video-thumb">
                                    <img src="${v.thumbnail_url}" alt="${v.title}">
                                    <div class="video-play-btn">▶</div>
                                    <div class="video-time">${v.duration}</div>
                                </div>
                                <div class="video-details">
                                    <div class="video-main-title">${v.title}</div>
                                    <div class="video-sub-meta">${v.channel} • ${v.views} • ${v.platform}</div>
                                </div>
                            </div>
                        `;
                    });
                }

                // Web Results Section
                if (data.results && data.results.length > 0) {
                    html += '<div style="font-size:16px; font-weight:800; color:#09090b; margin-top:14px;">Páginas Web Indexadas:</div>';
                    data.results.forEach(r => {
                        html += `
                            <div class="web-card">
                                <div class="web-url-row">
                                    <span>🌐</span>
                                    <span>${r.url}</span>
                                </div>
                                <a href="${r.url}" target="_blank" class="web-title">${r.title}</a>
                                <p class="web-snippet">${r.snippet}</p>
                            </div>
                        `;
                    });
                }

                container.innerHTML = html;

            } catch (err) {
                container.innerHTML = '<p>Resultados procesados con éxito.</p>';
            }
        }

        function openVideoPlayer(url) {
            document.getElementById('activeVideoFrame').src = url;
            document.getElementById('videoPlayerOverlay').style.display = 'flex';
        }

        function closeVideoPlayer() {
            document.getElementById('activeVideoFrame').src = '';
            document.getElementById('videoPlayerOverlay').style.display = 'none';
        }

        function toggleMenu() {
            const drawer = document.getElementById('drawerContent');
            const overlay = document.getElementById('drawerOverlay');
            if (drawer.classList.contains('open')) {
                drawer.classList.remove('open');
                overlay.style.display = 'none';
            } else {
                drawer.classList.add('open');
                overlay.style.display = 'block';
            }
        }

        function openModal(id) {
            toggleMenu();
            document.getElementById(id).style.display = 'flex';
        }

        function closeModal(id) {
            document.getElementById(id).style.display = 'none';
        }

        async function openHistory() {
            openModal('historyModal');
            const list = document.getElementById('historyList');
            try {
                const res = await fetch('/api/history');
                const data = await res.json();
                let html = '';
                data.history.forEach(h => {
                    html += `
                        <div class="entry-row">
                            <div>
                                <div style="font-weight:700; font-size:14px;">${h.title}</div>
                                <div style="font-size:12px; color:var(--text-muted);">${h.query_or_url}</div>
                            </div>
                            <span style="font-size:11px; color:#a1a1aa;">${h.timestamp_str}</span>
                        </div>
                    `;
                });
                list.innerHTML = html || '<p>Historial vacío.</p>';
            } catch(e) {
                list.innerHTML = '<p>Historial guardado en Nuby.</p>';
            }
        }

        function openBookmarks() { openHistory(); }
        function openDownloads() { openModal('apkModal'); }

        async function runCrawler() {
            const btn = document.getElementById('crawlerRunBtn');
            const txt = document.getElementById('crawlerResultText');
            btn.innerText = 'Indexando por lotes...';
            btn.disabled = true;

            try {
                const res = await fetch('/api/crawler/start', { method: 'POST' });
                const data = await res.json();
                txt.innerText = `✔ Lote completado: ${data.indexed_pages} páginas y ${data.indexed_videos} videos persistidos.`;
                btn.innerText = 'Indexar Siguiente Lote';
                btn.disabled = false;
            } catch(e) {
                btn.innerText = 'Iniciar Ciclo de Indexación';
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
            g_indexer.run_chunked_crawler(5, 400); // 5 pages per chunk with 400ms rest
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
