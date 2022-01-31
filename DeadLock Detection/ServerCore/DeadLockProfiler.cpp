#include "pch.h"
#include "DeadLockProfiler.h"

/*--------------------
	DeadLockProfiler
---------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	//��Ƽ������ ȯ�濡�� ���ư����ϴ� LockGuard�� ��´�.
	LockGuard guard(_lock);

	// ���̵� ã�ų� �߱��Ѵ�.
	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end())
	{
		//���̵� ������ ���̵� �߱����ش�.
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		//������ �����̵� ����
		lockId = findIt->second;
	}

	// ��� �ִ� ���� �־��ٸ�
	if (_lockStack.empty() == false)
	{
		// ������ �߰ߵ��� ���� ���̽���� ����� ���� �ٽ� Ȯ���Ѵ�.
		//������ �� ���������� ����־��� �� ���� ����
		const int32 prevId = _lockStack.top();
		//���� ���̵�� �ٸ���?
		if (lockId != prevId)
		{
			//����Ŭ Ȯ��
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end())
			{
				//���� ���� ������ ����? �׷� �����丮�� �־��ش�.
				//�׷��� ������ ��� �׷��ִ� ����
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	_lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	//��Ƽ������ ȯ���̴� �����带 ����ش�.
	LockGuard guard(_lock);

	//�� ������ ������ ����
	//���� ��������
	if (_lockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	//���� ���´µ� �̸��� ��ġ���� �ʴ� ��� push/pop ������ ���� �����ִ�.
	//���� �������� 
	int32 lockId = _nameToId[name];
	if (_lockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	//�ش� dfs�� �Ϻ��ϰ� ��������?
	_finished = vector<bool>(lockCount, false);
	//��� ����
	_parent = vector<int32>(lockCount, -1);

	//lockCount ��ŭ DFS�� ������.
	for (int32 lockId = 0; lockId < lockCount; lockId++)
		Dfs(lockId);

	// ������ �������� �����Ѵ�. ���ҽ� ����
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	//�̹� �湮�� ��� ����
	if (_discoveredOrder[here] != -1)
		return;

	//�湮�ϰ� �ִ� ������ ������ �־��ش�.
	_discoveredOrder[here] = _discoveredCount++;

	// ��� ������ ������ ��ȸ�Ѵ�.
	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;
	//here ������ ������ �������� ����
	for (int32 there : nextSet)
	{
		// ���� �湮�� ���� ���ٸ� �湮�Ѵ�.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// here�� there���� ���� �߰ߵǾ��ٸ�, there�� here�� �ļ��̴�. (������ ����)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		// �������� �ƴϰ�, Dfs(there)�� ���� �������� �ʾҴٸ�, there�� here�� �����̴�. (������ ����)
		//����Ŭ�� �ִ� ��Ȳ�̹Ƿ� Crash�� ����.
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);

			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}
