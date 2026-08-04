// NewUIJewelBank.cpp: implementation of the CNewUIJewelBank class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I18N/All.h"

#include "UI/NewUI/JewelBank/NewUIJewelBank.h"
#include "UI/NewUI/NewUISystem.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInventory.h"

using namespace SEASON3B;

namespace
{
    constexpr int JEWELBANK_ROW_HEIGHT = 28;
    constexpr int JEWELBANK_ROW_START_Y = 36;
    constexpr int JEWELBANK_ROW_X = 10;
    constexpr int JEWELBANK_ROW_WIDTH = 170;
}

CNewUIJewelBank::CNewUIJewelBank()
{
    m_pNewUIMng = nullptr;
    m_Pos.x = m_Pos.y = 0;
    m_eView = VIEW_LIST;
    m_iSelectedJewelIndex = 0;
    m_bWasVisible = false;
}

CNewUIJewelBank::~CNewUIJewelBank()
{
    Release();
}

bool CNewUIJewelBank::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_JEWELBANK, this);
    g_pNewUI3DRenderMng->Add3DRenderObj(this, INFORMATION_CAMERA_Z_ORDER);

    SetPos(x, y);

    LoadImages();
    InitButtons();

    Show(false);

    return true;
}

void CNewUIJewelBank::Release()
{
    UnloadImages();

    g_pNewUI3DRenderMng->Remove3DRenderObj(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void CNewUIJewelBank::InitButtons()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_JEWELBANK_BTN_EXIT);
    m_BtnExit.ChangeToolTipText(&I18N::Game::Close, true);

    constexpr const wchar_t* aszLabels[MAX_ACTION_BTN] =
    {
        L"Sacar 1",
        L"10 un",
        L"10 pack",
        L"20 un",
        L"20 pack",
        L"30 un",
        L"30 pack",
        L"Sacar Max",
        L"Depositar",
        L"Cancelar",
    };

    for (int i = 0; i < MAX_ACTION_BTN; ++i)
    {
        InitActionButton(m_aActionBtn[i], aszLabels[i]);
    }
}

void CNewUIJewelBank::InitActionButton(CNewUIButton& btn, const wchar_t* pszLabel)
{
    btn.ChangeText(pszLabel);
    btn.SetFont(g_hFont);
    btn.ChangeTextColor(RGBA(255, 255, 255, 255));
}

void CNewUIJewelBank::RenderActionButtonBox(CNewUIButton& btn)
{
    const POINT& pos = btn.GetPos();
    const POINT& size = btn.GetSize();
    bool bHover = (btn.GetBTState() != BUTTON_STATE_UP);

    // RenderColor()'s texture-disable is skipped whenever the renderer's
    // cached "texture enabled" flag is already out of sync with the real GL
    // state (e.g. after a RenderText() call, which re-enables texturing
    // without updating that cache) — rebinding a real texture immediately
    // before each RenderColor() forces the cache back in sync first.
    BindTexture(IMAGE_JEWELBANK_BACK);
    glColor4f(0.12f, 0.12f, 0.14f, 1.f);
    RenderColor(float(pos.x), float(pos.y), float(size.x), float(size.y));
    EndRenderColor();

    BindTexture(IMAGE_JEWELBANK_BACK);

    if (bHover)
    {
        glColor4f(0.2f, 0.55f, 0.75f, 0.6f);
        RenderColor(float(pos.x), float(pos.y), float(size.x), float(size.y));
        EndRenderColor();
        BindTexture(IMAGE_JEWELBANK_BACK);
    }

    BindTexture(IMAGE_JEWELBANK_BACK);
}

void CNewUIJewelBank::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + int(JEWELBANK_HEIGHT) - 37, 36, 29);

    constexpr int iFullWidth = 170;
    constexpr int iHalfWidth = 83;
    constexpr int iBtnHeight = 22;
    constexpr int iBtnGap = 3;
    int iStartY = m_Pos.y + 100;

    // Row 0: Sacar 1 (full width).
    m_aActionBtn[BTN_WITHDRAW_1].ChangeButtonInfo(m_Pos.x + JEWELBANK_ROW_X, iStartY, iFullWidth, iBtnHeight);

    // Rows 1-3: paired withdraw buttons (10un/10pack, 20un/20pack, 30un/30pack).
    const ACTION_BUTTON aPairs[3][2] =
    {
        { BTN_WITHDRAW_10, BTN_WITHDRAW_10PACK },
        { BTN_WITHDRAW_20, BTN_WITHDRAW_20PACK },
        { BTN_WITHDRAW_30, BTN_WITHDRAW_30PACK },
    };
    for (int iRow = 0; iRow < 3; ++iRow)
    {
        int iY = iStartY + (iRow + 1) * (iBtnHeight + iBtnGap);
        m_aActionBtn[aPairs[iRow][0]].ChangeButtonInfo(m_Pos.x + JEWELBANK_ROW_X, iY, iHalfWidth, iBtnHeight);
        m_aActionBtn[aPairs[iRow][1]].ChangeButtonInfo(m_Pos.x + JEWELBANK_ROW_X + iHalfWidth + iBtnGap, iY, iHalfWidth, iBtnHeight);
    }

    // Rows 4-6: Sacar Max, Depositar Tudo, Cancelar (full width).
    int iFullStartY = iStartY + 4 * (iBtnHeight + iBtnGap);
    m_aActionBtn[BTN_WITHDRAW_MAX].ChangeButtonInfo(m_Pos.x + JEWELBANK_ROW_X, iFullStartY, iFullWidth, iBtnHeight);
    m_aActionBtn[BTN_DEPOSIT_ALL].ChangeButtonInfo(m_Pos.x + JEWELBANK_ROW_X, iFullStartY + (iBtnHeight + iBtnGap), iFullWidth, iBtnHeight);
    m_aActionBtn[BTN_CANCEL].ChangeButtonInfo(m_Pos.x + JEWELBANK_ROW_X, iFullStartY + 2 * (iBtnHeight + iBtnGap), iFullWidth, iBtnHeight);
}

bool CNewUIJewelBank::ProcessListViewBtns()
{
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, INTERFACE_JEWELBANK))
        return true;

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(INTERFACE_JEWELBANK);
        return true;
    }

    if (IsVisible() && IsRelease(VK_LBUTTON))
    {
        for (int i = 0; i < JEWEL_TYPE_COUNT; ++i)
        {
            int iPosY = m_Pos.y + JEWELBANK_ROW_START_Y + i * JEWELBANK_ROW_HEIGHT;
            if (CheckMouseIn(m_Pos.x + JEWELBANK_ROW_X, iPosY, JEWELBANK_ROW_WIDTH, JEWELBANK_ROW_HEIGHT - 2))
            {
                m_iSelectedJewelIndex = i;
                m_eView = VIEW_DETAIL;
                PlayBuffer(SOUND_CLICK01);
                return true;
            }
        }
    }

    return false;
}

bool CNewUIJewelBank::ProcessDetailViewBtns()
{
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, INTERFACE_JEWELBANK))
        return true;

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(INTERFACE_JEWELBANK);
        return true;
    }

    if (m_aActionBtn[BTN_WITHDRAW_1].UpdateMouseEvent())
    {
        SendWithdrawRequest(1);
        return true;
    }

    if (m_aActionBtn[BTN_WITHDRAW_10].UpdateMouseEvent() || m_aActionBtn[BTN_WITHDRAW_10PACK].UpdateMouseEvent())
    {
        SendWithdrawRequest(10);
        return true;
    }

    if (m_aActionBtn[BTN_WITHDRAW_20].UpdateMouseEvent() || m_aActionBtn[BTN_WITHDRAW_20PACK].UpdateMouseEvent())
    {
        SendWithdrawRequest(20);
        return true;
    }

    if (m_aActionBtn[BTN_WITHDRAW_30].UpdateMouseEvent() || m_aActionBtn[BTN_WITHDRAW_30PACK].UpdateMouseEvent())
    {
        SendWithdrawRequest(30);
        return true;
    }

    if (m_aActionBtn[BTN_WITHDRAW_MAX].UpdateMouseEvent())
    {
        int iMax = GetJewelBalance(m_iSelectedJewelIndex);
        if (iMax > 0)
        {
            SendWithdrawRequest(iMax);
        }
        else
        {
            PlayBuffer(SOUND_ERROR01);
        }
        return true;
    }

    if (m_aActionBtn[BTN_DEPOSIT_ALL].UpdateMouseEvent())
    {
        SendDepositAllRequest();
        return true;
    }

    if (m_aActionBtn[BTN_CANCEL].UpdateMouseEvent())
    {
        m_eView = VIEW_LIST;
        PlayBuffer(SOUND_CLICK01);
        return true;
    }

    return false;
}

bool CNewUIJewelBank::UpdateMouseEvent()
{
    bool bHandled = (m_eView == VIEW_LIST) ? ProcessListViewBtns() : ProcessDetailViewBtns();
    if (bHandled)
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, JEWELBANK_WIDTH, JEWELBANK_HEIGHT))
        return false;

    return true;
}

bool CNewUIJewelBank::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(INTERFACE_JEWELBANK) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            if (m_eView == VIEW_DETAIL)
            {
                m_eView = VIEW_LIST;
            }
            else
            {
                g_pNewUISystem->Hide(INTERFACE_JEWELBANK);
            }
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool CNewUIJewelBank::Update()
{
    bool bVisibleNow = IsVisible();
    if (!bVisibleNow && m_bWasVisible)
    {
        m_eView = VIEW_LIST;
    }
    m_bWasVisible = bVisibleNow;

    return true;
}

bool CNewUIJewelBank::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFFFFFFF);

    DWORD dwPreBGColor = g_pRenderText->GetBgColor();
    g_pRenderText->SetBgColor(RGBA(0, 0, 0, 0));

    RenderImage(IMAGE_JEWELBANK_BACK, m_Pos.x, m_Pos.y, JEWELBANK_WIDTH, JEWELBANK_HEIGHT);
    RenderImage(IMAGE_JEWELBANK_TOP, m_Pos.x, m_Pos.y, JEWELBANK_WIDTH, 64.f);
    RenderImage(IMAGE_JEWELBANK_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, JEWELBANK_HEIGHT - 64.f - 45.f);
    RenderImage(IMAGE_JEWELBANK_RIGHT, m_Pos.x + JEWELBANK_WIDTH - 21.f, m_Pos.y + 64.f, 21.f, JEWELBANK_HEIGHT - 64.f - 45.f);
    RenderImage(IMAGE_JEWELBANK_BOTTOM, m_Pos.x, m_Pos.y + JEWELBANK_HEIGHT - 45.f, JEWELBANK_WIDTH, 45.f);

    m_BtnExit.Render();

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(RGBA(255, 215, 0, 255));
    g_pRenderText->RenderText(m_Pos.x + int(JEWELBANK_WIDTH / 2) - 40, m_Pos.y + 12, L"Item Bank", 80, 0, RT3_SORT_CENTER);

    if (m_eView == VIEW_LIST)
    {
        RenderListView();
    }
    else
    {
        RenderDetailView();
    }

    g_pRenderText->SetBgColor(dwPreBGColor);

    DisableAlphaBlend();

    return true;
}

void CNewUIJewelBank::RenderListView()
{
    for (int i = 0; i < JEWEL_TYPE_COUNT; ++i)
    {
        RenderJewelRow(i);
    }
}

void CNewUIJewelBank::RenderJewelRow(int iIndex)
{
    int iPosX = m_Pos.x + JEWELBANK_ROW_X;
    int iPosY = m_Pos.y + JEWELBANK_ROW_START_Y + iIndex * JEWELBANK_ROW_HEIGHT;
    int iRowHeight = JEWELBANK_ROW_HEIGHT - 2;

    bool bHover = CheckMouseIn(iPosX, iPosY, JEWELBANK_ROW_WIDTH, iRowHeight);

    // RenderColor()'s texture-disable is skipped whenever the renderer's
    // cached "texture enabled" flag is already out of sync with the real GL
    // state (e.g. after a RenderText() call, which re-enables texturing
    // without updating that cache) — rebinding a real texture immediately
    // before each RenderColor() forces the cache back in sync first.
    BindTexture(IMAGE_JEWELBANK_BACK);
    glColor4f(0.f, 0.f, 0.f, 1.f);
    RenderColor(float(iPosX), float(iPosY), float(JEWELBANK_ROW_WIDTH), float(iRowHeight));
    EndRenderColor();

    BindTexture(IMAGE_JEWELBANK_BACK);

    if (bHover)
    {
        glColor4f(0.2f, 0.55f, 0.75f, 0.6f);
        RenderColor(float(iPosX), float(iPosY), float(JEWELBANK_ROW_WIDTH), float(iRowHeight));
        EndRenderColor();
        BindTexture(IMAGE_JEWELBANK_BACK);
    }

    wchar_t szBalance[32] = { 0, };
    mu_swprintf(szBalance, L"x%06d", GetJewelBalance(iIndex));

    g_pRenderText->SetFont(g_hFont);

    glColor4f(1.f, 1.f, 1.f, 1.f);
    g_pRenderText->SetTextColor(RGBA(255, 255, 255, 255));

    const wchar_t* const* pNameSlot = GetJewelNameSlot(iIndex);
    const wchar_t* pszName = (pNameSlot != nullptr) ? *pNameSlot : L"Gemstone";
    g_pRenderText->RenderText(iPosX + 32, iPosY + 7, pszName, JEWELBANK_ROW_WIDTH - 90, 0, RT3_SORT_LEFT);

    glColor4f(1.f, 1.f, 1.f, 1.f);
    g_pRenderText->SetTextColor(RGBA(255, 215, 0, 255));
    g_pRenderText->RenderText(iPosX + JEWELBANK_ROW_WIDTH - 60, iPosY + 7, szBalance, 55, 0, RT3_SORT_RIGHT);
}

void CNewUIJewelBank::RenderDetailView()
{
    int iCenterX = m_Pos.x + int(JEWELBANK_WIDTH / 2);

    wchar_t szBalance[64] = { 0, };
    mu_swprintf(szBalance, L"%d x ", GetJewelBalance(m_iSelectedJewelIndex));

    const wchar_t* const* pNameSlot = GetJewelNameSlot(m_iSelectedJewelIndex);
    const wchar_t* pszName = (pNameSlot != nullptr) ? *pNameSlot : L"Gemstone";

    wchar_t szFull[96] = { 0, };
    mu_swprintf(szFull, L"%s%s", szBalance, pszName);

    g_pRenderText->SetFont(g_hFontBold);
    glColor4f(1.f, 1.f, 1.f, 1.f);
    g_pRenderText->SetTextColor(RGBA(0, 255, 0, 255));
    g_pRenderText->RenderText(m_Pos.x + int(JEWELBANK_WIDTH / 2) - 60, m_Pos.y + 72, szFull, 120, 0, RT3_SORT_CENTER);

    for (int i = 0; i < MAX_ACTION_BTN; ++i)
    {
        RenderActionButtonBox(m_aActionBtn[i]);
    }

    for (int i = 0; i < MAX_ACTION_BTN; ++i)
    {
        m_aActionBtn[i].Render();
    }
}

float CNewUIJewelBank::GetLayerDepth()
{
    return 2.4f;
}

bool CNewUIJewelBank::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

void CNewUIJewelBank::Render3D()
{
    glColor4f(1.f, 1.f, 1.f, 1.f);

    if (m_eView == VIEW_LIST)
    {
        for (int i = 0; i < JEWEL_TYPE_COUNT; ++i)
        {
            int iPosX = m_Pos.x + JEWELBANK_ROW_X;
            int iPosY = m_Pos.y + JEWELBANK_ROW_START_Y + i * JEWELBANK_ROW_HEIGHT;
            RenderItem3D(float(iPosX + 2), float(iPosY + 2), 24.f, 24.f, GetJewelItemType(i), 0, 0, 0, false);
        }
    }
    else
    {
        int iCenterX = m_Pos.x + int(JEWELBANK_WIDTH / 2);
        RenderItem3D(float(iCenterX - 17), float(m_Pos.y + 34), 34.f, 34.f, GetJewelItemType(m_iSelectedJewelIndex), 0, 0, 0, false);
    }
}

void CNewUIJewelBank::ProcessJewelBankUpdate()
{
    // Nothing to do besides repaint — RenderJewelRow/RenderDetailView read
    // CharacterMachine live every frame, same pattern as
    // CNewUIStorageInventory's gold display.
}

int CNewUIJewelBank::GetJewelBalance(int iIndex) const
{
    switch (iIndex)
    {
    case 0: return CharacterMachine->JewelBankBless;
    case 1: return CharacterMachine->JewelBankSoul;
    case 2: return CharacterMachine->JewelBankLife;
    case 3: return CharacterMachine->JewelBankCreation;
    case 4: return CharacterMachine->JewelBankGuardian;
    case 5: return CharacterMachine->JewelBankGemstone;
    case 6: return CharacterMachine->JewelBankHarmony;
    case 7: return CharacterMachine->JewelBankChaos;
    default: return 0;
    }
}

int CNewUIJewelBank::GetJewelItemType(int iIndex) const
{
    switch (iIndex)
    {
    case 0: return ITEM_JEWEL_OF_BLESS;
    case 1: return ITEM_JEWEL_OF_SOUL;
    case 2: return ITEM_JEWEL_OF_LIFE;
    case 3: return ITEM_JEWEL_OF_CREATION;
    case 4: return ITEM_JEWEL_OF_GUARDIAN;
    case 5: return ITEM_GEMSTONE;
    case 6: return ITEM_JEWEL_OF_HARMONY;
    case 7: return ITEM_JEWEL_OF_CHAOS;
    default: return ITEM_JEWEL_OF_BLESS;
    }
}

const wchar_t* const* CNewUIJewelBank::GetJewelNameSlot(int iIndex) const
{
    switch (iIndex)
    {
    case 0: return &I18N::Game::JewelOfBless;
    case 1: return &I18N::Game::JewelOfSoul;
    case 2: return &I18N::Game::JewelOfLife;
    case 3: return &I18N::Game::JewelOfCreation;
    case 4: return &I18N::Game::JewelOfGuardian;
    case 5: return nullptr; // Gemstone has no dedicated I18N slot; falls back to literal.
    case 6: return &I18N::Game::JewelOfHarmony;
    case 7: return &I18N::Game::JewelOfChaos;
    default: return nullptr;
    }
}

void CNewUIJewelBank::SendWithdrawRequest(int iAmount)
{
    if (iAmount <= 0)
        return;

    int iBalance = GetJewelBalance(m_iSelectedJewelIndex);
    if (iBalance < iAmount)
        return;

    SocketClient->ToGameServer()->SendJewelBankMoveRequest(
        JewelBankMoveDirection::WithdrawToInventory,
        static_cast<JewelBankJewelType>(m_iSelectedJewelIndex),
        static_cast<uint32_t>(iAmount));

    PlayBuffer(SOUND_CLICK01);
}

void CNewUIJewelBank::SendDepositAllRequest()
{
    // The server enumerates matching inventory items itself and deposits
    // however many the player actually owns — the client doesn't need to
    // know the owned count up front, it just asks for "a lot" and the
    // server clamps to what's really in the inventory.
    constexpr uint32_t DEPOSIT_ALL_AMOUNT = 9999;

    SocketClient->ToGameServer()->SendJewelBankMoveRequest(
        JewelBankMoveDirection::DepositToBank,
        static_cast<JewelBankJewelType>(m_iSelectedJewelIndex),
        DEPOSIT_ALL_AMOUNT);

    PlayBuffer(SOUND_CLICK01);
}

void CNewUIJewelBank::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_JEWELBANK_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_JEWELBANK_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_JEWELBANK_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_JEWELBANK_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_JEWELBANK_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_JEWELBANK_BTN_EXIT, GL_LINEAR);
}

void CNewUIJewelBank::UnloadImages()
{
    DeleteBitmap(IMAGE_JEWELBANK_BTN_EXIT);
    DeleteBitmap(IMAGE_JEWELBANK_BOTTOM);
    DeleteBitmap(IMAGE_JEWELBANK_RIGHT);
    DeleteBitmap(IMAGE_JEWELBANK_LEFT);
    DeleteBitmap(IMAGE_JEWELBANK_TOP);
    DeleteBitmap(IMAGE_JEWELBANK_BACK);
}
