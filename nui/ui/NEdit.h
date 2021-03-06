#pragma once


#include "NFrame.h"

namespace nui
{
    namespace Ui
    {
        BEGIN_USE_UNEXPORT_TEMPLATE()

        class NUI_CLASS NEdit : public NNative
        {
            DECLARE_REFLECTION(TEXT("nui"), TEXT("edit"))
        public:

            enum TooltipIconType
            {
                IconNone    = 0,
                IconError   = 3,
                IconInfo    = 1,
                IconWarning = 2,
            };

            NEdit();
            ~NEdit();

            NEdit* SetHintText(LPCTSTR hintText);
            NEdit* SetReadOnly(bool readOnly);
            NEdit* SelectAll();
            NEdit* ShowTooltip(TooltipIconType iconType, LPCTSTR szTitle, LPCTSTR szText);
            NEdit* PasteText(LPCTSTR text);

        protected:
            virtual void OnCreate();
            virtual NCursor* GetCursor() const;

            virtual bool OnParentCommand(WORD notifyCode);
            virtual bool GetWndData(Base::NString& wndClassName, DWORD& style, DWORD& exStyle);
            virtual void OnRealWindowCreated();

        private:
            Base::NString hintText_;
        };
        END_USE_UNEXPORT_TEMPLATE()
    }
}
