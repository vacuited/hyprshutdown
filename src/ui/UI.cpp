#include "UI.hpp"
#include "../helpers/Logger.hpp"
#include "../state/AppState.hpp"
#include "../state/HyprlandIPC.hpp"

#include <algorithm>
#include <format>
#include <utility>

#include <hyprtoolkit/core/Output.hpp>
#include <hyprtoolkit/types/SizeType.hpp>
#include <hyprutils/memory/SharedPtr.hpp>
#include <hyprutils/os/Process.hpp>

using namespace Hyprutils::OS;

namespace {
    std::string escapeMarkup(std::string_view text) {
        std::string result;
        result.reserve(text.size());
        for (char c : text) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '\'': result += "&apos;"; break;
                case '"': result += "&quot;"; break;
                default: result += c; break;
            }
        }
        return result;
    }

    template <typename OnClick>
    std::pair<SP<Hyprtoolkit::CRectangleElement>, SP<Hyprtoolkit::CButtonElement>> makeTonalButton(
        std::string_view label,
        OnClick&& onClick,
        Hyprtoolkit::colorFn bgColor,
        Hyprtoolkit::colorFn borderColor) {
        auto rect = Hyprtoolkit::CRectangleBuilder::begin()
                        ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {132.F, 42.F}})
                        ->color(std::move(bgColor))
                        ->rounding(g_theme->roundingLarge())
                        ->borderThickness(1)
                        ->borderColor(std::move(borderColor))
                        ->commence();

        auto btn = Hyprtoolkit::CButtonBuilder::begin()
                       ->label(std::string{label})
                       ->noBg(true)
                       ->noBorder(true)
                       ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {132.F, 42.F}})
                       ->fontFamily(std::string{g_theme->fontFamily()})
                       ->fontSize(Hyprtoolkit::CFontSize{Hyprtoolkit::CFontSize::HT_FONT_ABSOLUTE, 13.F})
                       ->onMainClick(std::forward<OnClick>(onClick))
                       ->commence();

        rect->addChild(btn);
        return {rect, btn};
    }
}

CUI::CUI()  = default;
CUI::~CUI() = default;

CMonitorState::SAppListApp::SAppListApp(const std::string_view& clazz, const std::string_view& title) {
    m_null = Hyprtoolkit::CNullBuilder::begin()
                 ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, 58.F}})
                 ->commence();

    m_card = Hyprtoolkit::CRectangleBuilder::begin()
                 ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                 ->color([] { return g_theme->surfaceContainerHigh(); })
                 ->rounding(g_theme->roundingLarge())
                 ->borderThickness(1)
                 ->borderColor([] {
                     auto col = g_theme->outlineVariant();
                     col.a    = 0.35F;
                     return col;
                 })
                 ->commence();

    m_inner = Hyprtoolkit::CNullBuilder::begin()
                  ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                  ->commence();
    m_inner->setMargin(12);

    m_layout = Hyprtoolkit::CColumnLayoutBuilder::begin()
                   ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                   ->gap(4)
                   ->commence();

    m_class = Hyprtoolkit::CTextBuilder::begin()
                  ->text(std::format("<b>{}</b>", escapeMarkup(clazz)))
                  ->color([] { return g_theme->primary(); })
                  ->fontFamily(std::string{g_theme->fontFamily()})
                  ->fontSize(Hyprtoolkit::CFontSize{Hyprtoolkit::CFontSize::HT_FONT_H3})
                  ->commence();

    m_title = Hyprtoolkit::CTextBuilder::begin()
                  ->text(escapeMarkup(title))
                  ->color([] { return g_theme->onSurfaceVariant(); })
                  ->fontFamily(std::string{g_theme->fontFamily()})
                  ->fontSize(Hyprtoolkit::CFontSize{Hyprtoolkit::CFontSize::HT_FONT_SMALL})
                  ->commence();

    m_layout->addChild(m_class);
    m_layout->addChild(m_title);
    m_inner->addChild(m_layout);
    m_card->addChild(m_inner);
    m_null->addChild(m_card);
}

CMonitorState::CMonitorState(SP<Hyprtoolkit::IOutput> output) : m_monitorName(output->port()) {
    m_window = Hyprtoolkit::CWindowBuilder::begin()
                   ->type(Hyprtoolkit::HT_WINDOW_LAYER)
                   ->prefferedOutput(output)
                   ->anchor(0xF)
                   ->layer(3)
                   ->preferredSize({0, 0})
                   ->exclusiveZone(-1)
                   ->kbInteractive(1)
                   ->appClass("hyprshutdown")
                   ->commence();

    m_keyListener = m_window->m_events.keyboardKey.listen([](const Hyprtoolkit::Input::SKeyboardKeyEvent& event) {
        if (event.down && (event.xkbKeysym == 0xff1b || event.xkbKeysym == 1 || event.utf8 == "\x1b")) {
            g_ui->exit(false);
        }
    });

    m_bg = Hyprtoolkit::CRectangleBuilder::begin()
               ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
               ->color([] { return g_theme->scrim(); })
               ->commence();

    m_null = Hyprtoolkit::CNullBuilder::begin()
                 ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {540.F, 480.F}})
                 ->commence();
    m_null->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_ABSOLUTE);
    m_null->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_CENTER, true);

    m_cardRect = Hyprtoolkit::CRectangleBuilder::begin()
                     ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                     ->color([] { return g_theme->surfaceContainer(); })
                     ->rounding(g_theme->roundingExtraLarge())
                     ->borderThickness(1)
                     ->borderColor([] { return g_theme->outlineVariant(); })
                     ->commence();

    m_cardPad = Hyprtoolkit::CNullBuilder::begin()
                    ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                    ->commence();
    m_cardPad->setMargin(28);

    m_layout = Hyprtoolkit::CColumnLayoutBuilder::begin()
                   ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                   ->gap(10)
                   ->commence();

    m_headerRow = Hyprtoolkit::CRowLayoutBuilder::begin()
                      ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO, {1.F, 1.F}})
                      ->gap(8)
                      ->commence();

    m_topText = Hyprtoolkit::CTextBuilder::begin()
                    ->text(std::string{g_ui->m_shutdownLabel})
                    ->color([] { return g_theme->onSurface(); })
                    ->fontFamily(std::string{g_theme->fontFamily()})
                    ->fontSize(Hyprtoolkit::CFontSize{Hyprtoolkit::CFontSize::HT_FONT_H1})
                    ->commence();

    auto headerSpacer = Hyprtoolkit::CNullBuilder::begin()
                            ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, 1.F}})
                            ->commence();
    headerSpacer->setGrow(true, false);

    m_badgeRect = Hyprtoolkit::CRectangleBuilder::begin()
                      ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {110.F, 28.F}})
                      ->color([] { return g_theme->secondaryContainer(); })
                      ->rounding(14)
                      ->commence();

    m_badgeText = Hyprtoolkit::CTextBuilder::begin()
                      ->text(std::format("{} active", State::state()->apps().size()))
                      ->color([] { return g_theme->onSecondaryContainer(); })
                      ->fontFamily(std::string{g_theme->fontFamily()})
                      ->fontSize(Hyprtoolkit::CFontSize{Hyprtoolkit::CFontSize::HT_FONT_SMALL})
                      ->commence();
    m_badgeText->setPositionMode(Hyprtoolkit::IElement::HT_POSITION_ABSOLUTE);
    m_badgeText->setPositionFlag(Hyprtoolkit::IElement::HT_POSITION_FLAG_CENTER, true);

    m_badgeRect->addChild(m_badgeText);

    m_headerRow->addChild(m_topText);
    m_headerRow->addChild(headerSpacer);
    m_headerRow->addChild(m_badgeRect);

    m_subText = Hyprtoolkit::CTextBuilder::begin()
                    ->text("Waiting for running applications to close gracefully.\n<i>Force quitting immediately terminates remaining apps and may lose unsaved work.</i>")
                    ->color([] { return g_theme->onSurfaceVariant(); })
                    ->fontFamily(std::string{g_theme->fontFamily()})
                    ->fontSize(Hyprtoolkit::CFontSize{Hyprtoolkit::CFontSize::HT_FONT_TEXT})
                    ->commence();

    m_appListNull = Hyprtoolkit::CNullBuilder::begin()
                        ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, 1.F}})
                        ->commence();
    m_appListNull->setGrow(true);

    m_appListRect = Hyprtoolkit::CRectangleBuilder::begin()
                        ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                        ->color([] { return g_theme->surfaceContainerLow(); })
                        ->rounding(g_theme->roundingLarge())
                        ->borderThickness(1)
                        ->borderColor([] {
                            auto c = g_theme->outlineVariant();
                            c.a    = 0.45F;
                            return c;
                        })
                        ->commence();

    m_appListPad = Hyprtoolkit::CNullBuilder::begin()
                       ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                       ->commence();
    m_appListPad->setMargin(10);

    m_appListScroll = Hyprtoolkit::CScrollAreaBuilder::begin()
                          ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})
                          ->scrollY(true)
                          ->scrollX(false)
                          ->commence();

    m_appListLayout = Hyprtoolkit::CColumnLayoutBuilder::begin()
                          ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO, {1.F, 1.F}})
                          ->gap(8)
                          ->commence();

    m_appListScroll->addChild(m_appListLayout);
    m_appListPad->addChild(m_appListScroll);
    m_appListRect->addChild(m_appListPad);
    m_appListNull->addChild(m_appListRect);

    auto [cancelRect, cancelBtn] = makeTonalButton(
        "Cancel",
        [](auto) { g_ui->exit(false); },
        [] { return g_theme->secondaryContainer(); },
        [] { return g_theme->outlineVariant(); });
    m_cancelRect = cancelRect;
    m_cancel     = cancelBtn;

    auto [forceQuitRect, forceQuitBtn] = makeTonalButton(
        "Force quit",
        [](auto) {
            State::state()->killAllApps();
            g_ui->exit(true);
        },
        [] { return g_theme->errorContainer(); },
        [] { return g_theme->error(); });
    m_forceQuitRect = forceQuitRect;
    m_forceQuit     = forceQuitBtn;

    m_buttonLayout = Hyprtoolkit::CRowLayoutBuilder::begin()
                         ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_PERCENT, Hyprtoolkit::CDynamicSize::HT_SIZE_AUTO, {1.F, 1.F}})
                         ->gap(12)
                         ->commence();

    auto btnSpacer = Hyprtoolkit::CNullBuilder::begin()
                         ->size({Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, Hyprtoolkit::CDynamicSize::HT_SIZE_ABSOLUTE, {1.F, 1.F}})
                         ->commence();
    btnSpacer->setGrow(true, false);

    m_buttonLayout->addChild(btnSpacer);
    m_buttonLayout->addChild(m_cancelRect);
    m_buttonLayout->addChild(m_forceQuitRect);

    m_window->m_rootElement->addChild(m_bg);
    m_window->m_rootElement->addChild(m_null);

    m_null->addChild(m_cardRect);
    m_cardRect->addChild(m_cardPad);
    m_cardPad->addChild(m_layout);

    m_layout->addChild(m_headerRow);
    m_layout->addChild(m_subText);
    m_layout->addChild(m_appListNull);
    m_layout->addChild(m_buttonLayout);

    update();

    m_window->open();
}

void CMonitorState::update() {
    m_apps.clear();
    m_appListLayout->clearChildren();

    const auto& APPS = State::state()->apps();

    m_badgeText->setText(std::format("{} active", APPS.size()));

    for (const auto& APP : APPS) {
        m_apps.emplace_back(makeUnique<SAppListApp>(APP->m_class, APP->m_title));
        m_appListLayout->addChild(m_apps.back()->m_null);
    }
}

void CUI::registerOutput(const SP<Hyprtoolkit::IOutput>& mon) {
    m_states.emplace_back(makeUnique<CMonitorState>(mon));
    mon->m_events.removed.listenStatic([this, m = WP<Hyprtoolkit::IOutput>{mon}] { std::erase_if(m_states, [&m](const auto& e) { return e->m_monitorName == m->port(); }); });
}

void CUI::exit(bool closeHl) {
    g_ui->m_states.clear();

    g_ui->backend()->addIdle([this, closeHl] {
        g_ui->m_backend->destroy();
        g_ui->m_backend.reset();

        if (closeHl && !m_noExit && !State::state()->m_dryRun) {
            std::string cmd = State::state()->m_useLua ? "/dispatch hl.dsp.exit()" : "/dispatch exit";
            (void)HyprlandIPC::getFromSocket(cmd);
            if (m_postExitCmd) {
                CProcess proc("/bin/sh", {"-c", m_postExitCmd.value()});
                proc.runAsync();
            }
        }
    });
}

void CUI::setTimer() {
    static uint16_t          counter     = 0;
    constexpr const uint16_t COUNTER_MAX = 30;

    m_updateTimer = m_backend->addTimer(
        std::chrono::milliseconds(150),
        [this](ASP<Hyprtoolkit::CTimer> timer, void* d) {
            if (State::state()->apps().empty()) {
                exit(true);
                return;
            }

            counter++;

            if (counter > COUNTER_MAX) {
                g_logger->log(LOG_DEBUG, "Re-closing apps");
                counter = 0;
                State::state()->reexitApps();
            }

            if (!State::state()->updateState()) {
                setTimer();
                return;
            }

            for (const auto& s : m_states) {
                s->update();
            }

            setTimer();
        },
        nullptr);
}

bool CUI::run() {
    auto data           = Hyprtoolkit::IBackend::SBackendCreationData();
    data.pLogConnection = makeShared<Hyprutils::CLI::CLoggerConnection>(*g_logger);
    data.pLogConnection->setName("hyprtoolkit");
    data.pLogConnection->setLogLevel(LOG_DEBUG);
    m_backend = Hyprtoolkit::IBackend::createWithData(data);

    if (!m_backend)
        return false;

    g_theme = makeUnique<CCaelestiaTheme>();
    g_theme->load();
    g_theme->applyToPalette(m_backend->getPalette());

    {
        const auto MONITORS = m_backend->getOutputs();

        for (const auto& m : MONITORS) {
            registerOutput(m);
        }

        m_listeners.newMon = m_backend->m_events.outputAdded.listen([this](SP<Hyprtoolkit::IOutput> mon) { registerOutput(mon); });

        g_logger->log(LOG_DEBUG, "Found {} output(s)", MONITORS.size());

        setTimer();
    }

    m_backend->enterLoop();

    return true;
}

SP<Hyprtoolkit::IBackend> CUI::backend() {
    return m_backend;
}
