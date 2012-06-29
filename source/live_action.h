//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
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
