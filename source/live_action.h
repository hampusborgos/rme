//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef _RME_NETWORK_ACTION_H_
#define _RME_NETWORK_ACTION_H_

#include "action.h"

class NetworkedActionQueue;

class NetworkedAction : public Action {
protected:
	NetworkedAction(Editor& editor, ActionIdentifier ident);
	~NetworkedAction();
public:
	uint32_t owner;

	friend class NetworkedActionQueue;
};

class NetworkedBatchAction : public BatchAction {
	NetworkedActionQueue& queue;
protected:
	NetworkedBatchAction(Editor& editor, NetworkedActionQueue& queue, ActionIdentifier ident);
	~NetworkedBatchAction();

public:
	void addAndCommitAction(Action* action);

protected:
	void commit();
	void undo();
	void redo();

	friend class NetworkedActionQueue;
};

class NetworkedActionQueue : public ActionQueue {
public:
	NetworkedActionQueue(Editor& editor);
	~NetworkedActionQueue();

	Action* createAction(ActionIdentifier ident);
	BatchAction* createBatch(ActionIdentifier ident);

protected:
	void broadcast(DirtyList& dirty_list);

	friend class NetworkedBatchAction;
};

#endif
