#pragma once
#include <stack>
#include <map>
#include <vector>

/*--------------------
	DeadLockProfiler
---------------------*/
//���� ��ų� ������ �� ����Ŭ�� �߻��ϴ��� Check
class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);

private:
	//���������� ������ ���� �̸����� ������ ���ϹǷ� id�� ġȯ�ϴ� Map Container�� �����.
	unordered_map<const char*, int32>	_nameToId;
	unordered_map<int32, const char*>	_idToName;
	stack<int32>						_lockStack;
	//�׷������� ������ �ش��ϴ� ����... 0�� ���� 1������ ��Ҵ� -> _lockHistory[0] = {1,...}
	map<int32, set<int32>>				_lockHistory;
	//DeadLockProfiler�� ��Ƽ������ ȯ�濡�� �����ϰ� �����ϰԲ� Mutex�ϳ��� �����.
	Mutex _lock;

private:
	vector<int32>	_discoveredOrder; // ��尡 �߰ߵ� ������ ����ϴ� �迭
	int32			_discoveredCount = 0; // ��尡 �߰ߵ� ����
	vector<bool>	_finished; // Dfs(i)�� ���� �Ǿ����� ����
	vector<int32>	_parent;
};

