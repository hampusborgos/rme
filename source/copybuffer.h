//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/copybuffer.h $
// $Id: copybuffer.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_COPYBUFFER_H_
#define RME_COPYBUFFER_H_

#include <wx/dataobj.h>

#include "position.h"
#include "basemap.h"

class Editor;

class CopyBuffer
{
public:
	CopyBuffer();
	virtual ~CopyBuffer();

	// In-editor implantation
	void copy(Editor& editor, int floor);
	void cut(Editor& editor, int floor);
	void paste(Editor& editor, Position topos);
	bool canPaste() const;
	// Returns the upper-left corner of the copybuffer
	Position getPosition() const;

	// Clears the copybuffer (eg. resets it)
	void clear();

	size_t GetTileCount();

	BaseMap& getBufferMap();
private:
	Position copyPos;
	BaseMap* tiles;
	unsigned char* serialized;
	size_t serialized_size;
};

#endif
