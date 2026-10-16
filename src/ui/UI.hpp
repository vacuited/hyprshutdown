#pragma once

#include <vector>
#include <string_view>

#include <hyprtoolkit/core/Backend.hpp>
#include <hyprtoolkit/window/Window.hpp>
#include <hyprtoolkit/element/Text.hpp>
#include <hyprtoolkit/element/Null.hpp>
#include <hyprtoolkit/element/Image.hpp>
#include <hyprtoolkit/element/Rectangle.hpp>
#include <hyprtoolkit/element/ColumnLayout.hpp>
#include <hyprtoolkit/element/RowLayout.hpp>
#include <hyprtoolkit/element/Button.hpp>
#include <hyprtoolkit/element/ScrollArea.hpp>

#include <hyprutils/signal/Listener.hpp>

#include "../helpers/Memory.hpp"
#include "Theme.hpp"

class CMonitorState {
  public:
    CMonitorState(SP<Hyprtoolkit::IOutput> output);
    ~CMonitorState() = default;

    CMonitorState(const CMonitorState&) = delete;
    CMonitorState(CMonitorState&)       = delete;
    CMonitorState(CMonitorState&&)      = delete;

    void        update();

    std::string m_monitorName;

  private:
    SP<Hyprtoolkit::IWindow>               m_window;

    SP<Hyprtoolkit::CRectangleElement>     m_bg;
    SP<Hyprtoolkit::CNullElement>          m_null;
    SP<Hyprtoolkit::CRectangleElement>     m_cardRect;
    SP<Hyprtoolkit::CNullElement>          m_cardPad;
    SP<Hyprtoolkit::CColumnLayoutElement>  m_layout;

    SP<Hyprtoolkit::CRowLayoutElement>     m_headerRow;
    SP<Hyprtoolkit::CTextElement>          m_topText;
    SP<Hyprtoolkit::CRectangleElement>     m_badgeRect;
    SP<Hyprtoolkit::CTextElement>          m_badgeText;

    SP<Hyprtoolkit::CTextElement>          m_subText;

    SP<Hyprtoolkit::CNullElement>          m_appListNull;
    SP<Hyprtoolkit::CRectangleElement>     m_appListRect;
    SP<Hyprtoolkit::CNullElement>          m_appListPad;
    SP<Hyprtoolkit::CScrollAreaElement>    m_appListScroll;
    SP<Hyprtoolkit::CColumnLayoutElement>  m_appListLayout;

    SP<Hyprtoolkit::CRowLayoutElement>     m_buttonLayout;
    SP<Hyprtoolkit::CRectangleElement>     m_cancelRect;
    SP<Hyprtoolkit::CRectangleElement>     m_forceQuitRect;
    SP<Hyprtoolkit::CButtonElement>        m_cancel;
    SP<Hyprtoolkit::CButtonElement>        m_forceQuit;

    Hyprutils::Signal::CHyprSignalListener m_keyListener;

    struct SAppListApp {
        SAppListApp(const std::string_view& clazz, const std::string_view& title);

        SP<Hyprtoolkit::CNullElement>         m_null;
        SP<Hyprtoolkit::CRectangleElement>    m_card;
        SP<Hyprtoolkit::CNullElement>         m_inner;
        SP<Hyprtoolkit::CColumnLayoutElement> m_layout;
        SP<Hyprtoolkit::CTextElement>         m_class;
        SP<Hyprtoolkit::CTextElement>         m_title;
    };

    std::vector<UP<SAppListApp>> m_apps;
};

class CUI {
  public:
    CUI();
    ~CUI();

    bool                       run();
    SP<Hyprtoolkit::IBackend>  backend();

    bool                       m_noExit = false;
    std::optional<std::string> m_postExitCmd;
    std::string                m_shutdownLabel;

  private:
    void                           registerOutput(const SP<Hyprtoolkit::IOutput>& mon);
    void                           setTimer();

    void                           exit(bool closeHl = false);

    SP<Hyprtoolkit::IBackend>      m_backend;
    ASP<Hyprtoolkit::CTimer>       m_updateTimer;

    std::vector<UP<CMonitorState>> m_states;

    struct {
        Hyprutils::Signal::CHyprSignalListener newMon;
    } m_listeners;

    friend class CMonitorState;
};

inline UP<CUI> g_ui;
