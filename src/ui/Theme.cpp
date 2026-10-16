#include "Theme.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string_view>

#include <glaze/glaze.hpp>

namespace {
    std::optional<uint32_t> parseHexColor(std::string_view str) {
        while (!str.empty() && (std::isspace(static_cast<unsigned char>(str.front())) || str.front() == '#' || str.front() == '"' || str.front() == '\''))
            str.remove_prefix(1);
        while (!str.empty() && (std::isspace(static_cast<unsigned char>(str.back())) || str.back() == ',' || str.back() == ';' || str.back() == '"' || str.back() == '\''))
            str.remove_suffix(1);

        if (str.empty())
            return std::nullopt;

        uint32_t val = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val, 16);
        if (ec != std::errc{} || ptr != str.data() + str.size())
            return std::nullopt;

        if (str.size() <= 6)
            val |= 0xFF000000;

        return val;
    }
}

CCaelestiaTheme::CCaelestiaTheme() = default;

void CCaelestiaTheme::load() {
    const char* home = getenv("HOME");
    std::string homeDir = home ? home : "";

    const char* xdgState = getenv("XDG_STATE_HOME");
    std::string stateDir = (xdgState && xdgState[0] != '\0') ? xdgState : (homeDir + "/.local/state");

    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    std::string configDir = (xdgConfig && xdgConfig[0] != '\0') ? xdgConfig : (homeDir + "/.config");

    loadSchemeJson(stateDir + "/caelestia/scheme.json");

    if (m_colours.empty()) {
        loadHyprSchemeFile(configDir + "/hypr/scheme/current.conf");
        loadHyprSchemeFile(configDir + "/hypr/scheme/current.lua");
    }

    loadShellJson(configDir + "/caelestia/shell.json");
}

void CCaelestiaTheme::applyToPalette(SP<Hyprtoolkit::CPalette> palette) const {
    if (!palette)
        return;

    palette->m_colors.background       = surface();
    palette->m_colors.base             = surfaceContainer();
    palette->m_colors.alternateBase    = surfaceContainerHigh();
    palette->m_colors.text             = onSurface();
    palette->m_colors.brightText       = onPrimaryContainer();
    palette->m_colors.linkText         = primary();
    palette->m_colors.accent           = primary();
    palette->m_colors.accentSecondary  = secondaryContainer();

    palette->m_vars.bigRounding        = roundingExtraLarge();
    palette->m_vars.smallRounding      = roundingLarge();
    palette->m_vars.h1Size             = 28;
    palette->m_vars.h2Size             = 18;
    palette->m_vars.h3Size             = 15;
    palette->m_vars.fontSize           = 13;
    palette->m_vars.smallFontSize      = 11;
    palette->m_vars.fontFamily         = m_fontFamily;
    palette->m_vars.fontFamilyMonospace= m_monoFontFamily;
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::scrim() const {
    return getColor("background", 0xFF121212, m_transparencyEnabled ? m_baseAlpha : 0.92F);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::surface() const {
    return getColor("surface", 0xFF141218);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::surfaceContainerLow() const {
    return getColor("surfaceContainerLow", 0xFF1D1B20);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::surfaceContainer() const {
    return getColor("surfaceContainer", 0xFF211F26, m_transparencyEnabled ? m_layerAlpha : 1.0F);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::surfaceContainerHigh() const {
    return getColor("surfaceContainerHigh", 0xFF2B2930);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::surfaceContainerHighest() const {
    return getColor("surfaceContainerHighest", 0xFF36343B);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::onSurface() const {
    return getColor("onSurface", 0xFFE6E0E9);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::onSurfaceVariant() const {
    return getColor("onSurfaceVariant", 0xFFCAC4D0);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::outline() const {
    return getColor("outline", 0xFF938F99);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::outlineVariant() const {
    return getColor("outlineVariant", 0xFF49454F);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::primary() const {
    return getColor("primary", 0xFFD0BCFF);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::onPrimary() const {
    return getColor("onPrimary", 0xFF381E72);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::primaryContainer() const {
    return getColor("primaryContainer", 0xFF4F378B);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::onPrimaryContainer() const {
    return getColor("onPrimaryContainer", 0xFFEADDFF);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::secondary() const {
    return getColor("secondary", 0xFFCCC2DC);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::secondaryContainer() const {
    return getColor("secondaryContainer", 0xFF4A4458);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::onSecondaryContainer() const {
    return getColor("onSecondaryContainer", 0xFFE8DEF8);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::error() const {
    return getColor("error", 0xFFF2B8B5);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::errorContainer() const {
    return getColor("errorContainer", 0xFF8C1D18);
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::onErrorContainer() const {
    return getColor("onErrorContainer", 0xFFF9DEDC);
}

int CCaelestiaTheme::roundingExtraLarge() const {
    return std::clamp(static_cast<int>(std::round(28.0F * m_roundingScale)), 0, 100);
}

int CCaelestiaTheme::roundingLarge() const {
    return std::clamp(static_cast<int>(std::round(16.0F * m_roundingScale)), 0, 100);
}

int CCaelestiaTheme::roundingMedium() const {
    return std::clamp(static_cast<int>(std::round(12.0F * m_roundingScale)), 0, 100);
}

const std::string& CCaelestiaTheme::fontFamily() const {
    return m_fontFamily;
}

const std::string& CCaelestiaTheme::monoFontFamily() const {
    return m_monoFontFamily;
}

Hyprtoolkit::CHyprColor CCaelestiaTheme::getColor(const std::string& key, uint32_t fallbackArgb, float alphaOverride) const {
    uint32_t argb = fallbackArgb;
    auto it = m_colours.find(key);
    if (it != m_colours.end())
        argb = it->second;

    float a = alphaOverride >= 0.0F ? alphaOverride : static_cast<float>((argb >> 24) & 0xFF) / 255.0F;
    float r = static_cast<float>((argb >> 16) & 0xFF) / 255.0F;
    float g = static_cast<float>((argb >> 8) & 0xFF) / 255.0F;
    float b = static_cast<float>(argb & 0xFF) / 255.0F;

    return Hyprtoolkit::CHyprColor(r, g, b, a);
}

void CCaelestiaTheme::loadSchemeJson(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        return;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto jsonRaw = glz::read_json<glz::generic>(content);
    if (!jsonRaw || !jsonRaw->is_object())
        return;

    const auto& root = jsonRaw->get_object();
    auto it = root.find("colours");
    if (it == root.end() || !it->second.is_object())
        return;

    for (const auto& [name, colVal] : it->second.get_object()) {
        if (colVal.is_string()) {
            if (auto hex = parseHexColor(colVal.get_string()))
                m_colours[std::string(name)] = *hex;
        }
    }
}

void CCaelestiaTheme::loadHyprSchemeFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line)) {
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line.erase(commentPos);
        commentPos = line.find("--");
        if (commentPos != std::string::npos)
            line.erase(commentPos);

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos)
            continue;

        std::string_view keyPart(line.data(), eqPos);
        std::string_view valPart(line.data() + eqPos + 1, line.size() - (eqPos + 1));

        while (!keyPart.empty() && (std::isspace(static_cast<unsigned char>(keyPart.front())) || keyPart.front() == '$'))
            keyPart.remove_prefix(1);
        while (!keyPart.empty() && std::isspace(static_cast<unsigned char>(keyPart.back())))
            keyPart.remove_suffix(1);

        if (keyPart.empty())
            continue;

        if (auto hex = parseHexColor(valPart)) {
            m_colours[std::string(keyPart)] = *hex;
        }
    }
}

void CCaelestiaTheme::loadShellJson(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        return;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto jsonRaw = glz::read_json<glz::generic>(content);
    if (!jsonRaw || !jsonRaw->is_object())
        return;

    const auto& root = jsonRaw->get_object();
    auto appIt = root.find("appearance");
    if (appIt == root.end() || !appIt->second.is_object())
        return;

    const auto& app = appIt->second.get_object();

    auto transIt = app.find("transparency");
    if (transIt != app.end() && transIt->second.is_object()) {
        const auto& trans = transIt->second.get_object();
        auto enIt = trans.find("enabled");
        if (enIt != trans.end() && enIt->second.is_boolean())
            m_transparencyEnabled = enIt->second.get_boolean();

        auto baseIt = trans.find("base");
        if (baseIt != trans.end() && baseIt->second.is_number())
            m_baseAlpha = std::clamp(static_cast<float>(baseIt->second.get_number()), 0.35F, 0.85F);
    }

    auto rndIt = app.find("rounding");
    if (rndIt != app.end() && rndIt->second.is_object()) {
        const auto& rnd = rndIt->second.get_object();
        auto scIt = rnd.find("scale");
        if (scIt != rnd.end() && scIt->second.is_number())
            m_roundingScale = std::clamp(static_cast<float>(scIt->second.get_number()), 0.0F, 3.0F);
    }

    auto fontIt = app.find("font");
    if (fontIt != app.end() && fontIt->second.is_object()) {
        const auto& font = fontIt->second.get_object();
        auto bodyIt = font.find("body");
        if (bodyIt != font.end() && bodyIt->second.is_object()) {
            const auto& body = bodyIt->second.get_object();
            auto famIt = body.find("family");
            if (famIt != body.end() && famIt->second.is_string())
                m_fontFamily = famIt->second.get_string();
        }

        auto monoIt = font.find("mono");
        if (monoIt != font.end() && monoIt->second.is_object()) {
            const auto& mono = monoIt->second.get_object();
            auto famIt = mono.find("family");
            if (famIt != mono.end() && famIt->second.is_string())
                m_monoFontFamily = famIt->second.get_string();
        }
    }
}
