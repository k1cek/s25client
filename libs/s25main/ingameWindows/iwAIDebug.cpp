// Copyright (c) 2005 - 2020 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "iwAIDebug.h"
#include "Loader.h"
#include "ai/AIEvents.h"
#include "ai/aijh/AIPlayerJH.h"
#include "ai/aijh/Jobs.h"
#include "controls/ctrlComboBox.h"
#include "controls/ctrlMultiline.h"
#include "helpers/toString.h"
#include "ogl/FontStyle.h"
#include "ogl/glFont.h"
#include "world/GameWorldView.h"
#include "gameData/BuildingConsts.h"
#include "gameData/const_gui_ids.h"
#include "s25util/colors.h"
#include <array>
#include <sstream>

namespace {
enum
{
    ID_CbPlayer,
    ID_CbOverlay,
    ID_Text
};
}

class iwAIDebug::DebugPrinter : public IDrawNodeCallback
{
    std::array<glArchivItem_Bitmap*, 7> bqImgs;
    std::array<glArchivItem_Bitmap*, 2> ticks;
    glFont& font;

public:
    DebugPrinter(const AIJH::AIPlayerJH* ai, unsigned overlay) : font(*NormalFont), ai(ai), overlay(overlay)
    {
        // Cache images
        bqImgs[0] = nullptr;
        for(int i = 1; i < 6; i++)
            bqImgs[i] = LOADER.GetMapImageN(49 + i);
        ticks[0] = LOADER.GetImageN("io", 40);
        ticks[1] = LOADER.GetImageN("io", 32);
        bqImgs[6] = ticks[0]; // Invalid marker
    }

    const AIJH::AIPlayerJH* ai;
    unsigned overlay;

    void onDraw(const MapPoint& pt, const DrawPoint& curPos) override
    {
        if(overlay == 0)
            return;
        if(overlay == 1)
        {
            glArchivItem_Bitmap* img = bqImgs[std::min<unsigned>(ai->GetAINode(pt).bq, bqImgs.size() - 1)];
            if(img)
                img->DrawFull(curPos);
        } else if(overlay == 2)
            ticks[ai->GetAINode(pt).reachable]->DrawFull(curPos);
        else if(overlay == 3)
            ticks[ai->GetAINode(pt).farmed]->DrawFull(curPos);
        else if(overlay < 13)
            font.Draw(curPos, helpers::toString(ai->GetResMapValue(pt, AIResource(overlay - 4))), FontStyle{}, COLOR_YELLOW);
    }
};

iwAIDebug::iwAIDebug(GameWorldView& gwv, const std::vector<const AIPlayer*>& ais)
    : IngameWindow(CGI_AI_DEBUG, IngameWindow::posLastOrCenter, Extent(280, 515), _("AI Debug"), LOADER.GetImageN("resource", 41)),
      gwv(gwv), text(nullptr), printer(nullptr)
{
    for(const AIPlayer* ai : ais)
    {
        const auto* aijh = dynamic_cast<const AIJH::AIPlayerJH*>(ai);
        if(aijh)
            ais_.push_back(aijh);
    }
    // Wenn keine KI-Spieler, schließen
    if(ais_.empty())
    {
        Close();
        return;
    }

    ctrlComboBox* players = AddComboBox(ID_CbPlayer, DrawPoint(15, 30), Extent(250, 20), TC_GREY, NormalFont, 100);
    for(const AIJH::AIPlayerJH* ai : ais_)
    {
        players->AddString(ai->GetPlayerName());
    }

    ctrlComboBox* overlays = AddComboBox(ID_CbOverlay, DrawPoint(15, 60), Extent(250, 20), TC_GREY, NormalFont, 100);
    overlays->AddString("None");
    overlays->AddString("BuildingQuality");
    overlays->AddString("Reachability");
    overlays->AddString("Farmed");
    overlays->AddString("Wood");
    overlays->AddString("Stones");
    overlays->AddString("Gold");
    overlays->AddString("Ironore");
    overlays->AddString("Coal");
    overlays->AddString("Granite");
    overlays->AddString("Plantspace");
    overlays->AddString("Borderland");
    overlays->AddString("Fish");

    // Show 7 lines of text and 1 empty line
    text = AddMultiline(ID_Text, DrawPoint(15, 120), Extent(250, 8 * NormalFont->getHeight()), TC_GREY, NormalFont, FontStyle::NO_OUTLINE);

    SetIwSize(Extent(GetIwSize().x, text->GetPos().y + text->GetSize().y));

    players->SetSelection(0);
    overlays->SetSelection(0);
    printer = new DebugPrinter(ais_[0], 0);
    gwv.AddDrawNodeCallback(printer);
}

iwAIDebug::~iwAIDebug()
{
    if(printer)
    {
        gwv.RemoveDrawNodeCallback(printer);
        delete printer;
    }
}

void iwAIDebug::Msg_ComboSelectItem(const unsigned ctrl_id, const unsigned selection)
{
    switch(ctrl_id)
    {
        case ID_CbPlayer: printer->ai = ais_[selection]; break;
        case ID_CbOverlay: printer->overlay = selection; break;
    }
}

void iwAIDebug::Msg_PaintBefore()
{
    IngameWindow::Msg_PaintBefore();
    std::stringstream ss;

    const AIJH::Job* currentJob = printer->ai->GetCurrentJob();
    if(!currentJob)
    {
        text->Clear();
        text->AddString(_("No current job"), COLOR_YELLOW);
        return;
    }

    ss << "Jobs to do: " << printer->ai->GetNumJobs() << std::endl << std::endl;

    const auto* bj = dynamic_cast<const AIJH::BuildJob*>(currentJob);
    const auto* ej = dynamic_cast<const AIJH::EventJob*>(currentJob);

    if(bj)
    {
        ss << "BuildJob:" << std::endl;
        ss << BUILDING_NAMES[bj->GetType()] << std::endl;
        ss << bj->GetTarget().x << " / " << bj->GetTarget().y << std::endl;
    } else if(ej)
    {
#define RTTR_PRINT_EV(ev) \
    case AIEvent::ev: ss << #ev << std::endl; break
        switch(ej->GetEvent().GetType())
        {
            RTTR_PRINT_EV(BuildingDestroyed);
            RTTR_PRINT_EV(BuildingConquered);
            RTTR_PRINT_EV(BuildingLost);
            RTTR_PRINT_EV(BorderChanged);
            RTTR_PRINT_EV(NoMoreResourcesReachable);
            RTTR_PRINT_EV(BuildingFinished);
            RTTR_PRINT_EV(ExpeditionWaiting);
            RTTR_PRINT_EV(TreeChopped);
            RTTR_PRINT_EV(ShipBuilt);
            RTTR_PRINT_EV(ResourceUsed);
            RTTR_PRINT_EV(RoadConstructionComplete);
            RTTR_PRINT_EV(RoadConstructionFailed);
            RTTR_PRINT_EV(NewColonyFounded);
            RTTR_PRINT_EV(LuaConstructionOrder);
            RTTR_PRINT_EV(ResourceFound);
            RTTR_PRINT_EV(LostLand);
            default: ss << "Unknown Event" << std::endl; break;
        }
#undef RTTR_PRINT_EV

        const auto* evb = dynamic_cast<const AIEvent::Building*>(&ej->GetEvent());
        if(evb)
        {
            ss << evb->GetX() << " / " << evb->GetY() << std::endl;
            ss << BUILDING_NAMES[evb->GetBuildingType()] << std::endl;
        }
    }

#define RTTR_PRINT_STATUS(state) \
    case AIJH::state: ss << #state << std::endl; break
    switch(currentJob->GetState())
    {
        RTTR_PRINT_STATUS(JOB_WAITING);
        RTTR_PRINT_STATUS(JOB_EXECUTING_START);
        RTTR_PRINT_STATUS(JOB_EXECUTING_ROAD1);
        RTTR_PRINT_STATUS(JOB_EXECUTING_ROAD2);
        RTTR_PRINT_STATUS(JOB_EXECUTING_ROAD2_2);
        RTTR_PRINT_STATUS(JOB_FINISHED);
        RTTR_PRINT_STATUS(JOB_FAILED);
        default: ss << "Unknown status"; break;
    }

    text->Clear();
    text->AddString(ss.str(), COLOR_YELLOW);
}
