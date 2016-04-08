// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
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
#ifndef iwHQ_H_INCLUDED
#define iwHQ_H_INCLUDED

#pragma once

#include "iwBaseWarehouse.h"

class GameWorldViewer;
class dskGameInterface;
class nobBaseWarehouse;

class iwHQ : public iwBaseWarehouse
{
    public:
        /// Konstruktor von @p iwHQ.
        iwHQ(GameWorldView& gwv, dskGameInterface& gi, nobBaseWarehouse* wh,  const char* const title, const unsigned pages_count);

    protected:

        void Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id) override;

};

#endif // !iwHQ_H_INCLUDED
