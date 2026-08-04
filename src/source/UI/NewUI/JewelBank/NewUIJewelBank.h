// NewUIJewelBank.h: interface for the CNewUIJewelBank class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/NewUI3DRenderMng.h"
#include "UI/NewUI/Inventory/NewUIMyInventory.h"
#include "UI/NewUI/Widgets/NewUIButton.h"

namespace SEASON3B
{
    class CNewUIJewelBank : public CNewUIObj, public INewUI3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_JEWELBANK_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_JEWELBANK_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_JEWELBANK_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_JEWELBANK_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_JEWELBANK_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_JEWELBANK_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,

            IMAGE_JEWELBANK_BTN_EMPTY = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
        };

        enum { JEWEL_TYPE_COUNT = 8 };

    private:
        static constexpr float JEWELBANK_WIDTH = 190.0f;
        static constexpr float JEWELBANK_HEIGHT = 330.0f;

        enum VIEW_STATE
        {
            VIEW_LIST = 0,
            VIEW_DETAIL,
        };

        enum ACTION_BUTTON
        {
            BTN_WITHDRAW_1 = 0,
            BTN_WITHDRAW_10,
            BTN_WITHDRAW_10PACK,
            BTN_WITHDRAW_20,
            BTN_WITHDRAW_20PACK,
            BTN_WITHDRAW_30,
            BTN_WITHDRAW_30PACK,
            BTN_WITHDRAW_MAX,
            BTN_DEPOSIT_ALL,
            BTN_CANCEL,
            MAX_ACTION_BTN
        };

        CNewUIManager* m_pNewUIMng;
        POINT               m_Pos;

        CNewUIButton        m_BtnExit;
        CNewUIButton        m_aActionBtn[MAX_ACTION_BTN];

        VIEW_STATE          m_eView;
        int                 m_iSelectedJewelIndex;
        bool                m_bWasVisible;

    public:
        CNewUIJewelBank();
        virtual ~CNewUIJewelBank();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();
        bool IsVisible() const override;

        float GetLayerDepth();

        void ProcessJewelBankUpdate();

    private:
        void LoadImages();
        void UnloadImages();

        void InitButtons();
        void InitActionButton(CNewUIButton& btn, const wchar_t* pszLabel);
        void RenderActionButtonBox(CNewUIButton& btn);

        void RenderListView();
        void RenderDetailView();
        void RenderJewelRow(int iIndex);

        bool ProcessListViewBtns();
        bool ProcessDetailViewBtns();

        int GetJewelBalance(int iIndex) const;
        int GetJewelItemType(int iIndex) const;
        const wchar_t* const* GetJewelNameSlot(int iIndex) const;

        void SendWithdrawRequest(int iAmount);
        void SendDepositAllRequest();
    };
}
