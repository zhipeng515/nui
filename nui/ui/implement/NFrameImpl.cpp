#include "stdafx.h"
#include "NFrameImpl.h"


namespace nui
{
    namespace Ui
    {
        IMPLEMENT_REFLECTION(NFrameImpl);

        NFrameImpl::NFrameImpl()
        {
            parentFrame_ = NULL;
            topMostCount_ = 0;
            bottomMostCount_ = 0;
            frameFlags_ = FlagNone;
        }

        NFrameImpl::~NFrameImpl()
        {
            NSafeRelease(parentFrame_);
            RemoveAllChilds();
        }

        bool NFrameImpl::AddChild(NFrame* child)
        {
            NFrameImpl* childImpl = dynamic_cast<NFrameImpl*>(child);
            NAssertError(childImpl != NULL, _T("wrong type"));
            if(childImpl == NULL)
                return FALSE;

            // check if exists
            size_t zorder;
            NAssertError(GetChildHelper(childImpl, zorder) == childs_.end(), _T("child already exists"));
            if(GetChildHelper(childImpl, zorder) != childs_.end())
                return true;

            childs_.push_back(childImpl);
            SetParentHelper(childImpl, this);
            return true;
        }

        bool NFrameImpl::RemoveChild(NFrame* child)
        {
            NFrameImpl* childImpl = dynamic_cast<NFrameImpl*>(child);
            NAssertError(childImpl != NULL, _T("wrong type"));
            if(childImpl == NULL)
                return FALSE;

            size_t zorder;
            FrameList::const_iterator ite = GetChildHelper(childImpl, zorder);
            if(ite == childs_.end())
                return false;

            if(zorder < topMostCount_)
                -- topMostCount_;
            if(zorder >= childs_.size() - bottomMostCount_)
                -- bottomMostCount_;

            childs_.erase(ite);
            SetParentHelper(childImpl, NULL);
            return true;
        }

        void NFrameImpl::RemoveAllChilds()
        {
            FrameList::iterator ite = childs_.begin();
            while(ite != childs_.end())
            {
                NFrameImpl*& child = *ite;
                NSafeRelease(child->parentFrame_);
                child->OnParentChanged();
                child->Release();
                ++ ite;
            }
            childs_.clear();
            topMostCount_ = 0;
            bottomMostCount_ = 0;
        }

        size_t NFrameImpl::SetChildZOrder(NFrame* child, size_t zorder)
        {
            NFrameImpl* childImpl = dynamic_cast<NFrameImpl*>(child);
            NAssertError(childImpl != NULL, _T("wrong type"));
            if(childImpl == NULL)
                return static_cast<size_t>(-1);

            size_t tmpZorder;
            FrameList::const_iterator ite = GetChildHelper(childImpl, tmpZorder);
            NAssertError(ite != childs_.end(), _T("child not exists"));
            if(ite == childs_.end())
                return static_cast<size_t>(-1);

            if(tmpZorder < topMostCount_ || tmpZorder >= childs_.size() - bottomMostCount_)
                return tmpZorder;

            childs_.erase(ite);
            zorder = (zorder > childs_.size()) ? childs_.size() : zorder;
            zorder = (zorder < 0) ? 0 : zorder;
            ite = childs_.begin();
            for(size_t i=0; i<zorder; ++ i)
                ++ ite;
            childs_.insert(ite, childImpl);
            return zorder;
        }

        size_t NFrameImpl::GetChildZOrder(NFrame* child) const
        {
            NFrameImpl* childImpl = dynamic_cast<NFrameImpl*>(child);
            NAssertError(childImpl != NULL, _T("wrong type"));
            if(childImpl == NULL)
                return static_cast<size_t>(-1);

            size_t zorder;
            FrameList::const_iterator ite = GetChildHelper(childImpl, zorder);
            NAssertError(ite != childs_.end(), _T("child not exists"));
            return zorder;
        }

        void NFrameImpl::SetChildTopmost(NFrame* child)
        {
            NFrameImpl* childImpl = dynamic_cast<NFrameImpl*>(child);
            NAssertError(childImpl != NULL, _T("wrong type"));
            if(childImpl == NULL)
                return;

            size_t zorder;
            FrameList::const_iterator ite = GetChildHelper(childImpl, zorder);
            NAssertError((ite != childs_.end()), _T("child not exists"));
            if(ite == childs_.end())
                return;
            if(zorder < topMostCount_)
                return;
            if(zorder >= childs_.size() - bottomMostCount_)
                -- bottomMostCount_;
            // move
            childs_.erase(ite);
            ite = childs_.begin();
            for(size_t i=0; i<topMostCount_; ++ i)
            {
                ++ ite;
            }
            childs_.insert(ite, childImpl);
            ++ topMostCount_;
        }

        void NFrameImpl::SetChildBottommost(NFrame* child)
        {
            NFrameImpl* childImpl = dynamic_cast<NFrameImpl*>(child);
            NAssertError(childImpl != NULL, _T("wrong type"));
            if(childImpl == NULL)
                return;

            size_t zorder;
            FrameList::const_iterator ite = GetChildHelper(childImpl, zorder);
            NAssertError((ite != childs_.end()), _T("child not exists"));
            if(ite == childs_.end())
                return;
            if(zorder >= childs_.size() - bottomMostCount_)
                return;
            if(zorder < topMostCount_)
                -- topMostCount_;
            // move
            childs_.erase(ite);
            ite = childs_.begin();
            size_t offset = childs_.size() - bottomMostCount_;
            for(size_t i=0; i<offset; ++ i)
            {
                ++ ite;
            }
            childs_.insert(ite, childImpl);
            ++ bottomMostCount_;
        }

        bool NFrameImpl::EnumChilds(UiContainerEnumCallback callback, LPARAM lParam) const
        {
            FrameList::const_iterator ite = childs_.begin();
            while(ite != childs_.end())
            {
                if(!callback(*ite, lParam))
                    return false;
                ++ ite;
            }
            return true;
        }

        NFrame* NFrameImpl::GetParent() const
        {
            return parentFrame_;
        }

        void NFrameImpl::SetVisible(bool visible)
        {
            if(visible == IsVisible())
                return;
            Util::Misc::CheckFlag(frameFlags_, NFrameImpl::FlagVisible, visible);
            Invalidate();
        }

        bool NFrameImpl::IsVisible() const
        {
            return Util::Misc::IsFlagChecked(frameFlags_, NFrameImpl::FlagVisible);
        }

        void NFrameImpl::SetEnabled(bool enabled)
        {
            if(enabled == IsEnabled())
                return;
            Util::Misc::CheckFlag(frameFlags_, NFrameImpl::FlagEnabled, enabled);
            Invalidate();
        }

        bool NFrameImpl::IsEnabled() const
        {
            return Util::Misc::IsFlagChecked(frameFlags_, NFrameImpl::FlagEnabled);
        }

        void NFrameImpl::SetAutoSize(bool autosize)
        {
            if(autosize == IsAutoSize())
                return;
            Util::Misc::CheckFlag(frameFlags_, NFrameImpl::FlagAutoSize, autosize);
            if(autosize)
                AutoSize();
        }

        bool NFrameImpl::IsAutoSize() const
        {
            return Util::Misc::IsFlagChecked(frameFlags_, NFrameImpl::FlagAutoSize);
        }

        void NFrameImpl::SetValid(bool valid)
        {
            if(valid == IsValid())
                return;
            Util::Misc::CheckFlag(frameFlags_, NFrameImpl::FlagValid, valid);
            Invalidate();
        }

        bool NFrameImpl::IsValid() const
        {
            return Util::Misc::IsFlagChecked(frameFlags_, NFrameImpl::FlagValid);
        }

        void NFrameImpl::SetText(const Base::NString& text)
        {
            if(text_->GetText() == text)
                return;
            text_->SetText(text);
            Invalidate();
        }

        Base::NString NFrameImpl::GetText() const
        {
            return text_->GetText();
        }

        const Base::NRect& NFrameImpl::GetRect() const
        {
            return frameRect_;
        }

        Base::NRect NFrameImpl::GetRootRect() const
        {
            Base::NRect result = frameRect_;
            if(parentFrame_ != NULL)
            {
                Base::NRect parentRect = parentFrame_->GetRootRect();
                result.Offset(parentRect.Left, parentRect.Top);
            }
            return result;
        }

        void NFrameImpl::AutoSize()
        {
            NAssertError(window_ != NULL, _T("window is null"));
            if(!window_)
                return;
            Base::NRect txtRect;
            window_->GetRender()->GetTextSize(text_, txtRect);
            SetSize(txtRect.Width(), txtRect.Height());
        }

        void NFrameImpl::SetPos(int left, int top)
        {
            if(frameRect_.Left == left && frameRect_.Top == top)
                return;
            frameRect_.Left = left;
            frameRect_.Top = top;
            Invalidate();
        }

        void NFrameImpl::SetSize(int width, int height)
        {
            int frameWidth = (minSize_.Width < 0) ? width : (width >= minSize_.Width ? width : minSize_.Width);
            int frameHeight = (minSize_.Height < 0) ? height : (height >= minSize_.Height ? height : minSize_.Height);
            if(frameRect_.Width() == width && frameRect_.Height() == height)
                return;

            frameRect_.SetSize(frameWidth, frameHeight);
            Invalidate();
        }

        void NFrameImpl::SetMinSize(int minWidth, int minHeight)
        {
            minSize_.Width = minWidth;
            minSize_.Height = minHeight;
        }

        void NFrameImpl::Invalidate()
        {
            if(!window_)
                return;
            Base::NRect rootRect = GetRootRect();
            window_->InvalidateRect(rootRect);
        }

        void NFrameImpl::Draw(NRender* render, const Base::NRect& clipRect)
        {
            if(!IsVisible() || !IsValid())
                return;

            if(!clipRect.Intersect(clipRect))
                return;

            Base::NRect newClipRect(clipRect);
            newClipRect.Offset(frameRect_.Left, frameRect_.Top);

            render->OffsetRender(frameRect_.Left, frameRect_.Top);
            if(render->IsRectVisible(frameRect_))
            {
                FrameList::const_iterator ite = childs_.begin();
                for(; ite != childs_.end(); ++ ite)
                {
                    NFrameImpl* const & childImpl = *ite;
                    childImpl->Draw(render, clipRect);
                }
            }
            render->OffsetRender(- frameRect_.Left, - frameRect_.Top);
        }

        void NFrameImpl::OnParentChanged()
        {
            if(window_)
                window_ = NULL;
            if(parentFrame_)
                window_ = parentFrame_->window_;
        }

        void NFrameImpl::SetParentHelper(NFrameImpl* childImpl, NFrameImpl* newParent)
        {
            if(newParent == childImpl->parentFrame_)
                return;
            NAssertError(childImpl != NULL, _T("wrong type"));

            if(childImpl->parentFrame_ != NULL)
            {
                childImpl->parentFrame_->RemoveChild(childImpl);
                NSafeRelease(childImpl->parentFrame_);
            }
            if(newParent == NULL)
            {
                childImpl->Release();
            }
            else
            {
                childImpl->parentFrame_ = newParent;
                newParent->AddRef();
                childImpl->AddRef();
            }
            childImpl->OnParentChanged();
        }

        NFrameImpl::FrameList::const_iterator NFrameImpl::GetChildHelper(NFrameImpl* childImpl, size_t& zorder) const
        {
            zorder = 0;
            FrameList::const_iterator ite = childs_.begin();
            while(ite != childs_.end() && (*ite) != childImpl)
            {
                ++ zorder;
                ++ ite;
            }
            if(ite == childs_.end())
            {
                zorder = static_cast<size_t>(-1);
                return childs_.end();
            }
            else
            {
                return ite;
            }
        }
    }
}