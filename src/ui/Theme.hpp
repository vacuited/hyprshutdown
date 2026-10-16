#pragma once

#include <string>
#include <unordered_map>
#include <hyprtoolkit/palette/Color.hpp>
#include <hyprtoolkit/palette/Palette.hpp>
#include "../helpers/Memory.hpp"

class CCaelestiaTheme {
  public:
    CCaelestiaTheme();
    ~CCaelestiaTheme() = default;

    void                       load();
    void                       applyToPalette(SP<Hyprtoolkit::CPalette> palette) const;

    Hyprtoolkit::CHyprColor    scrim() const;
    Hyprtoolkit::CHyprColor    surface() const;
    Hyprtoolkit::CHyprColor    surfaceContainerLow() const;
    Hyprtoolkit::CHyprColor    surfaceContainer() const;
    Hyprtoolkit::CHyprColor    surfaceContainerHigh() const;
    Hyprtoolkit::CHyprColor    surfaceContainerHighest() const;
    Hyprtoolkit::CHyprColor    onSurface() const;
    Hyprtoolkit::CHyprColor    onSurfaceVariant() const;
    Hyprtoolkit::CHyprColor    outline() const;
    Hyprtoolkit::CHyprColor    outlineVariant() const;
    Hyprtoolkit::CHyprColor    primary() const;
    Hyprtoolkit::CHyprColor    onPrimary() const;
    Hyprtoolkit::CHyprColor    primaryContainer() const;
    Hyprtoolkit::CHyprColor    onPrimaryContainer() const;
    Hyprtoolkit::CHyprColor    secondary() const;
    Hyprtoolkit::CHyprColor    secondaryContainer() const;
    Hyprtoolkit::CHyprColor    onSecondaryContainer() const;
    Hyprtoolkit::CHyprColor    error() const;
    Hyprtoolkit::CHyprColor    errorContainer() const;
    Hyprtoolkit::CHyprColor    onErrorContainer() const;

    int                        roundingExtraLarge() const;
    int                        roundingLarge() const;
    int                        roundingMedium() const;
    const std::string&         fontFamily() const;
    const std::string&         monoFontFamily() const;

  private:
    Hyprtoolkit::CHyprColor    getColor(const std::string& key, uint32_t fallbackArgb, float alphaOverride = -1.0F) const;
    void                       loadSchemeJson(const std::string& path);
    void                       loadHyprSchemeFile(const std::string& path);
    void                       loadShellJson(const std::string& path);

    std::unordered_map<std::string, uint32_t> m_colours;
    bool                                      m_transparencyEnabled = true;
    float                                     m_baseAlpha           = 0.55F;
    float                                     m_layerAlpha          = 0.88F;
    float                                     m_roundingScale       = 1.0F;
    std::string                               m_fontFamily          = "GoogleSansFlex";
    std::string                               m_monoFontFamily      = "CaskaydiaCove NF";
};

inline UP<CCaelestiaTheme> g_theme;
