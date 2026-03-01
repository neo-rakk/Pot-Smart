# 🌿 Pot Connecté Industriel – ESP32 (Arduino Framework)

Ce projet est une solution complète de gestion de pot de fleur connecté, basée sur le **framework Arduino** (compatible PlatformIO) pour ESP32. Cette version allie la puissance de **FreeRTOS** à la richesse de l'écosystème Arduino.

---

## 🚀 Fonctionnalités Clés

- **Automatisme Intelligent** : Arrosage basé sur des seuils d'humidité du sol.
- **Fail-safe Pompe** : Sécurité logicielle coupant la pompe après 10s pour éviter les inondations.
- **Multitâche FreeRTOS** : Gestion temps réel thread-safe avec **Mutex** pour les données capteurs et le bus **I2C**.
- **Connectivité WiFi (AP+STA)** : Mode Point d'accès pour la configuration initiale et Station pour l'accès réseau.
- **Découverte mDNS** : Accessible via `http://pot-connecte.local`.
- **API REST & Web UI** : Interface moderne basée sur **ArduinoJson** et servie par SPIFFS.
- **Drivers Matériels Robustes** : Utilisation des bibliothèques standards `DHTesp`, `U8g2` et `Wire`.

---

## 🔧 Installation & Téléversement

### 1. Prérequis
- [PlatformIO](https://platformio.org/) (extension VSCode).
- Les dépendances (`ArduinoJson`, `DHTesp`, `U8g2`) sont gérées automatiquement.

### 2. Compilation
```bash
# Compiler et téléverser le firmware
pio run --target upload

# Générer et téléverser l'image SPIFFS (Interface Web)
pio run --target uploadfs
```

---

## 🧠 Architecture
- `lib/` : Composants métiers (Sensors, Display, Relay, Web, Storage, Config).
- `src/` : Application principale (`setup`, `loop`, `wifi_manager`, `arrosage`).
- `data/` : Fichiers statiques (HTML/CSS/JS) pour le dashboard.

---

## 👤 Auteur & Licence
[Neo-Rakk] - Projet Grade Industriel / Arduino Framework.
