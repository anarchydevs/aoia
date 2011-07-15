#ifndef SUMMARYVIEW_H
#define SUMMARYVIEW_H

#include <PluginSDK/ItemAssistView.h>
#include "WebView.h"

namespace aoia { namespace sv {

    class SummaryView
        : public ItemAssistView<SummaryView>
    {
    public:
        SummaryView();
        virtual ~SummaryView();

        BEGIN_MSG_MAP_EX(SummaryView)
            MSG_WM_CREATE(OnCreate)
            MSG_WM_SIZE(OnSize)
            REFLECT_NOTIFICATIONS()
            DEFAULT_REFLECTION_HANDLER()
        END_MSG_MAP();

        // ItemAssistView overrides
        virtual void OnActive(bool doActivation);

    protected:
        LRESULT OnCreate(LPCREATESTRUCT createStruct);
        LRESULT OnSize(UINT wParam, CSize newSize);
        void UpdateSummary();
        std::tstring SummaryView::GetHtmlTemplate();

    private:
        aoia::WebView m_webview;
        std::tstring m_template;
    };

}}  // end of namespace

#endif // SUMMARYVIEW_H
