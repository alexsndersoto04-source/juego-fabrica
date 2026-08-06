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

## 📱 Generar el APK de Android (de verdad)

El proyecto está envuelto con **Capacitor**, de modo que el mismo juego web se
empaqueta como una app nativa de Android. La compilación del APK se hace en
**GitHub Actions** (allí sí hay Java y el SDK de Android), no en este sandbox.

### Pasos

1. Sube a `main` el archivo `.github/workflows/build-apk.yml` (Arena no tiene
   permiso para subir archivos de `.github/workflows`; crea tú el archivo en
   GitHub con ese contenido, o súbelo con un usuario que tenga ese permiso).
2. En tu repo, abre la pestaña **Actions** → **Build Android APK** →
   **Run workflow**.
3. Cuando termine, abre esa ejecución y descarga el artefacto
   **`sombra-debug-apk`**. Dentro viene `app-debug.apk`.
4. Pásalo a tu celular e instálalo (activa "instalar apps de orígenes
   desconocidos").

El APK es una versión *debug* (funciona y se instala sin problemas). Para
publicarla en Play Store luego habría que firmarla con una clave de release.

### Si prefieres compilar en tu PC

Necesitas Android Studio (o Android SDK) + JDK 17:

```bash
npm install
npx cap sync android
npx cap open android      # se abre Android Studio; Build > Build APK
```

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
capacitor.config.json + android/   Proyecto nativo para el APK
.github/workflows/build-apk.yml    Compila el APK en GitHub Actions
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
