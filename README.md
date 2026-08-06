# SOMBRA

Un juego de plataformas y sigilo en 2D, de atmósfera oscura inspirada en *Inside* (Playdead).
Hecho con **HTML5, Canvas y JavaScript puro**: sin motores ni dependencias que instalar.

Corre, salta y escabúllete entre drones de vigilancia cuyos conos de luz te cazan.
Llega a la salida de luz al final del nivel.

## 🎮 Cómo se juega

- **Moverse:** flechas ← → o teclas **A / D**
- **Saltar:** **Espacio**, **W** o flecha ↑
- En **celular**: aparecen botones táctiles en pantalla (◀ ▶ ▲)

Si el haz de un dron te ilumina el tiempo suficiente, estás muerto.
Reinicia con el botón **Reintentar**.

## ▶️ Cómo ejecutarlo

No requiere instalación. Simplemente sirve la carpeta con un servidor web:

```bash
# Python 3
python3 -m http.server 8000
```

Abre `http://localhost:8000` en tu navegador.

> También puedes abrir `index.html`, pero el audio funciona mejor a través de `http://`.

## 🌐 Publicarlo en internet (GitHub Pages)

Cada vez que se haga `push` a `main`, el workflow de
`.github/workflows/deploy.yml` publica el juego automáticamente en GitHub Pages.
Activa Pages en los ajustes del repositorio (Source: GitHub Actions).

## 📱 Sobre el APK

El juego está hecho para **web (HTML5)** a propósito. Es la forma más confiable de
tener algo que *de verdad se pueda jugar* en cualquier dispositivo sin procesos
de compilación frágiles. Una vez publicado en la web, se puede convertir a APK
de Android de forma independiente (por ejemplo envolviéndolo en una WebView con
Capacitor/Cordova). Ese paso se puede hacer después, cuando el juego ya exista.

## 🗂️ Estructura

```
index.html          Pantalla, HUD y controles
css/style.css       Estilo atmosférico
js/audio.js         Sonido sintetizado (Web Audio API)
js/level.js         Plataformas, drones, fondo y meta
js/player.js        Física y animación del personaje
js/drone.js         Enemigos y detección por cono de luz
js/particles.js     Polvo flotante ambiental
js/game.js          Bucle principal, cámara y estados
```

## 🛣️ Ideas para seguir

- Más niveles y checkpoints
- Palancas, cajas empujables y zonas de sigilo
- Enemigos adicionales (perros mecánicos en lugar de drones, etc.)
- Banda sonora musical
- Guardado de progreso
- Envoltorio APK para Android

---
Hecho para que exista de verdad, no para prometerlo.
