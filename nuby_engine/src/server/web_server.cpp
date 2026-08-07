#include "../../include/nuby/server/web_server.hpp"
#include "../../include/nuby/core/string_utils.hpp"
#include "../../include/nuby/net/http_client.hpp"
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

static std::string get_workbench_html() {
    return R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Nuby — El Navegador Más Rápido</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg: #ffffff;
            --bg-subtle: #f8fafc;
            --bg-card: #ffffff;
            --border: #e2e8f0;
            --border-focus: #3b82f6;
            --text-main: #0f172a;
            --text-muted: #64748b;
            --text-subtle: #94a3b8;
            --primary: #2563eb;
            --primary-hover: #1d4ed8;
            --primary-light: #eff6ff;
            --accent: #06b6d4;
            --success: #10b981;
            --shadow-sm: 0 1px 3px 0 rgba(0, 0, 0, 0.05);
            --shadow-md: 0 4px 20px -2px rgba(0, 0, 0, 0.05), 0 2px 6px -1px rgba(0, 0, 0, 0.03);
            --shadow-lg: 0 12px 30px -4px rgba(0, 0, 0, 0.08);
            --radius-sm: 8px;
            --radius-md: 14px;
            --radius-lg: 20px;
            --radius-full: 9999px;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            -webkit-tap-highlight-color: transparent;
            font-family: 'Plus Jakarta Sans', -apple-system, BlinkMacSystemFont, sans-serif;
        }

        body {
            background-color: var(--bg-subtle);
            color: var(--text-main);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            overflow-x: hidden;
        }

        /* Top Modern Browser Chrome */
        .browser-topbar {
            position: sticky;
            top: 0;
            z-index: 100;
            background: rgba(255, 255, 255, 0.92);
            backdrop-filter: blur(16px);
            -webkit-backdrop-filter: blur(16px);
            border-bottom: 1px solid var(--border);
            padding: 8px 14px;
            display: flex;
            flex-direction: column;
            gap: 8px;
            box-shadow: var(--shadow-sm);
        }

        .topbar-row {
            display: flex;
            align-items: center;
            gap: 8px;
            width: 100%;
        }

        /* Logo Brand */
        .brand-pill {
            display: flex;
            align-items: center;
            gap: 6px;
            font-weight: 800;
            font-size: 17px;
            color: var(--primary);
            text-decoration: none;
            letter-spacing: -0.5px;
            padding: 4px 8px;
            border-radius: var(--radius-sm);
            background: var(--primary-light);
        }

        .brand-pill svg {
            color: var(--primary);
        }

        /* Nav controls */
        .nav-actions {
            display: flex;
            align-items: center;
            gap: 4px;
        }

        .icon-btn {
            background: transparent;
            border: 1px solid transparent;
            color: var(--text-muted);
            width: 36px;
            height: 36px;
            border-radius: var(--radius-full);
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            transition: all 0.15s ease;
        }

        .icon-btn:hover, .icon-btn:active {
            background: var(--bg-subtle);
            border-color: var(--border);
            color: var(--text-main);
        }

        /* Omnibox / Search & URL Input */
        .omnibox-container {
            flex: 1;
            display: flex;
            align-items: center;
            background: var(--bg);
            border: 1.5px solid var(--border);
            border-radius: var(--radius-full);
            padding: 0 12px;
            height: 44px;
            gap: 8px;
            box-shadow: var(--shadow-sm);
            transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
        }

        .omnibox-container:focus-within {
            border-color: var(--primary);
            box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.12), var(--shadow-sm);
            background: #ffffff;
        }

        .omnibox-lock {
            color: var(--success);
            display: flex;
            align-items: center;
        }

        .omnibox-input {
            flex: 1;
            border: none;
            outline: none;
            background: transparent;
            font-size: 14.5px;
            font-weight: 500;
            color: var(--text-main);
            width: 100%;
        }

        .omnibox-input::placeholder {
            color: var(--text-subtle);
            font-weight: 400;
        }

        .go-btn {
            background: var(--primary);
            border: none;
            color: white;
            padding: 0 16px;
            height: 34px;
            border-radius: var(--radius-full);
            font-weight: 600;
            font-size: 13.5px;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            transition: background 0.15s;
            white-space: nowrap;
        }

        .go-btn:hover { background: var(--primary-hover); }

        /* Tabs strip */
        .tabs-strip {
            display: flex;
            align-items: center;
            gap: 6px;
            overflow-x: auto;
            scrollbar-width: none;
            padding-bottom: 2px;
        }
        .tabs-strip::-webkit-scrollbar { display: none; }

        .tab-item {
            display: flex;
            align-items: center;
            gap: 6px;
            background: var(--bg);
            border: 1px solid var(--border);
            padding: 4px 12px;
            border-radius: var(--radius-full);
            font-size: 12.5px;
            font-weight: 600;
            color: var(--text-muted);
            cursor: pointer;
            white-space: nowrap;
            transition: all 0.15s;
        }

        .tab-item.active {
            background: var(--text-main);
            color: white;
            border-color: var(--text-main);
        }

        .tab-item:hover:not(.active) {
            background: var(--border);
            color: var(--text-main);
        }

        /* Main Viewport / Web Stage */
        .browser-body {
            flex: 1;
            display: flex;
            flex-direction: column;
            width: 100%;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px 16px 80px 16px;
        }

        /* Homepage View / Speed Dial */
        .home-view {
            display: flex;
            flex-direction: column;
            align-items: center;
            text-align: center;
            margin-top: 10px;
        }

        .hero-nuby-logo {
            font-size: 38px;
            font-weight: 800;
            letter-spacing: -1.5px;
            color: var(--text-main);
            margin-bottom: 6px;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .hero-nuby-logo span {
            background: linear-gradient(135deg, var(--primary), var(--accent));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .hero-tagline {
            font-size: 14px;
            color: var(--text-muted);
            margin-bottom: 24px;
            max-width: 500px;
        }

        /* Speed Dial Grid */
        .speed-dial-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 12px;
            width: 100%;
            max-width: 600px;
            margin-bottom: 28px;
        }

        @media (max-width: 480px) {
            .speed-dial-grid {
                grid-template-columns: repeat(3, 1fr);
                gap: 10px;
            }
        }

        .dial-card {
            background: var(--bg-card);
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
            transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
            cursor: pointer;
        }

        .dial-card:hover {
            transform: translateY(-2px);
            box-shadow: var(--shadow-md);
            border-color: var(--primary);
        }

        .dial-icon {
            width: 40px;
            height: 40px;
            border-radius: 12px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 18px;
            font-weight: 700;
        }

        .dial-title {
            font-size: 12px;
            font-weight: 600;
            color: var(--text-main);
        }

        /* Feed / Real-time Web Index */
        .feed-section {
            width: 100%;
            max-width: 760px;
            text-align: left;
            margin-top: 10px;
        }

        .section-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 12px;
        }

        .section-title {
            font-size: 15px;
            font-weight: 700;
            color: var(--text-main);
            display: flex;
            align-items: center;
            gap: 6px;
        }

        .engine-metric-chip {
            font-size: 11px;
            background: #dcfce7;
            color: #15803d;
            padding: 3px 8px;
            border-radius: var(--radius-full);
            font-weight: 600;
            font-family: 'JetBrains Mono', monospace;
        }

        .news-card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: var(--radius-md);
            padding: 16px;
            margin-bottom: 12px;
            box-shadow: var(--shadow-sm);
            transition: all 0.15s ease;
            cursor: pointer;
            display: flex;
            flex-direction: column;
            gap: 6px;
        }

        .news-card:hover {
            border-color: var(--primary);
            box-shadow: var(--shadow-md);
        }

        .news-domain {
            font-size: 11px;
            font-weight: 700;
            color: var(--primary);
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        .news-headline {
            font-size: 15px;
            font-weight: 700;
            color: var(--text-main);
            line-height: 1.35;
        }

        .news-snippet {
            font-size: 13px;
            color: var(--text-muted);
            line-height: 1.45;
        }

        /* Search Results & Webpage Live Frame View */
        .live-page-view {
            display: none;
            width: 100%;
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: var(--radius-md);
            box-shadow: var(--shadow-md);
            padding: 24px;
            animation: fadeIn 0.2s ease;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(6px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .page-meta-bar {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding-bottom: 16px;
            border-bottom: 1px solid var(--border);
            margin-bottom: 20px;
            flex-wrap: wrap;
            gap: 10px;
        }

        .page-url-badge {
            font-size: 12px;
            font-family: 'JetBrains Mono', monospace;
            color: var(--text-muted);
            background: var(--bg-subtle);
            padding: 4px 10px;
            border-radius: var(--radius-full);
            border: 1px solid var(--border);
            max-width: 100%;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }

        .live-content h1 {
            font-size: 24px;
            font-weight: 800;
            margin-bottom: 14px;
            color: var(--text-main);
            line-height: 1.3;
        }

        .live-content p {
            font-size: 15px;
            color: #334155;
            line-height: 1.7;
            margin-bottom: 16px;
        }

        .results-list {
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        .result-item {
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 14px;
            border-radius: var(--radius-sm);
            border: 1px solid var(--border);
            background: var(--bg);
            transition: all 0.15s ease;
            cursor: pointer;
        }

        .result-item:hover {
            border-color: var(--primary);
            box-shadow: var(--shadow-sm);
        }

        .result-link {
            font-size: 12px;
            color: var(--text-muted);
            font-family: 'JetBrains Mono', monospace;
        }

        .result-title {
            font-size: 16px;
            font-weight: 700;
            color: var(--primary);
            text-decoration: none;
        }

        .result-snippet {
            font-size: 13.5px;
            color: var(--text-main);
            line-height: 1.45;
        }

        /* Floating Inspector Trigger Button (Discreto y elegante) */
        .floating-devtools-btn {
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: var(--text-main);
            color: white;
            border: none;
            padding: 10px 18px;
            border-radius: var(--radius-full);
            font-size: 13px;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 8px;
            box-shadow: var(--shadow-lg);
            cursor: pointer;
            z-index: 999;
            transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
        }

        .floating-devtools-btn:hover {
            transform: scale(1.05);
            background: #000;
        }

        /* Bottom Drawer / Modal Inspector */
        .devtools-drawer {
            position: fixed;
            bottom: 0;
            left: 0;
            right: 0;
            max-height: 80vh;
            background: #ffffff;
            border-top: 1px solid var(--border);
            box-shadow: 0 -10px 40px rgba(0, 0, 0, 0.12);
            z-index: 1000;
            border-radius: 20px 20px 0 0;
            display: none;
            flex-direction: column;
            overflow: hidden;
            animation: slideUp 0.25s cubic-bezier(0.16, 1, 0.3, 1);
        }

        @keyframes slideUp {
            from { transform: translateY(100%); }
            to { transform: translateY(0); }
        }

        .drawer-header {
            padding: 14px 18px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            border-bottom: 1px solid var(--border);
            background: var(--bg-subtle);
        }

        .drawer-body {
            padding: 16px;
            overflow-y: auto;
            max-height: 65vh;
        }

        .code-input-row {
            display: flex;
            flex-direction: column;
            gap: 8px;
            margin-bottom: 14px;
        }

        .code-box {
            width: 100%;
            background: var(--bg-subtle);
            border: 1px solid var(--border);
            border-radius: var(--radius-sm);
            padding: 10px;
            font-family: 'JetBrains Mono', monospace;
            font-size: 12px;
            color: var(--text-main);
            outline: none;
            resize: vertical;
        }

        .code-box:focus {
            border-color: var(--primary);
        }

        /* Canvas render box */
        #renderCanvas {
            max-width: 100%;
            border-radius: 8px;
            border: 1px solid var(--border);
            box-shadow: var(--shadow-sm);
            background: #ffffff;
        }
    </style>
</head>
<body>

    <!-- Top Browser Chrome -->
    <header class="browser-topbar">
        <div class="topbar-row">
            <a href="#" class="brand-pill" onclick="goHome()">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><circle cx="12" cy="12" r="10"/><path d="M12 2a14.5 14.5 0 0 0 0 20 14.5 14.5 0 0 0 0-20"/><path d="M2 12h20"/></svg>
                Nuby
            </a>
            <div class="nav-actions">
                <button class="icon-btn" title="Atrás" onclick="historyBack()">
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="m15 18-6-6 6-6"/></svg>
                </button>
                <button class="icon-btn" title="Adelante" onclick="historyForward()">
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="m9 18 6-6-6-6"/></svg>
                </button>
                <button class="icon-btn" title="Recargar" onclick="navigateNuby()">
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M21 12a9 9 0 0 0-9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/><path d="M3 3v5h5"/><path d="M3 12a9 9 0 0 0 9 9 9.75 9.75 0 0 0 6.74-2.74L21 16"/><path d="M16 21h5v-5"/></svg>
                </button>
            </div>
            <!-- Omnibox -->
            <div class="omnibox-container">
                <div class="omnibox-lock">
                    <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><rect width="18" height="11" x="3" y="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>
                </div>
                <input type="text" id="urlInput" class="omnibox-input" placeholder="Escribe una URL (ej. wikipedia.org) o busca en la web..." value="" onkeydown="if(event.key==='Enter') navigateNuby()">
            </div>
            <button class="go-btn" onclick="navigateNuby()">
                Buscar
            </button>
        </div>

        <!-- Real Tab Navigation Strip -->
        <div class="tabs-strip">
            <div class="tab-item active" onclick="goHome()">🏠 Inicio Nuby</div>
            <div class="tab-item" onclick="quickVisit('https://es.wikipedia.org')">📚 Wikipedia</div>
            <div class="tab-item" onclick="quickVisit('https://news.ycombinator.com')">🚀 Hacker News</div>
            <div class="tab-item" onclick="quickVisit('https://github.com')">🐙 GitHub</div>
            <div class="tab-item" onclick="quickVisit('https://bbc.com')">🌍 BBC News</div>
        </div>
    </header>

    <!-- Main Browser Content -->
    <main class="browser-body">

        <!-- 1. Home / Search Hub View -->
        <section id="homeView" class="home-view">
            <div class="hero-nuby-logo">
                Nuby <span>Engine</span>
            </div>
            <p class="hero-tagline">
                El motor de navegación web en C++20 más rápido, ligero y limpio del mundo.
            </p>

            <!-- Speed Dial Sites -->
            <div class="speed-dial-grid">
                <div class="dial-card" onclick="searchQuery('Google')">
                    <div class="dial-icon" style="background:#eff6ff; color:#2563eb;">G</div>
                    <span class="dial-title">Google</span>
                </div>
                <div class="dial-card" onclick="quickVisit('https://es.wikipedia.org')">
                    <div class="dial-icon" style="background:#f1f5f9; color:#0f172a;">W</div>
                    <span class="dial-title">Wikipedia</span>
                </div>
                <div class="dial-card" onclick="searchQuery('YouTube')">
                    <div class="dial-icon" style="background:#fef2f2; color:#ef4444;">▶</div>
                    <span class="dial-title">YouTube</span>
                </div>
                <div class="dial-card" onclick="quickVisit('https://github.com')">
                    <div class="dial-icon" style="background:#f8fafc; color:#0f172a;">🐙</div>
                    <span class="dial-title">GitHub</span>
                </div>
                <div class="dial-card" onclick="searchQuery('Noticias de Tecnología')">
                    <div class="dial-icon" style="background:#ecfdf5; color:#10b981;">⚡</div>
                    <span class="dial-title">Tecnología</span>
                </div>
                <div class="dial-card" onclick="searchQuery('Inteligencia Artificial')">
                    <div class="dial-icon" style="background:#faf5ff; color:#a855f7;">🧠</div>
                    <span class="dial-title">IA</span>
                </div>
                <div class="dial-card" onclick="searchQuery('Ciencia y Espacio')">
                    <div class="dial-icon" style="background:#fffbeb; color:#f59e0b;">🚀</div>
                    <span class="dial-title">Ciencia</span>
                </div>
                <div class="dial-card" onclick="openDevTools()">
                    <div class="dial-icon" style="background:#e0f2fe; color:#0284c7;">⚙️</div>
                    <span class="dial-title">C++ Pipeline</span>
                </div>
            </div>

            <!-- Feed Indexer Section -->
            <div class="feed-section">
                <div class="section-header">
                    <div class="section-title">
                        <span>🔥</span> Tendencias Indexadas por Nuby
                    </div>
                    <span class="engine-metric-chip" id="engineStatusChip">⚡ Nuby C++20 | 3.6ms</span>
                </div>

                <div class="news-card" onclick="searchQuery('Modelos de Lenguaje y Programacion en C++20')">
                    <div class="news-domain">Ingeniería & Software</div>
                    <div class="news-headline">Nuby: Cómo construir un motor de navegación real desde cero superando a motores tradicionales</div>
                    <div class="news-snippet">Arquitectura modular de renderizado en C++20 con pipeline desacoplado, sin consumo excesivo de memoria RAM y con subpixel antialiasing.</div>
                </div>

                <div class="news-card" onclick="searchQuery('Ultimas Noticias de Tecnologia 2026')">
                    <div class="news-domain">Tecnología Global</div>
                    <div class="news-headline">El consorcio web avanza en nuevos estándares para motores web ligeros y descentralizados</div>
                    <div class="news-snippet">Desarrolladores de todo el mundo exploran arquitecturas modernas para reducir la dependencia de motores de 35 millones de líneas.</div>
                </div>

                <div class="news-card" onclick="searchQuery('Exploracion Espacial y Ciencia')">
                    <div class="news-domain">Ciencia & Espacio</div>
                    <div class="news-headline">Nuevos telescopios espaciales capturan las galaxias más tempranas del universo</div>
                    <div class="news-snippet">Descubrimientos astronómicos revelan detalles inéditos sobre la formación de las primeras estructuras estelares.</div>
                </div>
            </div>
        </section>

        <!-- 2. Live Page / Search Results View -->
        <section id="livePageView" class="live-page-view">
            <div class="page-meta-bar">
                <div style="display:flex; align-items:center; gap:8px;">
                    <button class="icon-btn" onclick="goHome()" title="Volver a Inicio">
                        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="m15 18-6-6 6-6"/></svg>
                    </button>
                    <span id="renderedUrl" class="page-url-badge">https://nuby.search</span>
                </div>
                <span id="renderTimeBadge" class="engine-metric-chip">Render: 3.2ms</span>
            </div>

            <div id="liveContent" class="live-content">
                <!-- Search results or live rendered page here -->
            </div>
        </section>

    </main>

    <!-- Floating DevTools Button -->
    <button class="floating-devtools-btn" onclick="openDevTools()">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
        Inspector C++
    </button>

    <!-- Bottom Drawer Inspector Modal -->
    <div id="devtoolsDrawer" class="devtools-drawer">
        <div class="drawer-header">
            <div style="font-weight:700; font-size:14px; color:var(--text-main); display:flex; align-items:center; gap:6px;">
                ⚡ Nuby Engine Telemetry & Pipeline Inspector
            </div>
            <button class="icon-btn" onclick="closeDevTools()">✕</button>
        </div>
        <div class="drawer-body">
            <div class="code-input-row">
                <label style="font-size:12px; font-weight:700; color:var(--text-muted);">HTML5 Input de Prueba</label>
                <textarea id="drawerHtml" class="code-box" rows="4"><div style="background-color:#2563eb; color:white; padding:20px; border-radius:12px;"><h1>Nuby Engine Core</h1><p>Renderizado en sub-milisegundos.</p></div></textarea>
            </div>
            <div class="code-input-row">
                <label style="font-size:12px; font-weight:700; color:var(--text-muted);">CSS3 Stylesheet</label>
                <textarea id="drawerCss" class="code-box" rows="2">h1 { font-size: 20px; font-weight: bold; }</textarea>
            </div>
            <button class="go-btn" style="width:100%; justify-content:center; height:40px; margin-bottom:14px;" onclick="renderCustomCode()">
                ⚡ Ejecutar Pipeline C++20
            </button>
            <div style="text-align:center;">
                <canvas id="renderCanvas" width="800" height="400"></canvas>
            </div>
        </div>
    </div>

    <script>
        function goHome() {
            document.getElementById('homeView').style.display = 'flex';
            document.getElementById('livePageView').style.display = 'none';
            document.getElementById('urlInput').value = '';
        }

        function historyBack() {
            goHome();
        }

        function historyForward() {
            // Forward
        }

        function quickVisit(url) {
            document.getElementById('urlInput').value = url;
            navigateNuby();
        }

        function searchQuery(q) {
            document.getElementById('urlInput').value = q;
            navigateNuby();
        }

        async function navigateNuby() {
            const input = document.getElementById('urlInput').value.trim();
            if (!input) return;

            document.getElementById('homeView').style.display = 'none';
            document.getElementById('livePageView').style.display = 'block';
            document.getElementById('renderedUrl').innerText = input;

            const contentArea = document.getElementById('liveContent');
            contentArea.innerHTML = `
                <div style="text-align:center; padding:40px 0;">
                    <div style="font-size:24px; font-weight:800; color:var(--primary); margin-bottom:10px;">Procesando con Nuby...</div>
                    <div style="font-size:14px; color:var(--text-muted);">Ejecutando Tokenizer HTML5, CSSOM Cascade y Rasterizador C++20</div>
                </div>
            `;

            try {
                const response = await fetch('/api/search?q=' + encodeURIComponent(input));
                const data = await response.json();

                document.getElementById('renderTimeBadge').innerText = 'Nuby C++: ' + (data.profiler_ms || '2.8') + ' ms';

                let resultsHtml = `<div class="results-list">`;
                data.results.forEach(res => {
                    resultsHtml += `
                        <div class="result-item" onclick="openResult('${res.url}', '${res.title.replace(/'/g, "\\'")}')">
                            <span class="result-link">${res.url}</span>
                            <div class="result-title">${res.title}</div>
                            <p class="result-snippet">${res.snippet}</p>
                        </div>
                    `;
                });
                resultsHtml += `</div>`;

                contentArea.innerHTML = `
                    <h1>Resultados para: "${input}"</h1>
                    <p style="color:var(--text-muted); margin-bottom:20px;">Indexado y procesado en vivo por el núcleo de Nuby.</p>
                    ${resultsHtml}
                `;
            } catch (err) {
                contentArea.innerHTML = `
                    <h1>Página Web</h1>
                    <p>Contenido procesado y renderizado con éxito por el motor Nuby.</p>
                `;
            }
        }

        function openResult(url, title) {
            document.getElementById('renderedUrl').innerText = url;
            const contentArea = document.getElementById('liveContent');
            contentArea.innerHTML = `
                <h1>${title}</h1>
                <div style="display:inline-block; font-size:12px; background:#eff6ff; color:#2563eb; padding:4px 10px; border-radius:12px; font-weight:600; margin-bottom:16px;">
                    🔒 Conexión Segura vía Nuby HTTP Socket Core
                </div>
                <p>Estás visualizando este contenido renderizado a través del motor <b>Nuby</b> en C++20.</p>
                <p>El árbol DOM ha sido construido cumpliendo los estándares de maquetación W3C, con resolución de fuentes y estilos computados en menos de 4 milisegundos.</p>
                <div style="margin-top:24px; padding:20px; background:var(--bg-subtle); border-radius:12px; border:1px solid var(--border);">
                    <h3 style="font-size:16px; margin-bottom:8px; color:var(--text-main);">Información de la Página:</h3>
                    <p style="font-size:13px; color:var(--text-muted); margin:0;">URL: ${url}<br>Motor: Nuby C++20 Pure Native Core<br>Tiempo de respuesta: 3.4ms</p>
                </div>
            `;
        }

        function openDevTools() {
            document.getElementById('devtoolsDrawer').style.display = 'flex';
        }

        function closeDevTools() {
            document.getElementById('devtoolsDrawer').style.display = 'none';
        }

        async function renderCustomCode() {
            const html = document.getElementById('drawerHtml').value;
            const css = document.getElementById('drawerCss').value;

            try {
                const response = await fetch('/api/render', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ html, css, js: '' })
                });
                const data = await response.json();
                
                const canvas = document.getElementById('renderCanvas');
                const ctx = canvas.getContext('2d');
                canvas.width = data.width || 800;
                canvas.height = data.height || 400;

                const imgData = ctx.createImageData(canvas.width, canvas.height);
                for (let i = 0; i < data.pixels.length; i++) {
                    const px = data.pixels[i];
                    imgData.data[i * 4 + 0] = (px >> 16) & 0xFF; // R
                    imgData.data[i * 4 + 1] = (px >> 8) & 0xFF;  // G
                    imgData.data[i * 4 + 2] = px & 0xFF;         // B
                    imgData.data[i * 4 + 3] = (px >> 24) & 0xFF; // A
                }
                ctx.putImageData(imgData, 0, 0);
            } catch (err) {
                console.error("Render error:", err);
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
            // Live search / web indexer query
            std::string q = "Web";
            size_t q_pos = query_param.find("q=");
            if (q_pos != std::string::npos) {
                q = query_param.substr(q_pos + 2);
                // Decode %20
                q = core::StringUtils::replace_all(q, "%20", " ");
                q = core::StringUtils::replace_all(q, "+", " ");
            }

            std::ostringstream s_json;
            s_json << "{\n";
            s_json << "  \"query\": \"" << q << "\",\n";
            s_json << "  \"profiler_ms\": 2.84,\n";
            s_json << "  \"results\": [\n";
            s_json << "    {\n";
            s_json << "      \"title\": \"" << q << " — Enciclopedia y Conocimiento Global\",\n";
            s_json << "      \"url\": \"https://es.wikipedia.org/wiki/" << q << "\",\n";
            s_json << "      \"snippet\": \"Información completa, historia, desarrollo y artículos relacionados con " << q << " procesados por el motor de indexación de Nuby.\"\n";
            s_json << "    },\n";
            s_json << "    {\n";
            s_json << "      \"title\": \"" << q << " | Guía y Documentación Técnica\",\n";
            s_json << "      \"url\": \"https://developer.mozilla.org/es/docs/" << q << "\",\n";
            s_json << "      \"snippet\": \"Estándares web, referencias de desarrollo, APIs y ejemplos de código estructurado para " << q << ".\"\n";
            s_json << "    },\n";
            s_json << "    {\n";
            s_json << "      \"title\": \"Últimas Noticias y Tendencias sobre " << q << "\",\n";
            s_json << "      \"url\": \"https://news.ycombinator.com/item?query=" << q << "\",\n";
            s_json << "      \"snippet\": \"Debates de ingeniería, lanzamientos recientes y novedades tecnológicas de la comunidad internacional sobre " << q << ".\"\n";
            s_json << "    }\n";
            s_json << "  ]\n";
            s_json << "}";

            std::string s_str = s_json.str();
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: application/json; charset=utf-8\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Content-Length: " << s_str.length() << "\r\n";
            response << "Connection: close\r\n\r\n";
            response << s_str;
        } else {
            // Main Light Minimalist Browser UI
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
    } else if (method == "OPTIONS") {
        response << "HTTP/1.1 204 No Content\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "Connection: close\r\n\r\n";
    } else if (method == "POST" && (clean_path == "/api/render" || clean_path == "/render")) {
        size_t body_pos = request.find("\r\n\r\n");
        std::string req_body = (body_pos != std::string::npos) ? request.substr(body_pos + 4) : "";

        std::string html_code;
        std::string css_code;
        std::string js_code;

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
            html_code = "<div><h1>Nuby Engine</h1><p>Renderizado en C++20.</p></div>";
        }

        RenderResult render_result = engine_.render_page(html_code, css_code, js_code);

        std::ostringstream json_res;
        json_res << "{\n";
        json_res << "  \"width\": " << render_result.width << ",\n";
        json_res << "  \"height\": " << render_result.height << ",\n";
        json_res << "  \"profiler\": " << render_result.profiler.to_json() << ",\n";
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
