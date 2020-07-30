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

#include "main.h"

#include "live_action.h"
#include "editor.h"

NetworkedAction::NetworkedAction(Editor& editor, ActionIdentifier ident) :
	Action(editor, ident),
	owner(0)
{
	;
}

NetworkedAction::~NetworkedAction()
{
	;
}

NetworkedBatchAction::NetworkedBatchAction(Editor& editor, NetworkedActionQueue& queue, ActionIdentifier ident) :
	BatchAction(editor, ident),
	queue(queue)
{
	;
}

NetworkedBatchAction::~NetworkedBatchAction()
{
	;
}

void NetworkedBatchAction::addAndCommitAction(Action* action)
{
	// If empty, do nothing.
	if(action->size() == 0) {
		delete action;
		return;
	}

	// Track changed nodes...
	DirtyList dirty_list;
	NetworkedAction* netact = dynamic_cast<NetworkedAction*>(action);
	if(netact)
		dirty_list.owner = netact->owner;

	// Add it!
	action->commit(type != ACTION_SELECT? &dirty_list : nullptr);
	batch.push_back(action);
	timestamp = time(nullptr);

	// Broadcast changes!
	queue.broadcast(dirty_list);
}

void NetworkedBatchAction::commit()
{
	// Track changed nodes...
	DirtyList dirty_list;

	for(ActionVector::iterator it = batch.begin(); it != batch.end(); ++it) {
		NetworkedAction* action = static_cast<NetworkedAction*>(*it);
		if(!action->isCommited()) {
			action->commit(type != ACTION_SELECT? &dirty_list : nullptr);
			if(action->owner != 0)
				dirty_list.owner = action->owner;
		}
	}
	// Broadcast changes!
	queue.broadcast(dirty_list);
}

void NetworkedBatchAction::undo()
{
	// Track changed nodes...
	DirtyList dirty_list;

	for(ActionVector::reverse_iterator it = batch.rbegin(); it != batch.rend(); ++it) {
		(*it)->undo(type != ACTION_SELECT? &dirty_list : nullptr);
	}
	// Broadcast changes!
	queue.broadcast(dirty_list);
}

void NetworkedBatchAction::redo()
{
	commit();
}


//===================
// Action queue

NetworkedActionQueue::NetworkedActionQueue(Editor& editor) : ActionQueue(editor)
{
}

NetworkedActionQueue::~NetworkedActionQueue()
{
}

Action* NetworkedActionQueue::createAction(ActionIdentifier ident)
{
	return newd NetworkedAction(editor, ident);
}

BatchAction* NetworkedActionQueue::createBatch(ActionIdentifier ident)
{
	return newd NetworkedBatchAction(editor, *this, ident);
}

void NetworkedActionQueue::broadcast(DirtyList& dirty_list)
{
	editor.BroadcastNodes(dirty_list);
}
